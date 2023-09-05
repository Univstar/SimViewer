#include "ShaderModule.h"

#include "Utils/IO.h"

#include <glad/glad.h>

namespace Pivot {
	static ShaderStage GetShaderStage(std::filesystem::path const & filename) {
		auto ext = filename.extension();
		if (ext == ".vert") return ShaderStage::Vertex;
		else if (ext == ".tesc") return ShaderStage::TessControl;
		else if (ext == ".tese") return ShaderStage::TessEvaluation;
		else if (ext == ".geom") return ShaderStage::Geometry;
		else if (ext == ".frag") return ShaderStage::Fragment;
		else {
			spdlog::error("Failed to determine shader stage of {}", filename.string());
			return ShaderStage::None;
		}
	}

	static void CheckGlShader(GLuint const shader) {
		GLint success;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (success) {
			spdlog::trace("Succeeded to compile ShaderModule {}", shader);
		} else {
			std::array<GLchar, 1024> buf;
			glGetShaderInfoLog(shader, buf.size(), nullptr, buf.data());
			spdlog::error("Failed to compile ShaderModule {}\n{}", shader, buf.data());
		}
	}

	ShaderModule::ShaderModule(std::filesystem::path const & filename) :
		ShaderModule(GetShaderStage(filename), filename) {
	}

	ShaderModule::ShaderModule(ShaderStage stage, std::filesystem::path const & filename) :
		ShaderModule(stage, IO::ReadFile(filename)) {
	}

	ShaderModule::ShaderModule(ShaderStage stage, std::vector<std::byte> const & blob) {
		if (stage == ShaderStage::None) return;
		m_Id = glCreateShader(static_cast<GLenum>(stage));
		auto source = reinterpret_cast<GLchar const *>(blob.data());
		auto length = static_cast<GLint>(blob.size());
		glShaderSource(m_Id, 1, &source, &length);
		glCompileShader(m_Id);
		CheckGlShader(m_Id);
	}

	ShaderModule::~ShaderModule() {
		glDeleteShader(m_Id);
	}
}
