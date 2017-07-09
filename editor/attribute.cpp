#include "..\src\gui.h"

#include "editor.h"
#include "select.h"
#include "attribute.h"

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
			static bool use_camera_position, use_camera_target_position;
			ImGui::Checkbox("Use Camera Position", &use_camera_position);
			if (use_camera_position)
				ImGui::Checkbox("Use Camera Target Position", &use_camera_target_position);
			static glm::vec3 coord;
			static bool randCX, randCY, randCZ;
			ImGui::DragFloat("CoordX", &coord.x, 0.5f);
			ImGui::SameLine();
			ImGui::Checkbox("Rand##cx", &randCX);
			ImGui::DragFloat("CoordY", &coord.y, 0.5f);
			ImGui::SameLine();
			ImGui::Checkbox("Rand##cy", &randCY);
			ImGui::DragFloat("CoordZ", &coord.z, 0.5f);
			ImGui::SameLine();
			ImGui::Checkbox("Rand##cz", &randCZ);
			static float coordRandRange = 1.f;
			ImGui::DragFloat("Coord Rand Range", &coordRandRange, 0.1f);

			ImGui::Separator();
			static glm::vec3 euler;
			static bool randRX, randRY, randRZ;
			ImGui::DragFloat("EulerX", &euler.x, 0.5f);
			ImGui::SameLine();
			ImGui::Checkbox("Rand##rx", &randRX);
			ImGui::DragFloat("EulerY", &euler.y, 0.5f);
			ImGui::SameLine();
			ImGui::Checkbox("Rand##ry", &randRY);
			ImGui::DragFloat("EulerZ", &euler.z, 0.5f);
			ImGui::SameLine();
			ImGui::Checkbox("Rand##rz", &randRZ);
			static float eulerRandRange = 360.f;
			ImGui::DragFloat("Euler Rand Range", &eulerRandRange, 1.f, 0.f, 360.f);

			ImGui::Separator();
			static glm::vec3 scale = glm::vec3(1.f);
			static bool randSX, randSY, randSZ;
			ImGui::DragFloat("ScaleX", &scale.x, 0.5f);
			ImGui::SameLine();
			ImGui::Checkbox("Rand##sx", &randSX);
			ImGui::DragFloat("ScaleY", &scale.y, 0.5f);
			ImGui::SameLine();
			ImGui::Checkbox("Rand##sy", &randSY);
			ImGui::DragFloat("ScaleZ", &scale.z, 0.5f);
			ImGui::SameLine();
			ImGui::Checkbox("Rand##sz", &randSZ);
			static float scaleRandRange = 1.f;
			ImGui::DragFloat("Scale Rand Range", &scaleRandRange, 0.1f);
			static bool same_scale_rand;
			ImGui::Checkbox("Same Scale Rand", &same_scale_rand);

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
				if (use_camera_position)
				{
					if (use_camera_target_position)
						_coord = scene->camera.target;
					else
						_coord = scene->camera.getCoord();
				}
				else
				{
					_coord = coord;
				}
				if (randCX) _coord.x += (((float)rand() / (float)RAND_MAX) - 0.5f) * 2.f * coordRandRange;
				if (randCY) _coord.y += (((float)rand() / (float)RAND_MAX) - 0.5f) * 2.f * coordRandRange;
				if (randCZ) _coord.z += (((float)rand() / (float)RAND_MAX) - 0.5f) * 2.f * coordRandRange;
				o->setCoord(_coord);

				glm::vec3 _euler = euler;
				if (randRX) _euler.x += (((float)rand() / (float)RAND_MAX) - 0.5f) * 2.f * eulerRandRange;
				if (randRY) _euler.y += (((float)rand() / (float)RAND_MAX) - 0.5f) * 2.f * eulerRandRange;
				if (randRZ) _euler.z += (((float)rand() / (float)RAND_MAX) - 0.5f) * 2.f * eulerRandRange;
				o->setEuler(_euler);

				glm::vec3 _scale = scale;
				auto scale_rand = ((float)rand() / (float)RAND_MAX) * scaleRandRange;
				if (randSX) _scale.x += scale_rand;
				if (randSY) _scale.y += same_scale_rand ? scale_rand : ((float)rand() / (float)RAND_MAX) * scaleRandRange;
				if (randSZ) _scale.z += same_scale_rand ? scale_rand : ((float)rand() / (float)RAND_MAX) * scaleRandRange;
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