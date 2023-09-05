#pragma once

#include "Core/Gui.h"
#include "Core/Kernel.h"
#include "Core/Window.h"
#include "Utils/Common.h"

int main(int, char **);

struct GLFWwindow;

namespace Pivot {
	using CmdLineArgs = std::span<char *>;

	struct AppCreateOptions {
		WindowCreateOptions WindowCreateOptions;
		GuiStyleOptions     GuiStyleOptions;
	};

	class App final {
	private:
		friend int ::main(int, char **);

		template <typename TKernel, typename ...Args>
			requires std::is_base_of_v<Kernel, TKernel>
		friend std::unique_ptr<App> CreateApp(AppCreateOptions const &, Args &&...);

		explicit App(AppCreateOptions const &options);

	public:
		~App();

		Window *GetWindow() { return m_Window.get(); }
		float   GetFps()    const { return m_Fps; }

		static App *Get() { return s_This; }

	private:
		void  SetHandlers();
		void  Run();
		float GetDeltaTime();
		void  Tick(float deltaTime);
	
	private:
		std::unique_ptr<Window> m_Window;
		std::unique_ptr<Kernel> m_Kernel;

		std::chrono::steady_clock::time_point m_LastFrameTime;
		std::chrono::steady_clock::time_point m_LastCheckTime;
		std::uint32_t                         m_FramesCnt     = 0;
		float                                 m_Fps           = 0;

		static inline App *s_This = nullptr;
	};

	template <typename TKernel, typename ...Args>
		requires std::is_base_of_v<Kernel, TKernel>
	std::unique_ptr<App> CreateApp(AppCreateOptions const &options, Args &&...args) {
		auto app = std::unique_ptr<App>(new App(options));
		app->m_Kernel = std::make_unique<TKernel>(std::forward<Args>(args)...);
		return app;
	}

	std::unique_ptr<App> CreateApp(CmdLineArgs args);
}
