#include "../../../src/ui/ui.h"

#include "model_editor.h"

std::string ModelEditorClass::getName()
{
	return "model editor";
}

Window *ModelEditorClass::load(tke::AttributeTreeNode *n)
{
	auto a = n->firstAttribute("filename");
	if (a)
	{
		auto i = tke::getModel(a->value);
		if (i)
		{
			auto w = new ModelEditor(i);
			return w;
		}
	}
	return nullptr;
}

ModelEditorClass modelEditorClass;

ModelEditor::ModelEditor(std::shared_ptr<tke::Model> _model)
	:Window(&modelEditorClass), model(_model)
{
	renderer = std::make_unique<tke::PlainRenderer>();
}

void ModelEditor::show()
{
	ImGui::Begin("Model -", &opened);

	ImGui::Text("indice count:%d", model->indice_count);
	ImGui::Text("indice base:%d", model->indiceBase);
	ImGui::Text("vertex base:%d", model->vertexBase);
	if (ImGui::Button("Save Data"))
		model->saveData(false);

	//auto funShowAndSelectAnim = [&](tke::ModelStateAnimationKind kind, const char *name) {
	//	int index = 0;
	//	for (int i = 0; i < tke::animations.size(); i++)
	//	{
	//		if (tke::animations[i].get() == m->stateAnimations[kind]->animation)
	//		{
	//			index = i + 1;
	//			break;
	//		}
	//	}
	//	if (ImGui::Combo("Stand Animation", &index, [](void *, int idx, const char **out_text) {
	//		if (idx == 0)
	//			*out_text = "[NULL]";
	//		else
	//			*out_text = tke::animations[idx - 1]->filename.c_str();
	//		return true;
	//	}, nullptr, tke::animations.size() + 1))
	//	{
	//		auto b = m->bindAnimation(tke::animations[index - 1].get());
	//		m->setStateAnimation(kind, b);
	//	}
	//};

	//for (int i = 0; i < tke::ModelStateAnimationCount; i++)
	//{
	//	const char *names[] = {
	//		"Stand Animation",
	//		"Forward Animation",
	//		"Backward Animation",
	//		"Leftward Animation",
	//		"Rightward Animation",
	//		"Jump Animation",
	//	};
	//	funShowAndSelectAnim((tke::ModelStateAnimationKind)i, names[i]);
	//}

	ImGui::DragFloat("Controller Height", &model->controller_height, 0.1f, 0.f, 10000.f);
	ImGui::DragFloat("Controller Radius", &model->controller_radius, 0.1f, 0.f, 10000.f);
	ImGui::DragFloat3("Controller Position", &model->controller_position[0], 0.1f, 0.f, 10000.f);
	ImGui::DragFloat3("Eye Position", &model->eye_position[0]);

	ImVec2 image_pos = ImGui::GetCursorScreenPos();
	ImVec2 image_size = ImVec2(layer.image->levels[0].cx, layer.image->levels[0].cy);
	ImGui::InvisibleButton("canvas", image_size);
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	draw_list->AddImage(ImTextureID(layer.image->index), image_pos, image_pos + image_size);

	ImGui::End();
}
