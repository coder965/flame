#include "show_material.h"

void show_material(tke::Material *m)
{
	ImGui::ColorEdit4("albedo", &m->albedo_alpha.x, ImGuiColorEditFlags_NoInputs);
}
