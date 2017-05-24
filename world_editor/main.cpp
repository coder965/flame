#include "world_editor.h"
#include <QtWidgets/QApplication>

#include "../src/core.h"
#include "../src/gui.h"
#include "../src/event.h"
#include "../src/image.file.h"

#include <process.h>

tke::UniformBuffer pasteBuffer;

struct MainWindow : tke::GuiWindow
{
	VkCommandBuffer progressCmd[2];

	tke::Image *titleImage;

	tke::Pipeline pastePipeline;

	tke::Renderer *progressRenderer;

	void init()
	{
		{
			titleImage = tke::createImage("../misc/title.jpg", true, false);

			pasteBuffer.create(sizeof(float));

			static tke::ResourceBank _resources;
			_resources.setImage(titleImage, "Paste.Texture");
			_resources.setBuffer(&pasteBuffer, "Paste.UniformBuffer");
			_resources.setPipeline(&pastePipeline, "Paste.Pipeline");

			progressRenderer = new tke::Renderer();
			progressRenderer->filename = "../renderer/progress.xml";
			progressRenderer->loadXML();
			progressRenderer->pResource = &_resources;

			pastePipeline.pResource = &_resources;

			_resources.setImage(image, "Window.Image");

			progressRenderer->setup();

			pastePipeline.create("../pipeline/paste/paste.xml", &tke::zeroVertexInputState, progressRenderer->vkRenderPass, 0);

			progressRenderer->getDescriptorSets();

			for (int i = 0; i < 2; i++)
			{
				progressCmd[i] = tke::vk::commandPool.allocate();
				tke::vk::beginCommandBuffer(progressCmd[i]);
				progressRenderer->execute(progressCmd[i], i);
				vkCmdSetEvent(progressCmd[i], tke::renderFinished, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
				vkEndCommandBuffer(progressCmd[i]);
			}

			auto list = new tke::EventList;
			tke::Event e0;
			e0.tickFunc = [](int _t) {
				if (_t < 500)
				{
					float alpha = _t / 500.f;
					pasteBuffer.update(&alpha, &tke::stagingBuffer);
				}
			};
			e0.duration = 1500;
			list->events.push_back(e0);
			//tke::Event e1;
			//e1.tickFunc = [](int _t) {
			//	float alpha = 1.f - _t / 500.f;
			//	pasteBuffer.update(&alpha, &tke::stagingBuffer);
			//};
			//e1.duration = 500;
			//list->events.push_back(e1);
			tke::addEventList(list);
		}
	}

	virtual void drawUi() override
	{
		ImGui::Begin("Progress", nullptr, ImVec2(0, 0), 0.3f, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
		ImGui::TextUnformatted(tke::majorProgressText().c_str());
		ImGui::TextUnformatted(tke::minorProgressText().c_str());
		ImGui::End();
	}
	virtual void renderEvent() override
	{
		tke::beginFrame();

		VkCommandBuffer cmds[2] = { progressCmd[tke::imageIndex], tke::uiCmd[tke::imageIndex] };
		tke::vk::queueSubmitFence(tke::imageAvailable, 2, cmds, tke::frameDone);

		tke::endFrame();
	}
};

MainWindow *pMainWindow;

void _thread(void*)
{
	auto resCx = 1600, resCy = 900;
	tke::enginePath = "../";
	tke::init("TK Engine World Editor", resCx, resCy);

	pMainWindow = new MainWindow;
	pMainWindow->create(resCx, resCy, "TK Engine World Editor", false);
	pMainWindow->init();
	tke::currentWindow = pMainWindow;

	tke::mainLoop();
}

int main(int argc, char *argv[])
{
	_beginthread(_thread, 0, nullptr);

	QApplication a(argc, argv);
	WorldEditor w;
	w.show();

	return a.exec();
}
