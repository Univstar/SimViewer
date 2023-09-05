#pragma once

#include "Utils/Common.h"

namespace Pivot {
	class IO {
	public:
		static std::vector<std::byte> ReadFile(const std::filesystem::path &filename);

		template <typename T>
		static void Read(std::istream &in, T &val) {
			if constexpr (requires { std::span(val); }) {
				auto buf = std::span(val);
				in.read(reinterpret_cast<char *>(buf.data()), buf.size_bytes());
			} else {
				in.read(reinterpret_cast<char *>(&val), sizeof(val));
			}
		}

		template <typename T>
		static void Write(std::ostream &out, T const &val) {
			if constexpr (requires { std::span(val); }) {
				auto buf = std::span(val);
				out.write(reinterpret_cast<char const *>(buf.data()), buf.size_bytes());
			} else {
				out.write(reinterpret_cast<char const *>(&val), sizeof(val));
			}
		}
	};
}
