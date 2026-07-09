#include "AnimationExporter.h"

namespace Pivot {
	void AnimationExporter::SetDefaultFilename(std::filesystem::path const &filename) {
		m_State.Filename = filename;
		m_State.FilenameStr = m_State.Filename.string();
	}

	void AnimationExporter::PreparePopup(std::uint32_t availableFrames, float defaultFrameRate) {
		m_State.Popup = true;
		m_State.Requested = false;
		m_State.Completed = false;
		m_State.Failed = false;
		m_State.Status.clear();
		m_State.FrameRate = std::max(defaultFrameRate, 1.f);
		m_State.StartFrame = 0;
		m_State.EndFrame = static_cast<int>(std::max(1u, availableFrames) - 1);
	}

	bool AnimationExporter::Start(std::uint32_t availableFrames) {
		if (availableFrames == 0) {
			Fail("No frames are currently available.");
			return false;
		}

		m_State.StartFrame = std::clamp(m_State.StartFrame, 0, static_cast<int>(availableFrames - 1));
		m_State.EndFrame = std::clamp(m_State.EndFrame, 0, static_cast<int>(availableFrames - 1));
		m_State.FrameRate = std::max(m_State.FrameRate, 1.f);
		if (m_State.EndFrame < m_State.StartFrame) {
			Fail("The end frame must be greater than or equal to the start frame.");
			return false;
		}

		m_State.Filename = std::filesystem::absolute(m_State.FilenameStr);
		if (m_State.Filename.empty()) {
			Fail("Please choose an output filename.");
			return false;
		}
		if (m_State.Filename.extension() != ".mp4") {
			m_State.Filename.replace_extension(".mp4");
			m_State.FilenameStr = m_State.Filename.string();
		}

		auto const ffmpeg = FindFfmpegExecutable();
		if (ffmpeg.empty()) {
			Fail("Unable to find ffmpeg. Install ffmpeg or place ffmpeg.exe next to the viewer executable.");
			return false;
		}

		std::error_code ec;
		std::filesystem::create_directories(m_State.Filename.parent_path(), ec);
		m_State.TempDir = MakeTempDir();
		std::filesystem::create_directories(m_State.TempDir, ec);
		if (ec) {
			Fail(fmt::format("Failed to create temporary frame directory: {}", ec.message()));
			return false;
		}

		m_State.NextFrame = static_cast<std::uint32_t>(m_State.StartFrame);
		m_State.TotalFrames = static_cast<std::uint32_t>(m_State.EndFrame - m_State.StartFrame + 1);
		m_State.Exporting = true;
		m_State.Completed = false;
		m_State.Failed = false;
		m_State.Status = fmt::format("Exporting frame {}/{}", 1u, m_State.TotalFrames);
		spdlog::info("Started animation export to \"{}\"", m_State.Filename.string());
		return true;
	}

	bool AnimationExporter::Step(SaveFrameCallback const &saveFrame) {
		if (!m_State.Exporting) {
			return false;
		}

		auto const frameIndex = m_State.NextFrame;
		auto const frameOffset = frameIndex - static_cast<std::uint32_t>(m_State.StartFrame);
		auto const frameImage = m_State.TempDir / fmt::format("frame_{:06d}.png", frameOffset);
		if (!saveFrame(frameImage)) {
			Fail(fmt::format("Failed to write frame image: {}", frameImage.string()));
			return false;
		}

		if (frameIndex >= static_cast<std::uint32_t>(m_State.EndFrame)) {
			return Finalize();
		}

		++m_State.NextFrame;
		auto const exportedFrames = m_State.NextFrame - static_cast<std::uint32_t>(m_State.StartFrame) + 1;
		m_State.Status = fmt::format("Exporting frame {}/{}", exportedFrames, m_State.TotalFrames);
		return true;
	}

	float AnimationExporter::GetProgress() const {
		if (m_State.TotalFrames == 0) {
			return 0.f;
		}
		auto const done = m_State.NextFrame - static_cast<std::uint32_t>(m_State.StartFrame) + 1;
		return std::clamp(1.f * done / m_State.TotalFrames, 0.f, 1.f);
	}

	bool AnimationExporter::Finalize() {
		auto const ffmpeg = FindFfmpegExecutable();
		if (ffmpeg.empty()) {
			Fail("Unable to find ffmpeg while finalizing export.");
			return false;
		}

		auto const inputPattern = m_State.TempDir / "frame_%06d.png";
		auto const ffmpegLog = (m_State.Filename.parent_path() / "ffmpeg.log").string();
		auto const command = fmt::format(
			"{} -y -framerate {} -i {} -vf scale=trunc(iw/2)*2:trunc(ih/2)*2 -c:v libx264 -pix_fmt yuv420p {} 2>{}",
			QuoteCommandExecutable(ffmpeg),
			m_State.FrameRate,
			QuoteCommandArg(inputPattern),
			QuoteCommandArg(m_State.Filename),
			QuoteCommandArg(std::string_view(ffmpegLog)));

		m_State.Status = "Encoding mp4 with ffmpeg...";
		spdlog::info("Running: {}", command);
		if (std::system(command.c_str()) != 0) {
			Fail("ffmpeg failed to encode the mp4 file.");
			return false;
		}

		std::error_code ec;
		std::filesystem::remove_all(m_State.TempDir, ec);
		m_State.Exporting = false;
		m_State.Completed = true;
		m_State.Failed = false;
		m_State.Status = fmt::format("Saved animation to {}", m_State.Filename.string());
		spdlog::info("Saved animation to \"{}\"", m_State.Filename.string());
		return true;
	}

	void AnimationExporter::Fail(std::string message) {
		m_State.Exporting = false;
		m_State.Completed = false;
		m_State.Failed = true;
		m_State.Status = std::move(message);
		spdlog::error("{}", m_State.Status);
	}

	std::filesystem::path AnimationExporter::MakeTempDir() const {
		auto const timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()).count();
		return m_State.Filename.parent_path() / fmt::format("{}.frames.{}", m_State.Filename.stem().string(), timestamp);
	}

	std::filesystem::path AnimationExporter::FindFfmpegExecutable() const {
		auto const local = std::filesystem::current_path() / "ffmpeg.exe";
		if (std::filesystem::exists(local)) {
			return local;
		}
		if (std::system("where ffmpeg >nul 2>&1") == 0) {
			return "ffmpeg";
		}
		return { };
	}

	std::string AnimationExporter::QuoteCommandExecutable(std::filesystem::path const &path) {
		auto const value = path.string();
		if (value.find_first_of(" \t\"") == std::string::npos && !path.has_parent_path()) {
			return value;
		}
		return QuoteCommandArg(std::string_view(value));
	}

	std::string AnimationExporter::QuoteCommandArg(std::filesystem::path const &path) {
		auto const value = path.string();
		return QuoteCommandArg(std::string_view(value));
	}

	std::string AnimationExporter::QuoteCommandArg(std::string_view value) {
		std::string quoted;
		quoted.reserve(value.size() + 2);
		quoted.push_back('"');
		for (char ch : value) {
			if (ch == '"') {
				quoted += '\\';
			}
			quoted.push_back(ch);
		}
		quoted.push_back('"');
		return quoted;
	}
}