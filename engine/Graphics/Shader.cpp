#include "Shader.h"

#include <glad/glad.h>

namespace Pivot {
	static void CheckGlProgram(GLuint const program) {
		GLint success;
		glGetProgramiv(program, GL_LINK_STATUS, &success);
		if (success) {
			spdlog::trace("Succeeded to link Shader {}", program);
		} else {
			std::array<GLchar, 1024> buf;
			glGetProgramInfoLog(program, buf.size(), nullptr, buf.data());
			spdlog::error("Failed to link Shader {}!\n{}", program, buf.data());
		}
	}
	
	Shader::Shader(std::initializer_list<ShaderModule const *> shaderModules) {
		m_Id = glCreateProgram();
		for (auto const module : shaderModules) {
			glAttachShader(m_Id, module->m_Id);
		}
		glLinkProgram(m_Id);
		CheckGlProgram(m_Id);
	}

	Shader::~Shader() {
		glDeleteProgram(m_Id);
	}

	void Shader::Bind() const {
		glUseProgram(m_Id);
	}

	void Shader::Unbind() const {
		glUseProgram(0);
	}

	void Shader::BindUniformBlock(std::string_view name, std::uint32_t bindingPoint) {
		auto index = glGetUniformBlockIndex(m_Id, name.data());
		if (index == GL_INVALID_INDEX) {
			spdlog::error("Failed to get uniform block index of {}", name);
			return;
		}
		glUniformBlockBinding(m_Id, index, bindingPoint);
	}

	void Shader::SetUniform(std::string_view name, float         val) { Bind(); glUniform1fv(GetLocationByName(name), 1, &val); Unbind(); }
	void Shader::SetUniform(std::string_view name, std::int32_t  val) { Bind(); glUniform1iv(GetLocationByName(name), 1, &val); Unbind(); }
	void Shader::SetUniform(std::string_view name, std::uint32_t val) { Bind(); glUniform1uiv(GetLocationByName(name), 1, &val); Unbind(); }

	void Shader::SetUniform(std::string_view name, glm::vec2 const &val) { Bind(); glUniform2fv(GetLocationByName(name), 1, glm::value_ptr(val)); Unbind(); }
	void Shader::SetUniform(std::string_view name, glm::vec3 const &val) { Bind(); glUniform3fv(GetLocationByName(name), 1, glm::value_ptr(val)); Unbind(); }
	void Shader::SetUniform(std::string_view name, glm::vec4 const &val) { Bind(); glUniform4fv(GetLocationByName(name), 1, glm::value_ptr(val)); Unbind(); }

	void Shader::SetUniform(std::string_view name, glm::mat2 const &val) { Bind(); glUniformMatrix2fv(GetLocationByName(name), 1, GL_FALSE, glm::value_ptr(val)); Unbind(); }
	void Shader::SetUniform(std::string_view name, glm::mat3 const &val) { Bind(); glUniformMatrix3fv(GetLocationByName(name), 1, GL_FALSE, glm::value_ptr(val)); Unbind(); }
	void Shader::SetUniform(std::string_view name, glm::mat4 const &val) { Bind(); glUniformMatrix4fv(GetLocationByName(name), 1, GL_FALSE, glm::value_ptr(val)); Unbind(); }

	void Shader::SetUniform(std::string_view name, std::span<float const>         val) { Bind(); glUniform1fv(GetLocationByName(name), val.size(), val.data()); Unbind(); }
	void Shader::SetUniform(std::string_view name, std::span<std::int32_t const>  val) { Bind(); glUniform1iv(GetLocationByName(name), val.size(), val.data()); Unbind(); }
	void Shader::SetUniform(std::string_view name, std::span<std::uint32_t const> val) { Bind(); glUniform1uiv(GetLocationByName(name), val.size(), val.data()); Unbind(); }

	void Shader::SetUniform(std::string_view name, std::span<glm::vec2 const> val) { Bind(); glUniform2fv(GetLocationByName(name), val.size(), glm::value_ptr(val[0])); Unbind(); }
	void Shader::SetUniform(std::string_view name, std::span<glm::vec3 const> val) { Bind(); glUniform3fv(GetLocationByName(name), val.size(), glm::value_ptr(val[0])); Unbind(); }
	void Shader::SetUniform(std::string_view name, std::span<glm::vec4 const> val) { Bind(); glUniform4fv(GetLocationByName(name), val.size(), glm::value_ptr(val[0])); Unbind(); }

	void Shader::SetUniform(std::string_view name, std::span<glm::mat2 const> val) { Bind(); glUniformMatrix2fv(GetLocationByName(name), val.size(), GL_FALSE, glm::value_ptr(val[0])); Unbind(); }
	void Shader::SetUniform(std::string_view name, std::span<glm::mat3 const> val) { Bind(); glUniformMatrix3fv(GetLocationByName(name), val.size(), GL_FALSE, glm::value_ptr(val[0])); Unbind(); }
	void Shader::SetUniform(std::string_view name, std::span<glm::mat4 const> val) { Bind(); glUniformMatrix4fv(GetLocationByName(name), val.size(), GL_FALSE, glm::value_ptr(val[0])); Unbind(); }

	std::int32_t Shader::GetLocationByName(std::string_view name) {
		if (auto iter = m_Locations.find(name.data()); iter != m_Locations.end()) {
			return iter->second;
		} else {
			auto loc = glGetUniformLocation(m_Id, name.data());
			if (loc < 0) {
				spdlog::error("Failed to get uniform location of {}", name);
			};
			m_Locations[name.data()] = loc;
			return loc;
		}
	}
}
