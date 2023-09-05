#pragma once

#include <imgui.h>

#include <string>

#include <cstdarg>

namespace ImGuiEx {
	bool InputText(char const* label, std::string *str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void *user_data = nullptr);

	void TextRightAligned(char const *fmt, ...);
	void TextRightAlignedV(char const *fmt, std::va_list args);

	bool BeginMainStatusBar();
	void EndMainStatusBar();
}
