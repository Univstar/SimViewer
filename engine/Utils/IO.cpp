#include "IO.h"

namespace Pivot {
	std::vector<std::byte> IO::ReadFile(const std::filesystem::path &filename) {
		std::ifstream fin(filename, std::ios::ate | std::ios::binary);
		if (!fin.is_open()) {
			spdlog::error("Failed to open {}", filename.string());
			return { };
		}
		std::size_t fileSize = fin.tellg();
		auto buffer = std::vector<std::byte>(fileSize);
		fin.seekg(0);
		fin.read(reinterpret_cast<char *>(buffer.data()), fileSize);
		fin.close();
		return buffer;
	}
}
