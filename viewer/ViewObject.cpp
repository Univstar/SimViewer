#include "ViewObject.h"

#include "Graphics/Renderer.h"
#include "Utils/IO.h"

namespace YAML {
	template <>
	struct convert<Pivot::PrimitiveType> {
		static bool decode(Node const &node, Pivot::PrimitiveType &rhs) {
			auto const str = node.as<std::string>();
			// TODO
			if (str == "Points") rhs = Pivot::PrimitiveType::Points;
			else if (str == "Lines") rhs = Pivot::PrimitiveType::Lines;
			else if (str == "Triangles") rhs = Pivot::PrimitiveType::Triangles;
			else return false;
			return true;
		}
	};

	template <>
	struct convert<Pivot::BlendMode> {
		static bool decode(Node const &node, Pivot::BlendMode &rhs) {
			auto const str = node.as<std::string>();
			if (str == "Opaque") rhs = Pivot::BlendMode::Opaque;
			else if (str == "Cutout") rhs = Pivot::BlendMode::Cutout;
			else if (str == "Transparent") rhs = Pivot::BlendMode::Transparent;
			else if (str == "Fade") rhs = Pivot::BlendMode::Fade;
			else return false;
			return true;
		}
	};

	template<glm::length_t N, typename T, glm::qualifier Q>
		requires std::is_arithmetic_v<T>
	struct convert<glm::vec<N, T, Q>> {
		static bool decode(Node const &node, glm::vec<N, T, Q> &rhs) {
			if (!node.IsSequence() || node.size() > N)
				return false;
			rhs = glm::vec<N, T, Q>(1);
			for (std::size_t i = 0; i < node.size(); i++)
				rhs[i] = node[i].as<T>();
			return true;
		}
	};
}

namespace Pivot {
	ViewObject::ViewObject(std::size_t nodeId, YAML::Node const &node, std::uint32_t dimension, ShaderPool const &shaderPool) {
		m_VecSizeBytes = dimension * sizeof(float);
		if (node["Name"]) {
			m_Name = node["Name"].as<std::string>();
			m_DisplayName = std::to_string(nodeId + 1) + ": " + m_Name;
		} else {
			spdlog::error("Failed to find the name in an object description");
		}

		constexpr auto SetValue = [] <typename T>(T &val, YAML::Node const & node) { if (node) val = node.as<T>(); };

		// Determine shader
		std::string shaderName = "default";
		SetValue(shaderName, node["Shader"]);
		shaderName += fmt::format("_{}d", dimension);
		if (auto iter = shaderPool.find(shaderName); iter != shaderPool.end()) {
			m_Shader = iter->second.get();
		} else {
			spdlog::error("Failed to find Shader \"{}\"", shaderName);
		}

		PrimitiveType primitive = PrimitiveType::Triangles;
		SetValue(primitive, node["Primitive"]);

		SetValue(m_Indexed,   node["Indexed"]);
		SetValue(m_Animated,  node["Animated"]);
		SetValue(m_TopoFixed, node["TopoFixed"]);
		
		if (node["Material"]) {
			SetValue(m_Material.Mode,      node["Material"]["Mode"]);
			SetValue(m_Material.Albedo,    node["Material"]["Albedo"]);
			SetValue(m_Material.Metallic,  node["Material"]["Metallic"]);
			SetValue(m_Material.Roughness, node["Material"]["Roughness"]);
			SetValue(m_Material.Visible,   node["Material"]["Visible"]);
		}

		m_AttribFlags = AttribFlagBits::Position;
		if (dimension == 3) m_AttribFlags |= AttribFlagBits::Normal;

		VertexLayout layout;
		if (dimension == 2) {
			if (m_AttribFlags & AttribFlagBits::Position)
				layout = std::move(layout).Add<glm::vec2>("Positions", 0);
			if (m_AttribFlags & AttribFlagBits::TexCoord)
				layout = std::move(layout).Add<glm::vec2>("TexCoords", 1);
		} else {
			if (m_AttribFlags & AttribFlagBits::Position)
				layout = std::move(layout).Add<glm::vec3>("Positions", 0);
			if (m_AttribFlags & AttribFlagBits::Normal)
				layout = std::move(layout).Add<glm::vec3>("Normals", 1);
			if (m_AttribFlags & AttribFlagBits::TexCoord)
				layout = std::move(layout).Add<glm::vec2>("TexCoords", 2);
		}

		if (m_Indexed) {
			m_VertexArray = Renderer::CreateIndexedVertexArray(layout, primitive);
		} else {
			m_VertexArray = Renderer::CreateVertexArray(layout, primitive);
		}
	}

	void ViewObject::ReserveFrames(std::uint32_t frameCount) {
		m_FramesData.reserve(m_Animated ? frameCount : 1);
	}

	bool ViewObject::LoadNewFrameData(std::filesystem::path const &dirname, bool initial) {
		if (!m_Animated && !initial) return true;
		std::ifstream fin(dirname / (m_Name + ".out"), std::ios::binary);
		if (!fin) {
			spdlog::error("Failed to open \"{}\"", (dirname / (m_Name + ".out")).string());
			return false;
		}
		FrameData frameData;

		std::uint32_t vtxCount;
		IO::Read(fin, vtxCount);
		if (m_AttribFlags & AttribFlagBits::Position) {
			frameData.Positions.resize(vtxCount * m_VecSizeBytes);
			IO::Read(fin, frameData.Positions);
		}
		if (m_AttribFlags & AttribFlagBits::Normal) {
			frameData.Normals.resize(vtxCount * m_VecSizeBytes);
			IO::Read(fin, frameData.Normals);
		}
		if ((!m_TopoFixed || initial) && (m_AttribFlags & AttribFlagBits::TexCoord)) {
			frameData.TexCoords.resize(vtxCount * sizeof(float) * 2);
			IO::Read(fin, frameData.TexCoords);
		}
		if ((!m_TopoFixed || initial) && m_Indexed) {
			std::uint32_t idxCount;
			IO::Read(fin, idxCount);
			frameData.Indices.resize(idxCount);
			IO::Read(fin, frameData.Indices);
		}

		m_FramesData.push_back(std::move(frameData));
		return true;
	}

	void ViewObject::UploadBuffers(std::uint32_t frame, bool initial) {
		if (initial) {
			auto const topoFrame = m_Animated && !m_TopoFixed ? frame : 0;
			auto const posFrame  = m_Animated ? frame : 0;
			m_CurrentFrame = posFrame;
			if (m_AttribFlags & AttribFlagBits::Position)
				m_VertexArray->GetBufferByName("Positions")->Upload(m_FramesData[posFrame].Positions);
			if (m_AttribFlags & AttribFlagBits::Normal)
				m_VertexArray->GetBufferByName("Normals")->Upload(m_FramesData[posFrame].Normals);
			if (m_AttribFlags & AttribFlagBits::TexCoord)
				m_VertexArray->GetBufferByName("TexCoords")->Upload(m_FramesData[topoFrame].TexCoords);
			if (m_Indexed) {
				auto indexedVA = reinterpret_cast<IndexedVertexArray *>(m_VertexArray.get());
				indexedVA->GetIndexBuffer()->Upload(m_FramesData[topoFrame].Indices);
			}
		} else if (m_Animated && frame != m_CurrentFrame) {
			m_CurrentFrame = frame;
			if (m_AttribFlags & AttribFlagBits::Position)
				m_VertexArray->GetBufferByName("Positions")->Upload(m_FramesData[frame].Positions);
			if (m_AttribFlags & AttribFlagBits::Normal)
				m_VertexArray->GetBufferByName("Normals")->Upload(m_FramesData[frame].Normals);
			if (!m_TopoFixed) {
				if (m_AttribFlags & AttribFlagBits::TexCoord)
					m_VertexArray->GetBufferByName("TexCoords")->Upload(m_FramesData[frame].TexCoords);
				if (m_Indexed) {
					auto indexedVA = reinterpret_cast<IndexedVertexArray *>(m_VertexArray.get());
					indexedVA->GetIndexBuffer()->Upload(m_FramesData[frame].Indices);
				}
			}
		}
	}

	void ViewObject::Draw() const {
		if (!m_Material.Visible) return;

		m_Shader->SetUniform("u_Albedo", m_Material.Albedo);
		if (m_VecSizeBytes != 2 * sizeof(float)) {
			m_Shader->SetUniform("u_Metallic", m_Material.Metallic);
			m_Shader->SetUniform("u_Roughness", m_Material.Roughness);
		}
		
		m_Shader->Bind();
		if (m_Indexed) {
			auto indexedVA = reinterpret_cast<IndexedVertexArray *>(m_VertexArray.get());
			Renderer::Draw(indexedVA);
		} else {
			Renderer::Draw(m_VertexArray.get());
		}
		m_Shader->Unbind();
	}
}
