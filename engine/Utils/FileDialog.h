#pragma once

#include <filesystem>
#include <string_view>
#include <vector>

class FileDialog {
public:
	using Filter = std::pair<char const *, char const *>;
	using Filters = std::vector<Filter>;

	static void Init();
	static void Terminate();

	static std::filesystem::path Open(Filters filters = { }, std::filesystem::path const &defaultPath = { });
	static std::filesystem::path Save(Filters filters = { }, std::filesystem::path const &defaultPath = { }, std::string_view defaultName = { });
	static std::filesystem::path PickFolder(std::filesystem::path const &defaultPath = { });
};
