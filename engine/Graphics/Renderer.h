#pragma once

#include "Graphics/Shader.h"
#include "Graphics/UniformBuffer.h"
#include "Graphics/VertexArray.h"

namespace Pivot {
	using BufferFlags = std::uint16_t;

	struct BufferFlagBits {
		enum : BufferFlags {
			Depth   = 1 << 8,
			Stencil = 1 << 10,
			Color   = 1 << 14,
		};
	};

	class Renderer {
	public:
		static char const *GetDeviceName();
		static char const *GetApiName();

		static void SetViewport(glm::ivec2 const &offset, glm::uvec2 const &extent);
		static void SetDepthTested(bool enabled = true);
		static void SetWireframed(bool enabled = true);
		static void SetBackFaceCulled(bool enabled = true);
		static void SetMultisampled(bool enabled = true);
		static void SetClearColor(glm::vec4 const &color);
		static void Clear(BufferFlags bufferFlags);

		static void Draw(VertexArray *vertexArray, std::int32_t first = 0, std::uint32_t count = 0, std::uint32_t instanceCount = 1);
		static void Draw(IndexedVertexArray *vertexArray, std::uint32_t count = 0, std::uint32_t instanceCount = 1, std::int32_t baseVertex = 0);

		static std::vector<std::byte> ReadPixels(glm::ivec2 const &offset, glm::uvec2 const &extent, std::uint32_t numChannels = 4);

		static std::unique_ptr<Shader> CreateShader(std::initializer_list<ShaderModule const *> shaderModules) {
			return std::unique_ptr<Shader>(new Shader(shaderModules));
		}

		DECLARE_CREATOR(ShaderModule)
		DECLARE_CREATOR(VertexArray)
		DECLARE_CREATOR(IndexedVertexArray)
		DECLARE_CREATOR(UniformBuffer)
	};
}
