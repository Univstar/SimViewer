#pragma once

#include "Utils/Common.h"

namespace Pivot {
	struct GuiStyleOptions {
		std::span<std::string_view const> FontFilenames;
		std::uint32_t                     FontSize      = 13;
	};

	class Gui {
	public:
		static void Init();
		static void Terminate();

		static void ResetStyle(GuiStyleOptions const & options);
		static void ResetScale(float scale);

		static bool AreKeyEventsBlocked();
		static bool AreMouseEventsBlocked();

		static void Prepare();
		static void Submit();
	};
}
