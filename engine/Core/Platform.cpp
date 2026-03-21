#include "Platform.h"

#include <glad/glad.h>
#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif
#include <GLFW/glfw3.h>

#undef CreateWindow

namespace Pivot {
	static bool g_ApiLoaderInitialized = false;

	void Platform::Init() {
		glfwSetErrorCallback([](int error, char const * desc) {
			spdlog::critical("[GLFW {}] {}", error, desc);
			std::exit(EXIT_FAILURE);
		});
		if (glfwInit()) {
			spdlog::trace("Succeeded to initialize GLFW");
		} else {
			spdlog::critical("Failed to initialize GLFW");
			std::exit(EXIT_FAILURE);
		}
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	}

	void Platform::Terminate() {
		glfwTerminate();
	}

	void Platform::MakeContextCurrent(Window *window) {
		glfwMakeContextCurrent(window->m_Native);
		if (!g_ApiLoaderInitialized) {
			if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
				spdlog::trace("Succeeded to initialize GLAD");
			} else {
				spdlog::critical("Failed to initialize GLAD");
				std::exit(EXIT_FAILURE);
			}
			g_ApiLoaderInitialized = true;
		}
	}

	void Platform::PollEvents() {
		glfwPollEvents();
	}

	void Platform::SwapBuffers(Window *window) {
		glfwSwapBuffers(window->m_Native);
	}

	void Platform::SetVSynchronized(bool enabled) {
		glfwSwapInterval(enabled);
	}
}
