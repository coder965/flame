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
			ImGui::BeginTabBar("##tab");
			if (ImGui::AddTab("Model"))
			{

			}
			ImGui::EndTabBar();
			break;
		}
		break;
	case LastWindowTypeMonitor:
		ImGui::BeginTabBar("##tab");
		if (ImGui::AddTab("Scene"))
		{
			if (ImGui::TreeNode("Lights"))
			{
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Objects"))
			{
				ImGui::TreePop();
			}
		}
		if (ImGui::AddTab("Select"))
		{
			if (selectedItem)
			{
				switch (selectedItem.type)
				{
				case ItemTypeObject:
				{
					auto o = selectedItem.toObject();

					auto str = tke::translate(936, CP_UTF8, o->model->name.c_str());
					ImGui::Text(str.c_str());

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