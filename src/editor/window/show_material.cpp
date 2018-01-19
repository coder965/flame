#include "../../file_utils.h"
#include "../../graphics/image.h"
#include "show_material.h"

static void show_map(std::function<std::shared_ptr<tke::Image>()> getter, const char *tooltip, std::function<void(std::shared_ptr<tke::Image>)> setter)
{
	auto i = getter();
	ImGui::ImageBorder(i ? ImTextureID(tke::get_ui_image_index(i)) : nullptr, ImVec2(16, 16), ImVec4(1, 1, 1, 1));
	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip(tooltip);
		if (ImGui::IsMouseClicked(1))
			setter(nullptr);
	}
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("file"))
		{
			static char filename[260];
			strcpy(filename, (char*)payload->Data);
			std::fs::path path(filename);
			auto ext = path.extension();
			if (tke::is_image_file(ext.string()))
			{
				auto i = tke::get_image(filename);
				if (i)
					setter(i);
			}
		}
		ImGui::EndDragDropTarget();
	}
}

void show_material(tke::Material *m)
{
	show_map(std::bind(&tke::Material::get_albedo_alpha_map, m), "albedo(rgb) alpha(a) map, [right click] to remove", std::bind(&tke::Material::set_albedo_alpha_map, m, std::placeholders::_1));
	ImGui::SameLine();
	ImGui::ColorEdit4("albedo alpha", &m->albedo_alpha[0], ImGuiColorEditFlags_NoInputs);
	show_map(std::bind(&tke::Material::get_spec_roughness_map, m), "spec(r) roughness(g) map, [right click] to remove", std::bind(&tke::Material::set_spec_roughness_map, m, std::placeholders::_1));
	ImGui::SameLine();
	ImGui::DragFloat2("spec roughness", &m->spec_roughness[0], 0.01f, 0.f, 1.f);
	show_map(std::bind(&tke::Material::get_normal_height_map, m), "normal(rgb) height(a) map, [right click] to remove", std::bind(&tke::Material::set_normal_height_map, m, std::placeholders::_1));
	ImGui::SameLine();
	ImGui::TextUnformatted("normal height");
}
