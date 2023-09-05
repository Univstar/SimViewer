#pragma once

#include "Utils/Common.h"

namespace Pivot {
	class Coordinates {
	public:
		template <typename T>
			requires std::is_floating_point_v<T>
		static glm::vec3 SphericalToCartesian(T rad, T theta, T phi) {
			return {
				rad * std::sin(theta) * std::cos(phi),
				rad * std::cos(theta),
				-rad * std::sin(theta) * std::sin(phi)
			};
		}
	};
}
