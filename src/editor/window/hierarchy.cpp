#include "../../ui/ui.h"
#include "../../entity/object.h"
#include "../../entity/terrain.h"
#include "../../entity/water.h"
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
		auto sel = scene_editor->selected.lock();

		auto node_style = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
		if (sel && (tke::Scene*)sel.get() == scene.get())
			node_style |= ImGuiTreeNodeFlags_Selected;
		auto node_opened = ImGui::TreeNodeEx("scene", node_style);
		if (ImGui::IsItemClicked())
			scene_editor->selected = scene;
		if (node_opened)
		{
			for (int i = 0; i < scene->lights.size(); i++)
			{
				auto &l = scene->lights[i];
				if (ImGui::Selectable(("Light - " + std::to_string(i)).c_str(), sel && (tke::Light*)sel.get() == l.get()))
					scene_editor->selected = l;
			}
			for (int i = 0; i < scene->objects.size(); i++)
			{
				auto &o = scene->objects[i];
				if (ImGui::Selectable(("Object - " + std::to_string(i)).c_str(), sel && (tke::Object*)sel.get() == o.get()))
					scene_editor->selected = o;
			}
			for (int i = 0; i < scene->terrains.size(); i++)
			{
				auto &t = scene->terrains[i];
				if (ImGui::Selectable(("Terrain - " + std::to_string(i)).c_str(), sel && (tke::Terrain*)sel.get() == t.get()))
					scene_editor->selected = t;
			}
			for (int i = 0; i < scene->waters.size(); i++)
			{
				auto &w = scene->waters[i];
				if (ImGui::Selectable(("Water - " + std::to_string(i)).c_str(), sel && (tke::Water*)sel.get() == w.get()))
					scene_editor->selected = w;
			}

			ImGui::TreePop();
		}
	}

	ImGui::End();
}
