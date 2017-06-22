#include "..\src\gui.h"

#include "editor.h"
#include "attribute.h"
#include "bone_motion.h"

void AttributeWidget::show()
{
	ImGui::BeginDock("Attribute", &opened);

	//auto str = tke::translate(936, CP_UTF8, "ÎÒ");

	//ImGui::TextUnformatted(str.c_str());
	//ImGui::TextUnformatted("\xe3\x81\x8b\xe3\x81\x8d\xe3\x81\x8f\xe3\x81\x91\xe3\x81\x93");
	
	if (selectedItem)
	{
		switch (selectedItem.type)
		{
		case ItemTypeObject:
		{
			auto o = selectedItem.toObject();
			if (o->model->animated)
			{
				if (ImGui::TreeNode("Bones"))
				{
					for (int i = 0; i < o->model->bones.size(); i++)
					{
						auto str = tke::translate(936, CP_UTF8, o->model->bones[i].name);
						if (ImGui::Selectable(str.c_str()))
						{
							if (boneMotionWdiget)
								boneMotionWdiget->boneID = i;
						}
					}

					ImGui::TreePop();
				}
			}
		}
			break;
		}
	}

	ImGui::EndDock();

	if (!opened)
	{
		attributeWidget = nullptr;
		delete this;
	}
}

AttributeWidget *attributeWidget = nullptr;