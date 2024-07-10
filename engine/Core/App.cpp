#include "App.h"

#include "Core/FileDialog.h"
#include "Core/Platform.h"
#include "Graphics/Renderer.h"

#undef CreateWindow

namespace Pivot {
	using namespace std::chrono_literals;
	
	App::App(AppCreateOptions const &options) {
		s_This = this;
		
		Platform::Init();
		FileDialog::Init();

		m_Window = Platform::CreateWindow(options.WindowCreateOptions);
		SetHandlers();

		Platform::MakeContextCurrent(m_Window.get());
		Platform::SetVSynchronized(true);

		Gui::Init();
		Gui::ResetScale(m_Window->GetScale());
		Gui::ResetStyle(options.GuiStyleOptions);
	}

	App::~App() {
		Gui::Terminate();

		m_Window.reset();

		FileDialog::Terminate();
		Platform::Terminate();
	}

	void App::SetHandlers() {
		m_Window->SetHandler<Event::WindowRefresh>([](Window *window) {
			s_This->Tick(s_This->GetDeltaTime());
		});
		m_Window->SetHandler<Event::WindowContentScale>([](Window *window, float scale) {
			Gui::ResetScale(scale);
		});
		m_Window->SetHandler<Event::Key>([](Window *window, Key key, ButtonAction action, ModifierFlags mods) {
			if (s_This->m_Kernel && !Gui::AreKeyEventsBlocked()) {
				if (action == ButtonAction::Press) {
					s_This->m_Kernel->OnKeyDown(key, mods);
				} else if (action == ButtonAction::Release) {
					s_This->m_Kernel->OnKeyUp(key, mods);
				} else if (action == ButtonAction::Repeat) {
					s_This->m_Kernel->OnKeyRepeat(key, mods);
				}
			}
		});
		m_Window->SetHandler<Event::MouseButton>([](Window *window, MouseButton button, ButtonAction action, ModifierFlags mods) {
			if (s_This->m_Kernel && !Gui::AreMouseEventsBlocked()) {
				if (action == ButtonAction::Press) {
					s_This->m_Kernel->OnMouseDown(button, mods);
				} else if (action == ButtonAction::Release) {
					s_This->m_Kernel->OnMouseUp(button, mods);
				}
			}
		});
		m_Window->SetHandler<Event::MouseWheel>([](Window *window, float xOffset, float yOffset) {
			if (s_This->m_Kernel && !Gui::AreMouseEventsBlocked()) {
				s_This->m_Kernel->OnMouseWheel(xOffset, yOffset);
			}
		});
		m_Window->SetHandler<Event::MouseMove>([](Window *window, float xPos, float yPos) {
			if (s_This->m_Kernel && !Gui::AreMouseEventsBlocked()) {
				s_This->m_Kernel->OnMouseMove(xPos, yPos);
			}
		});
	}

	void App::Run() {
		m_LastFrameTime = std::chrono::steady_clock::now();
		m_LastCheckTime = m_LastCheckTime;
		m_FramesCnt     = 0;

		m_Window->Show();
		while (!m_Window->IsClosed()) {
			auto deltaTime = GetDeltaTime();
			Tick(deltaTime);
			Platform::PollEvents();
		}
	}

	float App::GetDeltaTime() {
		auto currentTime = std::chrono::steady_clock::now();
		auto deltaTime = std::chrono::duration<float>(currentTime - m_LastFrameTime).count();
		m_LastFrameTime = currentTime;
		if (m_FramesCnt++; currentTime - m_LastCheckTime >= 1s) {
			m_Fps = m_FramesCnt / std::chrono::duration<float>(currentTime - m_LastCheckTime).count();
			m_FramesCnt = 0;
			m_LastCheckTime = currentTime;
		}
		return deltaTime;
	}

	void App::Tick(float deltaTime) {
        Renderer::SetViewport({ 0, 0 }, m_Window->GetFbSize());
		Renderer::Clear(BufferFlagBits::Color | BufferFlagBits::Depth | BufferFlagBits::Stencil);

		if (m_Kernel) m_Kernel->Tick(deltaTime);

		Gui::Prepare();
		if (m_Kernel) m_Kernel->RenderGui();
		Gui::Submit();

		Platform::SwapBuffers(m_Window.get());
	}
}
