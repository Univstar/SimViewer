#pragma once

#include "Graphics/ShaderModule.h"

namespace Pivot {
	class Shader {
	private:
		friend class Renderer;

		explicit Shader(std::initializer_list<ShaderModule const *> shaderModules);

	public:
		~Shader();

		void Bind() const;
		void Unbind() const;

		void BindUniformBlock(std::string_view name, std::uint32_t bindingPoint);

		void SetUniform(std::string_view name, float val);
		void SetUniform(std::string_view name, std::int32_t val);
		void SetUniform(std::string_view name, std::uint32_t val);

		void SetUniform(std::string_view name, glm::vec2 const &val);
		void SetUniform(std::string_view name, glm::vec3 const &val);
		void SetUniform(std::string_view name, glm::vec4 const &val);
		
		void SetUniform(std::string_view name, glm::mat2 const &val);
		void SetUniform(std::string_view name, glm::mat3 const &val);
		void SetUniform(std::string_view name, glm::mat4 const &val);

		void SetUniform(std::string_view name, std::span<float const> val);
		void SetUniform(std::string_view name, std::span<std::int32_t const> val);
		void SetUniform(std::string_view name, std::span<std::uint32_t const> val);

		void SetUniform(std::string_view name, std::span<glm::vec2 const> val);
		void SetUniform(std::string_view name, std::span<glm::vec3 const> val);
		void SetUniform(std::string_view name, std::span<glm::vec4 const> val);

		void SetUniform(std::string_view name, std::span<glm::mat2 const> val);
		void SetUniform(std::string_view name, std::span<glm::mat3 const> val);
		void SetUniform(std::string_view name, std::span<glm::mat4 const> val);
	
	private:
		std::int32_t GetLocationByName(std::string_view name);

	private:
		std::uint32_t m_Id = 0;

		std::unordered_map<std::string, std::int32_t> m_Locations;
	};
}
