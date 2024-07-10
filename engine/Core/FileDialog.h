#pragma once

#include "Core/Window.h"

namespace Pivot {
	class FileDialog {
	public:
		using Filter = std::pair<char const *, char const *>;
		using Filters = std::vector<Filter>;

		static void Init();
		static void Terminate();

		static std::filesystem::path Open(Filters filters = { }, Window *window = nullptr, std::filesystem::path const &defaultPath = { });
		static std::filesystem::path Save(Filters filters = { }, Window *window = nullptr, std::filesystem::path const &defaultPath = { }, std::string_view defaultName = { });
		static std::filesystem::path PickFolder(Window *window = nullptr, std::filesystem::path const &defaultPath = { });
	};
}
