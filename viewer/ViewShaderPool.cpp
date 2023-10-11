#include "ViewShaderPool.h"

#include "Graphics/Renderer.h"

namespace Pivot {
	ViewShader ViewShaderPool::GetViewShader(std::string const &name) {
		if (auto iter = c_NameToViewShader.find(name); iter != c_NameToViewShader.end()) {
			return iter->second;
		} else {
			spdlog::error("Failed to find Shader \"{}\"", name);
			return ViewShader::Count;
		}
	}

	void ViewShaderPool::CompileShaders() {
		{ // Default 2D shader
			auto vs = Renderer::CreateShaderModule("assets/shaders/default_2d.vert");
			auto fs = Renderer::CreateShaderModule("assets/shaders/default_2d.frag");
			auto shader = Renderer::CreateShader({ vs.get(), fs.get() });
			s_Shaders[static_cast<std::size_t>(ViewShader::Default2D)] = std::move(shader);
		}
		{ // Default 3D shader of triangles
			auto vs = Renderer::CreateShaderModule("assets/shaders/default_3d_triangles.vert");
			auto gs = Renderer::CreateShaderModule("assets/shaders/default_3d_triangles.geom");
			auto fs = Renderer::CreateShaderModule("assets/shaders/default_3d_triangles.frag");
			auto shader = Renderer::CreateShader({ vs.get(), gs.get(), fs.get() });
			s_Shaders[static_cast<std::size_t>(ViewShader::Default3D_Triangles)] = std::move(shader);
		}
		{ // Points 3D shader of points
			auto vs = Renderer::CreateShaderModule("assets/shaders/default_3d_points.vert");
			auto fs = Renderer::CreateShaderModule("assets/shaders/default_3d_points.frag");
			auto shader = Renderer::CreateShader({ vs.get(), fs.get() });
			s_Shaders[static_cast<std::size_t>(ViewShader::Default3d_Points)] = std::move(shader);
		}
		{ // Heatmap 2D shader
			auto vs = Renderer::CreateShaderModule("assets/shaders/heatmap_2d.vert");
			auto fs = Renderer::CreateShaderModule("assets/shaders/heatmap_2d.frag");
			auto shader = Renderer::CreateShader({ vs.get(), fs.get() });
			s_Shaders[static_cast<std::size_t>(ViewShader::Heatmap2D)] = std::move(shader);
		}
	}

	void ViewShaderPool::DestroyShaders() {
		for (auto &shader : s_Shaders) {
			shader.reset();
		}
	}
}
