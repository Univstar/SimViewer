#include "ViewObject.h"

#include "ViewShaderPool.h"

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
	ViewObject::ViewObject(std::size_t nodeId, YAML::Node const &node, std::uint32_t dimension) {
		m_VecSizeBytes = dimension * sizeof(float);
		if (node["Name"]) {
			m_Name = node["Name"].as<std::string>();
			m_DisplayName = std::to_string(nodeId + 1) + ": " + m_Name;
		} else {
			spdlog::error("Failed to find the name in an object description");
		}

		constexpr auto SetValue = [] <typename T>(T &val, YAML::Node const & node) { if (node) val = node.as<T>(); };

		PrimitiveType primitive = PrimitiveType::Triangles;
		SetValue(primitive, node["Primitive"]);

		static constexpr auto c_PrimitiveNames = std::to_array<std::string_view>({
			"points", "lines", "lines", "lines", "triangles", "triangles", "triangles", "", "", "", "lines", "lines", "triangles", "triangles",
		});

		// Determine shader
		std::string shaderName = "default";
		SetValue(shaderName, node["Shader"]);
		m_ViewShader = ViewShaderPool::GetViewShader(fmt::format("{}_{}d_{}", shaderName, dimension, c_PrimitiveNames[static_cast<std::size_t>(primitive)]));
		if (m_ViewShader == ViewShader::Count) { // invalid shader name
			m_ViewShader = ViewShaderPool::GetViewShader(fmt::format("default_{}d_{}", dimension, c_PrimitiveNames[static_cast<std::size_t>(primitive)]));
		}

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
		if (shaderName == "heatmap") {
			m_AttribFlags |= AttribFlagBits::Heat;
		}

		VertexLayout layout;
		if (dimension == 2) {
			if (m_AttribFlags & AttribFlagBits::Position)
				layout = std::move(layout).Add<glm::vec2>("Positions", 0);
			if (m_AttribFlags & AttribFlagBits::TexCoord)
				layout = std::move(layout).Add<glm::vec2>("TexCoords", 1);
			if (m_AttribFlags & AttribFlagBits::Heat)
				layout = std::move(layout).Add<float>("Heats", 1);
		} else {
			if (m_AttribFlags & AttribFlagBits::Position)
				layout = std::move(layout).Add<glm::vec3>("Positions", 0);
			if (m_AttribFlags & AttribFlagBits::Normal)
				layout = std::move(layout).Add<glm::vec3>("Normals", 1);
			if (m_AttribFlags & AttribFlagBits::TexCoord)
				layout = std::move(layout).Add<glm::vec2>("TexCoords", 2);
			if (m_AttribFlags & AttribFlagBits::Heat)
				layout = std::move(layout).Add<float>("Heats", 2);
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
		if (m_AttribFlags & AttribFlagBits::Heat) {
			frameData.Heats.resize(vtxCount * sizeof(float));
			IO::Read(fin, frameData.Heats);
			std::span<float const> heats(reinterpret_cast<float const *>(frameData.Heats.data()), vtxCount);
			m_Material.HeatMax = std::max(m_Material.HeatMax, *std::max_element(heats.begin(), heats.end()));
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
			if (m_AttribFlags & AttribFlagBits::Heat)
				m_VertexArray->GetBufferByName("Heats")->Upload(m_FramesData[posFrame].Heats);
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
			if (m_AttribFlags & AttribFlagBits::Heat)
				m_VertexArray->GetBufferByName("Heats")->Upload(m_FramesData[frame].Heats);
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
		auto shader = ViewShaderPool::GetShader(m_ViewShader);

		switch (m_ViewShader) {
		case ViewShader::Default2D:
			shader->SetUniform("u_Albedo", m_Material.Albedo);
			break;
		case ViewShader::Default3D_Triangles:
		case ViewShader::Default3d_Points:
			shader->SetUniform("u_Albedo", m_Material.Albedo);
			shader->SetUniform("u_Metallic", m_Material.Metallic);
			shader->SetUniform("u_Roughness", m_Material.Roughness);
			break;
		case ViewShader::Heatmap2D:
			shader->SetUniform("u_Scale", m_Material.HeatScale / m_Material.HeatMax);
			break;
		case ViewShader::Heatmap3D_Triangles:
			shader->SetUniform("u_Metallic", m_Material.Metallic);
			shader->SetUniform("u_Roughness", m_Material.Roughness);
			shader->SetUniform("u_Scale", m_Material.HeatScale / m_Material.HeatMax);
			break;
		}
		
		shader->Bind();
		if (m_Indexed) {
			auto indexedVA = reinterpret_cast<IndexedVertexArray *>(m_VertexArray.get());
			Renderer::Draw(indexedVA);
		} else {
			Renderer::Draw(m_VertexArray.get());
		}
		shader->Unbind();
	}
}
