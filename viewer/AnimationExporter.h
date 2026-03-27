#pragma once

#include "Utils/Common.h"

namespace Pivot {
	struct AnimationExportState {
		bool Requested = false;
		bool Popup     = false;
		bool Exporting = false;
		bool Completed = false;
		bool Failed    = false;

		int   StartFrame = 0;
		int   EndFrame   = 0;
		float FrameRate  = 25.f;

		std::filesystem::path Filename;
		std::string           FilenameStr;
		std::filesystem::path TempDir;

		std::uint32_t NextFrame   = 0;
		std::uint32_t TotalFrames = 0;
		std::string   Status;
	};

	class AnimationExporter {
	public:
		using SaveFrameCallback = std::function<bool(std::filesystem::path const &filename)>;

		AnimationExportState &GetState() { return m_State; }
		AnimationExportState const &GetState() const { return m_State; }

		void SetDefaultFilename(std::filesystem::path const &filename);
		void PreparePopup(std::uint32_t availableFrames, float defaultFrameRate);
		bool Start(std::uint32_t availableFrames);
		bool Step(SaveFrameCallback const &saveFrame);

		bool IsExporting() const { return m_State.Exporting; }
		std::uint32_t GetFrameToRender() const { return m_State.NextFrame; }
		float GetProgress() const;

	private:
		bool Finalize();
		void Fail(std::string message);
		std::filesystem::path MakeTempDir() const;
		std::filesystem::path FindFfmpegExecutable() const;
		static std::string QuoteCommandExecutable(std::filesystem::path const &path);
		static std::string QuoteCommandArg(std::filesystem::path const &path);
		static std::string QuoteCommandArg(std::string_view value);

	private:
		AnimationExportState m_State;
	};
}