#pragma once

#include "Graphics/VertexLayout.h"

namespace Pivot {
	class VertexBuffer {
	private:
		friend class Renderer;
		friend class VertexArray;

		explicit VertexBuffer(VertexAttribBlock const &attribBlock);
	
	public:
		~VertexBuffer();

		void Upload(std::span<std::byte const> buf, DrawFrequency frequency = DrawFrequency::Static);

	private:
		void Bind() const;
		void Unbind() const;

	private:
		std::uint32_t m_Id          = 0;
		std::uint32_t m_VertexCount = 0;

		VertexAttribBlock const &m_AttribBlock;
	};

	class IndexBuffer {
	private:
		friend class Renderer;
		friend class IndexedVertexArray;

		IndexBuffer();
	
	public:
		~IndexBuffer();

		void Upload(std::span<std::uint32_t const> buf, DrawFrequency frequency = DrawFrequency::Static);

	private:
		void Bind() const;
		void Unbind() const;
	
	private:
		std::uint32_t m_Id         = 0;
		std::uint32_t m_IndexCount = 0;
	};

	class VertexArray {
	private:
		friend class Renderer;

	protected:
		explicit VertexArray(VertexLayout const &layout, PrimitiveType primitiveType = PrimitiveType::Triangles);

	public:
		virtual ~VertexArray();

		PrimitiveType  GetPrimitiveType() const { return m_PrimitiveType; }
		VertexBuffer  *GetBufferByName(std::string_view name) const;
	
	protected:
		void Bind() const;
		void Unbind() const;

		DECLARE_CREATOR(VertexBuffer)

	private:
		std::uint32_t m_Id = 0;

		VertexLayout  m_Layout;
		PrimitiveType m_PrimitiveType;

		std::vector<std::unique_ptr<VertexBuffer>> m_VertexBuffers;
	};

	class IndexedVertexArray : public VertexArray {
	private:
		friend class Renderer;

		explicit IndexedVertexArray(VertexLayout const &layout, PrimitiveType primitiveType = PrimitiveType::Triangles);
	
	public:
		~IndexedVertexArray() = default;

		IndexBuffer *GetIndexBuffer() const;
	
	private:
		DECLARE_CREATOR(IndexBuffer)
	
	private:
		std::unique_ptr<IndexBuffer> m_IndexBuffer;
	};
}
