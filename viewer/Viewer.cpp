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
		m_AnimationExporter.SetDefaultFilename(m_Dirname / "export" / "animation.mp4");
		auto &exportAnimation = m_AnimationExporter.GetState();
		exportAnimation.EndFrame = static_cast<int>(m_FrameCount - 1);
		exportAnimation.FrameRate = m_Animation.FrameRate;
	}

	Viewer::~Viewer() {
		m_Loader.Stop();

		ViewShaderPool::DestroyShaders();
	}

	void Viewer::Tick(float deltaTime) {
		UpdateCurrentFrame(deltaTime);
		UploadCurrentFrame();
		RenderScene();
		if (m_AnimationExporter.IsExporting()) {
			m_AnimationExporter.Step([this](std::filesystem::path const &filename) {
				return SaveFrameImage(filename);
			});
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
				.FontFilenames = Assets::UbuntuFonts,
			},
		};

		return CreateApp<Viewer>(options, dirname);
	}
}
