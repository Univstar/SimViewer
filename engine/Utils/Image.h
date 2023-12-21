#pragma once

#include <filesystem>
#include <vector>

#include <cstddef>

namespace Pivot {
	class Image {
	public:
		static void WriteBytes(std::filesystem::path const &filename, std::vector<std::byte> const &bytes, int width, int height, int numChannels, bool flipped);
	};
}
