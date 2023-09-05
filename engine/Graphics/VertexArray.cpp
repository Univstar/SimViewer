#include "VertexArray.h"

#include <glad/glad.h>

namespace Pivot {
	VertexBuffer::VertexBuffer(VertexAttribBlock const &attribBlock) : m_AttribBlock { attribBlock } {
		glGenBuffers(1, &m_Id);
	}

	VertexBuffer::~VertexBuffer() {
		glDeleteBuffers(1, &m_Id);
	}

	void VertexBuffer::Upload(std::span<std::byte const> buf, DrawFrequency frequency) {
		Bind();
		glBufferData(GL_ARRAY_BUFFER, buf.size(), buf.data(), static_cast<GLenum>(frequency));
		m_VertexCount = static_cast<std::uint32_t>(buf.size() / m_AttribBlock.Stride);
		Unbind();
	}

	void VertexBuffer::Bind() const {
		glBindBuffer(GL_ARRAY_BUFFER, m_Id);
	}

	void VertexBuffer::Unbind() const {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	IndexBuffer::IndexBuffer() {
		glGenBuffers(1, &m_Id);
	}

	IndexBuffer::~IndexBuffer() {
		glDeleteBuffers(1, &m_Id);
	}

	void IndexBuffer::Upload(std::span<std::uint32_t const> buf, DrawFrequency frequency) {
		Bind();
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, buf.size_bytes(), buf.data(), static_cast<GLenum>(frequency));
		m_IndexCount = static_cast<std::uint32_t>(buf.size());
		Unbind();
	}

	void IndexBuffer::Bind() const {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Id);
	}

	void IndexBuffer::Unbind() const {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	VertexArray::VertexArray(VertexLayout const &layout, PrimitiveType primitiveType) : m_Layout { layout }, m_PrimitiveType { primitiveType } {
		glGenVertexArrays(1, &m_Id);
		Bind();
		for (auto const &attrBlock : m_Layout.AttribBlocks) {
			m_VertexBuffers.push_back(CreateVertexBuffer(attrBlock));
			m_VertexBuffers.back()->Bind();
			for (auto const &attr : attrBlock.Attributes) {
				glVertexAttribPointer(attr.Location, attr.Size, attr.Type, attr.Normalized, attrBlock.Stride, reinterpret_cast<void *>(static_cast<std::uintptr_t>(attr.Offset)));
				glEnableVertexAttribArray(attr.Location);
			}
			m_VertexBuffers.back()->Unbind();
		}
		Unbind();
	}

	VertexArray::~VertexArray() {
		for (auto &vertexBuffer : m_VertexBuffers) {
			vertexBuffer.reset();
		}
		glDeleteVertexArrays(1, &m_Id);
	}

	VertexBuffer *VertexArray::GetBufferByName(std::string_view name) const {
		return m_VertexBuffers[m_Layout.GetIndexByName(name)].get();
	}

	void VertexArray::Bind() const {
		glBindVertexArray(m_Id);
	}

	void VertexArray::Unbind() const {
		glBindVertexArray(0);
	}
	
	IndexedVertexArray::IndexedVertexArray(VertexLayout const &layout, PrimitiveType primitiveType) :
		VertexArray(layout, primitiveType),
		m_IndexBuffer { CreateIndexBuffer() } {
		Bind();
		m_IndexBuffer->Bind();
		Unbind();
		m_IndexBuffer->Unbind();
	}

	IndexBuffer *IndexedVertexArray::GetIndexBuffer() const {
		return m_IndexBuffer.get();
	}
}
