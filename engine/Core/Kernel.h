#pragma once

#include "Core/Codes.h"

namespace Pivot {
	class Kernel {
	public:
		/**
		 *  @brief Make draw calls, ImGui calls and other per-frame updates.
		 */
		virtual void Tick(float deltaTime) = 0;

		virtual void RenderGui() { }

		virtual void OnKeyDown(Key key, ModifierFlags mods) { }
		virtual void OnKeyUp(Key key, ModifierFlags mods) { }
		virtual void OnKeyRepeat(Key key, ModifierFlags mods) { }
		virtual void OnMouseDown(MouseButton button, ModifierFlags mods) { }
		virtual void OnMouseUp(MouseButton button, ModifierFlags mods) { }
		virtual void OnMouseWheel(float xOffset, float yOffset) { }
		virtual void OnMouseMove(float xPos, float yPos) { }
	};
}
