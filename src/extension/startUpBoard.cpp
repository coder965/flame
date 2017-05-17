#include "..\core\core.h"
#include "..\core\window.h"
#include "..\core\gui.h"
#include "..\core\render.h"
#include "image.file.h"

namespace tke
{
	namespace StartUpBoard
	{
		enum class State
		{
			eReady,
			eRunning
		}state;

		GuiWindow *pWindow;

		Renderer *renderer;

		Image *pImage;
		UniformBuffer uniformBuffer;
		Pipeline pipeline;
		VkCommandBuffer cmd[2];
		VkSemaphore renderFinishedSemaphore;

		struct MainWindow : GuiWindow
		{
			using GuiWindow::GuiWindow;

			virtual void renderEvent() override
			{
				glm::vec2 progress;
				progress.x = majorProgress();
				progress.y = minorProgress();
				uniformBuffer.update(&progress, &stagingBuffer);

				pWindow->perpareFrame();

				pWindow->m_uiFramebuffer = renderer->vkFramebuffer[pWindow->m_imageIndex];
				lockUi();
				ImGui::Begin("StartUp", nullptr, ImVec2(0, 0), 0.3f, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
				ImGui::TextUnformatted(majorProgressText().c_str());
				ImGui::TextUnformatted(minorProgressText().c_str());
				ImGui::End();
				unlockUi();

				vk::queueSubmit(pWindow->m_imageAvailable, renderFinishedSemaphore, cmd[pWindow->m_imageIndex]);

				pWindow->endFrame(renderFinishedSemaphore);

				Sleep(10);
			}
		};

		void run()
		{
			static ResourceBank _resources;

			if (state == State::eRunning) return;

			static bool first = true;
			if (first)
			{
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
				pipeline.create("../pipeline/startUp/startUp.xml", &zeroVertexInputState, renderer->vkRenderPass, 0);
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

			reportMajorProgress(0.f);
			reportMinorProgress(0.f);
			state = State::eRunning;
			pWindow->show();
		}

		void complete()
		{
			if (state != State::eRunning) return;
			reportMajorProgress(1.f);
			reportMinorProgress(1.f);
			delete pWindow;
			tke::currentWindow = nullptr;
			state = State::eReady;
		}
	}
}
