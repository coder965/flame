#include "../../input.h"
#include "../../global.h"
#include "../../graphics/image.h"
#include "../../graphics/command_buffer.h"
#include "../../ui/ui.h"

#include "model_editor.h"

ModelEditor::ModelEditor(std::shared_ptr<tke::Model> _model)
	:model(_model), layer(true)
{
	first_cx = 800;
	first_cy = 600;

	draw_data.mode = tke::PlainRenderer::mode_just_texture;
	draw_data.obj_data.resize(1);
	draw_data.obj_data[0].mat = glm::mat4(1);
	draw_data.obj_data[0].color = glm::vec4(0.f, 1.f, 0.f, 1.f);
	draw_data.obj_data[0].fill_with_model_texture_mode(model.get());

	camera.setMode(tke::CameraMode::targeting);
	renderer = std::make_unique<tke::PlainRenderer>();
}

void ModelEditor::do_show()
{
	ImGui::Begin(("Model - " + model->filename).c_str(), &opened, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoSavedSettings);

	ImGui::BeginMenuBar();
	if (ImGui::BeginMenu("File"))
	{
		if (ImGui::MenuItem("Save"))
			;

		ImGui::EndMenu();
	}
	ImGui::EndMenuBar();

	ImVec2 image_pos = ImGui::GetCursorScreenPos();
	ImVec2 image_size = ImVec2(layer.image->levels[0].cx, layer.image->levels[0].cy);
	ImGui::InvisibleButton("canvas", image_size);
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	draw_list->AddImage(ImTextureID(tke::get_ui_image_index(layer.image)), image_pos, image_pos + image_size);
	if (ImGui::IsItemHovered())
	{
		if (tke::mouseDispX != 0 || tke::mouseDispY != 0)
		{
			auto distX = (float)tke::mouseDispX / (float)tke::res_cx;
			auto distY = (float)tke::mouseDispY / (float)tke::res_cy;
			if (tke::mouseMiddle.pressing)
				camera.rotateByCursor(distX, distY);
		}
		if (tke::mouseScroll != 0)
			camera.scroll(tke::mouseScroll);
	}
	if (camera.transform_dirty)
		camera.lookAtTarget();

	ImGui::SameLine();
	ImGui::BeginGroup();
	ImGui::BeginChild("right", ImVec2(500, 0));

	ImGui::Text("indice count:%d", model->indice_count);
	ImGui::Text("indice base:%d", model->indiceBase);
	ImGui::Text("vertex base:%d", model->vertexBase);

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

	ImGui::EndChild();
	ImGui::EndGroup();

	ImGui::End();

	renderer->render(layer.framebuffer.get(), true, &camera, &draw_data);
	renderer->add_to_drawlist();
}
