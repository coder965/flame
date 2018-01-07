#include "../../input.h"
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

	static auto entity_count = 0;
	if (entity_count == 0)
		ImGui::TextUnformatted("Empty");
	else
	{
		ImGui::Separator();
		ImGui::Text("Entities:%d", entity_count);
	}
	entity_count = 0;
	if (scene_editor)
	{
		auto scene = scene_editor->scene.get();
		auto sel = scene_editor->selected.lock();
		if (scene->sky)
		{
			entity_count++;
			if (ImGui::Selectable("Sky", sel && (tke::Sky*)sel.get() == scene->sky.get()))
				scene_editor->selected = scene->sky;
		}
		for (int i = 0; i < scene->lights.size(); i++)
		{
			entity_count++;
			auto &l = scene->lights[i];
			if (ImGui::Selectable(("Light - " + std::to_string(i)).c_str(), sel && (tke::Light*)sel.get() == l.get()))
				scene_editor->selected = l;
		}
		for (int i = 0; i < scene->objects.size(); i++)
		{
			entity_count++;
			auto &o = scene->objects[i];
			if (ImGui::Selectable(("Object - " + std::to_string(i)).c_str(), sel && (tke::Object*)sel.get() == o.get()))
				scene_editor->selected = o;
		}
		for (int i = 0; i < scene->terrains.size(); i++)
		{
			entity_count++;
			auto &t = scene->terrains[i];
			if (ImGui::Selectable(("Terrain - " + std::to_string(i)).c_str(), sel && (tke::Terrain*)sel.get() == t.get()))
				scene_editor->selected = t;
		}
		for (int i = 0; i < scene->waters.size(); i++)
		{
			entity_count++;
			auto &w = scene->waters[i];
			if (ImGui::Selectable(("Water - " + std::to_string(i)).c_str(), sel && (tke::Water*)sel.get() == w.get()))
				scene_editor->selected = w;
		}

		if (sel && sel->type == tke::NodeTypeObject)
		{
			auto obj = (tke::Object*)sel.get();
			obj->setState(tke::Controller::State::forward, tke::keyStates[VK_UP].pressing);
			obj->setState(tke::Controller::State::backward, tke::keyStates[VK_DOWN].pressing);
			obj->setState(tke::Controller::State::left, tke::keyStates[VK_LEFT].pressing);
			obj->setState(tke::Controller::State::right, tke::keyStates[VK_RIGHT].pressing);
		}
	}

	ImGui::End();
}
