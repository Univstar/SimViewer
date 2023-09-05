#include "VertexLayout.h"

namespace Pivot {
	std::size_t VertexLayout::GetIndexByName(std::string_view name) const {
		if (auto iter = m_Indices.find(name.data()); iter != m_Indices.end()) {
			return iter->second;
		} else {
			spdlog::error("Failed to get the buffer index of \"{}\"", name);
			return std::numeric_limits<std::size_t>::max();
		}
	}
}
