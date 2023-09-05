#pragma once

#include "ViewObject.h"

namespace Pivot {
	class ViewLoader {
	private:
		void Run(std::filesystem::path const &dirname, std::uint32_t frameCount, std::vector<std::unique_ptr<ViewObject>> &objects);

	public:
		std::uint32_t GetAvailFrameCount() const { return m_AvailFrameCount; }
		
		void Load(std::filesystem::path &dirname, std::uint32_t frameCount, std::vector<std::unique_ptr<ViewObject>> &objects);
		void Stop();

	private:
		std::thread          m_Thread;
		std::atomic_uint32_t m_AvailFrameCount;
		std::atomic_bool     m_Stopped;
	};
}
