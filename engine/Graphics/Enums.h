#pragma once

#include "Utils/Common.h"

namespace Pivot {
	template <typename T> inline constexpr std::uint16_t TypeEnumOf = 0;
	template <> inline constexpr std::uint16_t TypeEnumOf<char>           = 0x1400;
	template <> inline constexpr std::uint16_t TypeEnumOf<unsigned char>  = 0x1401;
	template <> inline constexpr std::uint16_t TypeEnumOf<short>          = 0x1402;
	template <> inline constexpr std::uint16_t TypeEnumOf<unsigned short> = 0x1403;
	template <> inline constexpr std::uint16_t TypeEnumOf<int>            = 0x1404;
	template <> inline constexpr std::uint16_t TypeEnumOf<unsigned int>   = 0x1405;
	template <> inline constexpr std::uint16_t TypeEnumOf<float>          = 0x1406;

	enum class ShaderStage : std::uint16_t {
		None           = 0x0000,
		Vertex         = 0x8B31,
		TessControl    = 0x8E88,
		TessEvaluation = 0x8E87,
		Geometry       = 0x8DD9,
		Fragment       = 0x8B30,
	};
	
	enum class DrawFrequency : std::uint16_t {
		Stream  = 0x88E0,
		Static  = 0x88E4,
		Dynamic = 0x88E8,
	};

	enum class PrimitiveType : std::uint16_t {
		Points                 = 0x0000,
		Lines                  = 0x0001,
		LineLoop               = 0x0002,
		LineStrip              = 0x0003,
		Triangles              = 0x0004,
		TriangleStrip          = 0x0005,
		TriangleFan            = 0x0006,
		LinesAdjacency         = 0x000A,
		LineStripAdjacency     = 0x000B,
		TrianglesAdjacency     = 0x000C,
		TriangleStripAdjacency = 0x000D,
	};
}
