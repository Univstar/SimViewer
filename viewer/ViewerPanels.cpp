#include "Viewer.h"

#include "ViewShaderPool.h"

#include "Core/App.h"

#include <imgui.h>

namespace Pivot {
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
			case ViewShader::Default2D: {
				ImGui::ColorEdit4("Albedo", glm::value_ptr(curMat.Albedo));
				break;
			}
			case ViewShader::Default2D_Points: {
				ImGui::ColorEdit4("Albedo", glm::value_ptr(curMat.Albedo));
				ImGui::SliderFloat("Scale", &curMat.RadScale, .1f, 1.f, "%.3f", ImGuiSliderFlags_NoInput | ImGuiSliderFlags_NoRoundToFormat);
				break;
			}
			case ViewShader::Default3D_Triangles:
			case ViewShader::Default3D_Points: {
				ImGui::ColorEdit4("Albedo", glm::value_ptr(curMat.Albedo));
				ImGui::SliderFloat("Metallic", &curMat.Metallic, 0.f, 1.f, "%.3f", ImGuiSliderFlags_NoInput | ImGuiSliderFlags_NoRoundToFormat);
				ImGui::SliderFloat("Roughness", &curMat.Roughness, 0.f, 1.f, "%.3f", ImGuiSliderFlags_NoInput | ImGuiSliderFlags_NoRoundToFormat);
				break;
			}
			case ViewShader::Heatmap2D: {
				ImGui::Spacing();
				ImGui::SeparatorText("Heat");
				if (ImGui::RadioButton("Sequential", !curMat.HeatDiv)) {
					curMat.HeatDiv = false;
				}
				ImGui::SameLine();
				if (ImGui::RadioButton("Diverging", curMat.HeatDiv)) {
					curMat.HeatDiv = true;
				}
				ImGui::BeginDisabled();
				std::array heatRange = { curMat.HeatMin, curMat.HeatMax };
				ImGui::InputFloat2("Range", heatRange.data(), "%g");
				ImGui::EndDisabled();
				break;
			}
			case ViewShader::Heatmap2D_Points: {
				ImGui::SliderFloat("Scale", &curMat.RadScale, .1f, 1.f, "%.3f", ImGuiSliderFlags_NoInput | ImGuiSliderFlags_NoRoundToFormat);
				ImGui::Spacing();
				ImGui::SeparatorText("Heat");
				if (ImGui::RadioButton("Sequential", !curMat.HeatDiv)) {
					curMat.HeatDiv = false;
				}
				ImGui::SameLine();
				if (ImGui::RadioButton("Diverging", curMat.HeatDiv)) {
					curMat.HeatDiv = true;
				}
				ImGui::BeginDisabled();
				std::array heatRange = { curMat.HeatMin, curMat.HeatMax };
				ImGui::InputFloat2("Range", heatRange.data(), "%g");
				ImGui::EndDisabled();
				break;
			}
			case ViewShader::Heatmap3D_Triangles: {
				ImGui::SliderFloat("Metallic", &curMat.Metallic, 0.f, 1.f, "%.3f", ImGuiSliderFlags_NoInput | ImGuiSliderFlags_NoRoundToFormat);
				ImGui::SliderFloat("Roughness", &curMat.Roughness, 0.f, 1.f, "%.3f", ImGuiSliderFlags_NoInput | ImGuiSliderFlags_NoRoundToFormat);
				ImGui::Spacing();
				ImGui::SeparatorText("Heat");
				if (ImGui::RadioButton("Sequential", !curMat.HeatDiv)) {
					curMat.HeatDiv = false;
				}
				ImGui::SameLine();
				if (ImGui::RadioButton("Diverging", curMat.HeatDiv)) {
					curMat.HeatDiv = true;
				}
				ImGui::BeginDisabled();
				std::array heatRange = { curMat.HeatMin, curMat.HeatMax };
				ImGui::InputFloat2("Range", heatRange.data(), "%g");
				ImGui::EndDisabled();
				break;
			}
			}
		}
		ImGui::End();
	}
}