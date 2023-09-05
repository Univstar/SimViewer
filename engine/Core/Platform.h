#pragma once

#include "Core/Window.h"
#include "Utils/Common.h"

namespace Pivot {
	class Platform {
	public:
		static void Init();
		static void Terminate();

		static void MakeContextCurrent(Window *window);
		static void PollEvents();
		static void SwapBuffers(Window *window);
		static void SetVSynchronized(bool enabled);

		DECLARE_CREATOR(Window)
	};
}
