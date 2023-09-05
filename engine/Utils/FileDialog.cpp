#include "FileDialog.h"

#include <nfd.h>

#include <stdexcept>

void FileDialog::Init() {
	if (NFD_Init() != NFD_OKAY) {
		throw std::runtime_error(NFD_GetError());
	}
}

void FileDialog::Terminate() {
	NFD_Quit();
}

std::filesystem::path FileDialog::Open(Filters filters, std::filesystem::path const &defaultPath) {
	char *buf;
	auto result = NFD_OpenDialog(
		&buf,
		reinterpret_cast<nfdfilteritem_t const *>(filters.data()),
		filters.size(),
		defaultPath.empty() ? nullptr : defaultPath.string().c_str());
	if (result == NFD_OKAY) {
		auto ret = std::filesystem::path(buf);
		NFD_FreePath(buf);
		return ret;
	} else if (result == NFD_CANCEL) {
		return { };
	} else {
		throw std::runtime_error(NFD_GetError());
	}
}

std::filesystem::path FileDialog::Save(Filters filters, std::filesystem::path const &defaultPath, std::string_view defaultName) {
	char *buf;
	auto result = NFD_SaveDialog(
		&buf,
		reinterpret_cast<nfdfilteritem_t const *>(filters.data()),
		filters.size(),
		defaultPath.empty() ? nullptr : defaultPath.string().c_str(),
		defaultName.data());
	if (result == NFD_OKAY) {
		auto ret = std::filesystem::path(buf);
		NFD_FreePath(buf);
		return ret;
	} else if (result == NFD_CANCEL) {
		return { };
	} else {
		throw std::runtime_error(NFD_GetError());
	}
}

std::filesystem::path FileDialog::PickFolder(std::filesystem::path const &defaultPath) {
	char *buf;
	auto result = NFD_PickFolder(
		&buf,
		defaultPath.empty() ? nullptr : defaultPath.string().c_str());
	if (result == NFD_OKAY) {
		auto ret = std::filesystem::path(buf);
		NFD_FreePath(buf);
		return ret;
	} else if (result == NFD_CANCEL) {
		return { };
	} else {
		throw std::runtime_error(NFD_GetError());
	}
}
