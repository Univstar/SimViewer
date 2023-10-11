#pragma once

#include "ViewStructs.h"

namespace Pivot {
	class ViewShaderPool {
	public:
		static std::string_view  GetName  (ViewShader viewShader) { return c_Names[static_cast<std::size_t>(viewShader)]; }
		static Shader           *GetShader(ViewShader viewShader) { return s_Shaders[static_cast<std::size_t>(viewShader)].get(); }
		
		static auto &GetShaders() { return s_Shaders; }
		
		static ViewShader GetViewShader(std::string const &name);

		static void CompileShaders();
		static void DestroyShaders();

	private:
		static constexpr auto c_Names = std::to_array<std::string_view>({
			"default",
			"default (triangles)",
			"default (points)",
			"heatmap",
		});

		static inline std::array<std::unique_ptr<Shader>, static_cast<std::size_t>(ViewShader::Count)> s_Shaders;

		static inline std::unordered_map<std::string, ViewShader> const c_NameToViewShader = {
			{ "default_2d_triangles", ViewShader::Default2D           },
			{ "default_2d_lines"    , ViewShader::Default2D           },
			{ "default_2d_points"   , ViewShader::Default2D           },
			{ "default_3d_triangles", ViewShader::Default3D_Triangles },
			// { "default_3d_lines"    , ViewShader::Default3D_Lines     },
			{ "default_3d_points"   , ViewShader::Default3d_Points    },
			{ "heatmap_2d_triangles", ViewShader::Heatmap2D },
			{ "heatmap_2d_lines"    , ViewShader::Heatmap2D },
			{ "heatmap_2d_points"   , ViewShader::Heatmap2D },
		};
	};
}
