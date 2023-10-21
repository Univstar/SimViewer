#include "Viewer.h"

#include "ViewShaderPool.h"

#include "Assets.h"
#include "Core/EntryPoint.h"
#include "Core/Input.h"
#include "Utils/FileDialog.h"
#include "Utils/ImGuiEx.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <cxxopts.hpp>

namespace Pivot {
	Viewer::Viewer() {
		ViewShaderPool::CompileShaders();

		m_PassConstantsBuffer = Renderer::CreateUniformBuffer(0);
		for (auto &shader : ViewShaderPool::GetShaders()) {
			shader->BindUniformBlock("PassConstants", 0);
		}

		Renderer::SetClearColor(m_Appearance.Background);
		Renderer::SetMultisampled(m_Appearance.Multisampled);
		Renderer::SetBackFaceCulled(m_Appearance.BackFaceCulled);
	}

	Viewer::Viewer(std::filesystem::path const &dirname) : Viewer() {
		if (!LoadDirectory(dirname)) {
			spdlog::critical("Failed to create Viewer without a valid directory");
			std::exit(EXIT_FAILURE);
		}

		m_Loader.Load(m_Dirname, m_FrameCount, m_Objects);

		for (auto &object : m_Objects) {
			object->UploadBuffers(0, true);
		}

		Renderer::SetDepthTested(m_Dimension == 3);
		m_ExportModels.Dirname = m_Dirname / "export" / "models";
		m_ExportModels.DirnameStr = m_ExportModels.Dirname.string();
		m_ExportModels.Exported.resize(m_Objects.size(), false);
	}

	Viewer::~Viewer() {
		m_Loader.Stop();

		ViewShaderPool::DestroyShaders();
	}

	void Viewer::Tick(float deltaTime) {
		// Frame calculation
		m_AvailFrameCount = m_Loader.GetAvailFrameCount();
		if (m_Animation.Playing) {
			m_Animation.CurrentFrame += deltaTime * m_Animation.FrameRate;
			if (m_Animation.CurrentFrame >= m_AvailFrameCount - 1) {
				m_Animation.CurrentFrame = m_AvailFrameCount - 1;
				m_Animation.Playing = false;
			}
		}
		m_CurrentFrame = static_cast<std::uint32_t>(m_Animation.CurrentFrame);
		for (auto &object : m_Objects) {
			object->UploadBuffers(m_CurrentFrame);
		}
		// Update uniforms
		PassConstants passConstants = {
			.Transform      = m_Camera->GetTransform(App::Get()->GetWindow()->GetAspect()),
			.LightIntensity = m_Appearance.LightColor * m_Appearance.LightIntensity * .1f,
			.LightDirection = Coordinates::SphericalToCartesian(1.f, glm::radians(90.f) - m_Appearance.LightAltitude, m_Appearance.LightAzimuth),
			.AmbientCoeff   = m_Appearance.EnvironColor * m_Appearance.EnvironIntensity * .001f,
			.CameraPosition = m_Camera->GetPosition(),
			.Wireframed     = m_Appearance.Wireframed,
			.Flat           = !m_Appearance.VertexNormalUsed,
		};
		m_PassConstantsBuffer->Upload(passConstants);
		// Draw calls
		for (auto const &layer : m_ObjectLayers) {
			for (auto const &object : layer) {
				object->Draw();
			}
		}
	}
	
	void Viewer::RenderGui() {
		RenderSideBars();
		RenderPopups();
		RenderAnimationPanel();
		RenderAppearancePanel();
		RenderCameraPanel();
		RenderObjectsPanel();
		RegisterGlobalShortcuts();
	}

	void Viewer::OnKeyDown(Key key, ModifierFlags mods) {
		switch (key) {
		case Key::Left:
			if (!m_Animation.Playing && m_Animation.CurrentFrame >= 1.f) m_Animation.CurrentFrame -= 1.f;
			break;
		case Key::Right:
			if (!m_Animation.Playing && m_Animation.CurrentFrame < m_AvailFrameCount - 1) m_Animation.CurrentFrame += 1.f;
			break;
		}
	}

	void Viewer::OnKeyRepeat(Key key, ModifierFlags mods) {
		switch (key) {
		case Key::Left:
			if (!m_Animation.Playing && m_Animation.CurrentFrame >= 1.f) m_Animation.CurrentFrame -= 1.f;
			break;
		case Key::Right:
			if (!m_Animation.Playing && m_Animation.CurrentFrame < m_AvailFrameCount - 1) m_Animation.CurrentFrame += 1.f;
			break;
		}
	}

	void Viewer::OnMouseDown(MouseButton button, ModifierFlags mods) {
		m_CameraInfo.LastMousePos = Input::GetMousePos();
	}

	void Viewer::OnMouseWheel(float xOffset, float yOffset) {
		m_Camera->Scale(yOffset);
	}

	void Viewer::OnMouseMove(float xPos, float yPos) {
		glm::vec2 mousePos = { xPos, yPos };
		if (Input::IsMouseButtonDown(MouseButton::Left)) {
			m_Camera->Rotate((mousePos - m_CameraInfo.LastMousePos) / (1.f * App::Get()->GetWindow()->GetHeight()));
		} else if (Input::IsMouseButtonDown(MouseButton::Right)) {
			m_Camera->Translate((mousePos - m_CameraInfo.LastMousePos) / (1.f * App::Get()->GetWindow()->GetHeight()));
		}
		m_CameraInfo.LastMousePos = mousePos;
	}

	bool Viewer::LoadDirectory(std::filesystem::path const &dirname) {
		m_Dirname = std::filesystem::absolute(dirname);
		{ // Load the description file
			std::ifstream fin(m_Dirname / "description.yaml");
			if (!fin) {
				spdlog::error("Failed to open \"{}\"", (m_Dirname / "description.yaml").string());
				return false;
			}
			m_Description = YAML::Load(fin);
		}

		// Load the dimension
		m_Dimension = m_Description["Dimension"] ? m_Description["Dimension"].as<std::uint32_t>() : 3;
		App::Get()->GetWindow()->SetTitle(fmt::format("Pivot Viewer ({}D) - {}", m_Dimension, m_Dirname.string()));
		auto radius = m_Description["Radius"] ? m_Description["Radius"].as<float>() : 1.f;
		if (m_Dimension == 2) {
			m_Camera = std::make_unique<OrthoCamera>(glm::vec2(0), radius * 2);
			m_InitialCamera = std::make_unique<OrthoCamera>(*reinterpret_cast<OrthoCamera *>(m_Camera.get()));
		} else {
			m_Camera = std::make_unique<OrbitCamera>(glm::vec3(0), radius);
			m_InitialCamera = std::make_unique<OrbitCamera>(*reinterpret_cast<OrbitCamera *>(m_Camera.get()));
		}

		// Load objects
		if (!m_Description["Objects"] || !m_Description["Objects"].IsSequence()) {
			spdlog::error("Failed to find any object in description.yaml");
			return false;
		}
		for (auto const &node : m_Description["Objects"]) {
			auto object = std::make_unique<ViewObject>(m_Objects.size(), node, m_Dimension);
			m_ObjectLayers[static_cast<std::size_t>(object->GetMaterial().Mode)].push_back(object.get());
			m_Objects.push_back(std::move(object));
		}

		{ // Load the number of total frames
			std::ifstream fin(m_Dirname / "frame_count.txt");
			if (!fin) {
				m_FrameCount = 1;
			} else {
				fin >> m_FrameCount;
				m_FrameCount = std::max(m_FrameCount, 1u);
			}
		}

		return true;
	}

	bool Viewer::SaveScreenshot() {
		auto filename = FileDialog::Save({ { "PNG Image", "png"} });
		if (!filename.empty()) {
			spdlog::debug("{}", filename.string());
			return true;
		} else {
			return false;
		}
	}

	void Viewer::RenderSideBars() {
		// Render main menu bar
		if (m_MenuBarVisible && ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("Save Screenshot", "Ctrl+S")) {
					SaveScreenshot();
				}
				if (ImGui::BeginMenu("Export", !m_Animation.Playing)) {
					if (ImGui::MenuItem("Animation", "Ctrl+N")) {
					}
					if (ImGui::MenuItem("Models", "Ctrl+M")) {
						m_ExportModels.Requested = true;
					}
					ImGui::EndMenu();
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Exit", "Alt+F4")) {
					App::Get()->GetWindow()->Close();
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Control")) {
				if (ImGui::MenuItem(m_Animation.Playing ? "Stop" : "Play", "Ctrl+P")) {
					m_Animation.Playing = !m_Animation.Playing;
				}
				if (ImGui::MenuItem("Reset Animation", "Ctrl+R", false, !m_Animation.Playing)) {
					m_Animation = { };
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Reset Camera", "Ctrl+Backspace")) {
					m_Camera->SetAs(*m_InitialCamera);
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("View")) {
				if (ImGui::MenuItem("Menu Bar", "F1", m_MenuBarVisible)) {
					m_MenuBarVisible = !m_MenuBarVisible;
				}
				if (ImGui::MenuItem("Status Bar", "F2", m_StatusBarVisible)) {
					m_StatusBarVisible = !m_StatusBarVisible;
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Animation", "F9", m_Animation.Visible)) {
					m_Animation.Visible = !m_Animation.Visible;
				}
				if (ImGui::MenuItem("Appearance", "F10", m_Appearance.Visible)) {
					m_Appearance.Visible = !m_Appearance.Visible;
				}
				if (ImGui::MenuItem("Camera", "F11", m_CameraInfo.Visible)) {
					m_CameraInfo.Visible = !m_CameraInfo.Visible;
				}
				if (ImGui::MenuItem("Objects", "F12", m_ObjectsInfo.Visible)) {
					m_ObjectsInfo.Visible = !m_ObjectsInfo.Visible;
				}
				ImGui::EndMenu();
			}
			// Show FPS calculation
			ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
			ImGuiEx::TextRightAligned("%.0f FPS", App::Get()->GetFps());
			ImGui::PopFont();
			ImGui::EndMainMenuBar();
		}
		// Render main status bar
		if (m_StatusBarVisible && ImGuiEx::BeginMainStatusBar()) {
			if (m_AvailFrameCount == m_FrameCount) {
				ImGui::Text("Ready");
			} else {
				ImGui::Text("Loading... %.1f%%", 100.f * m_AvailFrameCount / m_FrameCount);
			}
			// Show frame number
			ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
			ImGuiEx::TextRightAligned("%u/%u", m_CurrentFrame, m_AvailFrameCount - 1);
			ImGui::PopFont();
			ImGuiEx::EndMainStatusBar();
		}
	}

	void Viewer::RenderPopups() {
		if (m_ExportModels.Requested) {
			ImGui::OpenPopup("Export Models");
			m_ExportModels.Popup     = true;
			m_ExportModels.Requested = false;
		}
		// Export models
		if (ImGui::BeginPopupModal("Export Models", &m_ExportModels.Popup, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::Text("Output Directory");
			if (ImGuiEx::InputText("##Dirname", &m_ExportModels.DirnameStr, ImGuiInputTextFlags_EnterReturnsTrue)) {
				m_ExportModels.Dirname = std::filesystem::absolute(m_ExportModels.DirnameStr);
				m_ExportModels.DirnameStr = m_ExportModels.Dirname.string();
			}
			ImGui::SameLine();
			if (ImGui::Button("Browse")) {
				auto newDirname = FileDialog::PickFolder(m_ExportModels.Dirname);
				if (!newDirname.empty()) {
					m_ExportModels.Dirname = newDirname;
					m_ExportModels.DirnameStr = m_ExportModels.Dirname.string();
				}
			}
			ImGui::Spacing();
			ImGui::Text("Object Selection");
			ImGui::BeginListBox("##Objects", { -FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing() });
			for (std::size_t i = 0; i < m_Objects.size(); i++) {
				ImGui::Checkbox(m_Objects[i]->GetDisplayName().data(), reinterpret_cast<bool *>(&m_ExportModels.Exported[i]));
			}
			ImGui::EndListBox();
			ImGui::Spacing();
			ImGui::Separator();
			{
				auto const &style = ImGui::GetStyle();
				float buttonWidth1 = ImGui::CalcTextSize("OK").x + style.FramePadding.x * 2.f;
				float buttonWidth2 = ImGui::CalcTextSize("Cancel").x + style.FramePadding.x * 2.f;
				float widthNeeded = buttonWidth1 + style.ItemSpacing.x + buttonWidth2;
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - widthNeeded);
				ImGui::Button("OK");
				ImGui::SameLine();
				if (ImGui::Button("Cancel")) {
					ImGui::CloseCurrentPopup();
				}
			}
			ImGui::EndPopup();
		}
	}

	void Viewer::RenderAnimationPanel() {
		if (!m_Animation.Visible) return;
		if (ImGui::Begin("Animation", &m_Animation.Visible, ImGuiWindowFlags_AlwaysAutoResize)) {
			if (m_Animation.Playing) ImGui::BeginDisabled();
			ImGui::SeparatorText("Play Control");
			m_Animation.FrameNumber = m_CurrentFrame;
			if (ImGui::InputInt("Frame no.", &m_Animation.FrameNumber, 1, 10, ImGuiInputTextFlags_EnterReturnsTrue)) {
				m_Animation.FrameNumber = std::clamp<int>(m_Animation.FrameNumber, 0, m_AvailFrameCount - 1);
				m_Animation.CurrentFrame = m_Animation.FrameNumber;
			}
			if (ImGui::InputFloat("Frame rate", &m_Animation.FrameRate, 1.f, 10.f, "%.1f")) {
				m_Animation.FrameRate = std::max(m_Animation.FrameRate, 0.f);
			}
			if (m_Animation.Playing) ImGui::EndDisabled();

			ImGui::Spacing();
			ImGui::SeparatorText("Instructions");
			ImGui::BulletText("Press Ctrl+P to play/stop");
			ImGui::BulletText("Press Ctrl+R to reset");
			if (Gui::AreKeyEventsBlocked()) ImGui::BeginDisabled();
			ImGui::BulletText("Press Left for the previous frame");
			ImGui::BulletText("Press Right for the next frame");
			if (Gui::AreKeyEventsBlocked()) ImGui::EndDisabled();
		}
		ImGui::End();
	}

	void Viewer::RenderAppearancePanel() {
		if (!m_Appearance.Visible) return;
		if (ImGui::Begin("Appearance", &m_Appearance.Visible, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::SeparatorText("Renderer");
			if (ImGui::ColorEdit3("Background", glm::value_ptr(m_Appearance.Background))) {
				Renderer::SetClearColor(m_Appearance.Background);
			}
			if (auto msCnt = App::Get()->GetWindow()->GetMultisampleCount(); msCnt > 1 && ImGui::Checkbox(fmt::format("Anti-aliasing ({}x MSAA)", msCnt).c_str(), &m_Appearance.Multisampled)) {
				Renderer::SetMultisampled(m_Appearance.Multisampled);
			}
			if (ImGui::Checkbox("Wireframe mode", &m_Appearance.Wireframed)) {
				Renderer::SetWireframed(m_Appearance.Wireframed);
			}
			
			if (m_Dimension == 3) {
				if (ImGui::Checkbox("Back-face culling", &m_Appearance.BackFaceCulled)) {
					Renderer::SetBackFaceCulled(m_Appearance.BackFaceCulled);
				}
				if (ImGui::RadioButton("Vertex normal", m_Appearance.VertexNormalUsed)) {
					m_Appearance.VertexNormalUsed = true;
				}
				ImGui::SameLine();
				if (ImGui::RadioButton("Face normal", !m_Appearance.VertexNormalUsed)) {
					m_Appearance.VertexNormalUsed = false;
				}

				ImGui::Spacing();
				ImGui::SeparatorText("Light");
				ImGui::ColorEdit3("Color##Light", glm::value_ptr(m_Appearance.LightColor));
				ImGui::SliderFloat("Intensity##Light", &m_Appearance.LightIntensity, 0, 200, "%.1f", ImGuiSliderFlags_NoInput | ImGuiSliderFlags_NoRoundToFormat);
				ImGui::SliderAngle("Altitude", &m_Appearance.LightAltitude, -90.f, 90.f, "%.1f deg", ImGuiSliderFlags_NoInput | ImGuiSliderFlags_NoRoundToFormat);
				ImGui::SliderAngle("Azimuth", &m_Appearance.LightAzimuth, 0.f, 360.f, "%.1f deg", ImGuiSliderFlags_NoInput | ImGuiSliderFlags_NoRoundToFormat);

				ImGui::Spacing();
				ImGui::SeparatorText("Environment");
				ImGui::ColorEdit3("Color##Environ", glm::value_ptr(m_Appearance.EnvironColor));
				ImGui::SliderFloat("Intensity##Environ", &m_Appearance.EnvironIntensity, 0, 200, "%.1f", ImGuiSliderFlags_NoInput | ImGuiSliderFlags_NoRoundToFormat);
			}
		}
		ImGui::End();
	}

	void Viewer::RenderCameraPanel() {
		if (!m_CameraInfo.Visible) return;
		if (ImGui::Begin("Camera", &m_CameraInfo.Visible, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::SeparatorText("Parameters");
			ImGui::BeginDisabled();
			glm::ivec2 resolution = App::Get()->GetWindow()->GetSize();
			ImGui::InputInt2("Resolution", glm::value_ptr(resolution));
			if (m_Dimension == 2) {
				auto camera = reinterpret_cast<OrthoCamera *>(m_Camera.get());
				auto center = camera->GetCenter();
				ImGui::InputFloat2("Center", glm::value_ptr(center), "%g");
				auto canvas = glm::vec2(camera->GetHeight());
				canvas.x *= App::Get()->GetWindow()->GetAspect();
				ImGui::InputFloat2("Canvas", glm::value_ptr(canvas), "%g");
			} else {
				auto camera = reinterpret_cast<OrbitCamera *>(m_Camera.get());
				auto center = camera->GetCenter();
				ImGui::InputFloat3("Target", glm::value_ptr(center), "%.4g");
				auto eye = camera->GetEye();
				ImGui::InputFloat3("Viewpoint", glm::value_ptr(eye), "%.4g");
				auto fovy = camera->GetFovy();
				ImGui::SliderAngle("VFOV", &fovy, 30.f, 120.f, "%g deg");
				std::array zPlanes = { camera->GetZNear(), camera->GetZFar() };
				ImGui::InputFloat2("ZN/ZF", zPlanes.data(), "%g");
			}
			ImGui::EndDisabled();

			ImGui::Spacing();
			ImGui::SeparatorText("Instructions");
			if (m_Dimension == 2) {
				ImGui::BulletText("Right click and drag to move the canvas");
				ImGui::BulletText("Wheel to zoom in/out (scale the canvas)");
				ImGui::BulletText("Press Ctrl+Backspace to reset the parameters");
			} else {
				ImGui::BulletText("Left click and drag to rotate the viewing angle");
				ImGui::BulletText("Right click and drag to move the viewpoint");
				ImGui::BulletText("Wheel to zoom in/out (scale the distance)");
				ImGui::BulletText("Press Ctrl+Backspace to reset the parameters");
			}
		}
		ImGui::End();
	}

	void Viewer::RenderObjectsPanel() {
		if (!m_ObjectsInfo.Visible) return;
		if (ImGui::Begin("Objects", &m_ObjectsInfo.Visible, ImGuiWindowFlags_AlwaysAutoResize)) {
			if (ImGui::BeginCombo("Name", m_Objects[m_ObjectsInfo.CurIdx]->GetDisplayName().data())) {
				for (std::size_t i = 0; i < m_Objects.size(); i++) {
					bool const selected = (i == m_ObjectsInfo.CurIdx);
					if (ImGui::Selectable(m_Objects[i]->GetDisplayName().data(), selected))
						m_ObjectsInfo.CurIdx = i;
					if (selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			auto &curObj = m_Objects[m_ObjectsInfo.CurIdx];
			auto &curMat = curObj->GetMaterial();
			ImGui::Checkbox("Visible", &curMat.Visible);
			ImGui::SameLine();
			if (m_ObjectsInfo.CurIdx < 9) {
				ImGui::BeginDisabled();
				ImGui::Text("Global shortcut keys: Ctrl+%d", static_cast<int>(m_ObjectsInfo.CurIdx) + 1);
				ImGui::EndDisabled();
			}

			ImGui::Spacing();
			ImGui::SeparatorText("Metadata");
			ImGui::BulletText("Shader name: \"%s\";", ViewShaderPool::GetName(curObj->GetViewShader()).data());
			if (curObj->IsAnimated()) {
				ImGui::BulletText("Animation: %s topology;", curObj->IsTopoFixed() ? "invariant" : "variant");
			} else {
				ImGui::BulletText("Animation: unavailable;");
			}
			if (curObj->IsIndexed()) {
				ImGui::BulletText("Statistics: %u vertices, %u indices.", curObj->GetVertexCount(m_CurrentFrame), curObj->GetIndexCount(m_CurrentFrame));
			} else {
				ImGui::BulletText("Statistics: %u vertices.", curObj->GetVertexCount(m_CurrentFrame));
			}

			ImGui::Spacing();
			ImGui::SeparatorText("Material");
			ImGui::BeginDisabled();
			auto blending = static_cast<int>(curMat.Mode);
			ImGui::Combo("Blending", &blending, "Opaque\0Cutout\0Transparent\0Fade\0");
			ImGui::EndDisabled();
			switch (curObj->GetViewShader()) {
			case ViewShader::Default2D:
				ImGui::ColorEdit4("Albedo", glm::value_ptr(curMat.Albedo));
				break;
			case ViewShader::Default3D_Triangles:
			case ViewShader::Default3d_Points:
				ImGui::ColorEdit4("Albedo", glm::value_ptr(curMat.Albedo));
				ImGui::SliderFloat("Metallic", &curMat.Metallic, 0.f, 1.f, "%.3f", ImGuiSliderFlags_NoInput | ImGuiSliderFlags_NoRoundToFormat);
				ImGui::SliderFloat("Roughness", &curMat.Roughness, 0.f, 1.f, "%.3f", ImGuiSliderFlags_NoInput | ImGuiSliderFlags_NoRoundToFormat);
				break;
			case ViewShader::Heatmap2D:
				ImGui::BeginDisabled();
				ImGui::InputFloat("Maximum", &curMat.HeatMax, 0.f, 0.f, "%.4g");
				ImGui::EndDisabled();
				ImGui::SliderFloat("Scale", &curMat.HeatScale, 1.f, 10.f, "%.2f", ImGuiSliderFlags_NoInput | ImGuiSliderFlags_NoRoundToFormat);
				break;
			case ViewShader::Heatmap3D_Triangles:
				ImGui::SliderFloat("Metallic", &curMat.Metallic, 0.f, 1.f, "%.3f", ImGuiSliderFlags_NoInput | ImGuiSliderFlags_NoRoundToFormat);
				ImGui::SliderFloat("Roughness", &curMat.Roughness, 0.f, 1.f, "%.3f", ImGuiSliderFlags_NoInput | ImGuiSliderFlags_NoRoundToFormat);
				ImGui::BeginDisabled();
				ImGui::InputFloat("Maximum", &curMat.HeatMax, 0.f, 0.f, "%.4g");
				ImGui::EndDisabled();
				ImGui::SliderFloat("Scale", &curMat.HeatScale, 1.f, 10.f, "%.2f", ImGuiSliderFlags_NoInput | ImGuiSliderFlags_NoRoundToFormat);
				break;
			}
		}
		ImGui::End();
	}

	void Viewer::RegisterGlobalShortcuts() {
		// Menuitem shortcuts
		if (ImGui::Shortcut(ImGuiKey_ModCtrl | ImGuiKey_S, 0, ImGuiInputFlags_RouteGlobalLow)) {
			SaveScreenshot();
		}
		if (ImGui::Shortcut(ImGuiKey_ModCtrl | ImGuiKey_M, 0, ImGuiInputFlags_RouteGlobalLow)) {
			m_ExportModels.Requested = true;
		}
		// Visibility shortcuts
		if (ImGui::Shortcut(ImGuiKey_F1, 0, ImGuiInputFlags_RouteGlobalLow)) {
			m_MenuBarVisible = !m_MenuBarVisible;
		}
		if (ImGui::Shortcut(ImGuiKey_F2, 0, ImGuiInputFlags_RouteGlobalLow)) {
			m_StatusBarVisible = !m_StatusBarVisible;
		}
		if (ImGui::Shortcut(ImGuiKey_F9, 0, ImGuiInputFlags_RouteGlobalLow)) {
			m_Animation.Visible = !m_Animation.Visible;
		}
		if (ImGui::Shortcut(ImGuiKey_F10, 0, ImGuiInputFlags_RouteGlobalLow)) {
			m_Appearance.Visible = !m_Appearance.Visible;
		}
		if (ImGui::Shortcut(ImGuiKey_F11, 0, ImGuiInputFlags_RouteGlobalLow)) {
			m_CameraInfo.Visible = !m_CameraInfo.Visible;
		}
		if (ImGui::Shortcut(ImGuiKey_F12, 0, ImGuiInputFlags_RouteGlobalLow)) {
			m_ObjectsInfo.Visible = !m_ObjectsInfo.Visible;
		}
		// Animation shortcuts
		if (ImGui::Shortcut(ImGuiKey_ModCtrl | ImGuiKey_P, 0, ImGuiInputFlags_RouteGlobalLow)) {
			m_Animation.Playing = !m_Animation.Playing;
		}
		if (!m_Animation.Playing && ImGui::Shortcut(ImGuiKey_ModCtrl | ImGuiKey_R, 0, ImGuiInputFlags_RouteGlobalLow)) {
			m_Animation = { };
		}
		// Camera shortcuts
		if (ImGui::Shortcut(ImGuiKey_ModCtrl | ImGuiKey_Backspace, 0, ImGuiInputFlags_RouteGlobalLow)) {
			m_Camera->SetAs(*m_InitialCamera);
		}
		// Objects shortcuts
		for (std::size_t i = 0; i < m_Objects.size() && i < 9; i++) {
			if (ImGui::Shortcut(ImGuiKey_ModCtrl | ImGuiKey_1 + i, 0, ImGuiInputFlags_RouteGlobalLow)) {
				m_Objects[i]->GetMaterial().Visible = !m_Objects[i]->GetMaterial().Visible;
			}
		}
	}

	std::unique_ptr<App> CreateApp(CmdLineArgs args) {
		// Initialize logger
		spdlog::set_pattern("[%T] %^[%l]%$ %v");
		spdlog::set_level(spdlog::level::trace);
		spdlog::flush_on(spdlog::level::trace);

		std::string dirname;

		try {
			cxxopts::Options argParser("viewer", "The Pivot viewer of animations for research in computer graphics");
			argParser.add_options()
				("n,dirname", "Directory name", cxxopts::value<std::string>()->default_value("output"))
				("h,help",    "Print usage");
			auto result = argParser.parse(static_cast<int>(args.size()), args.data());
			if (result.count("help")) {
				std::cout << argParser.help() << std::endl;
				std::exit(EXIT_SUCCESS);
			}
			dirname = result["dirname"].as<std::string>();
		} catch (cxxopts::exceptions::exception const &e) {
			spdlog::critical("Failed to parse command line: {}", e.what());
			std::exit(EXIT_FAILURE);
		}

		AppCreateOptions options = {
			.WindowCreateOptions = {
				.Title = "Pivot Viewer",
				.MultisampleCount = 8,
				.MinWidth  = 320,
				.MinHeight = 240,
			},
			.GuiStyleOptions = {
				.FontFilenames = Assets::UbuntuFont,
			},
		};

		return CreateApp<Viewer>(options, dirname);
	}
}
