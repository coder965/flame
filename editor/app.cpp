#include "stdafx.h"

#include <iostream>
#include <memory>

#include "..\src\core\core.h"
#include "..\src\core\scene.h"
#include "..\src\core\window.h"
#include "..\src\core\gui.h"
#include "..\src\core\render.h"
#include "..\src\extension\pickUp.h"
#include "..\src\extension\script.h"
#include "..\src\extension\image.file.h"
#include "..\src\extension\model.file.h"
#include "..\src\extension\model.general.h"

#include "select.h"
#include "history.h"
#include "tool.h"

int funGetAnimID(tke::Model *pModel, tke::Animation *pAnim)
{
	if (pAnim == nullptr)
		return -1;
	int ID = 1;
	for (auto a : pModel->animations)
	{
		if (a == pAnim)
			return ID;
		ID++;
	}
	return -1;
}

bool funGetAnimName(void *_pModel, int index, const char **out_text) 
{
	if (index == 0)
	{
		*out_text = "none";
		return true;
	}
	auto pModel = (tke::Model*)_pModel;
	*out_text = pModel->animations[index - 1]->pTemplate->name.c_str();
	return true;
}

tke::Animation *funGetAnim(tke::Model *pModel, int ID)
{
	if (ID == 0 || ID - 1 >= pModel->animations.size())
		return nullptr;
	return pModel->animations[ID - 1];
}

tke::GuiWindow *pMainWindow = nullptr;

#include "dialogs\lightAttribute.h"
#include "dialogs\objectAttribute.h"
#include "dialogs\terrainAttribute.h"
#include "dialogs\scene.h"
#include "dialogs\modelEditor.h"
#include "dialogs\debug.h"
//#include "dialogs\image.h"

struct MainWindow : tke::GuiWindow
{
	using tke::GuiWindow::GuiWindow;

	tke::Pipeline lightFramePipeline;
	tke::Pipeline wireFramePipeline;
	tke::Pipeline pastePipeline;

	tke::Image *titleImage;

	tke::UniformBuffer pasteBuffer;

	tke::MasterRenderer *masterRenderer;
	tke::Renderer *progressRenderer;

	tke::DrawAction *miscLightFrameAction;
	tke::DrawAction *miscWireFrameLightAction;
	tke::DrawAction *miscWireFrameObjectAction;
	tke::DrawAction *miscToolAction;

	VkCommandBuffer progressCmd[2];
	VkCommandBuffer mainCmd[2];

	VkSemaphore renderFinished;

	static bool needRedraw;

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
		//tke::Texture *heightMap = nullptr;

		//auto TERRAIN_HEIGHT_SIZE = 1024;

		//{
		//	auto data = new unsigned short[TERRAIN_HEIGHT_SIZE * TERRAIN_HEIGHT_SIZE];

		//	for (int i = 0; i < TERRAIN_HEIGHT_SIZE; i++)
		//	{
		//		for (int j = 0; j < TERRAIN_HEIGHT_SIZE; j++)
		//		{
		//			data[i * TERRAIN_HEIGHT_SIZE + j] = tk::engine->fbm2d(glm::vec2(i, j)) * 0xffff;
		//		}
		//	}

		//	heightMap = new tke::Texture;
		//	heightMap->create(TERRAIN_HEIGHT_SIZE, TERRAIN_HEIGHT_SIZE, VK_FORMAT_R16_UNORM, (unsigned char*)data, TERRAIN_HEIGHT_SIZE * TERRAIN_HEIGHT_SIZE * 2);

		//	delete[] data;
		//}

		//auto terrain = new tke::Terrain;
		//terrain->heightMap = heightMap;
		//terrain->patchSize = 32;
		//terrain->ext = 10.f;
		//terrain->height = 200.f;
		//terrain->tessFactor = 0.75;

		//tke::scene->addTerrain(terrain);

		//terrain->heightMap = tke::createTexture("d:\\tk-sdk\\engine\\misc\\output.png", false, false);
		//tke::createTexture("d:\\tk-sdk\\engine\\misc\\1.bmp", false, false);
		//tke::createTexture("d:\\tk-sdk\\engine\\misc\\start_up.jpg", false, false);

		masterRenderer = new tke::MasterRenderer(1600, 900, this, &tke::scene->vertexBuffer, &tke::scene->indexBuffer, &tke::scene->objectIndirectBuffer);

		miscLightFrameAction = masterRenderer->miscPass->addAction(&lightFramePipeline);
		miscWireFrameLightAction = masterRenderer->miscPass->addAction(&wireFramePipeline);
		miscWireFrameObjectAction = masterRenderer->miscPass->addAction(&wireFramePipeline);
		int index;
		index = 0;
		miscWireFrameLightAction->addDrawcall(VK_SHADER_STAGE_VERTEX_BIT, &index);
		miscWireFrameLightAction->addDrawcall(VK_SHADER_STAGE_FRAGMENT_BIT, &glm::vec4(0.f, 1.f, 0.f, 1.f), 16);
		index = 1;
		miscWireFrameObjectAction->addDrawcall(VK_SHADER_STAGE_VERTEX_BIT, &index);
		miscWireFrameObjectAction->addDrawcall(VK_SHADER_STAGE_FRAGMENT_BIT, &glm::vec4(0.f, 1.f, 0.f, 1.f), 16);
		miscToolAction = masterRenderer->miscPass->addAction();
		miscToolAction->type = tke::DrawActionType::call_fuction;

		lightFramePipeline.create("../pipeline/lightFrame/lightFrame.xml", &tke::vertexInputState, masterRenderer->renderer->vkRenderPass, masterRenderer->miscPass->index);

		wireFramePipeline.create("../pipeline/wireFrame/wireFrame.xml", &tke::vertexInputState, masterRenderer->renderer->vkRenderPass, masterRenderer->miscPass->index);

		masterRenderer->renderer->getDescriptorSets();

		mainCmd[0] = tke::vk::allocateCommandBuffer();
		mainCmd[1] = tke::vk::allocateCommandBuffer();

		initUi();

		renderFinished = tke::vk::createSemaphore();

		initTransformTool(masterRenderer->renderer->vkRenderPass, masterRenderer->miscPass->index);

		{
			static tke::ResourceBank _resources;

			titleImage = tke::createImage("../misc/title.jpg", true, false);

			pasteBuffer.create(sizeof(float));

			_resources.setImage(titleImage, "Paste.Texture");
			_resources.setBuffer(&pasteBuffer, "Paste.UniformBuffer");
			_resources.setPipeline(&pastePipeline, "Paste.Pipeline");

			progressRenderer = new tke::Renderer();
			progressRenderer->filename = "../renderer/progress.xml";
			progressRenderer->loadXML();
			progressRenderer->pResource = &_resources;

			pastePipeline.pResource = &_resources;

			progressCmd[0] = tke::vk::allocateCommandBuffer();
			progressCmd[1] = tke::vk::allocateCommandBuffer();

			_resources.setImage(image, "Window.Image");

			progressRenderer->setup();

			pastePipeline.create("../pipeline/paste/paste.xml", &tke::zeroVertexInputState, progressRenderer->vkRenderPass, 0);

			progressRenderer->getDescriptorSets();

			//initUi(progressRenderer->vkRenderPass, 1);

			for (int i = 0; i < 2; i++)
			{
				tke::vk::beginCommandBuffer(progressCmd[i]);
				progressRenderer->execute(progressCmd[i], i);
				vkEndCommandBuffer(progressCmd[i]);
			}
		}
	}

	void makeMainCmd()
	{
		tke::vk::queueWaitIdle();

		static bool first = true;

		for (int i = 0; i < 2; i++)
		{
			if (!first) vkResetCommandBuffer(mainCmd[i], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

			tke::vk::beginCommandBuffer(mainCmd[i]);

			masterRenderer->skyAction->addDrawcall(tke::sphereModel);

			masterRenderer->mrtObjectDrawcall->indirect_count = tke::scene->drawCallCount;

			masterRenderer->mrtHeightMapTerrainAction->drawcalls.clear();
			auto terrainIndex = 0;
			for (auto pTerrain : tke::scene->pTerrains)
			{
				masterRenderer->mrtHeightMapTerrainAction->addDrawcall(4, 0, pTerrain->patchSize * pTerrain->patchSize, terrainIndex * 0xffff);
				terrainIndex++;
			}

			miscLightFrameAction->drawcalls.clear();
			for (auto pLight : tke::scene->pLights)
			{
				if (pLight->type == tke::Light::Type::eParallax)
					miscLightFrameAction->addDrawcall(tke::arrowModel);
				else if (pLight->type == tke::Light::Type::ePoint)
					miscLightFrameAction->addDrawcall(tke::sphereModel);
			}
			miscWireFrameLightAction->show = false;
			miscWireFrameObjectAction->show = false;
			if (showSelectLine)
			{
				if (selectType == SelectType::eLight)
				{
					miscWireFrameLightAction->show = true;
					miscWireFrameLightAction->drawcalls.resize(2);
					auto pLight = selectLight();
					if (pLight->type == tke::Light::Type::eParallax)
						miscWireFrameLightAction->addDrawcall(tke::arrowModel);
					else if (pLight->type == tke::Light::Type::ePoint)
						miscWireFrameLightAction->addDrawcall(tke::sphereModel);
				}
				else if (selectType == SelectType::eObject)
				{
					miscWireFrameObjectAction->show = true;
					miscWireFrameObjectAction->drawcalls.clear();
					auto pObject = selectObject();
					auto pModel = pObject->pModel;
					miscWireFrameObjectAction->addDrawcall(pModel, 1, pObject->sceneIndex);
				}
			}
			miscToolAction->m_pRenderable = currentTool;

			masterRenderer->renderer->execute(mainCmd[i], i);

			vkEndCommandBuffer(mainCmd[i]);
		}

		first = false;

		tke::vk::queueWaitIdle();
	}

	void menu_proj_perspective()
	{
		tke::changeProjMat(tke::PERSPECTIVE);
	}

	void menu_proj_top()
	{
		tke::changeProjMat(tke::ORTHO);
	}

	void menu_proj_front()
	{
		tke::changeProjMat(tke::ORTHO);
	}

	void menu_proj_side()
	{
		tke::changeProjMat(tke::ORTHO);
	}

	void menu_tool_select()
	{
		setTool(nullptr);
	}

	void menu_tool_move()
	{
		transformTool.setType(tke::Transformer::Type::eMove);
		setTool(&transformTool);
	}

	void menu_tool_rotate()
	{
		transformTool.setType(tke::Transformer::Type::eAsixRotate);
		setTool(&transformTool);
	}

	void menu_tool_scale()
	{
		transformTool.setType(tke::Transformer::Type::eScale);
		setTool(&transformTool);
	}

	void menu_edit_move_to_target()
	{
		auto transformer = selectTransformer();
		if (!transformer)
		{
			tke::scene->camera.setCoord(glm::vec3(0.f));
			return;
		}
		tke::scene->camera.setCoord(transformer->getCoord());
	}

	void menu_edit_create_light(tke::Light::Type type)
	{
		auto pLight = new tke::Light;
		pLight->type = type;
		tke::scene->addLight(pLight);
		select(pLight);
	}

	void menu_edit_create_object(tke::Model *pModel)
	{
		auto pObject = new tke::Object;
		pObject->pModel = pModel;
		tke::scene->addObject(pObject);
		addHistory(new ObjectCreationHistory(pObject, ObjectCreationHistory::CreationType::eCreate));
		select(pObject);
	}

	void menu_edit_duplicate()
	{
		if (selectType == SelectType::eNull)
		{
			//tke3_notiBoard.add("You need to select a target.");
			return;
		}
		if (selectType == SelectType::eLight)
		{
			auto pLight = new tke::Light(*selectLight());
			tke::scene->addLight(pLight);
			select(pLight);
		}
		else if (selectType == SelectType::eObject)
		{
			auto pObject = new tke::Object(*selectObject());
			tke::scene->addObject(pObject);
			select(pObject);
		}
		else if (selectType == SelectType::eTerrain)
		{
			auto pTerrain = new tke::Terrain(*selectTerrain());
			tke::scene->addTerrain(pTerrain);
			select(pTerrain);
		}

		//tke3_notiBoard.add("The target has been copied.");
	}

	void menu_edit_delete()
	{
		if (selectType == SelectType::eNull)
		{
			//tke3_notiBoard.add("You need to select a target.");
			return;
		}
		if (selectType ==SelectType::eLight)
		{
			select(tke::scene->deleteLight(selectLight()));
		}
		else if (selectType == SelectType::eObject)
		{
			addHistory(new ObjectCreationHistory(selectObject(), ObjectCreationHistory::CreationType::eDelete));
			select(tke::scene->deleteObject(selectObject()));
		}
		else if (selectType == SelectType::eTerrain)
		{
			select(tke::scene->deleteTerrain(selectTerrain()));
		}
	}

	void menu_edit_undo()
	{
		EnterCriticalSection(&tke::scene->cs);
		undo();
		LeaveCriticalSection(&tke::scene->cs);
	}

	void menu_edit_redo()
	{
		EnterCriticalSection(&tke::scene->cs);
		redo();
		LeaveCriticalSection(&tke::scene->cs);
	}

	void menu_connect_disconnect()
	{
		//if (!tkn_connecting)
		//{
		//	return;
		//}
		//tkn_disconnect();
		//tke3_notiBoard.add("Disconnected");
	}

	void main_menu_show()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("View"))
			{
				if (ImGui::MenuItem("Perspective", "", true))
				{
					menu_proj_perspective();
				}
				//if (ImGui::MenuItem("Top", "", false))
				//{
				//	menu_proj_top();
				//}
				//if (ImGui::MenuItem("Front", "", false))
				//{
				//	menu_proj_front();
				//}
				//if (ImGui::MenuItem("Side", "", false))
				//{
				//	menu_proj_side();
				//}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Move Camera To Target", ""))
					menu_edit_move_to_target();
				ImGui::Separator();
				if (ImGui::MenuItem("Duplicate", ""))
					menu_edit_duplicate();
				if (ImGui::MenuItem("Delete", ""))
					menu_edit_delete();
				if (ImGui::MenuItem("Cancel Select", ""))
					select();
				if (ImGui::MenuItem("Undo", ""))
					menu_edit_undo();
				if (ImGui::MenuItem("Redo", ""))
					menu_edit_redo();
				ImGui::Separator();
				if (ImGui::MenuItem("Control this object", ""))
				{
					if (selectType == SelectType::eObject)
						tke::controllingObject = selectObject();
				}
				if (ImGui::MenuItem("Release control object", ""))
					tke::controllingObject = nullptr;

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Create"))
			{
				if (ImGui::BeginMenu("Light"))
				{
					if (ImGui::MenuItem("Parallax", ""))
						menu_edit_create_light(tke::Light::Type::eParallax);
					if (ImGui::MenuItem("Point", ""))
						menu_edit_create_light(tke::Light::Type::ePoint);
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Object"))
				{
					for (auto m : tke::scene->pModels)
					{
						if (ImGui::MenuItem(m->name.c_str(), ""))
							menu_edit_create_object(m);
					}

					ImGui::EndMenu();
				}
				if (ImGui::MenuItem("Terrain"))
				{
					auto pTerrain = new tke::Terrain;
					tke::scene->addTerrain(pTerrain);
					select(pTerrain);
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Window"))
			{
				ImGui::MenuItem("Scene (F1)", "", &dialog_scene::opened);
				ImGui::MenuItem("Model Editor (F2)", "", &dialog_model_editor::opened);
				ImGui::MenuItem("Debug (F3)", "", &dialog_debug::opened);
				//if (ImGui::MenuItem("Image (F4)", "", dialog_image::isOpened()))
				//	dialog_image::addWindow();

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}


	bool controllerKeyDown(tke::Controller *pController, int key)
	{
		switch (key)
		{
		case 'W':
			pController->front = true;
			return true;
		case 'S':
			pController->back = true;
			return true;
		case 'A':
			pController->left = true;
			return true;
		case 'D':
			pController->right = true;
			return true;
		}
		return false;
	}

	bool controllerKeyUp(tke::Controller *pController, int key)
	{
		switch (key)
		{
		case 'W':
			pController->front = false;
			return true;
		case 'S':
			pController->back = false;
			return true;
		case 'A':
			pController->left = false;
			return true;
		case 'D':
			pController->right = false;
			return true;
		}
		return false;
	}

	virtual void keyDownEvent(int key) override
	{
		tke::Window::keyDownEvent(key);
		if (pMainWindow->uiAcceptedKey)
			return;

		if (controllerKeyDown(&tke::scene->camera, key))
			return;

		switch (key)
		{
		case VK_ESCAPE:
			select();
			return;
		case VK_RETURN:
			menu_edit_move_to_target();
			return;
		case VK_F1:
			menu_proj_perspective();
			return;
		case VK_F2:
			menu_proj_top();
			return;
		case VK_F3:
			menu_proj_front();
			return;
		case VK_F4:
			menu_proj_side();
			return;
		case '1':
			menu_tool_select();
			return;
		case '2':
			menu_tool_move();
			return;
		case '3':
			menu_tool_rotate();
			return;
		case '4':
			menu_tool_scale();
			return;
		case '5':
			if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
			{
				menu_edit_duplicate();
			}
			return;
		case 'Z':
			if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
				menu_edit_undo();
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
				menu_edit_redo();
			return;
		case VK_UP:
			tke::scene->camera.scroll(1);
			return;
		case VK_DOWN:
			tke::scene->camera.scroll(-1);
			return;
		case VK_DELETE:
			menu_edit_delete();
			return;
		case VK_F5:
			dialog_scene::opened = !dialog_scene::opened;
			break;
		case VK_F6:
			dialog_model_editor::opened = !dialog_model_editor::opened;
			break;
		case VK_F7:
			dialog_debug::opened = !dialog_debug::opened;
			break;
		case VK_F8:
			//dialog_image::addWindow();
			break;
		}
	}

	virtual void keyUpEvent(int key) override
	{
		tke::Window::keyUpEvent(key);
		if (controllerKeyUp(&tke::scene->camera, key))
			return;
	}

	virtual void mouseEvent() override
	{
		if (uiAcceptedMouse)
			return;

		if (leftDown)
		{
			if (!(GetAsyncKeyState(VK_MENU) & 0x8000))
			{
				if (!(currentTool && currentTool->mouseDown(mouseX, mouseY)))
				{
					auto index = tke::pickUp(mouseX, mouseY, 1, 1, [](VkCommandBuffer cmd, void*) {
						if (tke::scene->pStaticObjects.size() > 0 || tke::scene->pLights.size() > 0)
						{
							uint32_t pass;
							uint32_t index = 0;

							VkDeviceSize offsets[] = { 0 };
							vkCmdBindVertexBuffers(cmd, 0, 1, &tke::scene->vertexBuffer.m_buffer, offsets);
							vkCmdBindIndexBuffer(cmd, tke::scene->indexBuffer.m_buffer, 0, VK_INDEX_TYPE_UINT32);
							vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, tke::plainPickUpPipeline.m_pipeline);
							vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, tke::plainPickUpPipeline.m_pipelineLayout, 0, 1, &tke::plainPickUpPipeline.m_descriptorSet, 0, nullptr);

							pass = 0;
							vkCmdPushConstants(cmd, tke::plainPickUpPipeline.m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(uint32_t), &pass);
							auto lightIndex = 0;
							for (auto pLight : tke::scene->pLights)
							{
								vkCmdPushConstants(cmd, tke::plainPickUpPipeline.m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, sizeof(uint32_t), sizeof(uint32_t), &index);
								if (pLight->type == tke::Light::Type::eParallax)
									vkCmdDrawIndexed(cmd, tke::arrowModel->indices.size(), 1, tke::arrowModel->indiceBase, tke::arrowModel->vertexBase, lightIndex);
								else if (pLight->type == tke::Light::Type::ePoint)
									vkCmdDrawIndexed(cmd, tke::sphereModel->indices.size(), 1, tke::sphereModel->indiceBase, tke::sphereModel->vertexBase, lightIndex);
								lightIndex++;
								index++;
							}

							pass = 1;
							vkCmdPushConstants(cmd, tke::plainPickUpPipeline.m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(uint32_t), &pass);
							auto objectIndex = 0;
							for (auto pObject : tke::scene->pStaticObjects)
							{
								vkCmdPushConstants(cmd, tke::plainPickUpPipeline.m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, sizeof(uint32_t), sizeof(uint32_t), &index);
								auto pModel = pObject->pModel;
								vkCmdDrawIndexed(cmd, pModel->indices.size(), 1, pModel->indiceBase, pModel->vertexBase, objectIndex);
								objectIndex++;
								index++;
							}
						}
					}, nullptr);

					if (index == 0)
					{
						select();
						transformTool.m_pTransformer = nullptr;
					}
					else
					{
						index -= 1;
						if (index < tke::scene->pLights.size())
							select(tke::scene->pLights[index]);
						else if ((index -= tke::scene->pLights.size()) < tke::scene->pStaticObjects.size())
							select(tke::scene->pStaticObjects[index]);
						else if ((index -= tke::scene->pStaticObjects.size()) < tke::scene->pAnimatedObjects.size())
							select(tke::scene->pAnimatedObjects[index]);
						else if ((index -= tke::scene->pAnimatedObjects.size()) < tke::scene->pTerrains.size())
							select(tke::scene->pTerrains[index]);
						if (doubleClick)
						{
							switch (selectType)
							{
							case SelectType::eLight:
								dialog_lightAttribute::addWindow(selectLight());
								break;
							case SelectType::eObject:
								dialog_objectAttribute::addWindow(selectObject());
								break;
							case SelectType::eTerrain:
								dialog_terrainAttribute::addWindow(selectTerrain());
								break;
							}
						}
					}
				}
			}
		}

		if (leftUp)
		{
			if (currentTool)
				currentTool->mouseUp();
		}

		if (mouseScroll != 0)
			tke::scene->camera.scroll(mouseScroll);

		tke::mouseX = ((float)mouseX / tke::resCx) * 2.f - 1.f;
		tke::mouseY = ((float)mouseY / tke::resCy) * 2.f - 1.f;

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

		for (auto &dropFile : dropFiles)
		{
			auto attribute = GetFileAttributes(dropFile.c_str());
			if (!(attribute & FILE_ATTRIBUTE_DIRECTORY))
			{
				auto pModel = tke::createModel(dropFile.c_str());
				if (pModel) tke::scene->pModels.push_back(pModel);
			}
		}
	}

	void renderProgress()
	{
		float alpha;
		if (nowTime < 5000)
			alpha = nowTime / 5000.f;
		else
			alpha = 1.f;
		pasteBuffer.update(&alpha, &tke::stagingBuffer);

		if (nowTime > 3000)
			int cut = 1;

		perpareFrame();

		beginUi();
		ImGui::Begin("Progress", nullptr, ImVec2(0, 0), 0.3f, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
		//ImGui::TextUnformatted(tke::majorProgressText().c_str());
		//ImGui::TextUnformatted(tke::minorProgressText().c_str());
		ImGui::TextUnformatted("123");
		ImGui::End();
		endUi();

		tke::vk::queueSubmit(imageAvailable, renderFinished, progressCmd[imageIndex]);
		tke::vk::queueSubmit(renderFinished, tke::uiRenderFinished, tke::uiCmd);

		endFrame(tke::uiRenderFinished);

		Sleep(10);
	}

	void renderMain()
	{
		perpareFrame();

		beginUi();

		main_menu_show();
		dialog_debug::show();
		//dialog_image::show();
		dialog_lightAttribute::show();
		dialog_model_editor::show();
		dialog_objectAttribute::show();
		dialog_scene::show();
		dialog_terrainAttribute::show();

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
		}

		tke::showDialogs();

		tke::scene->update(masterRenderer);
		if (currentTool) currentTool->update();

		tke::scene->resetChange();

		endUi();

		if (needRedraw)
		{
			makeMainCmd();
			needRedraw = false;
		}

		tke::vk::queueSubmit(imageAvailable, renderFinished, mainCmd[imageIndex]);

		endFrame(renderFinished);

		Sleep(10);

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

bool MainWindow::needRedraw = true;

int main()
{
	auto resCx = 1600, resCy = 900;
	tke::init("TK Engine Editor", resCx, resCy, &MainWindow::needRedraw);
	tke::initPickUp();
	tke::initGeneralModels();

	tke::setReporter([](const std::string &str) { 
		tke::messageDialog.add(str); 
	});

	tke::scene->camera.setMode(tke::Camera::Mode::eTargeting);
	tke::scene->camera.lookAtTarget();

	pMainWindow = new MainWindow(resCx, resCy, "TK Engine Editor", WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, WS_EX_ACCEPTFILES, true);
	((MainWindow*)pMainWindow)->init();
	pMainWindow->show();

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

	tke::mainLoop();

	return 0;
}
