#include "Viewer.h"

#include "Core/App.h"

namespace Pivot {
	void Viewer::UpdateCurrentFrame(float deltaTime) {
		m_AvailFrameCount = m_Loader.GetAvailFrameCount();
		if (m_AvailFrameCount == 0) {
			m_CurrentFrame = 0;
			m_Animation.CurrentFrame = 0.f;
			return;
		}

		if (m_AnimationExporter.IsExporting()) {
			m_Animation.CurrentFrame = static_cast<float>(m_AnimationExporter.GetFrameToRender());
		} else if (m_Animation.Playing) {
			m_Animation.CurrentFrame += deltaTime * m_Animation.FrameRate;
			if (m_Animation.CurrentFrame >= m_AvailFrameCount - 1) {
				m_Animation.CurrentFrame = static_cast<float>(m_AvailFrameCount - 1);
				m_Animation.Playing = false;
			}
		}

		m_Animation.CurrentFrame = std::clamp(m_Animation.CurrentFrame, 0.f, static_cast<float>(m_AvailFrameCount - 1));
		m_CurrentFrame = static_cast<std::uint32_t>(m_Animation.CurrentFrame);
	}

	void Viewer::UploadCurrentFrame() {
		for (auto &object : m_Objects) {
			object->UploadBuffers(m_CurrentFrame);
		}
	}

	void Viewer::RenderScene() {
		PassConstants passConstants = {
			.Transform      = m_Camera->GetTransform(App::Get()->GetWindow()->GetAspect()),
			.LightIntensity = m_Appearance.LightColor * m_Appearance.LightIntensity * .1f,
			.LightDirection = Coordinates::SphericalToCartesian(1.f, glm::radians(90.f) - m_Appearance.LightAltitude, m_Appearance.LightAzimuth),
			.AmbientCoeff   = m_Appearance.EnvironColor * m_Appearance.EnvironIntensity * .001f,
			.CameraPosition = m_Camera->GetPosition(),
			.Wireframed     = m_Appearance.Wireframed,
			.Flat           = !m_Appearance.VertexNormalUsed,
		};
		m_PassConstantsBuffer->Upload(passConstants);

		for (auto const &layer : m_ObjectLayers) {
			for (auto const &object : layer) {
				object->Draw();
			}
		}
	}
}