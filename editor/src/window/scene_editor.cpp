#include "../../../src/ui/ui.h"
#include "../../../src/render/synchronization.h"

#include "../select.h"
#include "../editor.h"
#include "scene_editor.h"

std::string SceneEditorClass::getName()
{
	return "scene editor";
}

Window *SceneEditorClass::load(tke::AttributeTreeNode *n)
{
	auto a = n->firstAttribute("filename");
	if (a)
	{
		auto s = tke::getScene(a->value);
		if (s)
		{
			auto w = new SceneEditor(s);
			a = n->firstAttribute("follow");
			if (a)
				a->get(&w->follow);
			return w;
		}
	}
	return nullptr;
}

SceneEditorClass sceneEditorClass;

SceneEditor::SceneEditor(tke::Scene *_scene)
	:Window(&sceneEditorClass), scene(_scene)
{
	renderFinished = tke::createEvent();

	image = std::make_shared<tke::Image>(tke::resCx, tke::resCy, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
	fb_image = tke::getFramebuffer(image.get(), tke::renderPass_image8);
	tke::addUiImage(image.get());

	fb_scene = scene->createFramebuffer(image.get());
	scene_renderFinished = tke::createEvent();

	cb_physx = new tke::CommandBuffer();
	physx_renderFinished = tke::createEvent();

	cb_wireframe = new tke::CommandBuffer();
	ds_wireframe_anim = new tke::DescriptorSet(tke::pipeline_wireframe_anim);
	wireframe_renderFinished = tke::createEvent();

	VkImageView views[] = {
		image->getView(),
		tke::depthImage->getView()
	};
	fb_tool = tke::getFramebuffer(image->levels[0].cx, image->levels[0].cy, tke::renderPass_depthC_image8, ARRAYSIZE(views), views);
	transformerTool = new TransformerTool(fb_tool.get());

	cbs.push_back(scene->cb_shadow->v);
	cbs.push_back(scene->cb_deferred->v);
	cbs.push_back(cb_physx->v);
	cbs.push_back(cb_wireframe->v);
	cbs.push_back(transformerTool->cb->v);
}

SceneEditor::~SceneEditor()
{
	tke::destroyEvent(renderFinished);

	tke::destroyEvent(scene_renderFinished);

	tke::removeUiImage(image.get());

	delete cb_physx;
	tke::destroyEvent(physx_renderFinished);

	delete cb_wireframe;
	delete ds_wireframe_anim;
	tke::destroyEvent(wireframe_renderFinished);

	delete transformerTool;
}

void draw_pickup_frame(tke::CommandBuffer *cb, void *user_data)
{
	auto scene = (tke::Scene*)user_data;
	std::vector<VkWriteDescriptorSet> writes;
	for (int i = 0; i < scene->objects.size(); i++)
	{
		auto object = scene->objects[i].get();
		auto model = object->model;
		auto animated = model->animated;
		cb->bindVertexBuffer(animated ? tke::animatedVertexBuffer : tke::staticVertexBuffer);
		cb->bindIndexBuffer(animated ? tke::animatedIndexBuffer : tke::staticIndexBuffer);
		cb->bindPipeline(animated ? tke::pipeline_plain_anim : tke::pipeline_plain);
		if (animated)
		{
			writes.push_back(tke::pipeline_plain_anim->descriptorSet->bufferWrite(0, 0, object->animationComponent->boneMatrixBuffer));
			cb->bindDescriptorSet();
		}
		struct
		{
			glm::mat4 modelview;
			glm::mat4 proj;
			glm::vec4 color;
		}data;
		data.proj = tke::matPerspective;
		data.modelview = scene->camera.getMatInv() * object->getMat();
		data.color = glm::vec4((i + 1) / 255.f, 0.f, 0.f, 0.f);
		cb->pushConstant(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(data), &data);
		cb->drawIndex(model->indices.size(), model->indiceBase, model->vertexBase);
	}
	tke::updateDescriptorSets(writes.size(), writes.data());
}

void SceneEditor::show()
{
	ImGui::Begin(("Scene - " + scene->name).c_str(), &opened, ImGuiWindowFlags_MenuBar);

	ImGui::BeginMenuBar();
	if (ImGui::BeginMenu("File"))
	{
		if (ImGui::MenuItem("Save", "Ctrl+S"))
			scene->save(scene->filename);

		ImGui::EndMenu();
	}
	bool openCreateLightPopup = false;
	bool openCreateObjectPopup = false;
	bool openCreateTerrainPopup = false;
	bool openCreateWaterPopup = false;
	if (ImGui::BeginMenu("Create"))
	{
		if (ImGui::MenuItem("Light"))
			openCreateLightPopup = true;
		if (ImGui::MenuItem("Object"))
			openCreateObjectPopup = true;
		if (ImGui::MenuItem("Terrain", "", nullptr, !scene->terrain))
			openCreateTerrainPopup = true;
		if (ImGui::MenuItem("Water"))
			openCreateWaterPopup = true;

		ImGui::EndMenu();
	}
	if (openCreateLightPopup)
		ImGui::OpenPopup("Create Light");
	if (openCreateObjectPopup)
		ImGui::OpenPopup("Create Object");
	if (openCreateTerrainPopup)
		ImGui::OpenPopup("Create Terrain");
	if (openCreateWaterPopup)
		ImGui::OpenPopup("Create Water");
	if (ImGui::BeginPopupModal("Create Object", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (ocs.modelIndex >= tke::models.size())
			ocs.modelIndex = 0;
		if (ImGui::Combo("Model", &ocs.modelIndex, [](void *data, int idx, const char **out_text) {
			*out_text = tke::models[idx]->filename.c_str();
			return true;
		}, nullptr, tke::models.size()));

		ImGui::Checkbox("Use Camera Position", &ocs.use_camera_position);
		if (ocs.use_camera_position)
			ImGui::Checkbox("Use Camera Target Position", &ocs.use_camera_target_position);
		for (int i = 0; i < 3; i++)
		{
			if (!ocs.use_camera_position)
			{
				char *strs[] = {"CoordX", "CoordY", "CoordZ"};
				ImGui::DragFloat(strs[i], &ocs.coord[i], 0.5f);
			}
			else
			{
				char *strs[] = {"%f CoordX", "%f CoordY", "%f CoordZ"};
				if (!ocs.use_camera_target_position)
					ImGui::Text(strs[i], scene->camera.getCoord()[i]);
				else
					ImGui::Text(strs[i], scene->camera.target[i]);
			}
		}

		for (int i = 0; i < 3; i++)
		{
			char *strs0[] = {"EulerX", "EulerY", "EulerZ"};
			ImGui::DragFloat(strs0[i], &ocs.euler[i], 0.5f);
		}

		for (int i = 0; i < 3; i++)
		{
			char *strs0[] = {"ScaleX", "ScaleY", "ScaleZ"};
			ImGui::DragFloat(strs0[i], &ocs.scale[i], 0.5f);
		}

		static const char *physxTypeNames[] = {
			"Null",
			"Static",
			"Dynamic"
		};
		ImGui::Combo("Physx Type", &ocs.physxType, physxTypeNames, TK_ARRAYSIZE(physxTypeNames));
		static bool use_controller;
		if (ocs.physxType != 0)
			ImGui::Checkbox("Use Controller", &use_controller);

		if (ImGui::Button("Create"))
		{
			auto _physxType = tke::ObjectPhysicsType::null;
			if (ocs.physxType != 0)
			{
				_physxType = tke::ObjectPhysicsType(1 << (ocs.physxType - 1));
				if (use_controller)
					_physxType = tke::ObjectPhysicsType((int)_physxType | (int)tke::ObjectPhysicsType::controller);
			}
			auto o = new tke::Object(tke::models[ocs.modelIndex].get(), _physxType);

			glm::vec3 _coord;
			if (ocs.use_camera_position)
			{
				if (ocs.use_camera_target_position)
					_coord = scene->camera.target;
				else
					_coord = scene->camera.getCoord();
			}
			else
				_coord = ocs.coord;
			o->setCoord(_coord);
			o->setEuler(ocs.euler);
			o->setScale(ocs.scale);

			scene->addObject(o);
		}
		ImGui::SameLine();
		if (ImGui::Button("Close"))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}
	if (ImGui::BeginPopupModal("Create Terrain", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		//if (tcs.heightMapIndex >= tke::textures.size())
		//	tcs.heightMapIndex = 0;
		//if (tcs.blendMapIndex >= tke::textures.size())
		//	tcs.blendMapIndex = 0;
		//if (tcs.colorMap0Index >= tke::textures.size())
		//	tcs.colorMap0Index = 0;
		//if (tcs.colorMap1Index >= tke::textures.size())
		//	tcs.colorMap1Index = 0;
		//if (tcs.colorMap2Index >= tke::textures.size())
		//	tcs.colorMap2Index = 0;
		//if (tcs.colorMap3Index >= tke::textures.size())
		//	tcs.colorMap3Index = 0;
		//for (int i = 0; i < 3; i++)
		//{
		//	char *strs[] = {"CoordX", "CoordY", "CoordZ"};
		//	ImGui::DragFloat(strs[i], &tcs.coord[i], 0.5f);
		//}
		//if (tke::textures.size() > 0)
		//{
		//	if (ImGui::Combo("Height Map", &tcs.heightMapIndex, [](void *data, int idx, const char **out_text) {
		//		*out_text = tke::textures[idx]->filename.c_str();
		//		return true;
		//	}, nullptr, tke::textures.size()));
		//	if (ImGui::Combo("Blend Map", &tcs.blendMapIndex, [](void *data, int idx, const char **out_text) {
		//		*out_text = tke::textures[idx]->filename.c_str();
		//		return true;
		//	}, nullptr, tke::textures.size()));
		//	if (ImGui::Combo("Color Map 0", &tcs.colorMap0Index, [](void *data, int idx, const char **out_text) {
		//		*out_text = tke::textures[idx]->filename.c_str();
		//		return true;
		//	}, nullptr, tke::textures.size()));
		//	if (ImGui::Combo("Color Map 1", &tcs.colorMap1Index, [](void *data, int idx, const char **out_text) {
		//		*out_text = tke::textures[idx]->filename.c_str();
		//		return true;
		//	}, nullptr, tke::textures.size()));
		//	if (ImGui::Combo("Color Map 2", &tcs.colorMap2Index, [](void *data, int idx, const char **out_text) {
		//		*out_text = tke::textures[idx]->filename.c_str();
		//		return true;
		//	}, nullptr, tke::textures.size()));
		//	if (ImGui::Combo("Color Map 3", &tcs.colorMap3Index, [](void *data, int idx, const char **out_text) {
		//		*out_text = tke::textures[idx]->filename.c_str();
		//		return true;
		//	}, nullptr, tke::textures.size()));
		//}
		//ImGui::DragFloat("Height", &tcs.height);
		//ImGui::Checkbox("Use Physx", &tcs.usePhysx);
		//if (tke::textures.size() > 0)
		//{
		//	if (ImGui::Button("Create"))
		//	{
		//		auto t = new tke::Terrain(tcs.usePhysx, tke::textures[tcs.heightMapIndex].get(), nullptr, tke::textures[tcs.blendMapIndex].get(), tke::textures[tcs.colorMap0Index].get(), tke::textures[tcs.colorMap1Index].get(), tke::textures[tcs.colorMap2Index].get(), tke::textures[tcs.colorMap3Index].get());
		//		t->setCoord(tcs.coord);
		//		t->height = tcs.height;
		//		scene->addTerrain(t);
		//		ImGui::CloseCurrentPopup();
		//	}
		//	ImGui::SameLine();
		//}
		if (ImGui::Button("Close"))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}
	if (ImGui::BeginPopupModal("Create Water", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		for (int i = 0; i < 3; i++)
		{
			char *strs[] = {"CoordX", "CoordY", "CoordZ"};
			ImGui::DragFloat(strs[i], &wcs.coord[i], 0.5f);
		}
		ImGui::DragFloat("Height", &wcs.height);
		if (ImGui::Button("Create"))
		{
			auto w = new tke::Water;
			w->setCoord(wcs.coord);
			w->height = wcs.height;
			scene->addWater(w);
		}
		ImGui::SameLine();
		if (ImGui::Button("Close"))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}
	if (ImGui::BeginMenu("Edit"))
	{
		if (ImGui::MenuItem("Cut", "Ctrl+X"))
			;
		if (ImGui::MenuItem("Copy", "Ctrl+C"))
			;
		if (ImGui::MenuItem("Paste", "Ctrl+V"))
			;
		if (ImGui::MenuItem("Delete", "Del"))
			;

		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Show"))
	{
		ImGui::MenuItem("Show Selected Wire Frame", "", &showSelectedWireframe);
		if (ImGui::MenuItem("View Physx", "", &viewPhysx))
		{
			if (viewPhysx)
			{
				scene->pxScene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, 1.f);
				////scene->setVisualizationParameter(PxVisualizationParameter::eACTOR_AXES, 1.f);
				scene->pxScene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_SHAPES, 1.f);
				//scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_DYNAMIC, 1.f);
				//pxScene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LOCAL_FRAMES, 1.f);
				////scene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LIMITS, 1.f);
			}
			else
				scene->pxScene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, 0.f);
		}

		ImGui::EndMenu();
	}
	bool target = false;
	if (ImGui::BeginMenu("Camera"))
	{
		if (ImGui::MenuItem("Target To Selected"))
			target = true;
		ImGui::MenuItem("Follow", "", &follow);

		ImGui::EndMenu();
	}
	if (target || follow)
		scene->camera.object = selectedItem.toObject();
	static glm::vec2 sun_dir;
	bool openSunDirPopup = false;
	if (ImGui::BeginMenu("Sky"))
	{
		if (ImGui::MenuItem("Sun Dir"))
		{
			openSunDirPopup = true;
			sun_dir = scene->sunDir;
		}

		auto ambientColor = scene->ambientColor;
		if (ImGui::DragFloat3("Ambient Color", &ambientColor[0], 0.1f, 0.f, 100.f))
			scene->setAmbientColor(ambientColor);
		auto fogColor = scene->fogColor;
		if (ImGui::DragFloat3("Fog Color", &fogColor[0], 0.1f, 0.f, 100.f))
			scene->setFogColor(fogColor);

		ImGui::EndMenu();
	}
	ImGui::EndMenuBar();
	if (openSunDirPopup)
		ImGui::OpenPopup("Sun Dir");
	if (ImGui::BeginPopupModal("Sun Dir", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::DragFloat("x", &sun_dir[0]);
		ImGui::DragFloat("y", &sun_dir[1]);
		if (ImGui::Button("Ok"))
		{
			scene->setSunDir(sun_dir);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}

	ImVec2 image_pos = ImGui::GetCursorScreenPos();
	ImVec2 image_size = ImVec2(image->levels[0].cx, image->levels[0].cy);
	ImGui::InvisibleButton("canvas", image_size);
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	draw_list->AddImage(ImTextureID(image->index), image_pos, image_pos + image_size);
	if (ImGui::IsItemHovered())
	{
		if (tke::mouseDispX != 0 || tke::mouseDispY != 0)
		{
			auto distX = (float)tke::mouseDispX / (float)tke::resCx;
			auto distY = (float)tke::mouseDispY / (float)tke::resCy;
			if (tke::keyStates[VK_SHIFT].pressing && tke::mouseMiddle.pressing)
				scene->camera.moveByCursor(distX, distY);
			else if (tke::keyStates[VK_CONTROL].pressing && tke::mouseMiddle.pressing)
				scene->camera.scroll(distX);
			else if (tke::mouseMiddle.pressing)
				scene->camera.rotateByCursor(distX, distY);
			else if (!tke::keyStates[VK_SHIFT].pressing && !tke::keyStates[VK_CONTROL].pressing)
			{
				if (tke::mouseLeft.pressing)
					transformerTool->mouseMove(tke::mouseDispX, tke::mouseDispY);
			}
		}
		if (tke::mouseLeft.justDown)
		{
			if (!tke::keyStates[VK_SHIFT].pressing && !tke::keyStates[VK_CONTROL].pressing)
			{
				auto x = tke::mouseX - image_pos.x;
				auto y = tke::mouseY - image_pos.y;
				if (!transformerTool->leftDown(x, y))
				{
					auto index = tke::pickUp(x, y, draw_pickup_frame, scene);
					if (index == 0)
						selectedItem.reset();
					else
						selectedItem.select(scene->objects[index - 1].get());
				}
			}
		}
	}

	{
		//char *names[] = {
		//	ICON_FA_MOUSE_POINTER, "M", "R", "S"
		//};
		for (int i = 0; i < 4; i++)
		{
			auto needPopup = false;
			if (transformerTool->mode == TransformerTool::Mode(i))
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(0.f, 0.6f, 0.6f).Value);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(0.f, 0.7f, 0.7f).Value);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(0.f, 0.8f, 0.8f).Value);
				needPopup = true;
			}
			//if (ImGui::Button(names[i])) transformerTool->mode = TransformerTool::Mode(i);
			if (needPopup) ImGui::PopStyleColor(3);
			if (i < 3) ImGui::SameLine();
		}
	}

	ImGui::SameLine();
	ImGui::BeginGroup();
	ImGui::BeginChild("right", ImVec2(500, 0));
	if (ImGui::TreeNode(("Lights - " + std::to_string(scene->lights.size())).c_str()))
	{
		ImGui::TreePop();
	}
	if (ImGui::TreeNode(("Objects - " + std::to_string(scene->objects.size())).c_str()))
	{
		for (int i = 0; i < scene->objects.size(); i++)
		{
			auto o = scene->objects[i].get();
			if (ImGui::Selectable(std::to_string(i).c_str(), selectedItem.toObject() == o))
				selectedItem.select(o);
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode(("Terrain - " + std::to_string(scene->terrain ? 1 : 0)).c_str()))
	{
		auto terrain = scene->terrain.get();

		if (terrain)
		{
			ImGui::Text("Height Map:%s", terrain->heightMap->filename.c_str());
			ImGui::Text("Color Map 0:%s", terrain->colorMaps[0]->filename.c_str());
			ImGui::Text("Color Map 1:%s", terrain->colorMaps[1]->filename.c_str());
			ImGui::Text("Color Map 2:%s", terrain->colorMaps[2]->filename.c_str());
			ImGui::Text("Color Map 3:%s", terrain->colorMaps[3]->filename.c_str());
			ImGui::Text("Height:%f", terrain->height);
			ImGui::Text("Use Physx:%s", terrain->use_physx ? "Yse" : "No");
			if (ImGui::Button("Remove Terrain"))
				scene->removeTerrain();
		}

		ImGui::TreePop();
	}
	if (ImGui::TreeNode(("Waters - " + std::to_string(scene->waters.size())).c_str()))
	{

		ImGui::TreePop();
	}

	auto obj = selectedItem.toObject();
	if (obj)
	{
		obj->setState(tke::Controller::State::forward, tke::keyStates[VK_UP].pressing);
		obj->setState(tke::Controller::State::backward, tke::keyStates[VK_DOWN].pressing);
		obj->setState(tke::Controller::State::left, tke::keyStates[VK_LEFT].pressing);
		obj->setState(tke::Controller::State::right, tke::keyStates[VK_RIGHT].pressing);
	}
	
	ImGui::Separator();
	if (selectedItem)
	{
		switch (selectedItem.type)
		{
		case ItemTypeObject:
		{
			ImGui::TextUnformatted("Selected:Object");

			auto o = selectedItem.toObject();

			auto modelName = tke::translate(936, CP_UTF8, o->model->filename.c_str());
			ImGui::Text("model:%s", modelName.c_str());

			auto coord = o->getCoord();
			if (ImGui::DragFloat3("coord", &coord[0]))
				o->setCoord(coord);
			auto euler = o->getEuler();
			if (ImGui::DragFloat3("euler", &euler[0]))
				o->setEuler(euler);
			auto scale = o->getScale();
			if (ImGui::DragFloat3("scale", &scale[0]))
				o->setScale(scale);

			ImGui::DragFloat("ang offset", &o->ang_offset);
			ImGui::DragFloat("speed", &o->speed);
			ImGui::DragFloat("turn speed", &o->turn_speed);

			if (o->model->animated)
			{
				static int boneID = -1;
				if (boneID >= o->model->bones.size()) boneID = -1;

				if (ImGui::TreeNode("Bones Motion"))
				{
					for (int i = 0; i < o->model->bones.size(); i++)
					{
						auto str = tke::translate(936, CP_UTF8, o->model->bones[i].name);
						if (ImGui::Selectable(str.c_str(), i == boneID))
							boneID = i;
					}

					ImGui::TreePop();
				}
			}
		}
			break;
		}
	}
	else
		ImGui::TextUnformatted("Select:Null");
	ImGui::EndChild();
	ImGui::EndGroup();

	ImGui::End();

	scene->show(fb_scene.get(), scene_renderFinished);

	{ // view physx
		cb_physx->reset();
		cb_physx->begin();

		cb_physx->waitEvents(1, &scene_renderFinished);

		if (viewPhysx)
		{
			cb_physx->beginRenderPass(tke::renderPass_image8, fb_image.get());

			auto &rb = scene->pxScene->getRenderBuffer();

			//for (int i = 0; i < rb.getNbPoints(); i++)
			//{
			//	p.color = _intToRGB(rb.getPoints()[i].color);
			//	p.coord = _pxVec3ToVec3(rb.getPoints()[i].pos);
			//	tke3_debugBuffer.points.push_back(p);
			//}

			auto lineCount = rb.getNbLines();
			if (lineCount > 0)
			{
				auto vertexSize = lineCount * 2 * sizeof(tke::VertexLine);
				if (!physxBuffer || physxBuffer->size < vertexSize)
				{
					if (physxBuffer) delete physxBuffer;
					physxBuffer = new tke::OnceVertexBuffer(vertexSize);
				}

				{
					auto map = physxBuffer->map(0, vertexSize);
					auto vtx_dst = (tke::VertexLine*)map;
					for (int i = 0; i < lineCount; i++)
					{
						auto &line = rb.getLines()[i];					
						vtx_dst[0].position.x = line.pos0.x;
						vtx_dst[0].position.y = line.pos0.y;
						vtx_dst[0].position.z = line.pos0.z;
						vtx_dst[0].color.r = line.color0 % 256;
						vtx_dst[0].color.g = (line.color0 / 256) % 256;
						vtx_dst[0].color.b = (line.color0 / 65536) % 256;
						vtx_dst[1].position.x = line.pos1.x;
						vtx_dst[1].position.y = line.pos1.y;
						vtx_dst[1].position.z = line.pos1.z;
						vtx_dst[1].color.r = line.color1 % 256;
						vtx_dst[1].color.g = (line.color1 / 256) % 256;
						vtx_dst[1].color.b = (line.color1 / 65536) % 256;
						vtx_dst += 2;
					}
					physxBuffer->unmap();
				}

				cb_physx->bindVertexBuffer(physxBuffer);
				cb_physx->bindPipeline(tke::pipeline_lines);

				glm::mat4 mvp = tke::matPerspective * scene->camera.getMatInv();
				cb_physx->pushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &mvp);
				cb_physx->draw(lineCount * 2);
			}

			//glNamedBufferSubData(tke_dynamicVertexVBO, 0, sizeof(GLfloat) * 12 * lineCount, tke_dynamicVertexBuffer);
			//glBindVertexArray(tke3_dynamicVertexVAO);
			//glDrawArrays(GL_LINES, 0, lineCount * 2);

			//auto triangleCount = rb.getNbTriangles();
			//for (int i = 0; i < triangleCount; i++)
			//{
			//	auto &triangle = rb.getTriangles()[i];
			//	tke_dynamicVertexBuffer[i * 18 + 0] = triangle.pos0.x;
			//	tke_dynamicVertexBuffer[i * 18 + 1] = triangle.pos0.y;
			//	tke_dynamicVertexBuffer[i * 18 + 2] = triangle.pos0.z;
			//	tke_dynamicVertexBuffer[i * 18 + 3] = triangle.color0 % 256;
			//	tke_dynamicVertexBuffer[i * 18 + 4] = (triangle.color0 / 256) % 256;
			//	tke_dynamicVertexBuffer[i * 18 + 5] = (triangle.color0 / 65536) % 256;
			//	tke_dynamicVertexBuffer[i * 18 + 6] = triangle.pos1.x;
			//	tke_dynamicVertexBuffer[i * 18 + 7] = triangle.pos1.y;
			//	tke_dynamicVertexBuffer[i * 18 + 8] = triangle.pos1.z;
			//	tke_dynamicVertexBuffer[i * 18 + 9] = triangle.color1 % 256;
			//	tke_dynamicVertexBuffer[i * 18 + 10] = (triangle.color1 / 256) % 256;
			//	tke_dynamicVertexBuffer[i * 18 + 11] = (triangle.color1 / 65536) % 256;
			//	tke_dynamicVertexBuffer[i * 18 + 12] = triangle.pos2.x;
			//	tke_dynamicVertexBuffer[i * 18 + 13] = triangle.pos2.y;
			//	tke_dynamicVertexBuffer[i * 18 + 14] = triangle.pos2.z;
			//	tke_dynamicVertexBuffer[i * 18 + 15] = triangle.color2 % 256;
			//	tke_dynamicVertexBuffer[i * 18 + 16] = (triangle.color2 / 256) % 256;
			//	tke_dynamicVertexBuffer[i * 18 + 17] = (triangle.color2 / 65536) % 256;
			//}
			//glNamedBufferSubData(tke_dynamicVertexVBO, 0, sizeof(GLfloat) * 18 * triangleCount, tke_dynamicVertexBuffer);
			//glBindVertexArray(tke3_dynamicVertexVAO);
			//glDrawArrays(GL_TRIANGLES, 0, triangleCount * 3);

			cb_physx->endRenderPass();
		}

		cb_physx->resetEvent(scene_renderFinished);
		cb_physx->setEvent(physx_renderFinished);

		cb_physx->end();
	}

	{ // draw wireframe
		cb_wireframe->reset();
		cb_wireframe->begin();

		cb_wireframe->waitEvents(1, &physx_renderFinished);

		auto obj = selectedItem.toObject();
		if (obj)
		{
			auto model = obj->model;
			auto animated = model->animated;

			cb_wireframe->beginRenderPass(tke::renderPass_image8, fb_image.get());

			struct
			{
				glm::mat4 modelview;
				glm::mat4 proj;
				glm::vec4 color;
			}pc;
			pc.proj = tke::matPerspective;

			if (showSelectedWireframe)
			{
				cb_wireframe->bindVertexBuffer(animated ? tke::animatedVertexBuffer : tke::staticVertexBuffer);
				cb_wireframe->bindIndexBuffer(animated ? tke::animatedIndexBuffer : tke::staticIndexBuffer);
				cb_wireframe->bindPipeline(animated ? tke::pipeline_wireframe_anim : tke::pipeline_wireframe);
				if (animated)
				{
					if (last_obj != obj)
						tke::updateDescriptorSets(1, &ds_wireframe_anim->bufferWrite(0, 0, obj->animationComponent->boneMatrixBuffer));
					cb_wireframe->bindDescriptorSet(&ds_wireframe_anim->v);
				}
				pc.modelview = scene->camera.getMatInv() * obj->getMat();
				pc.color = glm::vec4(0.f, 1.f, 0.f, 1.f);
				cb_wireframe->pushConstant(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pc), &pc);
				cb_wireframe->drawModel(model);
			}

			cb_wireframe->endRenderPass();
		}
		last_obj = obj;

		cb_wireframe->resetEvent(physx_renderFinished);
		cb_wireframe->setEvent(wireframe_renderFinished);

		cb_wireframe->end();
	}

	transformerTool->transformer = selectedItem.toTransformer();
	transformerTool->show(&scene->camera, wireframe_renderFinished, renderFinished);

	tke::cbs.insert(tke::cbs.begin(), cbs.begin(), cbs.end());
	tke::ui_waitEvents.push_back(renderFinished);
}

void SceneEditor::save(tke::AttributeTreeNode *n)
{
	n->addAttribute("filename", scene->filename);
	n->addAttribute("follow", &follow);
}
