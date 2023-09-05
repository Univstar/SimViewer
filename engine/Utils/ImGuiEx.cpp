#include "ImGuiEx.h"

#include <imgui_internal.h>

namespace ImGuiEx {
	struct InputTextCallback_UserData
	{
		std::string            *Str;
		ImGuiInputTextCallback  ChainCallback;
		void                   *ChainCallbackUserData;
	};

	static int InputTextCallback(ImGuiInputTextCallbackData* data)
	{
		auto user_data = reinterpret_cast<InputTextCallback_UserData *>(data->UserData);
		if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
			// Resize string callback
			// If for some reason we refuse the new length (BufTextLen) and/or capacity (BufSize) we need to set them back to what we want.
			std::string *str = user_data->Str;
			IM_ASSERT(data->Buf == str->c_str());
			str->resize(data->BufTextLen);
			data->Buf = const_cast<char *>(str->c_str());
		}
		else if (user_data->ChainCallback) {
			// Forward to user callback, if any
			data->UserData = user_data->ChainCallbackUserData;
			return user_data->ChainCallback(data);
		}
		return 0;
	}

	bool InputText(const char *label, std::string *str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void *user_data) {
		IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
		flags |= ImGuiInputTextFlags_CallbackResize;

		InputTextCallback_UserData cb_user_data;
		cb_user_data.Str = str;
		cb_user_data.ChainCallback = callback;
		cb_user_data.ChainCallbackUserData = user_data;
		return ImGui::InputText(label, const_cast<char *>(str->c_str()), str->capacity() + 1, flags, InputTextCallback, &cb_user_data);
	}

	void TextRightAligned(char const *fmt, ...) {
		va_list args;
		va_start(args, fmt);
		TextRightAlignedV(fmt, args);
		va_end(args);
	}

	void TextRightAlignedV(char const *fmt, std::va_list args) {
		if (ImGui::GetCurrentWindow()->SkipItems) return;
		char const *begin, *end;
		ImFormatStringToTempBufferV(&begin, &end, fmt, args);
		auto const width = ImGui::CalcTextSize(begin, end).x;
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - width);
		ImGui::TextEx(begin, end, ImGuiTextFlags_NoWidthForLargeClippedText);
	}

	bool BeginMainStatusBar() {
		ImGuiWindowFlags const flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
		float const height = ImGui::GetFrameHeight();
		bool ret = ImGui::BeginViewportSideBar("(Main status bar)", ImGui::GetMainViewport(), ImGuiDir_Down, height, flags);
		if (ret) {
			ImGui::BeginMenuBar();
		} else {
			ImGui::End();
		}
		return ret;
	}

	void EndMainStatusBar() {
		ImGui::EndMenuBar();

		// (Copy from ImGui::EndMainMenuBar())
		// When the user has left the menu layer (typically: closed menus through activation of an item), we restore focus to the previous window
		// FIXME: With this strategy we won't be able to restore a NULL focus.
		auto &g = *ImGui::GetCurrentContext();
		if (g.CurrentWindow == g.NavWindow && g.NavLayer == ImGuiNavLayer_Main && !g.NavAnyRequest)
			ImGui::FocusTopMostWindowUnderOne(g.NavWindow, nullptr, nullptr, ImGuiFocusRequestFlags_UnlessBelowModal | ImGuiFocusRequestFlags_RestoreFocusedChild);
		
		ImGui::End();
	}
}
