#pragma once

#include "Graphics/Shader.h"

namespace Pivot {
	using ShaderPool = std::unordered_map<std::string, std::unique_ptr<Shader>>;

	struct AttribFlagBits {
		enum : std::uint16_t {
			Position = 1 << 0,
			Normal   = 1 << 1,
			TexCoord = 1 << 2,
		};
	};

	using AttribFlags = std::uint16_t;

	enum class BlendMode : std::uint16_t {
		Opaque,
		Cutout,
		Transparent,
		Fade,
		Count,
	};

	struct ViewMaterial {
		BlendMode Mode      = BlendMode::Opaque;
		glm::vec4 Albedo    = { 1, 1, 1, 1 };
		float     Metallic  = 0;
		float     Roughness = .5f;
		bool      Visible   = true;
	};

	struct ViewExportModels {
		bool Requested = false;
		bool Popup     = false;
		std::filesystem::path Dirname;
		std::string           DirnameStr;
		std::vector<char>     Exported;
	};

	struct ViewAnimation {
		bool      Visible      = false;
		float     CurrentFrame = 0;
		bool      Playing      = false;
		float     FrameRate    = 25;
		int       FrameNumber  = 0;
	};

	struct ViewAppearance {
		bool      Visible          = false;
		glm::vec4 Background       = { 0, 0, 0, 1 };
		bool      Multisampled     = true;
		bool      Wireframed       = false;
		bool      BackFaceCulled   = false;
		bool      VertexNormalUsed = true;
		glm::vec3 LightColor       = { 1, 1, 1 };
		float     LightIntensity   = 100.f;
		float     LightAltitude    = glm::radians(45.f);
		float     LightAzimuth     = glm::radians(0.f);
		glm::vec3 EnvironColor     = { 1, 1, 1 };
		float     EnvironIntensity = 40.f;
	};

	struct ViewCameraInfo {
		bool      Visible      = false;
		glm::vec2 LastMousePos;
	};

	struct ViewObjectsInfo {
		bool        Visible = false;
		std::size_t CurIdx  = 0;
	};

	struct PassConstants {
		alignas(64) glm::mat4 Transform;
		alignas(16) glm::vec3 LightIntensity;
		alignas(16) glm::vec3 LightDirection;
		alignas(16) glm::vec3 AmbientCoeff;
		alignas(16) glm::vec3 CameraPosition;
		alignas(4)  int       Wireframed;
		alignas(4)  int       Flat;
	};
}
