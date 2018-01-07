#include "../../ui/ui.h"
#include "../../entity/object.h"
#include "../../entity/terrain.h"
#include "../../entity/water.h"
#include "../select.h"
#include "scene_editor.h"
#include "hierarchy.h"

HierarchyWindow *hierarchy_window = nullptr;

HierarchyWindow::~HierarchyWindow()
{
	hierarchy_window = nullptr;
}

void HierarchyWindow::do_show()
{
	ImGui::Begin("Hierarchy", &opened);

	if (scene_editor)
	{
		auto scene = scene_editor->scene;
		auto n = selected.get_node();

		auto node_style = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
		if ((tke::Scene*)n == scene.get())
			node_style |= ImGuiTreeNodeFlags_Selected;
		auto node_opened = ImGui::TreeNodeEx("scene", node_style);
		if (ImGui::IsItemClicked())
			selected = scene;
		if (node_opened)
		{
			for (int i = 0; i < scene->lights.size(); i++)
			{
				auto &l = scene->lights[i];
				if (ImGui::Selectable(("Light - " + std::to_string(i)).c_str(), (tke::Light*)n == l.get()))
					selected = l;
			}
			for (int i = 0; i < scene->objects.size(); i++)
			{
				auto &o = scene->objects[i];
				if (ImGui::Selectable(("Object - " + std::to_string(i)).c_str(), (tke::Object*)n == o.get()))
					selected = o;
			}
			for (int i = 0; i < scene->terrains.size(); i++)
			{
				auto &t = scene->terrains[i];
				if (ImGui::Selectable(("Terrain - " + std::to_string(i)).c_str(), (tke::Terrain*)n == t.get()))
					selected = t;
			}
			for (int i = 0; i < scene->waters.size(); i++)
			{
				auto &w = scene->waters[i];
				if (ImGui::Selectable(("Water - " + std::to_string(i)).c_str(), (tke::Water*)n == w.get()))
					selected = w;
			}

			ImGui::TreePop();
		}
	}

	ImGui::End();
}
