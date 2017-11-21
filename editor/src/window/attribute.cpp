#include "../../../src/ui/ui.h"

#include "../editor.h"
#include "../select.h"
#include "attribute.h"

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
			break;
		}
		break;
	//case LastWindowTypeMonitor:
	//	{
	//		auto m = (SceneMonitorWidget*)lastMonitorWidget;

	//		ImGui::Separator();

	//		if (selectedItem)
	//		{
	//			switch (selectedItem.type)
	//			{
	//			case ItemTypeObject:
	//			{
	//				auto o = selectedItem.toObject();

	//				auto modelName = tke::translate(936, CP_UTF8, o->model->name.c_str());
	//				ImGui::Text("model:%s", modelName.c_str());

	//				auto coord = o->getCoord();
	//				if (ImGui::DragFloat3("coord", &coord[0]))
	//					o->setCoord(coord);
	//				auto euler = o->getEuler();
	//				if (ImGui::DragFloat3("euler", &euler[0]))
	//					o->setEuler(euler);
	//				auto scale = o->getScale();
	//				if (ImGui::DragFloat3("scale", &scale[0]))
	//					o->setScale(scale);

	//				ImGui::DragFloat("ang offset", &o->ang_offset);
	//				ImGui::DragFloat("speed", &o->speed);
	//				ImGui::DragFloat("turn speed", &o->turn_speed);

	//				if (o->model->animated)
	//				{
	//					static int boneID = -1;
	//					if (boneID >= o->model->bones.size()) boneID = -1;

	//					if (ImGui::TreeNode("Bones Motion"))
	//					{
	//						for (int i = 0; i < o->model->bones.size(); i++)
	//						{
	//							auto str = tke::translate(936, CP_UTF8, o->model->bones[i].name);
	//							if (ImGui::Selectable(str.c_str(), i == boneID))
	//								boneID = i;
	//						}

	//						ImGui::TreePop();
	//					}
	//				}
	//			}
	//				break;
	//			}
	//		}
	//	}
	//	break;
	}

	ImGui::End();
}

AttributeWidget *attributeWidget = nullptr;