#include "Input.h"

#include "Core/App.h"

#include <GLFW/glfw3.h>

namespace Pivot {
	bool Input::IsKeyDown(Key key) {
		auto window = App::Get()->GetWindow()->GetNative();
		return glfwGetKey(window, static_cast<int>(key)) == GLFW_PRESS;
	}

	bool Input::IsMouseButtonDown(MouseButton button) {
		auto window = App::Get()->GetWindow()->GetNative();
		return glfwGetMouseButton(window, static_cast<int>(button)) == GLFW_PRESS;
	}

	glm::vec2 Input::GetMousePos() {
		auto window = App::Get()->GetWindow()->GetNative();
		double xPos, yPos;
		glfwGetCursorPos(window, &xPos, &yPos);
		return { static_cast<float>(xPos), static_cast<float>(yPos) };
	}
}
