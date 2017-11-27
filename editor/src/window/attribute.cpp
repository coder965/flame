#include "../../../src/ui/ui.h"

#include "../editor.h"
#include "../select.h"
#include "attribute.h"

static void _show_model(tke::Model *m)
{
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

	//switch (lastWindowType)
	//{
	//case LastWindowTypeGameExplorer:
	//	switch (resourceExplorer->lastItemType)
	//	{
	//	case ResourceExplorer::lastItemTypeTexture:
	//	{
	//		//auto i = tke::textures[resourceExplorer->itemIndex].get();
	//		//ImGui::Text("filename:%s", i->filename.c_str());
	//		//ImGui::Text("size:%d x %d", i->levels[0].cx, i->levels[0].cy);
	//		//ImGui::Image((ImTextureID)i->index, ImVec2(i->levels[0].cx, i->levels[0].cy));
	//	}
	//		break;
	//	case ResourceExplorer::lastItemTypeModel:
	//		_show_model(tke::models[resourceExplorer->itemIndex].get());
	//		break;
	//	}
	//	break;
	//}

	ImGui::End();
}

AttributeWidget *attributeWidget = nullptr;