#include "../src/core.h"
#include "../src/scene.h"
#include "../src/gui.h"
#include "../src/event.h"
#include "../src/model.general.h"
#include "../src/image.file.h"

struct MainWindow : tke::GuiWindow
{
	tke::MasterRenderer *masterRenderer;

	VkCommandBuffer mainCmd[2];

	void init()
	{
		masterRenderer = new tke::MasterRenderer(1600, 900, this, &tke::scene->vertexBuffer, &tke::scene->indexBuffer, &tke::scene->objectIndirectBuffer);
		mainCmd[0] = tke::vk::commandPool.allocate();
		mainCmd[1] = tke::vk::commandPool.allocate();
	}
	void makeMainCmd()
	{
		tke::vk::queueWaitIdle();

		for (int i = 0; i < 2; i++)
		{
			vkResetCommandBuffer(mainCmd[i], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

			tke::vk::beginCommandBuffer(mainCmd[i]);

			masterRenderer->skyAction->addDrawcall(tke::sphereModel);
			masterRenderer->mrtObjectDrawcall->indirect_count = tke::scene->drawCallCount;

			masterRenderer->renderer->execute(mainCmd[i], i);

			vkCmdSetEvent(mainCmd[i], tke::renderFinished, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

			vkEndCommandBuffer(mainCmd[i]);
		}
	}

	virtual void renderEvent() override
	{
		tke::scene->update(masterRenderer);

		tke::scene->resetChange();

		if (tke::needRedraw)
		{
			makeMainCmd();
			tke::needRedraw = false;
		}

		tke::beginFrame();

		VkCommandBuffer cmds[2] = { mainCmd[tke::imageIndex], tke::uiCmd[tke::imageIndex] };
		tke::vk::queueSubmitFence(tke::imageAvailable, 2, cmds, tke::frameDone);

		tke::endFrame();
	}
};

int main(int argc, char** argv)
{
	auto resCx = 1600, resCy = 900;
	tke::init("Monster Card Game", resCx, resCy);
	tke::initGeneralModels();

	auto pMainWindow = new MainWindow;
	pMainWindow->create(resCx, resCy, "Monster Card Game", true);
	pMainWindow->init();
	tke::currentWindow = pMainWindow;

	ShowWindow(pMainWindow->hWnd, SW_NORMAL);

	tke::mainLoop();

	return 0;
}

