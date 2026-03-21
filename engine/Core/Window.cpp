#include "Window.h"

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif
#include <GLFW/glfw3.h>

namespace Pivot {
	Window::Window(WindowCreateOptions const &options) {
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
		glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
		glfwWindowHint(GLFW_SAMPLES, options.MultisampleCount);

		assert(*(options.Title.cend()) == '\0');

		m_Native = glfwCreateWindow(options.Width, options.Height, options.Title.data(), nullptr, nullptr);
		if (m_Native) {
			spdlog::trace("Succeeded to create the window");
		} else {
			spdlog::critical("Failed to create the window");
			std::exit(EXIT_FAILURE);
		}

		glfwSetWindowUserPointer(m_Native, this);

		m_Closed = glfwWindowShouldClose(m_Native);

		glfwGetWindowSize(m_Native, &m_Width, &m_Height);
		glfwGetFramebufferSize(m_Native, &m_FbWidth, &m_FbHeight);
		glfwGetWindowContentScale(m_Native, &m_XScale, &m_YScale);
		spdlog::debug("Window size: ({}, {}), framebuffer size: ({}, {}), scale: ({}, {})",
			m_Width, m_Height, m_FbWidth, m_FbHeight, m_XScale, m_YScale);
		m_Scale = std::min(m_XScale, m_YScale);

		m_Minimized = glfwGetWindowAttrib(m_Native, GLFW_ICONIFIED);
		m_Maximized = glfwGetWindowAttrib(m_Native, GLFW_MAXIMIZED);
		m_Focused   = glfwGetWindowAttrib(m_Native, GLFW_FOCUSED);
		m_Hovered   = glfwGetWindowAttrib(m_Native, GLFW_HOVERED);
		m_Visible   = glfwGetWindowAttrib(m_Native, GLFW_VISIBLE);

		m_MultisampleCount = options.MultisampleCount;
		m_MinWidth  = options.MinWidth;
		m_MinHeight = options.MinHeight;
		m_MaxWidth  = options.MaxWidth;
		m_MaxHeight = options.MaxHeight;

		ResetSizeLimits();
		SetCallbacks();
	}

	Window::~Window() {
		glfwDestroyWindow(m_Native);
	}

	void Window::ResetSizeLimits() {
		glfwSetWindowSizeLimits(
			m_Native,
			m_MinWidth.has_value()  ? static_cast<int>(m_MinWidth.value()  * m_Scale) : GLFW_DONT_CARE,
			m_MinHeight.has_value() ? static_cast<int>(m_MinHeight.value() * m_Scale) : GLFW_DONT_CARE,
			m_MaxWidth.has_value()  ? static_cast<int>(m_MaxWidth.value()  * m_Scale) : GLFW_DONT_CARE,
			m_MaxHeight.has_value() ? static_cast<int>(m_MaxHeight.value() * m_Scale) : GLFW_DONT_CARE);
	}

	void Window::SetCallbacks() {
		glfwSetWindowCloseCallback(m_Native, [](GLFWwindow *window) {
			auto win = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
			win->m_Closed = GLFW_TRUE;
			if (win->m_WindowCloseHandler) win->m_WindowCloseHandler(win);
		});
		glfwSetWindowSizeCallback(m_Native, [](GLFWwindow *window, int width, int height) {
			auto win = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
			win->m_Width  = width;
			win->m_Height = height;
			if (win->m_WindowSizeHandler) win->m_WindowSizeHandler(win, width, height);
		});
		glfwSetFramebufferSizeCallback(m_Native, [](GLFWwindow *window, int width, int height) {
			auto win = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
			win->m_FbWidth  = width;
			win->m_FbHeight = height;
			if (win->m_FramebufferSizeHandler) win->m_FramebufferSizeHandler(win, width, height);
		});
		glfwSetWindowContentScaleCallback(m_Native, [](GLFWwindow *window, float xScale, float yScale) {
			auto win = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
			win->m_XScale = xScale;
			win->m_YScale = yScale;
			win->m_Scale  = std::min(xScale, yScale);
			win->ResetSizeLimits();
			if (win->m_WindowContentScaleHandler) win->m_WindowContentScaleHandler(win, win->m_Scale);
		});
		glfwSetWindowIconifyCallback(m_Native, [](GLFWwindow *window, int iconified) {
			auto win = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
			win->m_Minimized = iconified;
			if (win->m_WindowMinimizeHandler) win->m_WindowMinimizeHandler(win, iconified);
		});
		glfwSetWindowMaximizeCallback(m_Native, [](GLFWwindow *window, int maximized) {
			auto win = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
			win->m_Maximized = maximized;
			if (win->m_WindowMaximizeHandler) win->m_WindowMaximizeHandler(win, maximized);
		});
		glfwSetWindowFocusCallback(m_Native, [](GLFWwindow *window, int focused) {
			auto win = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
			win->m_Focused = focused;
			if (win->m_WindowFocusHandler) win->m_WindowFocusHandler(win, focused);			
		});
		glfwSetCursorEnterCallback(m_Native, [](GLFWwindow *window, int entered) {
			auto win = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
			win->m_Hovered = entered;
			if (win->m_WindowHoverHandler) win->m_WindowHoverHandler(win, entered);			
		});
		glfwSetWindowRefreshCallback(m_Native, [](GLFWwindow *window) {
			auto win = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
			if (win->m_WindowRefreshHandler) win->m_WindowRefreshHandler(win);	
		});

		glfwSetKeyCallback(m_Native, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
			auto win = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
			if (win->m_KeyHandler) win->m_KeyHandler(win, static_cast<Key>(key), static_cast<ButtonAction>(action), mods);
		});
		glfwSetCursorPosCallback(m_Native, [](GLFWwindow *window, double xPos, double yPos) {
			auto win = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
			if (win->m_MouseMoveHandler) win->m_MouseMoveHandler(win, xPos, yPos);
		});
		glfwSetMouseButtonCallback(m_Native, [](GLFWwindow *window, int button, int action, int mods) {
			auto win = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
			if (win->m_MouseButtonHandler) win->m_MouseButtonHandler(win, static_cast<MouseButton>(button), static_cast<ButtonAction>(action), mods);
		});
		glfwSetScrollCallback(m_Native, [](GLFWwindow *window, double xOffset, double yOffset) {
			auto win = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
			if (win->m_MouseWheelHandler) win->m_MouseWheelHandler(win, xOffset, yOffset);
		});
		glfwSetDropCallback(m_Native, [](GLFWwindow *window, int count, char const **paths) {
			auto win = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
			if (win->m_PathsDropHandler) win->m_PathsDropHandler(win, std::span(paths, count));
		});
	}

	void Window::Close() {
		m_Closed = GLFW_TRUE;
		glfwSetWindowShouldClose(m_Native, GLFW_TRUE);
	}

	void Window::DoNotClose() {
		m_Closed = GLFW_FALSE;
		glfwSetWindowShouldClose(m_Native, GLFW_FALSE);
	}

	void Window::Show() {
		m_Visible = GLFW_TRUE;
		glfwShowWindow(m_Native);
	}

	void Window::Hide() {
		m_Visible = GLFW_FALSE;
		glfwHideWindow(m_Native);
	}

	void Window::SetTitle(std::string_view title) {
		glfwSetWindowTitle(m_Native, title.data());
	}

	void Window::SetSize(std::uint32_t width, std::uint32_t height) {
		glfwSetWindowSize(m_Native, width, height);
	}
}
