#include "..\src\gui.h"

#include "editor.h"
#include "select.h"
#include "attribute.h"

void ObjectCreationSetting::load_setting(tke::AttributeTreeNode *n)
{
	for (auto a : n->attributes)
	{
		if (a->name == "use_camera_position")
			a->get(&use_camera_position);
		else if (a->name == "use_camera_target_position")
			a->get(&use_camera_target_position);
		else if (a->name == "coord")
			a->get(&coord);
		else if (a->name == "randCX")
			a->get(&randC[0]);
		else if (a->name == "randCY")
			a->get(&randC[1]);
		else if (a->name == "randCZ")
			a->get(&randC[2]);
		else if (a->name == "coordRandRange")
			a->get(&coordRandRange);
		else if (a->name == "euler")
			a->get(&euler);
		else if (a->name == "randRX")
			a->get(&randR[0]);
		else if (a->name == "randRY")
			a->get(&randR[1]);
		else if (a->name == "randRZ")
			a->get(&randR[2]);
		else if (a->name == "eulerRandRange")
			a->get(&eulerRandRange);
		else if (a->name == "scale")
			a->get(&scale);
		else if (a->name == "randSX")
			a->get(&randS[0]);
		else if (a->name == "randSY")
			a->get(&randS[1]);
		else if (a->name == "randSZ")
			a->get(&randS[2]);
		else if (a->name == "scaleRandRange")
			a->get(&scaleRandRange);
		else if (a->name == "same_scale_rand")
			a->get(&same_scale_rand);
	}
}

void ObjectCreationSetting::save_setting(tke::AttributeTreeNode *n)
{
	n->addAttribute("use_camera_position", &use_camera_position);
	n->addAttribute("use_camera_target_position", &use_camera_target_position);
	n->addAttribute("coord", &coord);
	n->addAttribute("randCX", &randC[0]);
	n->addAttribute("randCY", &randC[1]);
	n->addAttribute("randCZ", &randC[2]);
	n->addAttribute("coordRandRange", &coordRandRange);
	n->addAttribute("euler", &euler);
	n->addAttribute("randRX", &randR[0]);
	n->addAttribute("randRY", &randR[1]);
	n->addAttribute("randRZ", &randR[2]);
	n->addAttribute("eulerRandRange", &eulerRandRange);
	n->addAttribute("scale", &scale);
	n->addAttribute("randSX", &randS[0]);
	n->addAttribute("randSY", &randS[1]);
	n->addAttribute("randSZ", &randS[2]);
	n->addAttribute("scaleRandRange", &scaleRandRange);
	n->addAttribute("same_scale_rand", &same_scale_rand);
}

ObjectCreationSetting ocs;

static void _show_scene(tke::Scene *scene)
{
	if (ImGui::TreeNode("Sky"))
	{
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
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Create"))
		{
			static int modelIndex = 0;
			if (ImGui::Combo("Model", &modelIndex, [](void *data, int idx, const char **out_text) {
				*out_text = tke::models[idx]->name.c_str();
				return true;
			}, nullptr, tke::models.size()));

			ImGui::Separator();
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
						ImGui::Text(strs[i], scene->camera.getCoord()[i]);
					else
						ImGui::Text(strs[i], scene->camera.target[i]);
				}
				ImGui::SameLine();
				char *strs[] = { "Rand##cx", "Rand##cy", "Rand##cz" };
				ImGui::Checkbox(strs[i], &ocs.randC[i]);
			}
			ImGui::DragFloat("Coord Rand Range", &ocs.coordRandRange, 0.1f);

			ImGui::Separator();
			for (int i = 0; i < 3; i++)
			{
				char *strs0[] = { "EulerX", "EulerY", "EulerZ" };
				char *strs1[] = { "Rand##rx", "Rand##ry", "Rand##rz" };
				ImGui::DragFloat(strs0[i], &ocs.euler[i], 0.5f);
				ImGui::SameLine();
				ImGui::Checkbox(strs1[i], &ocs.randR[i]);
			}
			ImGui::DragFloat("Euler Rand Range", &ocs.eulerRandRange, 1.f, 0.f, 360.f);

			ImGui::Separator();
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

			ImGui::Separator();
			static int physxType = 0;
			static const char *physxTypeNames[] = {
				"Null",
				"Static",
				"Dynamic"
			};
			ImGui::Combo("Physx Type", &physxType, physxTypeNames, TK_ARRAYSIZE(physxTypeNames));
			static bool use_controller;
			if (physxType != 0)
				ImGui::Checkbox("Use Controller", &use_controller);
			ImGui::Separator();

			if (ImGui::Button("Create Object"))
			{
				auto _physxType = tke::ObjectPhysicsType::null;
				if (physxType != 0)
				{
					_physxType = tke::ObjectPhysicsType(1 << (physxType - 1));
					if (use_controller)
						_physxType = tke::ObjectPhysicsType((int)_physxType | (int)tke::ObjectPhysicsType::controller);
				}
				auto o = new tke::Object(tke::models[modelIndex], _physxType);

				glm::vec3 _coord;
				if (ocs.use_camera_position)
				{
					if (ocs.use_camera_target_position)
						_coord = scene->camera.target;
					else
						_coord = scene->camera.getCoord();
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

				scene->addObject(o);
			}

			ImGui::TreePop();
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Terrain"))
	{
		auto terrain = scene->terrain;

		if (terrain)
		{
			static int height_map_index = 0;
			if (ImGui::Combo("Height Map", &height_map_index, [](void *data, int idx, const char **out_text) {
				if (idx == 0)
					*out_text = "[NULL]";
				else
					*out_text = tke::textures[idx - 1]->filename.c_str();
				return true;
			}, nullptr, tke::textures.size() + 1))
			{
				if (height_map_index > 0)
					terrain->heightMap = tke::textures[height_map_index - 1];
				else
					terrain->heightMap = nullptr;
				terrain->changed = true;
			}
			static int color_map_index = 0;
			if (ImGui::Combo("Color Map", &color_map_index, [](void *data, int idx, const char **out_text) {
				if (idx == 0)
					*out_text = "[NULL]";
				else
					*out_text = tke::textures[idx - 1]->filename.c_str();
				return true;
			}, nullptr, tke::textures.size() + 1))
			{
				if (color_map_index > 0)
					terrain->colorMap = tke::textures[color_map_index - 1];
				else
					terrain->colorMap = nullptr;
				terrain->changed = true;
			}
			if (ImGui::DragFloat("Height", &terrain->height))
				terrain->changed = true;
			if (ImGui::Button("Remove Terrain"))
				scene->removeTerrain();
		}
		else
		{
			if (ImGui::Button("Create Height Map Terrain"))
			{
				auto t = new tke::Terrain(tke::TerrainType::height_map);
				scene->addTerrain(t);
			}
			if (ImGui::Button("Create Procedural Terrain"))
			{
				auto t = new tke::Terrain(tke::TerrainType::procedural);
				scene->addTerrain(t);
			}
		}

		ImGui::TreePop();
	}
}

void AttributeWidget::show()
{
	ImGui::BeginDock("Attribute", &opened);

	switch (lastWindowType)
	{
	case LastWindowTypeGameExplorer:
		switch (gameExplorer->lastItemType)
		{
		case GameExplorer::lastItemTypeTexture:
		{
			auto i = tke::textures[gameExplorer->itemIndex];
			ImGui::Text("filename:%s", i->filename.c_str());
			ImGui::Text("size:%d x %d", i->width, i->height);
			ImGui::Image((ImTextureID)i->index, ImVec2(i->width, i->height));
		}
			break;
		case GameExplorer::lastItemTypeModel:
		{
			auto m = tke::models[gameExplorer->itemIndex];
			auto modelName = tke::translate(936, CP_UTF8, m->name.c_str());
			ImGui::Text("name:%s", modelName.c_str());
			ImGui::Text("filename:%s", m->filename.c_str());
			ImGui::Text("indice count:%d", m->indices.size());
			ImGui::Text("indice base:%d", m->indiceBase);
			ImGui::Text("vertex base:%d", m->vertexBase);
		}
			break;
		case GameExplorer::lastItemTypeScene:
			_show_scene(game.scenes[gameExplorer->itemIndex]);
			break;
		}
		break;
	case LastWindowTypeMonitor:
		ImGui::BeginTabBar("##tab_monitor");
		if (ImGui::AddTab("Scene"))
			_show_scene(monitorWidget->scene);
		if (monitorWidget->selectedItem)
		{
			if (ImGui::AddTab("Select"))
			{
				switch (monitorWidget->selectedItem.type)
				{
				case ItemTypeObject:
				{
					auto o = monitorWidget->selectedItem.toObject();

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
		ImGui::EndTabBar();
		break;
	}

	ImGui::EndDock();
}

AttributeWidget *attributeWidget = nullptr;