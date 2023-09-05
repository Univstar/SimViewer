#pragma once

#include "Graphics/Enums.h"

namespace Pivot {
	class ShaderModule {
	private:
		friend class Renderer;
		friend class Shader;

		explicit ShaderModule(std::filesystem::path const &filename);
		ShaderModule(ShaderStage stage, std::filesystem::path  const &filename);
		ShaderModule(ShaderStage stage, std::vector<std::byte> const &filename);

	public:
		~ShaderModule();
	
	private:
		std::uint32_t m_Id = 0;
	};
}
