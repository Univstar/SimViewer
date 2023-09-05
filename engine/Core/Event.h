#pragma once

#include "Core/Codes.h"

namespace Pivot {
	class Window;
}

namespace Pivot::Event {
	struct WindowClose        { using Handler = void(*)(Window *); };
	struct WindowSize         { using Handler = void(*)(Window *, std::uint32_t, std::uint32_t); };
	struct FramebufferSize    { using Handler = void(*)(Window *, std::uint32_t, std::uint32_t); };
	struct WindowContentScale { using Handler = void(*)(Window *, float); };
	struct WindowMinimize     { using Handler = void(*)(Window *, bool); };
	struct WindowMaximize     { using Handler = void(*)(Window *, bool); };
	struct WindowFocus        { using Handler = void(*)(Window *, bool); };
	struct WindowHover        { using Handler = void(*)(Window *, bool); };
	struct WindowRefresh      { using Handler = void(*)(Window *); };

	struct Key                { using Handler = void(*)(Window *, Pivot::Key, ButtonAction, ModifierFlags); };
	struct MouseMove          { using Handler = void(*)(Window *, float, float); };
	struct MouseButton        { using Handler = void(*)(Window *, Pivot::MouseButton, ButtonAction, ModifierFlags); };
	struct MouseWheel         { using Handler = void(*)(Window *, float, float); };
	struct PathsDrop          { using Handler = void(*)(Window *, std::span<char const *>); };
}
