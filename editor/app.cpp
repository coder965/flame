#include "stdafx.h"

#include <iostream>
#include <memory>
#include <future>

#include "..\src\core\math.h"
#include "..\src\core\light.h"
#include "..\src\core\scene.h"
#include "..\src\core\window.h"
#include "..\src\core\gui.h"
#include "..\src\core\render.h"
#include "..\src\extension\pickUp.h"
#include "..\src\extension\script.h"
#include "..\src\extension\model.file.h"
#include "..\src\extension\model.general.h"
#include "..\src\extension\editor\editor.h"
#include "..\src\extension\startUpBoard.h"

bool showWorldAxis = false;
bool showSelectLine = true;
bool showRotateAxis = false;
bool showSky = true;
bool showLight = true;
bool showObject = true;
bool showRigidBody = false;
bool showShadow = true;
bool showFriend = true;

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

int funGetModelID(tke::Model *pModel)
{
	if (pModel == nullptr)
		return -1;
	int ID = 1;
	for (auto m : tke::scene->pModels)
	{
		if (m == pModel)
			return ID;
		ID++;
	}
	return -1;
}

tke::UI::EngineGuiWindow *pMainWindow = nullptr;

#include "dialogs\lightAttribute.h"
#include "dialogs\objectAttribute.h"
#include "dialogs\terrainAttribute.h"
#include "dialogs\scene.h"
#include "dialogs\modelEditor.h"
#include "dialogs\debug.h"
//#include "..\dialogs\image.h"

static bool sayError(tke::Err err)
{
	if (err == tke::Err::eNoErr)
		return false;
	MessageBox(NULL, tke::getErrorString(err), "Error", MB_ICONWARNING);
	return true;
}

struct MainWindow : tke::UI::EngineGuiWindow
{
	using tke::UI::EngineGuiWindow::EngineGuiWindow;
	tke::Pipeline lightFramePipeline;
	tke::Pipeline wireFramePipeline;

	tke::MasterRenderer *masterRenderer;

	tke::DrawAction *miscLightFrameAction;
	tke::DrawAction *miscWireFrameLightAction;
	tke::DrawAction *miscWireFrameObjectAction;
	tke::DrawAction *miscToolAction;

	VkCommandBuffer cmd[2];

	VkSemaphore renderFinishedSemaphore;

	static bool needRedraw;

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

		lightFramePipeline.create("../pipeline/lightFrame/lightFrame.xml", &tke::vertexInputState,
			tke::resCx, tke::resCy, masterRenderer->renderer->vkRenderPass, masterRenderer->miscPass->index);

		wireFramePipeline.create("../pipeline/wireFrame/wireFrame.xml", &tke::vertexInputState,
			tke::resCx, tke::resCy, masterRenderer->renderer->vkRenderPass, masterRenderer->miscPass->index);

		masterRenderer->renderer->getDescriptorSets();

		cmd[0] = tke::vk::allocateCommandBuffer();
		cmd[1] = tke::vk::allocateCommandBuffer();

		initUi(masterRenderer->renderer->vkRenderPass, masterRenderer->uiPass->index);

		renderFinishedSemaphore = tke::vk::createSemaphore();

		tke::initTransformTool(masterRenderer->renderer->vkRenderPass, masterRenderer->miscPass->index);
	}

	void makeCmd()
	{
		tke::vk::queueWaitIdle();

		static bool first = true;

		for (int i = 0; i < 2; i++)
		{
			if (!first) vkResetCommandBuffer(cmd[i], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

			tke::vk::beginCommandBuffer(cmd[i]);

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
			auto lightIndex = 0;
			for (auto pLight : tke::scene->pLights)
			{
				if (pLight->type == tke::Light::Type::eParallax)
					miscLightFrameAction->addDrawcall(tke::arrowModel);
				else if (pLight->type == tke::Light::Type::ePoint)
					miscLightFrameAction->addDrawcall(tke::sphereModel);
				lightIndex++;
			}
			miscWireFrameLightAction->show = false;
			miscWireFrameObjectAction->show = false;
			if (showSelectLine)
			{
				if (tke::selectType == tke::SelectType::eLight)
				{
					miscWireFrameLightAction->show = true;
					miscWireFrameLightAction->drawcalls.resize(2);
					auto pLight = tke::selectLight();
					if (pLight->type == tke::Light::Type::eParallax)
						miscWireFrameLightAction->addDrawcall(tke::arrowModel);
					else if (pLight->type == tke::Light::Type::ePoint)
						miscWireFrameLightAction->addDrawcall(tke::sphereModel);
				}
				else if (tke::selectType == tke::SelectType::eObject)
				{
					miscWireFrameObjectAction->show = true;
					miscWireFrameObjectAction->drawcalls.clear();
					auto pObject = tke::selectObject();
					auto pModel = pObject->pModel;
					miscWireFrameObjectAction->addDrawcall(pModel, 1, pObject->sceneIndex);
				}
			}
			miscToolAction->m_pRenderable = tke::currentTool;

			masterRenderer->renderer->execute(cmd[i], i);

			vkEndCommandBuffer(cmd[i]);
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
		tke::setTool(nullptr);
	}

	void menu_tool_move()
	{
		tke::transformTool.setType(tke::Transformer::Type::eMove);
		tke::setTool(&tke::transformTool);
	}

	void menu_tool_rotate()
	{
		tke::transformTool.setType(tke::Transformer::Type::eAsixRotate);
		tke::setTool(&tke::transformTool);
	}

	void menu_tool_scale()
	{
		tke::transformTool.setType(tke::Transformer::Type::eScale);
		tke::setTool(&tke::transformTool);
	}

	void menu_edit_move_to_target()
	{
		auto transformer = tke::selectTransformer();
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
		tke::select(pLight);
	}

	void menu_edit_create_object(tke::Model *pModel)
	{
		auto pObject = new tke::Object;
		pObject->pModel = pModel;
		tke::scene->addObject(pObject);
		tke::addHistory(new tke::ObjectCreationHistory(pObject, tke::ObjectCreationHistory::CreationType::eCreate));
		tke::select(pObject);
	}

	void menu_edit_duplicate()
	{
		if (tke::selectType == tke::SelectType::eNull)
		{
			//tke3_notiBoard.add("You need to select a target.");
			return;
		}
		if (tke::selectType == tke::SelectType::eLight)
		{
			auto pLight = new tke::Light(*tke::selectLight());
			tke::scene->addLight(pLight);
			tke::select(pLight);
		}
		else if (tke::selectType == tke::SelectType::eObject)
		{
			auto pObject = new tke::Object(*tke::selectObject());
			tke::scene->addObject(pObject);
			tke::select(pObject);
		}
		else if (tke::selectType == tke::SelectType::eTerrain)
		{
			auto pTerrain = new tke::Terrain(*tke::selectTerrain());
			tke::scene->addTerrain(pTerrain);
			tke::select(pTerrain);
		}

		//tke3_notiBoard.add("The target has been copied.");
	}

	void menu_edit_delete()
	{
		if (tke::selectType == tke::SelectType::eNull)
		{
			//tke3_notiBoard.add("You need to select a target.");
			return;
		}
		if (tke::selectType == tke::SelectType::eLight)
		{
			tke::select(tke::scene->deleteLight(tke::selectLight()));
		}
		else if (tke::selectType == tke::SelectType::eObject)
		{
			tke::addHistory(new tke::ObjectCreationHistory(tke::selectObject(), tke::ObjectCreationHistory::CreationType::eDelete));
			tke::select(tke::scene->deleteObject(tke::selectObject()));
		}
		else if (tke::selectType == tke::SelectType::eTerrain)
		{
			tke::select(tke::scene->deleteTerrain(tke::selectTerrain()));
		}
	}

	void menu_edit_undo()
	{
		EnterCriticalSection(&tke::scene->cs);
		tke::undo();
		LeaveCriticalSection(&tke::scene->cs);
	}

	void menu_edit_redo()
	{
		EnterCriticalSection(&tke::scene->cs);
		tke::redo();
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
					tke::select();
				if (ImGui::MenuItem("Undo", ""))
					menu_edit_undo();
				if (ImGui::MenuItem("Redo", ""))
					menu_edit_redo();
				ImGui::Separator();
				if (ImGui::MenuItem("Control this object", ""))
				{
					if (tke::selectType == tke::SelectType::eObject)
						tke::controllingObject = tke::selectObject();
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
					tke::select(pTerrain);
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
		if (pMainWindow->m_uiAcceptedKey)
			return;

		if (controllerKeyDown(&tke::scene->camera, key))
			return;

		switch (key)
		{
		case VK_ESCAPE:
			tke::select();
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
		if (m_uiAcceptedMouse)
			return;

		if (leftDown)
		{
			if (!(GetAsyncKeyState(VK_MENU) & 0x8000))
			{
				if (!(tke::currentTool && tke::currentTool->mouseDown(mouseX, mouseY)))
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
						tke::select();
						tke::transformTool.m_pTransformer = nullptr;
					}
					else
					{
						index -= 1;
						if (index < tke::scene->pLights.size())
							tke::select(tke::scene->pLights[index]);
						else if ((index -= tke::scene->pLights.size()) < tke::scene->pStaticObjects.size())
							tke::select(tke::scene->pStaticObjects[index]);
						else if ((index -= tke::scene->pStaticObjects.size()) < tke::scene->pAnimatedObjects.size())
							tke::select(tke::scene->pAnimatedObjects[index]);
						else if ((index -= tke::scene->pAnimatedObjects.size()) < tke::scene->pTerrains.size())
							tke::select(tke::scene->pTerrains[index]);
						if (doubleClick)
						{
							switch (tke::selectType)
							{
							case tke::SelectType::eLight:
								dialog_lightAttribute::addWindow(tke::selectLight());
								break;
							case tke::SelectType::eObject:
								dialog_objectAttribute::addWindow(tke::selectObject());
								break;
							case tke::SelectType::eTerrain:
								dialog_terrainAttribute::addWindow(tke::selectTerrain());
								break;
							}
						}
					}
				}
			}
		}

		if (leftUp)
		{
			if (tke::currentTool)
				tke::currentTool->mouseUp();
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
				else if (tke::currentTool)
				{
					EnterCriticalSection(&tke::scene->cs);
					tke::currentTool->mouseMove();
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

	virtual void renderEvent() override
	{
		tke::UI::lock(this);

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

		m_uiDialogs->show();

		tke::scene->update(masterRenderer);
		if (tke::currentTool) tke::currentTool->update();

		tke::scene->resetChange();

		tke::UI::unlock();

		if (needRedraw)
		{
			makeCmd();
			needRedraw = false;
		}

		perpareFrame();

		tke::vk::queueSubmit(m_imageAvailable, renderFinishedSemaphore, cmd[m_imageIndex]);

		endFrame(renderFinishedSemaphore);

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
};

bool MainWindow::needRedraw = true;

int fuck;
struct A 
{
	int *p = &fuck;
};

int main()
{
	A a;

	auto resCx = 1600, resCy = 900;
	if (sayError(tke::init("TK Engine Editor", resCx, resCy, &MainWindow::needRedraw)))
		return 0;
	tke::initPickUp();
	tke::initGeneralModels();

	pMainWindow = new MainWindow(resCx, resCy, "TK Engine Editor", WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, WS_EX_ACCEPTFILES, true);

	tke::setReporter([](const std::string &str) { pMainWindow->m_uiDialogs->messageDialog.add(str); });
	tke::setProgressReporter([](int which, float v) { tke::StartUpBoard::setProgress(which, v); });

	tke::scene->camera.setMode(tke::Camera::Mode::eTargeting);
	tke::scene->camera.lookAtTarget();

	tke::StartUpBoard::run();

	_beginthread([](void*) {
		tke::StartUpBoard::setProgress(0, 0.1f);

		tke::StartUpBoard::setText(0, "Init");
		tke::StartUpBoard::setText(1, "");

		((MainWindow*)pMainWindow)->init();

		tke::StartUpBoard::setProgress(0, 0.9f);

		tke::StartUpBoard::setText(0, "Finish");
		tke::StartUpBoard::complete();

		pMainWindow->show();
	}, 0, nullptr);

	_beginthread([](void*) {
		auto pipeName = R"(\\.\pipe\tke)";

		auto hPipe = CreateNamedPipe(pipeName, PIPE_ACCESS_DUPLEX,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
			PIPE_UNLIMITED_INSTANCES, 0, 0, NMPWAIT_WAIT_FOREVER, 0);

		while (ConnectNamedPipe(hPipe, NULL))
		{
			char buf[MAX_PATH];
			DWORD length;
			ReadFile(hPipe, buf, MAX_PATH, &length, NULL);
			printf("%s\n", buf);
			tke::processCmdLine(buf);

			DisconnectNamedPipe(hPipe);
		}
	}, 0, nullptr);

	_beginthread([](void*) {
		for (;;)
		{
			char cmd[1024];
			std::cin.getline(cmd, sizeof(cmd));
			tke::processCmdLine(cmd);
		}
	}, 0, nullptr);

	tke::mainLoop();

	return 0;
}
