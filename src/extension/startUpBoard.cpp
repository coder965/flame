#include "..\core\core.h"
#include "..\core\window.h"
#include "..\core\gui.h"
#include "..\core\render.h"
#include "image.file.h"

#define WM_STARTUP_SETTEST (WM_USER + 100)

namespace tke
{
	namespace StartUpBoard
	{
		enum class State
		{
			eReady,
			eRunning
		}state;

		CRITICAL_SECTION cs;
		glm::vec2 progress;
		char text[2][200];

		UI::EngineGuiWindow *pWindow;

		Renderer *renderer;

		void setProgress(int which, float v)
		{
			if (state != State::eRunning) return;
			EnterCriticalSection(&cs);
			progress[which] = v;
			LeaveCriticalSection(&cs);
		}

		void setText(char which, const char *s)
		{
			if (state != State::eRunning) return;
			SendMessage(pWindow->hWnd, WM_STARTUP_SETTEST, (WPARAM)s, (LPARAM)which);
		}
		static bool first = true;

		Image *pImage;
		UniformBuffer uniformBuffer;
		Pipeline pipeline;
		VkCommandBuffer cmd[2];
		VkSemaphore renderFinishedSemaphore;

		struct MainWindow : UI::EngineGuiWindow
		{
			using UI::EngineGuiWindow::EngineGuiWindow;

			virtual void deadEvent() override
			{
				state = State::eReady;
			}

			virtual void renderEvent() override
			{
				EnterCriticalSection(&cs);

				uniformBuffer.update(&progress, &stagingBuffer);

				pWindow->perpareFrame();

				pWindow->m_uiFramebuffer = renderer->vkFramebuffer[pWindow->m_imageIndex];
				UI::lock(pWindow);
				ImGui::Begin("StartUp", nullptr, ImVec2(0, 0), 0.3f, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
				ImGui::TextUnformatted(text[0]);
				ImGui::TextUnformatted(text[1]);
				ImGui::End();
				UI::unlock();

				vk::queueSubmit(pWindow->m_imageAvailable, renderFinishedSemaphore, cmd[pWindow->m_imageIndex]);

				pWindow->endFrame(renderFinishedSemaphore);

				Sleep(10);

				LeaveCriticalSection(&cs);
			}

			virtual LRESULT extraMsgEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override
			{
				switch (message)
				{
				case WM_STARTUP_SETTEST:
					EnterCriticalSection(&cs);
					strcpy(text[lParam], (const char *)wParam);
					LeaveCriticalSection(&cs);
					break;
				}
				return 0;
			}
		};

		void run()
		{
			static ResourceBank _resources;

			if (state == State::eRunning) return;

			static bool first = true;
			if (first)
			{

				InitializeCriticalSection(&cs);
				renderFinishedSemaphore = vk::createSemaphore();

				pImage = createImage("../misc/start_up.jpg", true, false);
				uniformBuffer.create(sizeof glm::vec2);

				_resources.setImage(pImage, "StartUp.Texture");
				_resources.setBuffer(&uniformBuffer, "StartUp.UniformBuffer");
				_resources.setPipeline(&pipeline, "StartUp.Pipeline");

				renderer = new Renderer();
				renderer->filename = "../renderer/start_up_board.xml";
				renderer->loadXML();
				renderer->pResource = &_resources;

				pipeline.pResource = &_resources;

				for (int i = 0; i < 2; i++)
					cmd[i] = vk::allocateCommandBuffer();
			}

			pWindow = new MainWindow(renderer->cx, renderer->cy, "", WS_POPUP, 0, false);

			_resources.setImage(pWindow->m_image, "Window.Image");
			_resources.setCmd(pWindow->m_uiCommandBuffer, "Ui.Cmd");

			renderer->setup();

			if (first)
			{
				pipeline.create("../pipeline/startUp/startUp.xml", &zeroVertexInputState, renderer->cx, renderer->cy, renderer->vkRenderPass, 0);
				first = false;
			}

			renderer->getDescriptorSets();

			pWindow->initUi(renderer->vkRenderPass, 1);

			for (int i = 0; i < 2; i++)
			{
				if (!first) vkResetCommandBuffer(cmd[i], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
				vk::beginCommandBuffer(cmd[i]);
				renderer->execute(cmd[i], i);
				vkEndCommandBuffer(cmd[i]);
			}

			progress = glm::vec2(0.f);
			state = State::eRunning;
			pWindow->show();
		}

		void complete()
		{
			if (state != State::eRunning) return;
			EnterCriticalSection(&cs);
			progress.x = 1.f;
			progress.y = 1.f;
			LeaveCriticalSection(&cs);
			SendMessage(pWindow->hWnd, WM_CLOSE, 0, 0);
		}
	}
}
