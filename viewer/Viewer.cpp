#include "Viewer.h"

#include "ViewShaderPool.h"

#include "Assets.h"
#include "Core/EntryPoint.h"
#include "Core/Input.h"

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
		Renderer::SetBlended(true);
	}

	Viewer::Viewer(std::filesystem::path const &dirname, bool cliExport, float fps) : Viewer() {
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
		m_AnimationExporter.SetDefaultFilename(m_Dirname / "export" / "animation.mp4");
		auto &exportAnimation = m_AnimationExporter.GetState();
		exportAnimation.EndFrame = static_cast<int>(m_FrameCount - 1);
		exportAnimation.FrameRate = (cliExport && fps > 0.f) ? fps : m_Animation.FrameRate;

		if (cliExport) {
			StartCliExport(fps);
		}
	}

	Viewer::~Viewer() {
		m_Loader.Stop();

		ViewShaderPool::DestroyShaders();
	}

	void Viewer::StartCliExport(float fps) {
		m_CliExport = true;
		auto &state = m_AnimationExporter.GetState();
		state.StartFrame = 0;
		state.EndFrame = static_cast<int>(m_FrameCount - 1);
		state.FrameRate = fps;
		state.FilenameStr = (m_Dirname / "export" / "animation.mp4").string();
		m_AnimationExporter.Start(m_FrameCount);
	}

	bool Viewer::IsCliExportDone() const {
		return m_CliExport && m_CliExportDone;
	}

	void Viewer::Tick(float deltaTime) {
		if (m_CliExport) {
			if (!m_AnimationExporter.IsExporting() && m_CliExportDone) {
				App::Get()->GetWindow()->Close();
				return;
			}
			// Auto-play animation during CLI export
			m_Animation.Playing = true;
			m_Animation.FrameRate = m_AnimationExporter.GetState().FrameRate;
		}
		UpdateCurrentFrame(deltaTime);
		UploadCurrentFrame();
		RenderScene();
		if (m_AnimationExporter.IsExporting()) {
			m_AnimationExporter.Step([this](std::filesystem::path const &filename) {
				return SaveFrameImage(filename);
			});
			if (!m_AnimationExporter.IsExporting() && m_CliExport) {
				// Export finished (success or failure)
				m_CliExportDone = true;
				if (m_AnimationExporter.GetState().Failed) {
					spdlog::error("CLI export failed: {}", m_AnimationExporter.GetState().Status);
					App::Get()->GetWindow()->Close();
				}
			}
		}
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

		if (m_Dimension == 2 && m_GridRes.x > 0) {
			auto ortho = static_cast<OrthoCamera*>(m_Camera.get());
			auto window = App::Get()->GetWindow();
			auto fbSize = window->GetFbSize();
			auto winSize = window->GetSize();
			float sx = static_cast<float>(fbSize.x) / static_cast<float>(winSize.x);
			float sy = static_cast<float>(fbSize.y) / static_cast<float>(winSize.y);

			float mpx = xPos * sx;
			float mpy = static_cast<float>(fbSize.y) - yPos * sy;

			float aspect = static_cast<float>(fbSize.x) / static_cast<float>(fbSize.y);
			float halfH = ortho->GetHeight() * 0.5f;
			float halfW = halfH * aspect;
			glm::vec2 center = ortho->GetCenter();

			m_CursorWorldPos = {
				center.x + (mpx / static_cast<float>(fbSize.x) - 0.5f) * 2.f * halfW,
				center.y + (mpy / static_cast<float>(fbSize.y) - 0.5f) * 2.f * halfH
			};
			m_CursorCell = {
				static_cast<int>(std::floor((m_CursorWorldPos.x - m_GridOrigin.x) / m_GridSpacing)),
				static_cast<int>(std::floor((m_CursorWorldPos.y - m_GridOrigin.y) / m_GridSpacing))
			};
		}
	}

	std::unique_ptr<App> CreateApp(CmdLineArgs args) {
		// Initialize logger
		spdlog::set_pattern("[%T] %^[%l]%$ %v");
		spdlog::set_level(spdlog::level::trace);
		spdlog::flush_on(spdlog::level::trace);

		std::string dirname;
		bool exportMp4 = false;
		float fps = 25.f;

		try {
			cxxopts::Options argParser("viewer", "The Pivot viewer of animations for research in computer graphics");
			argParser.add_options()
				("n,dirname",   "Directory name", cxxopts::value<std::string>()->default_value("output"))
				("export-mp4",  "Export all frames to MP4 and exit")
				("fps",        "Frame rate for MP4 export (default: 25)", cxxopts::value<float>()->default_value("25"))
				("h,help",     "Print usage");
			auto result = argParser.parse(static_cast<int>(args.size()), args.data());
			if (result.count("help")) {
				std::cout << argParser.help() << std::endl;
				std::exit(EXIT_SUCCESS);
			}
			dirname   = result["dirname"].as<std::string>();
			exportMp4 = result.count("export-mp4") > 0;
			fps       = result["fps"].as<float>();
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
				.FontFilenames = Assets::UbuntuFonts,
			},
		};

		return CreateApp<Viewer>(options, dirname, exportMp4, fps);
	}
}
