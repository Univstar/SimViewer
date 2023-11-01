#pragma once

#include "ViewStructs.h"

#include "Graphics/VertexArray.h"

#include <yaml-cpp/yaml.h>

namespace Pivot {
	class ViewObject {
	private:
		struct FrameData {
			std::vector<std::byte> Positions;
			std::vector<std::byte> Normals;
			std::vector<std::byte> TexCoords;
			std::vector<std::byte> Heats;

			std::vector<std::uint32_t> Indices;
		};
	
	public:
		ViewObject(std::size_t nodeId, YAML::Node const &node, std::uint32_t dimension);
		virtual ~ViewObject() = default;

		void ReserveFrames(std::uint32_t frameCount);
		bool LoadNewFrameData(std::filesystem::path const &dirname, bool initial = false);
		void UploadBuffers(std::uint32_t frame, bool initial = false);
		void Draw() const;

		std::string_view  GetName()        const { return m_Name; }
		std::string_view  GetDisplayName() const { return m_DisplayName; }
		ViewShader        GetViewShader()  const { return m_ViewShader; }
		bool              IsIndexed()      const { return m_Indexed; }
		bool              IsAnimated()     const { return m_Animated; }
		bool              IsTopoFixed()    const { return m_TopoFixed; }
		auto             &GetMaterial()          { return m_Material; }
		auto              GetAttribFlags() const { return m_AttribFlags; }
		
		auto GetVertexCount(std::uint32_t frame) const { return static_cast<std::uint32_t>(m_FramesData[m_Animated ? frame : 0].Positions.size() / m_VecSizeBytes); }
		auto GetIndexCount (std::uint32_t frame) const { return static_cast<std::uint32_t>(m_FramesData[m_Animated && !m_TopoFixed ? frame : 0].Indices.size()); }

		void Export(std::uint32_t frame, std::filesystem::path const &dirname) const;

	private:
		std::size_t   m_VecSizeBytes;
		std::string   m_Name;
		std::string   m_DisplayName;
		ViewShader    m_ViewShader;

		bool          m_Indexed     = false;
		bool          m_Animated    = false;
		bool          m_TopoFixed   = false;

		ViewMaterial  m_Material;
		AttribFlags   m_AttribFlags;

		std::unique_ptr<VertexArray> m_VertexArray;
		std::vector<FrameData>       m_FramesData;
		std::uint32_t                m_CurrentFrame;
	};
}
