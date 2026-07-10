#include "Viewer.h"

#include "Core/App.h"
#include "Core/FileDialog.h"
#include "Utils/ImGuiEx.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace Pivot {
	void Viewer::RenderGui() {
		RenderSideBars();
		RenderPopups();
		RenderAnimationPanel();
		RenderAppearancePanel();
		RenderCameraPanel();
		RenderObjectsPanel();
		RegisterGlobalShortcuts();
	}

	void Viewer::RenderSideBars() {
		auto &exportAnimation = m_AnimationExporter.GetState();
		if (m_MenuBarVisible && ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("Save Screenshot", "Ctrl+S")) {
					SaveScreenshot();
				}
				if (ImGui::BeginMenu("Export", !m_Animation.Playing && !m_AnimationExporter.IsExporting())) {
					if (ImGui::MenuItem("Animation", "Ctrl+N")) {
						exportAnimation.Requested = true;
					}
					if (ImGui::MenuItem("Models", "Ctrl+M")) {
						m_ExportModels.Requested = true;
					}
					ImGui::EndMenu();
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Exit", "Alt+F4")) {
					App::Get()->GetWindow()->Close();
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Control")) {
				if (ImGui::MenuItem(m_Animation.Playing ? "Stop" : "Play", "Ctrl+P")) {
					m_Animation.Playing = !m_Animation.Playing;
				}
				if (ImGui::MenuItem("Reset Animation", "Ctrl+R", false, !m_Animation.Playing)) {
					m_Animation = { };
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Reset Camera", "Ctrl+Backspace")) {
					m_Camera->SetAs(*m_InitialCamera);
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("View")) {
				if (ImGui::MenuItem("Menu Bar", "F1", m_MenuBarVisible)) {
					m_MenuBarVisible = !m_MenuBarVisible;
				}
				if (ImGui::MenuItem("Status Bar", "F2", m_StatusBarVisible)) {
					m_StatusBarVisible = !m_StatusBarVisible;
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Animation", "F9", m_Animation.Visible)) {
					m_Animation.Visible = !m_Animation.Visible;
				}
				if (ImGui::MenuItem("Appearance", "F10", m_Appearance.Visible)) {
					m_Appearance.Visible = !m_Appearance.Visible;
				}
				if (ImGui::MenuItem("Camera", "F11", m_CameraInfo.Visible)) {
					m_CameraInfo.Visible = !m_CameraInfo.Visible;
				}
				if (ImGui::MenuItem("Objects", "F12", m_ObjectsInfo.Visible)) {
					m_ObjectsInfo.Visible = !m_ObjectsInfo.Visible;
				}
				ImGui::EndMenu();
			}
			ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
			ImGuiEx::TextRightAligned("%.0f FPS", App::Get()->GetFps());
			ImGui::PopFont();
			ImGui::EndMainMenuBar();
		}

		if (m_StatusBarVisible && ImGuiEx::BeginMainStatusBar()) {
			if (m_AvailFrameCount == m_FrameCount) {
				ImGui::Text("Ready");
			} else {
				ImGui::Text("Loading... %.1f%%", 100.f * m_AvailFrameCount / m_FrameCount);
			}
			if (m_GridRes.x > 0 && m_CursorCell.x >= 0 && m_CursorCell.x < m_GridRes.x
			                      && m_CursorCell.y >= 0 && m_CursorCell.y < m_GridRes.y) {
				ImGui::SameLine();
				ImGui::Text(" | Cell: (%d, %d)  World: (%.4f, %.4f)",
					m_CursorCell.x, m_CursorCell.y,
					m_CursorWorldPos.x, m_CursorWorldPos.y);
			}
			ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
			ImGuiEx::TextRightAligned("%u/%u", m_CurrentFrame, m_AvailFrameCount - 1);
			ImGui::PopFont();
			ImGuiEx::EndMainStatusBar();
		}
	}

	void Viewer::RenderPopups() {
		auto &exportAnimation = m_AnimationExporter.GetState();
		if (exportAnimation.Requested) {
			ImGui::OpenPopup("Export Animation");
			m_AnimationExporter.PreparePopup(m_AvailFrameCount, m_Animation.FrameRate);
		}
		if (m_ExportModels.Requested) {
			ImGui::OpenPopup("Export Models");
			m_ExportModels.Popup = true;
			m_ExportModels.Requested = false;
		}

		if (ImGui::BeginPopupModal("Export Animation", &exportAnimation.Popup, ImGuiWindowFlags_AlwaysAutoResize)) {
			if (exportAnimation.Exporting) {
				ImGui::PushTextWrapPos(0.f);
				ImGui::TextUnformatted(exportAnimation.Status.c_str());
				ImGui::PopTextWrapPos();
				ImGui::ProgressBar(m_AnimationExporter.GetProgress(), { 320.f, 0.f });
				ImGui::BeginDisabled();
				ImGui::Button("Working...");
				ImGui::EndDisabled();
			} else {
				ImGui::Text("Output File");
				if (ImGuiEx::InputText("##AnimationFilename", &exportAnimation.FilenameStr, ImGuiInputTextFlags_EnterReturnsTrue)) {
					exportAnimation.Filename = std::filesystem::absolute(exportAnimation.FilenameStr);
					exportAnimation.FilenameStr = exportAnimation.Filename.string();
				}
				ImGui::SameLine();
				if (ImGui::Button("Browse##Animation")) {
					auto filename = FileDialog::Save({ { "MPEG-4 Video", "mp4" } }, App::Get()->GetWindow(), exportAnimation.Filename.parent_path(), exportAnimation.Filename.filename().string());
					if (!filename.empty()) {
						exportAnimation.Filename = filename;
						exportAnimation.FilenameStr = exportAnimation.Filename.string();
					}
				}

				int const maxFrame = static_cast<int>(std::max(1u, m_AvailFrameCount) - 1);
				exportAnimation.StartFrame = std::clamp(exportAnimation.StartFrame, 0, maxFrame);
				exportAnimation.EndFrame = std::clamp(exportAnimation.EndFrame, 0, maxFrame);
				ImGui::Spacing();
				ImGui::InputInt("Start frame", &exportAnimation.StartFrame);
				ImGui::InputInt("End frame", &exportAnimation.EndFrame);
				if (ImGui::InputFloat("Frame rate", &exportAnimation.FrameRate, 1.f, 10.f, "%.1f")) {
					exportAnimation.FrameRate = std::max(exportAnimation.FrameRate, 1.f);
				}
				int loadedFrameCount = maxFrame + 1;
				ImGui::BeginDisabled();
				ImGui::InputInt("Loaded frames", &loadedFrameCount);
				ImGui::EndDisabled();

				if (!exportAnimation.Status.empty()) {
					ImGui::Spacing();
					ImGui::PushTextWrapPos(0.f);
					if (exportAnimation.Failed) {
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 0.45f, 0.45f, 1.f));
						ImGui::TextUnformatted(exportAnimation.Status.c_str());
						ImGui::PopStyleColor();
					} else if (exportAnimation.Completed) {
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.45f, 1.f, 0.45f, 1.f));
						ImGui::TextUnformatted(exportAnimation.Status.c_str());
						ImGui::PopStyleColor();
					} else {
						ImGui::TextUnformatted(exportAnimation.Status.c_str());
					}
					ImGui::PopTextWrapPos();
				}

				ImGui::Spacing();
				ImGui::Separator();
				auto const &style = ImGui::GetStyle();
				float buttonWidth1 = ImGui::CalcTextSize("Start Export").x + style.FramePadding.x * 2.f;
				float buttonWidth2 = ImGui::CalcTextSize("Cancel").x + style.FramePadding.x * 2.f;
				float widthNeeded = buttonWidth1 + style.ItemSpacing.x + buttonWidth2;
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - widthNeeded);
				if (ImGui::Button("Start Export")) {
					m_Animation.Playing = false;
					if (m_AnimationExporter.Start(m_AvailFrameCount)) {
						exportAnimation.Popup = true;
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel")) {
					exportAnimation.Popup = false;
					ImGui::CloseCurrentPopup();
				}
			}
			ImGui::EndPopup();
		}

		if (ImGui::BeginPopupModal("Export Models", &m_ExportModels.Popup, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::Text("Output Directory");
			if (ImGuiEx::InputText("##Dirname", &m_ExportModels.DirnameStr, ImGuiInputTextFlags_EnterReturnsTrue)) {
				m_ExportModels.Dirname = std::filesystem::absolute(m_ExportModels.DirnameStr);
				m_ExportModels.DirnameStr = m_ExportModels.Dirname.string();
			}
			ImGui::SameLine();
			if (ImGui::Button("Browse")) {
				auto newDirname = FileDialog::PickFolder(App::Get()->GetWindow(), m_ExportModels.Dirname);
				if (!newDirname.empty()) {
					m_ExportModels.Dirname = newDirname;
					m_ExportModels.DirnameStr = m_ExportModels.Dirname.string();
				}
			}
			ImGui::Spacing();
			ImGui::Text("Object Selection");
			ImGui::BeginListBox("##Objects", { -FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing() });
			for (std::size_t i = 0; i < m_Objects.size(); i++) {
				ImGui::Checkbox(m_Objects[i]->GetDisplayName().data(), reinterpret_cast<bool *>(&m_ExportModels.Exported[i]));
			}
			ImGui::EndListBox();
			ImGui::Spacing();
			if (ImGui::RadioButton("Current frame", !m_ExportModels.AllFramesSelected)) {
				m_ExportModels.AllFramesSelected = false;
			}
			ImGui::SameLine();
			if (ImGui::RadioButton("All frames", m_ExportModels.AllFramesSelected)) {
				m_ExportModels.AllFramesSelected = true;
			}
			ImGui::Spacing();
			ImGui::Separator();
			{
				auto const &style = ImGui::GetStyle();
				float buttonWidth1 = ImGui::CalcTextSize("OK").x + style.FramePadding.x * 2.f;
				float buttonWidth2 = ImGui::CalcTextSize("Cancel").x + style.FramePadding.x * 2.f;
				float widthNeeded = buttonWidth1 + style.ItemSpacing.x + buttonWidth2;
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - widthNeeded);
				if (ImGui::Button("OK")) {
					ExportModels();
					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel")) {
					ImGui::CloseCurrentPopup();
				}
			}
			ImGui::EndPopup();
		}
	}

	void Viewer::RegisterGlobalShortcuts() {
		auto &exportAnimation = m_AnimationExporter.GetState();
		if (ImGui::Shortcut(ImGuiKey_ModCtrl | ImGuiKey_S, 0, ImGuiInputFlags_RouteGlobalLow)) {
			SaveScreenshot();
		}
		if (!m_Animation.Playing && !m_AnimationExporter.IsExporting() && ImGui::Shortcut(ImGuiKey_ModCtrl | ImGuiKey_N, 0, ImGuiInputFlags_RouteGlobalLow)) {
			exportAnimation.Requested = true;
		}
		if (!m_Animation.Playing && ImGui::Shortcut(ImGuiKey_ModCtrl | ImGuiKey_M, 0, ImGuiInputFlags_RouteGlobalLow)) {
			m_ExportModels.Requested = true;
		}
		if (ImGui::Shortcut(ImGuiKey_F1, 0, ImGuiInputFlags_RouteGlobalLow)) {
			m_MenuBarVisible = !m_MenuBarVisible;
		}
		if (ImGui::Shortcut(ImGuiKey_F2, 0, ImGuiInputFlags_RouteGlobalLow)) {
			m_StatusBarVisible = !m_StatusBarVisible;
		}
		if (ImGui::Shortcut(ImGuiKey_F9, 0, ImGuiInputFlags_RouteGlobalLow)) {
			m_Animation.Visible = !m_Animation.Visible;
		}
		if (ImGui::Shortcut(ImGuiKey_F10, 0, ImGuiInputFlags_RouteGlobalLow)) {
			m_Appearance.Visible = !m_Appearance.Visible;
		}
		if (ImGui::Shortcut(ImGuiKey_F11, 0, ImGuiInputFlags_RouteGlobalLow)) {
			m_CameraInfo.Visible = !m_CameraInfo.Visible;
		}
		if (ImGui::Shortcut(ImGuiKey_F12, 0, ImGuiInputFlags_RouteGlobalLow)) {
			m_ObjectsInfo.Visible = !m_ObjectsInfo.Visible;
		}
		if (ImGui::Shortcut(ImGuiKey_ModCtrl | ImGuiKey_P, 0, ImGuiInputFlags_RouteGlobalLow)) {
			m_Animation.Playing = !m_Animation.Playing;
		}
		if (!m_Animation.Playing && ImGui::Shortcut(ImGuiKey_ModCtrl | ImGuiKey_R, 0, ImGuiInputFlags_RouteGlobalLow)) {
			m_Animation = { };
		}
		if (ImGui::Shortcut(ImGuiKey_ModCtrl | ImGuiKey_Backspace, 0, ImGuiInputFlags_RouteGlobalLow)) {
			m_Camera->SetAs(*m_InitialCamera);
		}
		for (std::size_t i = 0; i < m_Objects.size() && i < 9; i++) {
			if (ImGui::Shortcut(ImGuiKey_ModCtrl | ImGuiKey_1 + i, 0, ImGuiInputFlags_RouteGlobalLow)) {
				m_Objects[i]->GetMaterial().Visible = !m_Objects[i]->GetMaterial().Visible;
			}
		}
	}
}