#pragma once

#include <glm/ext.hpp>

#include <spdlog/spdlog.h>

// Fix including <Window.h>
#undef CreateWindow

#include <array>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <thread>
#include <type_traits>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#define DECLARE_CREATOR(type)                                                \
	template <typename ...Args>                                              \
	static std::unique_ptr<type> Create##type(Args &&...args) {              \
		return std::unique_ptr<type>(new type(std::forward<Args>(args)...)); \
	}
