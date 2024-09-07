#include "FileDialog.h"

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#elif __linux__
#define GLFW_EXPOSE_NATIVE_X11
#elif __APPLE__
#define GLFW_EXPOSE_NATIVE_COCOA
#endif

#include <nfd.h>
#include <nfd_glfw3.h>

namespace Pivot {
	void FileDialog::Init() {
		if (NFD_Init() != NFD_OKAY) {
			throw std::runtime_error(NFD_GetError());
		}
	}

	void FileDialog::Terminate() {
		NFD_Quit();
	}

	std::filesystem::path FileDialog::Open(Filters filters, Window *window, std::filesystem::path const &defaultPath) {
		nfdopendialogu8args_t args = {
			.filterList  = reinterpret_cast<nfdfilteritem_t const *>(filters.data()),
			.filterCount = static_cast<nfdfiltersize_t >(filters.size()),
			.defaultPath = defaultPath.empty() ? nullptr : defaultPath.string().c_str(),
		};
		if (window) {
			NFD_GetNativeWindowFromGLFWWindow(window->GetNative(), &args.parentWindow);
		}

		char *buf;
		auto result = NFD_OpenDialogU8_With(&buf, &args);
		if (result == NFD_OKAY) {
			std::wstring_convert<std::codecvt_utf8<wchar_t>> wcv;
			auto ret = std::filesystem::path(wcv.from_bytes(buf));
			NFD_FreePath(buf);
			return ret;
		} else if (result == NFD_CANCEL) {
			return { };
		} else {
			spdlog::error("Failed in OpenDialog. {}", NFD_GetError());
			return { };
		}
	}

	std::filesystem::path FileDialog::Save(Filters filters, Window *window, std::filesystem::path const &defaultPath, std::string_view defaultName) {
		nfdsavedialogu8args_t args = {
			.filterList  = reinterpret_cast<nfdfilteritem_t const *>(filters.data()),
			.filterCount = static_cast<nfdfiltersize_t >(filters.size()),
			.defaultPath = defaultPath.empty() ? nullptr : defaultPath.string().c_str(),
			.defaultName = defaultName.data(),
		};
		if (window) {
			NFD_GetNativeWindowFromGLFWWindow(window->GetNative(), &args.parentWindow);
		}

		char *buf;
		auto result = NFD_SaveDialogU8_With(&buf, &args);
		if (result == NFD_OKAY) {
			std::wstring_convert<std::codecvt_utf8<wchar_t>> wcv;
			auto ret = std::filesystem::path(wcv.from_bytes(buf));
			NFD_FreePath(buf);
			return ret;
		} else if (result == NFD_CANCEL) {
			return { };
		} else {
			spdlog::error("Failed in SaveDialog. {}", NFD_GetError());
			return { };
		}
	}

	std::filesystem::path FileDialog::PickFolder(Window *window, std::filesystem::path const &defaultPath) {
		nfdpickfolderu8args_t args = {
			.defaultPath = defaultPath.empty() ? nullptr : defaultPath.string().c_str(),
		};
		if (window) {
			NFD_GetNativeWindowFromGLFWWindow(window->GetNative(), &args.parentWindow);
		}
		
		char *buf;
		auto result = NFD_PickFolderU8_With(&buf, &args);
		if (result == NFD_OKAY) {
			std::wstring_convert<std::codecvt_utf8<wchar_t>> wcv;
			auto ret = std::filesystem::path(wcv.from_bytes(buf));
			NFD_FreePath(buf);
			return ret;
		} else if (result == NFD_CANCEL) {
			return { };
		} else {
			spdlog::error("Failed in PickFolder. {}", NFD_GetError());
			return { };
		}
	}
}
