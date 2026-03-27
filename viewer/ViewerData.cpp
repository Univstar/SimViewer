#include "Viewer.h"

#include "Core/App.h"
#include "Core/FileDialog.h"
#include "Utils/Image.h"

namespace Pivot {
	bool Viewer::LoadDirectory(std::filesystem::path const &dirname) {
		m_Dirname = std::filesystem::absolute(dirname);
		{
			std::ifstream fin(m_Dirname / "description.yaml");
			if (!fin) {
				spdlog::error("Failed to open \"{}\"", (m_Dirname / "description.yaml").string());
				return false;
			}
			m_Description = YAML::Load(fin);
		}

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

		if (!m_Description["Objects"] || !m_Description["Objects"].IsSequence()) {
			spdlog::error("Failed to find any object in description.yaml");
			return false;
		}
		for (auto const &node : m_Description["Objects"]) {
			auto object = std::make_unique<ViewObject>(m_Objects.size(), node, m_Dimension);
			m_ObjectLayers[static_cast<std::size_t>(object->GetMaterial().Mode)].push_back(object.get());
			m_Objects.push_back(std::move(object));
		}

		{
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
		auto filename = FileDialog::Save({ { "Portable Network Graphics", "png"}, { "JPEG File Interchange Format", "jpg" }, { "Windows Bitmap", "bmp" } }, App::Get()->GetWindow());
		if (!filename.empty()) {
			auto const size = App::Get()->GetWindow()->GetSize();
			auto const pixels = Renderer::ReadPixels({ 0, 0 }, size, 3);
			Image::WriteBytes(filename, pixels, size.x, size.y, 3, true);
			spdlog::info("Screenshot saved in \"{}\"", filename.string());
			return true;
		}
		return false;
	}

	bool Viewer::SaveFrameImage(std::filesystem::path const &filename) const {
		auto const size = App::Get()->GetWindow()->GetFbSize();
		auto const pixels = Renderer::ReadPixels({ 0, 0 }, size, 3);
		Image::WriteBytes(filename, pixels, static_cast<int>(size.x), static_cast<int>(size.y), 3, true);
		return true;
	}

	void Viewer::ExportModels() {
		std::filesystem::create_directories(m_ExportModels.Dirname);
		std::uint32_t const frameBegin = m_ExportModels.AllFramesSelected ? 0 : m_CurrentFrame;
		std::uint32_t const frameEnd   = m_ExportModels.AllFramesSelected ? m_AvailFrameCount : m_CurrentFrame + 1;
		for (std::size_t i = 0; i < m_Objects.size(); i++) {
			if (!m_ExportModels.Exported[i]) continue;
			for (auto frame = frameBegin; frame < frameEnd; frame++) {
				fmt::print(fmt::fg(fmt::color::yellow_green), "\r* Exporting \"{}\"... ({}/{})", m_Objects[i]->GetName(), frame - frameBegin + 1, frameEnd - frameBegin);
				m_Objects[i]->Export(frame, m_ExportModels.Dirname);
			}
			fmt::print("\r");
			spdlog::info("Completed exportation of \"{}\"", m_Objects[i]->GetName());
		}
	}
}