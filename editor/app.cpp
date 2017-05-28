#include "stdafx.h"

#include <iostream>
#include <memory>

#include "../src/core.h"
#include "../src/event.h"
#include "../src/scene.h"
#include "../src/gui.h"
#include "../src/render.h"
#include "../src/pickUp.h"
#include "../src/script.h"
#include "../src/image.file.h"
#include "../src/model.file.h"
#include "../src/model.general.h"

#include "select.h"
#include "history.h"
#include "tool.h"

tke::GuiWindow *pMainWindow = nullptr;

int state = 0;
tke::UniformBuffer *pasteBuffer = nullptr;

struct MainWindow : tke::GuiWindow
{
	tke::Pipeline lightFramePipeline;
	tke::Pipeline wireFramePipeline;

	tke::Image *titleImage;

	tke::Renderer *masterRenderer;
	tke::Renderer *progressRenderer;

	tke::DrawAction *miscLightFrameAction;
	tke::DrawAction *miscWireFrameLightAction;
	tke::DrawAction *miscWireFrameObjectAction;
	tke::DrawAction *miscToolAction;

	VkCommandBuffer progressCmd[2];
	VkCommandBuffer mainCmd[2];

	bool showWorldAxis = false;
	bool showSelectLine = true;
	bool showRotateAxis = false;
	bool showSky = true;
	bool showLight = true;
	bool showObject = true;
	bool showRigidBody = false;
	bool showShadow = true;
	bool showFriend = true;

	void init()
	{
		//auto terrain = new tke::Terrain;
		//terrain->heightMap = tke::createTexture("d:\\tk-sdk\\engine\\misc\\output.png", false, false);
		//terrain->patchSize = 32;
		//terrain->ext = 10.f;
		//terrain->height = 200.f;
		//terrain->tessFactor = 0.75;

		//tke::scene->addTerrain(terrain);

		int map[12][12] = {
			{ 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1 },
			{ 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 1, 1 },
			{ 1, 0, 0, 1,  0, 0, 0, 0,  0, 0, 0, 1 },
			{ 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },

			{ 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 1, 1 },
			{ 1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 0, 1 },
			{ 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
			{ 1, 0, 0, 0,  0, 0, 0, 1,  0, 0, 0, 1 },

			{ 1, 0, 1, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
			{ 1, 0, 0, 0,  0, 0, 0, 0,  1, 1, 0, 1 },
			{ 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
			{ 1, 1, 1, 0,  1, 1, 1, 1,  1, 1, 1, 1 },
		};

		for (int i = 0; i < 12; i++)
		{
			for (int j = 0; j < 12; j++)
			{
				if (map[i][j])
				{
					auto o = new tke::Object();
					o->pModel = tke::cubeModel;
					o->setCoord(glm::vec3(i, -4, j));
					tke::scene->addObject(o);
				}

				{
					auto o = new tke::Object();
					o->pModel = tke::cubeModel;
					o->setCoord(glm::vec3(i, -5, j));
					tke::scene->addObject(o);
				}
			}
		}

		masterRenderer = new tke::Renderer();		
		masterRenderer->loadXML("../renderer/master.xml");

		masterRenderer->resource.setImage(image, "Window.Image");

		masterRenderer->setup();

		tke::scene->setResources(masterRenderer);

		mainCmd[0] = tke::vk::commandPool.allocate();
		mainCmd[1] = tke::vk::commandPool.allocate();

		{
			progressRenderer = new tke::Renderer();
			progressRenderer->loadXML("../renderer/progress.xml");

			titleImage = tke::createImage("../misc/title.jpg", true);
			progressRenderer->resource.setImage(titleImage, "Paste.Texture");

			progressRenderer->resource.setImage(image, "Window.Image");

			progressRenderer->setup();

			for (int i = 0; i < 2; i++)
			{
				progressCmd[i] = tke::vk::commandPool.allocate();
				tke::vk::beginCommandBuffer(progressCmd[i]);
				progressRenderer->execute(progressCmd[i], i);
				vkCmdSetEvent(progressCmd[i], tke::renderFinished, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
				vkEndCommandBuffer(progressCmd[i]);
			}

			pasteBuffer = (tke::UniformBuffer*)progressRenderer->resource.getBuffer("Paste.UniformBuffer");

			auto list = new tke::EventList;
			list->events.push_back({
				[](int _t) {
					if (_t < 500)
					{
						float alpha = _t / 500.f;
						pasteBuffer->update(&alpha, &tke::stagingBuffer);
					}
				},
				1500
			});
			list->events.push_back({
				[](int _t) {
					float alpha = 1.f - _t / 500.f;
					pasteBuffer->update(&alpha, &tke::stagingBuffer);
				},
				500,
				[]() {
					state = 1;
				},
			});
			tke::addEventList(list);
		}
	}

	void makeMainCmd()
	{
		tke::vk::queueWaitIdle();

		for (int i = 0; i < 2; i++)
		{
			vkResetCommandBuffer(mainCmd[i], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

			tke::vk::beginCommandBuffer(mainCmd[i]);

			auto objectDrawcall = masterRenderer->findRenderPass("mrt")->findAction("1")->findDrawcall("1");
			objectDrawcall->indirect_count = tke::scene->drawCallCount;

			// TODO : FIX TERRAIN

			// TODO : FIX MISC

			masterRenderer->execute(mainCmd[i], i);

			vkCmdSetEvent(mainCmd[i], tke::renderFinished, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

			vkEndCommandBuffer(mainCmd[i]);
		}

		tke::vk::queueWaitIdle();
	}

	virtual void keyDownEvent(int key) override
	{
		if (tke::uiAcceptedKey)
			return;
		tke::Window::keyDownEvent(key);

		if (tke::scene->camera.keyDown(key))
			return;
	}

	virtual void keyUpEvent(int key) override
	{
		if (tke::uiAcceptedKey)
			return;
		tke::Window::keyUpEvent(key);

		if (tke::scene->camera.keyUp(key))
			return;
	}

	virtual void mouseEvent() override
	{
		if (tke::uiAcceptedMouse)
			return;

		if (leftDown)
		{
			if (!(GetAsyncKeyState(VK_MENU) & 0x8000))
			{
				// TODO : FIX MOUSE EVENT
				//if (!(currentTool && currentTool->mouseDown(mouseX, mouseY)))
				//{
				//	auto index = tke::pickUp(mouseX, mouseY, 1, 1, [](VkCommandBuffer cmd) {
				//		if (tke::scene->pStaticObjects.size() > 0 || tke::scene->pLights.size() > 0)
				//		{
				//			uint32_t pass;
				//			uint32_t index = 0;

				//			VkDeviceSize offsets[] = { 0 };
				//			vkCmdBindVertexBuffers(cmd, 0, 1, &tke::scene->vertexBuffer.m_buffer, offsets);
				//			vkCmdBindIndexBuffer(cmd, tke::scene->indexBuffer.m_buffer, 0, VK_INDEX_TYPE_UINT32);
				//			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, tke::plainPickUpPipeline.m_pipeline);
				//			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, tke::plainPickUpPipeline.m_pipelineLayout, 0, 1, &tke::plainPickUpPipeline.m_descriptorSet, 0, nullptr);

				//			pass = 0;
				//			vkCmdPushConstants(cmd, tke::plainPickUpPipeline.m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(uint32_t), &pass);
				//			auto lightIndex = 0;
				//			for (auto pLight : tke::scene->pLights)
				//			{
				//				vkCmdPushConstants(cmd, tke::plainPickUpPipeline.m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, sizeof(uint32_t), sizeof(uint32_t), &index);
				//				if (pLight->type == tke::Light::Type::eParallax)
				//					vkCmdDrawIndexed(cmd, tke::arrowModel->indices.size(), 1, tke::arrowModel->indiceBase, tke::arrowModel->vertexBase, lightIndex);
				//				else if (pLight->type == tke::Light::Type::ePoint)
				//					vkCmdDrawIndexed(cmd, tke::sphereModel->indices.size(), 1, tke::sphereModel->indiceBase, tke::sphereModel->vertexBase, lightIndex);
				//				lightIndex++;
				//				index++;
				//			}

				//			pass = 1;
				//			vkCmdPushConstants(cmd, tke::plainPickUpPipeline.m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(uint32_t), &pass);
				//			auto objectIndex = 0;
				//			for (auto pObject : tke::scene->pStaticObjects)
				//			{
				//				vkCmdPushConstants(cmd, tke::plainPickUpPipeline.m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, sizeof(uint32_t), sizeof(uint32_t), &index);
				//				auto pModel = pObject->pModel;
				//				vkCmdDrawIndexed(cmd, pModel->indices.size(), 1, pModel->indiceBase, pModel->vertexBase, objectIndex);
				//				objectIndex++;
				//				index++;
				//			}
				//		}
				//	});

				//	if (index == 0)
				//	{
				//		select();
				//		transformTool.m_pTransformer = nullptr;
				//	}
				//	else
				//	{
				//		index -= 1;
				//		if (index < tke::scene->pLights.size())
				//			select(tke::scene->pLights[index]);
				//		else if ((index -= tke::scene->pLights.size()) < tke::scene->pStaticObjects.size())
				//			select(tke::scene->pStaticObjects[index]);
				//		else if ((index -= tke::scene->pStaticObjects.size()) < tke::scene->pAnimatedObjects.size())
				//			select(tke::scene->pAnimatedObjects[index]);
				//		else if ((index -= tke::scene->pAnimatedObjects.size()) < tke::scene->pTerrains.size())
				//			select(tke::scene->pTerrains[index]);
				//	}
				//}
			}
		}

		if (leftUp)
		{
			if (currentTool)
				currentTool->mouseUp();
		}

		if (mouseScroll != 0)
			tke::scene->camera.scroll(mouseScroll);

		float distX = mouseX - mousePrevX;
		float distY = mouseY - mousePrevY;

		if (distX != 0 || distY != 0)
		{
			distX /= tke::resCx;
			distY /= tke::resCy;
			if (leftPressing)
			{
				if (GetAsyncKeyState(VK_MENU) & 0x8000)
				{
					tke::scene->camera.addAngAccrodingToScreen(distX, distY);
				}
				else if (currentTool)
				{
					EnterCriticalSection(&tke::scene->cs);
					currentTool->mouseMove();
					LeaveCriticalSection(&tke::scene->cs);
				}
			}
			else if (middlePressing)
			{
				if (GetAsyncKeyState(VK_MENU) & 0x8000)
					tke::scene->camera.moveAccrodingToScreen(tke::aspect, distX, distY);
			}
		}
	}

	void drawUiProgress()
	{
		ImGui::Begin("Progress", nullptr, ImVec2(0, 0), 0.3f, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
		ImGui::TextUnformatted(tke::majorProgressText().c_str());
		ImGui::TextUnformatted(tke::minorProgressText().c_str());
		ImGui::End();
	}

	void drawUiMain()
	{
		//ImGuiStyle& style = ImGui::GetStyle();
		//style.WindowRounding = 0.f;
		//ImGui::SetNextWindowPos(ImVec2(0, 19));
		//ImGui::Begin("Toolbar", nullptr, ImVec2(0, 0), 1.f, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
		//if (ImGui::ImageButton((ImTextureID)tke::editor->iconTest, (ImTextureID)tke::editor->iconTestHoverd, (ImTextureID)tke::editor->iconTestActive, ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 255)))
		//{
		//}
		//ImGui::End();
		//style.WindowRounding = 9.f;
		ImGui::SetNextWindowPos(ImVec2(0, 870));
		ImGui::Begin("Fps", nullptr, ImVec2(0, 0), 0.3f, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
		ImGui::Text("FPS:%d", getFPS());
		ImGui::SameLine();
		ImGui::InputFloat("Camera Forward Speed", &tke::scene->camera.frontSpeed);
		ImGui::End();

		tke::showDialogs();
	}

	void (MainWindow::*pDrawUi)() = &MainWindow::drawUiProgress;

	virtual void drawUi() override
	{
		(this->*pDrawUi)();
	}

	void renderProgress()
	{
		if (state == 1 && tke::majorProgress() == 100)
		{
			pRender = &MainWindow::renderMain;
			pDrawUi = &MainWindow::drawUiMain;
			return;
		}

		tke::beginFrame();

		VkCommandBuffer cmds[2] = { progressCmd[tke::imageIndex], tke::uiCmd[tke::imageIndex] };
		tke::vk::queueSubmitFence(tke::imageAvailable, 2, cmds, tke::frameDone);

		tke::endFrame();
	}

	void renderMain()
	{
		tke::scene->update();
		// TODO : FIX TOOL
		//if (currentTool) currentTool->update();

		if (tke::needRedraw)
		{
			makeMainCmd();
			tke::needRedraw = false;
		}

		tke::beginFrame();

		VkCommandBuffer cmds[2] = { mainCmd[tke::imageIndex], tke::uiCmd[tke::imageIndex] };
		tke::vk::queueSubmitFence(tke::imageAvailable, 2, cmds, tke::frameDone);

		tke::endFrame();

		//static auto lastItemActive = false;

		//if (ImGui::IsAnyItemActive())
		//{
		//	if (!lastItemActive)
		//		tke3_leftTransformerHistoryBegin();
		//	lastItemActive = true;
		//}
		//else
		//{
		//	if (lastItemActive)
		//		tke3_leftTransformerHistoryEnd();
		//	lastItemActive = false;
		//}
	}

	void (MainWindow::*pRender)() = &MainWindow::renderProgress;

	virtual void renderEvent() override
	{
		(this->*pRender)();
	}
};

int main()
{
	auto resCx = 1600, resCy = 900;
	tke::enginePath = "../";
	tke::init("TK Engine Editor", resCx, resCy);
	tke::initPickUp();
	tke::initGeneralModels();

	tke::setReporter([](const std::string &str) { 
		tke::messageDialog.add(str); 
	});

	tke::scene->camera.setMode(tke::Camera::Mode::eTargeting);
	tke::scene->camera.lookAtTarget();

	pMainWindow = new MainWindow();
	pMainWindow->create(resCx, resCy, "TK Engine Editor", true);
	((MainWindow*)pMainWindow)->init();
	pMainWindow->show();
	pMainWindow->startUiThread();

	_beginthread([](void*) {
		tke::reportMajorProgress(10);

		tke::setMajorProgressText("Init");
		tke::setMinorProgressText("");

		tke::reportMajorProgress(90);

		tke::setMajorProgressText("Finish");
		tke::reportMajorProgress(100);

	}, 0, nullptr);

	_beginthread([](void*) {
		auto pipeName = R"(\\.\pipe\tke)";

		auto hPipe = CreateNamedPipe(pipeName, PIPE_ACCESS_DUPLEX,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
			PIPE_UNLIMITED_INSTANCES, 0, 0, NMPWAIT_WAIT_FOREVER, 0);

		while (ConnectNamedPipe(hPipe, NULL))
		{
			char cmd[1024];
			DWORD length;
			ReadFile(hPipe, cmd, 1024, &length, NULL);
			printf("%s\n", cmd);
			tke::processCmdLine(cmd);

			DisconnectNamedPipe(hPipe);
		}
	}, 0, nullptr);

	_beginthread([](void*) {
		for (;;)
		{
			std::string cmd;
			std::getline(std::cin, cmd);
			tke::processCmdLine(cmd);
		}
	}, 0, nullptr);

	tke::mainLoop(pMainWindow);

	return 0;
}
