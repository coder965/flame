#include "../../ui/ui.h"
#include "../../core.h"
#include "../select.h"
#include "entity_window.h"

EntityWindow *entity_window = nullptr;

EntityWindow::EntityWindow(tke::Scene *_scene)
	:scene(_scene)
{
}

EntityWindow::~EntityWindow()
{
	entity_window = nullptr;
}

void EntityWindow::do_show()
{
	ImGui::Begin("Entity", &opened);

	if (ImGui::TreeNode(("Lights - " + std::to_string(scene->lights.size())).c_str()))
	{
		ImGui::TreePop();
	}
	if (ImGui::TreeNode(("Objects - " + std::to_string(scene->objects.size())).c_str()))
	{
		for (int i = 0; i < scene->objects.size(); i++)
		{
			auto o = scene->objects[i].get();
			if (ImGui::Selectable(std::to_string(i).c_str(), selectedItem.toObject() == o))
				selectedItem.select(o);
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode(("Terrain - " + std::to_string(scene->terrain ? 1 : 0)).c_str()))
	{
		auto terrain = scene->terrain.get();

		if (terrain)
		{
			ImGui::Text("Height Map:%s", terrain->heightMap->filename.c_str());
			ImGui::Text("Color Map 0:%s", terrain->colorMaps[0]->filename.c_str());
			ImGui::Text("Color Map 1:%s", terrain->colorMaps[1]->filename.c_str());
			ImGui::Text("Color Map 2:%s", terrain->colorMaps[2]->filename.c_str());
			ImGui::Text("Color Map 3:%s", terrain->colorMaps[3]->filename.c_str());
			ImGui::Text("Height:%f", terrain->height);
			ImGui::Text("Use Physx:%s", terrain->use_physx ? "Yse" : "No");
			if (ImGui::Button("Remove Terrain"))
				scene->removeTerrain();
		}

		ImGui::TreePop();
	}
	if (ImGui::TreeNode(("Waters - " + std::to_string(scene->waters.size())).c_str()))
	{

		ImGui::TreePop();
	}

	auto obj = selectedItem.toObject();
	if (obj)
	{
		obj->setState(tke::Controller::State::forward, tke::keyStates[VK_UP].pressing);
		obj->setState(tke::Controller::State::backward, tke::keyStates[VK_DOWN].pressing);
		obj->setState(tke::Controller::State::left, tke::keyStates[VK_LEFT].pressing);
		obj->setState(tke::Controller::State::right, tke::keyStates[VK_RIGHT].pressing);
	}

	ImGui::Separator();
	if (selectedItem)
	{
		switch (selectedItem.type)
		{
		case ItemTypeObject:
		{
			ImGui::TextUnformatted("Selected:Object");

			auto o = selectedItem.toObject();

			auto modelName = tke::translate(936, CP_UTF8, o->model->filename.c_str());
			ImGui::Text("model:%s", modelName.c_str());

			auto coord = o->getCoord();
			if (ImGui::DragFloat3("coord", &coord[0]))
				o->setCoord(coord);
			auto euler = o->getEuler();
			if (ImGui::DragFloat3("euler", &euler[0]))
				o->setEuler(euler);
			auto scale = o->getScale();
			if (ImGui::DragFloat3("scale", &scale[0]))
				o->setScale(scale);

			ImGui::DragFloat("ang offset", &o->ang_offset);
			ImGui::DragFloat("speed", &o->speed);
			ImGui::DragFloat("turn speed", &o->turn_speed);

			if (o->model->vertex_skeleton)
			{
				static int boneID = -1;
				if (boneID >= o->model->bones.size()) boneID = -1;

				if (ImGui::TreeNode("Bones Motion"))
				{
					for (int i = 0; i < o->model->bones.size(); i++)
					{
						auto str = tke::translate(936, CP_UTF8, o->model->bones[i].name);
						if (ImGui::Selectable(str.c_str(), i == boneID))
							boneID = i;
					}

					ImGui::TreePop();
				}
			}
		}
		break;
		}
	}
	else
		ImGui::TextUnformatted("Select:Null");

	ImGui::End();
}
