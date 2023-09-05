#pragma once

#include "Utils/Common.h"
#include "Graphics/Enums.h"

#include <glm/glm.hpp>

namespace Pivot {
	template <typename T> struct glm_unpack;

	template <typename T>
		requires std::is_arithmetic_v<T>
	struct glm_unpack<T> {
		using type = T;
		using size = std::integral_constant<std::size_t, 1>;
	};

	template <glm::length_t N, typename T, glm::qualifier Q>
		requires std::is_arithmetic_v<T>
	struct glm_unpack<glm::vec<N, T, Q>> {
		using type = T;
		using size = std::integral_constant<std::size_t, N>;
	};

	template <glm::length_t N, glm::length_t M, typename T, glm::qualifier Q>
		requires std::is_arithmetic_v<T>
	struct glm_unpack<glm::mat<N, M, T, Q>> {
		using type = T;
		using size = std::integral_constant<std::size_t, N * M>;
	};

	template <typename T> using glm_type_of = typename glm_unpack<T>::type;

	template <typename T> using glm_size_of = typename glm_unpack<T>::size;

	template <typename T> inline constexpr std::size_t glm_size_of_v = glm_size_of<T>::value;

	struct VertexAttrib {
		std::uint32_t Location;
		std::uint32_t Offset;
		std::uint32_t Type;
		std::uint32_t Size;
		bool          Normalized;
	};

	struct VertexAttribBlock {
		std::size_t               Stride;
		std::vector<VertexAttrib> Attributes;
	};

	class VertexLayout {
	public:
		std::vector<VertexAttribBlock> AttribBlocks;

		template <typename T>
		VertexLayout Add(std::string_view name, std::uint32_t location, bool normalized = false) && {
			m_Indices[name.data()] = AttribBlocks.size();
			AttribBlocks.push_back({
				.Stride     = sizeof(T),
			});
			AttribBlocks.back().Attributes.push_back({
				.Location   = location,
				.Offset     = 0,
				.Type       = TypeEnumOf<glm_type_of<T>>,
				.Size       = glm_size_of_v<T>,
				.Normalized = normalized,
			});
			return std::move(*this);
		}

		template <typename T>
		VertexLayout Add(std::string_view name) && {
			m_Indices[name.data()] = AttribBlocks.size();
			AttribBlocks.push_back({
				.Stride    = sizeof(T),
			});
			return std::move(*this);
		}

		template <typename T, typename TField>
		VertexLayout At(std::uint32_t location, TField T::* field, bool normalized = false) && {
			AttribBlocks.back().Attributes.push_back({
				.Location   = location,
				.Offset     = static_cast<std::uint32_t>(std::intptr_t(&((*(T *) 0).*field))),
				.Type       = TypeEnumOf<glm_type_of<TField>>,
				.Size       = glm_size_of_v<TField>,
				.Normalized = normalized,
			});
			return std::move(*this);
		}

		std::size_t GetIndexByName(std::string_view name) const;
	
	private:
		std::unordered_map<std::string, std::size_t> m_Indices;
	};
}
