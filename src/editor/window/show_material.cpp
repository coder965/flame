#include "../../file_utils.h"
#include "../../graphics/image.h"
#include "show_material.h"

static void show_map(std::function<std::string()> getter, const char *tooltip, std::function<void(const std::string &)> setter)
{
	auto image_name = getter();
	ImGui::Image_f(image_name.c_str(), ImVec2(16, 16), ImVec4(1, 1, 1, 1));
	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip(tooltip);
		if (ImGui::IsMouseClicked(1))
			setter("");
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
				setter(filename);
		}
		ImGui::EndDragDropTarget();
	}}

void show_material(tke::Material *m)
{
	show_map(std::bind(&tke::Material::get_albedo_alpha_map_name, m), "albedo(rgb) alpha(a) map, [right click] to remove", std::bind(&tke::Material::set_albedo_alpha_map, m, std::placeholders::_1));
	ImGui::SameLine();
	auto albedo_alpha = m->get_albedo_alpha();
	if (ImGui::ColorEdit4("albedo alpha", &albedo_alpha[0], ImGuiColorEditFlags_NoInputs))
		m->set_albedo_alpha(albedo_alpha);
	show_map(std::bind(&tke::Material::get_spec_roughness_map_name, m), "spec(r) roughness(g) map, [right click] to remove", std::bind(&tke::Material::set_spec_roughness_map, m, std::placeholders::_1));
	ImGui::SameLine();
	auto spec_roughness = m->get_spec_roughness();
	if (ImGui::DragFloat2("spec roughness", &spec_roughness[0], 0.01f, 0.f, 1.f))
		m->set_spec_roughness(spec_roughness);
	show_map(std::bind(&tke::Material::get_normal_height_map_name, m), "normal(rgb) height(a) map, [right click] to remove", std::bind(&tke::Material::set_normal_height_map, m, std::placeholders::_1));
	ImGui::SameLine();
	ImGui::TextUnformatted("normal height");
}
