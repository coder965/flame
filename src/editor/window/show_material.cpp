#include "show_material.h"

void show_material(tke::Material *m)
{
	ImGui::ColorPicker4("albedo", &m->albedo_alpha.x);
}
