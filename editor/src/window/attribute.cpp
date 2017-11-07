#include "../../../src/ui/ui.h"

#include "../editor.h"
#include "../select.h"
#include "attribute.h"

static void _show_scene(tke::Scene *s)
{
	if (ImGui::TreeNode("Sky"))
	{
		static glm::vec2 sun_dir;
		if (ImGui::Button("Change Sun Dir"))
		{
			ImGui::OpenPopup("Sun Dir");
			sun_dir = s->sunDir;
		}
		if (ImGui::BeginPopupModal("Sun Dir", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::DragFloat("x", &sun_dir[0]);
			ImGui::DragFloat("y", &sun_dir[1]);
			if (ImGui::Button("Ok"))
			{
				s->setSunDir(sun_dir);
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
				ImGui::CloseCurrentPopup();

			ImGui::EndPopup();
		}

		auto ambientColor = s->ambientColor;
		if (ImGui::DragFloat3("Ambient Color", &ambientColor[0], 0.1f, 0.f, 100.f))
			s->setAmbientColor(ambientColor);
		auto fogColor = s->fogColor;
		if (ImGui::DragFloat3("Fog Color", &fogColor[0], 0.1f, 0.f, 100.f))
			s->setFogColor(fogColor);

		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Lights"))
	{
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Objects"))
	{
		if (ImGui::TreeNode("List"))
		{
			for (int i = 0; i < s->objects.size(); i++)
			{
				auto o = s->objects[i].get();
				if (ImGui::Selectable(std::to_string(i).c_str(), selectedItem.toObject() == o))
					selectedItem.select(o);
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Create"))
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
					char *strs[] = { "CoordX", "CoordY", "CoordZ" };
					ImGui::DragFloat(strs[i], &ocs.coord[i], 0.5f);
				}
				else
				{
					char *strs[] = { "%f CoordX", "%f CoordY", "%f CoordZ" };
					if (!ocs.use_camera_target_position)
						ImGui::Text(strs[i], s->camera.getCoord()[i]);
					else
						ImGui::Text(strs[i], s->camera.target[i]);
				}
				ImGui::SameLine();
				char *strs[] = { "Rand##cx", "Rand##cy", "Rand##cz" };
				ImGui::Checkbox(strs[i], &ocs.randC[i]);
			}
			ImGui::DragFloat("Coord Rand Range", &ocs.coordRandRange, 0.1f);

			for (int i = 0; i < 3; i++)
			{
				char *strs0[] = { "EulerX", "EulerY", "EulerZ" };
				char *strs1[] = { "Rand##rx", "Rand##ry", "Rand##rz" };
				ImGui::DragFloat(strs0[i], &ocs.euler[i], 0.5f);
				ImGui::SameLine();
				ImGui::Checkbox(strs1[i], &ocs.randR[i]);
			}
			ImGui::DragFloat("Euler Rand Range", &ocs.eulerRandRange, 1.f, 0.f, 360.f);

			for (int i = 0; i < 3; i++)
			{
				char *strs0[] = { "ScaleX", "ScaleY", "ScaleZ" };
				char *strs1[] = { "Rand##sx", "Rand##sy", "Rand##sz" };
				ImGui::DragFloat(strs0[i], &ocs.scale[i], 0.5f);
				ImGui::SameLine();
				ImGui::Checkbox(strs1[i], &ocs.randS[i]);
			}
			ImGui::DragFloat("Scale Rand Range", &ocs.scaleRandRange, 0.1f);
			ImGui::Checkbox("Same Scale Rand", &ocs.same_scale_rand);

			static const char *physxTypeNames[] = {
				"Null",
				"Static",
				"Dynamic"
			};
			ImGui::Combo("Physx Type", &ocs.physxType, physxTypeNames, TK_ARRAYSIZE(physxTypeNames));
			static bool use_controller;
			if (ocs.physxType != 0)
				ImGui::Checkbox("Use Controller", &use_controller);

			if (ImGui::Button("Create Object"))
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
						_coord = s->camera.target;
					else
						_coord = s->camera.getCoord();
				}
				else
				{
					_coord = ocs.coord;
				}
				for (int i = 0; i < 3; i++)
					if (ocs.randC[i]) _coord[i] += (((float)rand() / (float)RAND_MAX) - 0.5f) * 2.f * ocs.coordRandRange;
				o->setCoord(_coord);

				glm::vec3 _euler = ocs.euler;
				for (int i = 0; i < 3; i++)
					if (ocs.randR[i]) _euler[i] += (((float)rand() / (float)RAND_MAX) - 0.5f) * 2.f * ocs.eulerRandRange;
				o->setEuler(_euler);

				glm::vec3 _scale = ocs.scale;
				auto scale_rand = ((float)rand() / (float)RAND_MAX) * ocs.scaleRandRange;
				if (ocs.randS[0]) _scale.x += scale_rand;
				for (int i = 0; i < 2; i++)
					if (ocs.randS[i + 1]) _scale[i + 1] += ocs.same_scale_rand ? scale_rand : ((float)rand() / (float)RAND_MAX) * ocs.scaleRandRange;
				o->setScale(_scale);

				s->addObject(o);
			}

			ImGui::TreePop();
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Terrain"))
	{
		auto terrain = s->terrain.get();

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
				s->removeTerrain();
		}
		else
		{
			if (tcs.heightMapIndex >= tke::textures.size())
				tcs.heightMapIndex = 0;
			if (tcs.blendMapIndex >= tke::textures.size())
				tcs.blendMapIndex = 0;
			if (tcs.colorMap0Index >= tke::textures.size())
				tcs.colorMap0Index = 0;
			if (tcs.colorMap1Index >= tke::textures.size())
				tcs.colorMap1Index = 0;
			if (tcs.colorMap2Index >= tke::textures.size())
				tcs.colorMap2Index = 0;
			if (tcs.colorMap3Index >= tke::textures.size())
				tcs.colorMap3Index = 0;
			for (int i = 0; i < 3; i++)
			{
				char *strs[] = { "CoordX", "CoordY", "CoordZ" };
				ImGui::DragFloat(strs[i], &tcs.coord[i], 0.5f);
			}
			if (tke::textures.size() > 0)
			{
				if (ImGui::Combo("Height Map", &tcs.heightMapIndex, [](void *data, int idx, const char **out_text) {
					*out_text = tke::textures[idx]->filename.c_str();
					return true;
				}, nullptr, tke::textures.size()));
				if (ImGui::Combo("Blend Map", &tcs.blendMapIndex, [](void *data, int idx, const char **out_text) {
					*out_text = tke::textures[idx]->filename.c_str();
					return true;
				}, nullptr, tke::textures.size()));
				if (ImGui::Combo("Color Map 0", &tcs.colorMap0Index, [](void *data, int idx, const char **out_text) {
					*out_text = tke::textures[idx]->filename.c_str();
					return true;
				}, nullptr, tke::textures.size()));
				if (ImGui::Combo("Color Map 1", &tcs.colorMap1Index, [](void *data, int idx, const char **out_text) {
					*out_text = tke::textures[idx]->filename.c_str();
					return true;
				}, nullptr, tke::textures.size()));
				if (ImGui::Combo("Color Map 2", &tcs.colorMap2Index, [](void *data, int idx, const char **out_text) {
					*out_text = tke::textures[idx]->filename.c_str();
					return true;
				}, nullptr, tke::textures.size()));
				if (ImGui::Combo("Color Map 3", &tcs.colorMap3Index, [](void *data, int idx, const char **out_text) {
					*out_text = tke::textures[idx]->filename.c_str();
					return true;
				}, nullptr, tke::textures.size()));
			}
			ImGui::DragFloat("Height", &tcs.height);
			ImGui::Checkbox("Use Physx", &tcs.usePhysx);
			if (tke::textures.size() > 0)
			{
				if (ImGui::Button("Create Terrain"))
				{
					auto t = new tke::Terrain(tcs.usePhysx, tke::textures[tcs.heightMapIndex].get(), tke::textures[tcs.blendMapIndex].get(), tke::textures[tcs.colorMap0Index].get(), tke::textures[tcs.colorMap1Index].get(), tke::textures[tcs.colorMap2Index].get(), tke::textures[tcs.colorMap3Index].get());
					t->setCoord(tcs.coord);
					t->height = tcs.height;
					s->addTerrain(t);
				}
			}
		}

		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Waters"))
	{
		if (ImGui::TreeNode("List"))
		{
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Create"))
		{
			for (int i = 0; i < 3; i++)
			{
				char *strs[] = { "CoordX", "CoordY", "CoordZ" };
				ImGui::DragFloat(strs[i], &wcs.coord[i], 0.5f);
			}
			ImGui::DragFloat("Height", &wcs.height);
			if (ImGui::Button("Create Water"))
			{
				auto w = new tke::Water;
				w->setCoord(wcs.coord);
				w->height = wcs.height;
				s->addWater(w);
			}

			ImGui::TreePop();
		}

		ImGui::TreePop();
	}
}

static void _show_model(tke::Model *m)
{
	auto modelName = tke::translate(936, CP_UTF8, m->name.c_str());
	ImGui::Text("name:%s", modelName.c_str());
	ImGui::Text("filename:%s", m->filename.c_str());
	ImGui::Text("indice count:%d", m->indices.size());
	ImGui::Text("indice base:%d", m->indiceBase);
	ImGui::Text("vertex base:%d", m->vertexBase);
	if (ImGui::Button("Save Data"))
		m->saveData(false);

	auto funShowAndSelectAnim = [&](tke::ModelStateAnimationKind kind, const char *name) {
		int index = 0;
		for (int i = 0; i < tke::animations.size(); i++)
		{
			if (tke::animations[i].get() == m->stateAnimations[kind]->animation)
			{
				index = i + 1;
				break;
			}
		}
		if (ImGui::Combo("Stand Animation", &index, [](void *, int idx, const char **out_text) {
			if (idx == 0)
				*out_text = "[NULL]";
			else
				*out_text = tke::animations[idx - 1]->filename.c_str();
			return true;
		}, nullptr, tke::animations.size() + 1))
		{
			auto b = m->bindAnimation(tke::animations[index - 1].get());
			m->setStateAnimation(kind, b);
		}
	};

	for (int i = 0; i < tke::ModelStateAnimationCount; i++)
	{
		const char *names[] = {
			"Stand Animation",
			"Forward Animation",
			"Backward Animation",
			"Leftward Animation",
			"Rightward Animation",
			"Jump Animation",
		};
		funShowAndSelectAnim((tke::ModelStateAnimationKind)i, names[i]);
	}

	ImGui::DragFloat("Controller Height", &m->controller_height, 0.1f, 0.f, 10000.f);
	ImGui::DragFloat("Controller Radius", &m->controller_radius, 0.1f, 0.f, 10000.f);
	ImGui::DragFloat3("Controller Position", &m->controller_position[0], 0.1f, 0.f, 10000.f);
	ImGui::DragFloat3("Eye Position", &m->eye_position[0]);
}

void AttributeWidget::show()
{
	ImGui::Begin("Attribute", &opened);

	switch (lastWindowType)
	{
	case LastWindowTypeGameExplorer:
		switch (resourceExplorer->lastItemType)
		{
		case ResourceExplorer::lastItemTypeTexture:
		{
			auto i = tke::textures[resourceExplorer->itemIndex].get();
			ImGui::Text("filename:%s", i->filename.c_str());
			ImGui::Text("size:%d x %d", i->levels[0].cx, i->levels[0].cy);
			ImGui::Image((ImTextureID)i->index, ImVec2(i->levels[0].cx, i->levels[0].cy));
		}
			break;
		case ResourceExplorer::lastItemTypeModel:
			_show_model(tke::models[resourceExplorer->itemIndex].get());
			break;
		case ResourceExplorer::lastItemTypeScene:
			_show_scene(tke::scenes[resourceExplorer->itemIndex].get());
			break;
		}
		break;
	case LastWindowTypeMonitor:
		if (lastMonitorWidget->mode == MonitorWidget::ModeScene)
		{
			auto m = (SceneMonitorWidget*)lastMonitorWidget;

			_show_scene(m->scene);
			ImGui::Separator();

			if (selectedItem)
			{
				switch (selectedItem.type)
				{
				case ItemTypeObject:
				{
					auto o = selectedItem.toObject();

					auto modelName = tke::translate(936, CP_UTF8, o->model->name.c_str());
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
		}
		else if (lastMonitorWidget->mode == MonitorWidget::ModeModel)
			_show_model(((ModelMonitorWidget*)lastMonitorWidget)->model);
		break;
	}

	ImGui::End();
}

AttributeWidget *attributeWidget = nullptr;