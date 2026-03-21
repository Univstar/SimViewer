#include "Gui.h"

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif
#include <GLFW/glfw3.h>

namespace Pivot {
	static float                             g_Scale = 1.f;
	static std::span<std::string_view const> g_FontFilenames;
	static std::uint32_t                     g_FontSize = 0;
	static ImGuiStyle                        g_Style;

	static void ResetInternalStyle() {
		ImGui_ImplOpenGL3_DestroyFontsTexture();
		ImGui::GetIO().Fonts->Clear();
		for (auto const &filename : g_FontFilenames) {
			assert(*(filename.cend()) == '\0');
			ImGui::GetIO().Fonts->AddFontFromFileTTF(filename.data(), std::floor(g_FontSize * g_Scale));
		}
		ImGui_ImplOpenGL3_CreateFontsTexture();

		ImGui::GetStyle() = g_Style;
		ImGui::GetStyle().ScaleAllSizes(g_Scale);
	}

	void Gui::Init() {
		ImGui::CreateContext();

		auto &io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;          // Enable Keyboard Controls
		// io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;        // Enable Gamepad Controls
		// io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		// io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		// io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports;
		// io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;
		// io.ConfigViewportsNoAutoMerge = true;
		// io.ConfigViewportsNoTaskBarIcon = true;
		io.IniFilename = nullptr;
		io.LogFilename = nullptr;

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		// ImGui::StyleColorsLight();

		auto *window = glfwGetCurrentContext();

		// Setup Platform/Renderer backends
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init();
	}

	void Gui::Terminate() {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void Gui::ResetStyle(GuiStyleOptions const &options) {
		g_FontFilenames = options.FontFilenames;
		g_FontSize      = options.FontSize;

		g_Style = { };
		
		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		// if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			// g_Style.WindowRounding = 0.f;
			// g_Style.Colors[ImGuiCol_WindowBg].w = 1.f;
		// }

		if (g_Scale) {
			ResetInternalStyle();
		}
	}

	void Gui::ResetScale(float scale) {
		g_Scale = scale;
		if (g_FontSize) {
			ResetInternalStyle();
		}
	}

	bool Gui::AreKeyEventsBlocked() {
		return ImGui::GetIO().WantCaptureKeyboard;
	}

	bool Gui::AreMouseEventsBlocked() {
		return ImGui::GetIO().WantCaptureMouse;
	}

	void Gui::Prepare() {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void Gui::Submit() {
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Update and Render additional Platform Windows
		// (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
		// if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		// {
			// GLFWwindow *backupCurrentContext = glfwGetCurrentContext();
			// ImGui::UpdatePlatformWindows();
			// ImGui::RenderPlatformWindowsDefault();
			// glfwMakeContextCurrent(backupCurrentContext);
		// }
	}
}
