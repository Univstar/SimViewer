#include "ViewLoader.h"

namespace Pivot {
	void ViewLoader::Run(std::filesystem::path const &dirname, std::uint32_t frameCount, std::vector<std::unique_ptr<ViewObject>> &objects) {
		while (!m_Stopped && m_AvailFrameCount < frameCount) {
			auto const frameDirname = dirname / "results" / std::to_string(m_AvailFrameCount);
			for (auto iter = objects.begin(); !m_Stopped && iter != objects.end(); iter++) {
				(*iter)->LoadNewFrameData(frameDirname);
			}
			m_AvailFrameCount++;
		}
	}

	void ViewLoader::Load(std::filesystem::path &dirname, std::uint32_t frameCount, std::vector<std::unique_ptr<ViewObject>> &objects) {
		for (auto &object : objects) {
			object->ReserveFrames(frameCount);
			object->LoadNewFrameData(dirname / "results" / "0", true);
		}

		m_AvailFrameCount = 1;
		m_Stopped = false;
		m_Thread = std::thread(&ViewLoader::Run, this, std::cref(dirname), frameCount, std::ref(objects));
	}

	void ViewLoader::Stop() {
		if (m_Thread.joinable()) {
			m_Stopped = true;
			m_Thread.join();
		}
	}
}
