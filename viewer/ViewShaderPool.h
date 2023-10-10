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
			"default",
			"heatmap",
			"points"
		});

		static inline std::array<std::unique_ptr<Shader>, static_cast<std::size_t>(ViewShader::Count)> s_Shaders;

		static inline std::unordered_map<std::string, ViewShader> const c_NameToViewShader = {
			{ "default_2d", ViewShader::Default2D },
			{ "default_3d", ViewShader::Default3D },
			{ "heatmap_2d", ViewShader::Heatmap2D },
			{ "points_3d" , ViewShader::Points3D  },
		};
	};
}
