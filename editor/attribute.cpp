#include "..\src\gui.h"

#include "select.h"
#include "attribute.h"

void AttributeWidget::show()
{
	ImGui::BeginDock("Attribute", &opened);
	
	if (selectedItem)
	{
		switch (selectedItem.type)
		{
		case ItemTypeObject:
		{
			auto o = selectedItem.toObject();

			ImGui::Text(o->model->name.c_str());
			
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

				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

				ImGui::BeginChild("child1", ImVec2(0, height), true);
				if (ImGui::TreeNode("Bones"))
				{
					for (int i = 0; i < o->model->bones.size(); i++)
					{
						auto str = tke::translate(936, CP_UTF8, o->model->bones[i].name);
						if (ImGui::Selectable(str.c_str(), i == boneID))
							boneID = i;
					}

					ImGui::TreePop();
				}
				ImGui::EndChild(); 

				ImGui::InvisibleButton("hsplitter", ImVec2(-1, 8.0f));
				if (ImGui::IsItemActive())
					height += ImGui::GetIO().MouseDelta.y;

				ImGui::BeginChild("child2", ImVec2(0, 0), true);
				if (boneID != -1)
				{
					if (ImGui::DragFloat("Coord X", &o->animationComponent->boneData[boneID].coord.x))
						o->animationComponent->refreshBone(boneID);
				}
				ImGui::EndChild();

				ImGui::PopStyleVar();
			}
		}
			break;
		}
	}
	else
	{
		ImGui::TextWrapped("Select Something");
	}

	ImGui::EndDock();
}

AttributeWidget *attributeWidget = nullptr;