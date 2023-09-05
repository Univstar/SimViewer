#pragma once

#include "Core/Codes.h"

namespace Pivot {
	class Input {
	public:
		static bool      IsKeyDown(Key key);
		static bool      IsMouseButtonDown(MouseButton button);
		static glm::vec2 GetMousePos();
	};
}
