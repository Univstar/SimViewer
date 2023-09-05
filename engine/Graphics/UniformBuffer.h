#pragma once

#include "Graphics/Enums.h"

namespace Pivot {
	class UniformBuffer {
	private:
		friend class Renderer;

		UniformBuffer(std::uint32_t bindingPoint);
	
	public:
		~UniformBuffer();

		void Reserve(std::size_t size, DrawFrequency frequency = DrawFrequency::Stream);
		void Update(std::span<std::byte const> buf, std::intptr_t offset);
		void Upload(std::span<std::byte const> buf, DrawFrequency frequency = DrawFrequency::Static);

		template <typename T>
		void Upload(T const &block, DrawFrequency frequency = DrawFrequency::Static) {
			Upload(std::span(reinterpret_cast<std::byte const *>(&block), sizeof(block)), frequency);
		}

	private:
		void Bind();
		void Unbind();
	
	private:
		std::uint32_t m_Id   = 0;
	};
}
