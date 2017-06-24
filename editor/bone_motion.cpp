#include "../src/gui.h"

#include "select.h"
#include "bone_motion.h"

void BoneMotionWidget::show()
{
	ImGui::BeginDock("Bone Motion");

	if (selectedItem)
	{
		if (selectedItem.type == ItemTypeObject)
		{
			auto o = selectedItem.toObject();
			if (o->model->animated)
			{
				if (boneID != -1)
				{
					if (ImGui::DragFloat("Coord X", &o->animationComponent->boneData[boneID].coord.x))
						o->animationComponent->refreshBone(boneID);
				}
			}
		}
	}

	ImGui::EndDock();
}

BoneMotionWidget *boneMotionWdiget = nullptr;