#include "..\src\gui.h"

#include "editor.h"
#include "select.h"
#include "attribute.h"

void AttributeWidget::show()
{
	ImGui::BeginDock("Attribute", &opened);

	switch (lastWindowType)
	{
	case LastWindowTypeGameExplorer:
		switch (gameExplorer->lastItemType)
		{
		case GameExplorer::lastItemTypeModel:
			ImGui::BeginTabBar("##tab_game_explorer_model");
			if (ImGui::AddTab("Model"))
			{

			}
			ImGui::EndTabBar();
			break;
		}
		break;
	case LastWindowTypeMonitor:
		ImGui::BeginTabBar("##tab_monitor");
		if (ImGui::AddTab("Scene"))
		{
			auto scene = monitorWidget->scene;

			if (ImGui::TreeNode("Lights"))
			{
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Objects"))
			{
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Terrain"))
			{
				if (scene->terrain)
				{
					static int index = 0;
					if (ImGui::Combo("Height Map", &index, [](void *data, int idx, const char **out_text) {
						if (idx == 0)
							*out_text = "[NULL]";
						else
							*out_text = tke::textures[idx - 1]->filename.c_str();
						return true;
					}, nullptr, tke::textures.size() + 1))
					{
						if (index > 0)
							scene->terrain->heightMap = tke::textures[index - 1];
						else
							scene->terrain->heightMap = nullptr;
						scene->terrain->changed = true;
					}
					if (ImGui::Button("Remove Terrain"))
						scene->removeTerrain();
				}
				else
				{
					if (ImGui::Button("Create Height Map Terrain"))
					{
						auto t = new tke::Terrain(tke::TerrainTypeHeightMap);
						scene->addTerrain(t);
					}
					if (ImGui::Button("Create Procedural Terrain"))
					{
						auto t = new tke::Terrain(tke::TerrainTypeProcedural);
						scene->addTerrain(t);
					}
				}

				ImGui::TreePop();
			}
		}
		if (ImGui::AddTab("Select"))
		{
			if (monitorWidget->selectedItem)
			{
				switch (monitorWidget->selectedItem.type)
				{
				case ItemTypeObject:
				{
					auto o = monitorWidget->selectedItem.toObject();

					auto str = tke::translate(936, CP_UTF8, o->model->name.c_str());
					ImGui::Text(str.c_str());

					auto coord = o->getCoord();
					if (ImGui::InputFloat3("coord", &coord[0]))
						o->setCoord(coord);
					auto euler = o->getEuler();
					if (ImGui::InputFloat3("euler", &euler[0]))
						o->setEuler(euler);
					auto scale = o->getScale();
					if (ImGui::InputFloat3("scale", &scale[0]))
						o->setScale(scale);

					if (o->model->animated)
					{
						static int boneID = -1;
						if (boneID >= o->model->bones.size()) boneID = -1;

						static float height = 400.f;

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
			{
				ImGui::TextWrapped("Select Something");
			}
		}
		if (ImGui::AddTab("Sub Select"))
		{
			//if (boneID != -1)
			//{
			//	if (ImGui::DragFloat3("coord", &o->animationComponent->boneData[boneID].coord[0]))
			//		o->animationComponent->refreshBone(boneID);
			//}
		}
		ImGui::EndTabBar();
		break;
	}

	ImGui::EndDock();
}

AttributeWidget *attributeWidget = nullptr;