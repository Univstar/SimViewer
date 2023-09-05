#pragma once

#include "Core/Event.h"
#include "Utils/Common.h"

struct GLFWwindow;

namespace Pivot {
	struct WindowCreateOptions {
		std::string_view Title  = "Pivot Engine";
		std::uint32_t    Width  = 800;
		std::uint32_t    Height = 600;
		// std::span<std::string_view const> IconFilenames;
		std::uint32_t    MultisampleCount = 0;

		std::optional<std::uint32_t> MinWidth;
		std::optional<std::uint32_t> MinHeight;
		std::optional<std::uint32_t> MaxWidth;
		std::optional<std::uint32_t> MaxHeight;
	};

	class Window {
	private:
		friend class Platform;

		explicit Window(WindowCreateOptions const &options);
	
	public:
		~Window();

		auto          GetNative()   const { return m_Native; }
		std::uint32_t GetWidth()    const { return m_Width; }
		std::uint32_t GetHeight()   const { return m_Height; }
		float         GetAspect()   const { return 1.f * m_Width / m_Height; }
		glm::uvec2    GetSize()     const { return { m_Width, m_Height}; }
		std::uint32_t GetFbWidth()  const { return m_FbWidth; }
		std::uint32_t GetFbHeight() const { return m_FbHeight; }
		glm::uvec2    GetFbSize()   const { return { m_FbWidth, m_FbHeight }; }
		float         GetXScale()   const { return m_XScale / (m_FbWidth / m_Width); }
		float         GetYScale()   const { return m_YScale / (m_FbHeight / m_Height); }
		float         GetScale()    const { return m_Scale; }
		bool          IsClosed()    const { return m_Closed; }
		bool          IsMinimized() const { return m_Minimized; }
		bool          IsMaximized() const { return m_Maximized; }
		bool          IsFocused()   const { return m_Focused; }
		bool          IsHovered()   const { return m_Hovered; }
		bool          IsVisible()   const { return m_Visible; }

		std::uint32_t GetMultisampleCount() const { return m_MultisampleCount; }

		void Close();
		void DoNotClose();
		void Show();
		void Hide();

		void SetTitle(std::string_view title);
		void SetSize(std::uint32_t width, std::uint32_t height);
		void SetSize(glm::uvec2 const &size) { SetSize(size.x, size.y); }
	
	public:
		template <typename T> void SetHandler(typename T::Handler handler);

#define DECLARE_HANDLER(event)                              \
	private:                                                \
		Event::event::Handler m_##event##Handler = nullptr; \
	public:                                                 \
		template <> void SetHandler<Event::event>(Event::event::Handler handler) { m_##event##Handler = handler; }
	
		DECLARE_HANDLER(WindowClose)
		DECLARE_HANDLER(WindowSize)
		DECLARE_HANDLER(FramebufferSize)
		DECLARE_HANDLER(WindowContentScale)
		DECLARE_HANDLER(WindowMinimize)
		DECLARE_HANDLER(WindowMaximize)
		DECLARE_HANDLER(WindowFocus)
		DECLARE_HANDLER(WindowHover)
		DECLARE_HANDLER(WindowRefresh)

		DECLARE_HANDLER(Key)
		DECLARE_HANDLER(MouseMove)
		DECLARE_HANDLER(MouseButton)
		DECLARE_HANDLER(MouseWheel)
		DECLARE_HANDLER(PathsDrop)
#undef DECLARE_HANDLER

	private:
		void ResetSizeLimits();
		void SetCallbacks();

	private:
		GLFWwindow *m_Native;

		// Window data
		int   m_Closed;
		int   m_Width;
		int   m_Height;
		int   m_FbWidth;
		int   m_FbHeight;
		float m_XScale;
		float m_YScale;
		float m_Scale;
		int   m_Minimized;
		int   m_Maximized;
		int   m_Focused;
		int   m_Hovered;
		int   m_Visible;

		std::uint32_t  m_MultisampleCount;
		std::optional<std::uint32_t> m_MinWidth;
		std::optional<std::uint32_t> m_MinHeight;
		std::optional<std::uint32_t> m_MaxWidth;
		std::optional<std::uint32_t> m_MaxHeight;
	};
}
