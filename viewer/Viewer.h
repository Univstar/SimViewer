#pragma once

#include "AnimationExporter.h"
#include "ViewLoader.h"
#include "ViewStructs.h"

#include "Core/Kernel.h"
#include "Graphics/Renderer.h"
#include "Scene/Camera.h"

#include <yaml-cpp/yaml.h>

namespace Pivot {
	class Viewer : public Kernel {
	public:
		Viewer();
		Viewer(std::filesystem::path const &dirname);
		~Viewer();

		virtual void Tick(float deltaTime) override;
		virtual void RenderGui() override;

		virtual void OnKeyDown(Key key, ModifierFlags mods) override;
		virtual void OnKeyRepeat(Key key, ModifierFlags mods) override;
		virtual void OnMouseDown(MouseButton button, ModifierFlags mods) override;
		virtual void OnMouseWheel(float xOffset, float yOffset) override;
		virtual void OnMouseMove(float xPos, float yPos) override;

	private:
		bool LoadDirectory(std::filesystem::path const &dirname);
		bool SaveScreenshot();
		bool SaveFrameImage(std::filesystem::path const &filename) const;
		void ExportModels();
		void UpdateCurrentFrame(float deltaTime);
		void UploadCurrentFrame();
		void RenderScene();

		void RenderSideBars();
		void RenderPopups();
		void RenderAnimationPanel();
		void RenderAppearancePanel();
		void RenderCameraPanel();
		void RenderObjectsPanel();
		void RegisterGlobalShortcuts();

	private:
		std::filesystem::path m_Dirname;
		YAML::Node            m_Description;
		std::uint32_t         m_Dimension;
		std::uint32_t         m_FrameCount;
		std::uint32_t         m_CurrentFrame;
		std::uint32_t         m_AvailFrameCount;

		std::unique_ptr<Camera> m_Camera;
		std::unique_ptr<Camera> m_InitialCamera;
		std::unique_ptr<UniformBuffer> m_PassConstantsBuffer;
		std::vector<std::unique_ptr<ViewObject>> m_Objects;
		std::array<std::vector<ViewObject *>, static_cast<std::size_t>(BlendMode::Count)> m_ObjectLayers;

		ViewLoader     m_Loader;

		ViewExportModels m_ExportModels;
		AnimationExporter m_AnimationExporter;
		ViewAnimation    m_Animation;
		ViewAppearance   m_Appearance;
		ViewCameraInfo   m_CameraInfo;
		ViewObjectsInfo  m_ObjectsInfo;

		bool m_MenuBarVisible   = true;
		bool m_StatusBarVisible = true;
	};
}
