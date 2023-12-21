#include "Renderer.h"

#include <glad/glad.h>

namespace Pivot {
	char const *Renderer::GetDeviceName() {
		static char const *s_DeviceName = reinterpret_cast<char const *>(glGetString(GL_RENDERER));
		return s_DeviceName;
	}

	char const *Renderer::GetApiName() {
		static char const *s_ApiName = "OpenGL 4.1";
		return s_ApiName;
	}

	void Renderer::SetViewport(glm::ivec2 const &offset, glm::uvec2 const &extent) {
		glViewport(offset.x, offset.y, extent.x, extent.y);
	}

	void Renderer::SetDepthTested(bool enabled) {
		if (enabled) {
			glEnable(GL_DEPTH_TEST);
		} else {
			glDisable(GL_DEPTH_TEST);
		}
	}

	void Renderer::SetWireframed(bool enabled) {
		glPolygonMode(GL_FRONT_AND_BACK, enabled ? GL_LINE : GL_FILL);
	}

	void Renderer::SetBackFaceCulled(bool enabled) {
		if (enabled) {
			glEnable(GL_CULL_FACE);
		} else {
			glDisable(GL_CULL_FACE);
		}
	}

	void Renderer::SetMultisampled(bool enabled) {
		if (enabled) {
			glEnable(GL_MULTISAMPLE);
		} else {
			glDisable(GL_MULTISAMPLE);
		}
	}

	void Renderer::SetClearColor(glm::vec4 const &color) {
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void Renderer::Clear(BufferFlags bufferFlags) {
		glClear(bufferFlags);
	}

	void Renderer::Draw(VertexArray *vertexArray, std::int32_t first, std::uint32_t count, std::uint32_t instanceCount) {
		vertexArray->Bind();
		glDrawArraysInstanced(
			static_cast<GLenum>(vertexArray->m_PrimitiveType),
			first,
			count ? count : vertexArray->m_VertexBuffers.front()->m_VertexCount,
			instanceCount);
		vertexArray->Unbind();
	}

	void Renderer::Draw(IndexedVertexArray *vertexArray, std::uint32_t count, std::uint32_t instanceCount, std::int32_t baseVertex) {
		vertexArray->Bind();
		glDrawElementsInstancedBaseVertex(
			static_cast<GLenum>(vertexArray->m_PrimitiveType),
			count ? count : vertexArray->m_IndexBuffer->m_IndexCount,
			GL_UNSIGNED_INT,
			nullptr,
			instanceCount,
			baseVertex);
		vertexArray->Unbind();
	}

	std::vector<std::byte> Renderer::ReadPixels(glm::ivec2 const &offset, glm::uvec2 const &extent, std::uint32_t numChannels) {
		std::vector<std::byte> pixels(extent.x * extent.y * numChannels);
		switch (numChannels) {
		case 1:
			glReadPixels(offset.x, offset.y, extent.x, extent.y, GL_RED, GL_UNSIGNED_BYTE, pixels.data());
			break;
		case 2:
			glReadPixels(offset.x, offset.y, extent.x, extent.y, GL_RG, GL_UNSIGNED_BYTE, pixels.data());
			break;
		case 3:
			glReadPixels(offset.x, offset.y, extent.x, extent.y, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
			break;
		case 4:
			glReadPixels(offset.x, offset.y, extent.x, extent.y, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
			break;
		default:
			spdlog::error("Encountered unsupported number of channels: {}", numChannels);
		}
		return pixels;
	}
}
