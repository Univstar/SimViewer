#include "UniformBuffer.h"

#include <glad/glad.h>

namespace Pivot {
	UniformBuffer::UniformBuffer(std::uint32_t bindingPoint) {
		glGenBuffers(1, &m_Id);
		Bind();
		glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, m_Id);
		Unbind();
	}

	UniformBuffer::~UniformBuffer() {
		glDeleteBuffers(1, &m_Id);
	}

	void UniformBuffer::Reserve(std::size_t size, DrawFrequency frequency) {
		Bind();
		glBufferData(GL_UNIFORM_BUFFER, size, nullptr, static_cast<GLenum>(frequency));
		Unbind();
	}

	void UniformBuffer::Update(std::span<std::byte const> buf, std::intptr_t offset) {
		Bind();
		glBufferSubData(GL_UNIFORM_BUFFER, offset, buf.size(), buf.data());
		Unbind();
	}

	void UniformBuffer::Upload(std::span<std::byte const> buf, DrawFrequency frequency) {
		Bind();
		glBufferData(GL_UNIFORM_BUFFER, buf.size(), buf.data(), static_cast<GLenum>(frequency));
		Unbind();
	}

	void UniformBuffer::Bind() {
		glBindBuffer(GL_UNIFORM_BUFFER, m_Id);
	}

	void UniformBuffer::Unbind() {
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
}
