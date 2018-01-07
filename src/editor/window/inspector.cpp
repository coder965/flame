#include "../../language.h"
#include "../../ui/ui.h"
#include "../../graphics/image.h"
#include "../../model/model.h"
#include "../../entity/scene.h"
#include "../../entity/object.h"
#include "../../entity/terrain.h"
#include "../select.h"
#include "inspector.h"
#include "show_material.h"

InspectorWindow *inspector_window = nullptr;

InspectorWindow::~InspectorWindow()
{
	inspector_window = nullptr;
}

void InspectorWindow::do_show()
{
	ImGui::Begin("Inspector", &opened);

	switch (selected.type)
	{
		case SelectTypeNode:
		{
			auto n = selected.get_node();
			if (n)
			{
				switch (n->type)
				{
					case tke::NodeTypeScene:
					{
						auto scene = (tke::Scene*)n;

						auto ambientColor = scene->ambientColor;
						if (ImGui::DragFloat3("Ambient Color", &ambientColor[0], 0.1f, 0.f, 100.f))
							scene->setAmbientColor(ambientColor);
						auto fogColor = scene->fogColor;
						if (ImGui::DragFloat3("Fog Color", &fogColor[0], 0.1f, 0.f, 100.f))
							scene->setFogColor(fogColor);

						int sky_type_index;
						if (!scene->sky)
							sky_type_index = 0;
						else
						{
							if (scene->sky->type == tke::SkyType::atmosphere_scattering)
								sky_type_index = 1;
							else if (scene->sky->type == tke::SkyType::panorama)
								sky_type_index = 2;
						}
						const char *sky_type_names[] = {
							"Null",
							"Atmosphere Scattering",
							"Panorama"
						};
						if (ImGui::Combo("sky type", &sky_type_index, sky_type_names, TK_ARRAYSIZE(sky_type_names)))
						{
							switch (sky_type_index)
							{
								case 0:
									scene->setSkyType(tke::SkyType::null);
									break;
								case 1:
									scene->setSkyType(tke::SkyType::atmosphere_scattering);
									break;
								case 2:
									scene->setSkyType(tke::SkyType::panorama);
									break;
							}
						}

						if (scene->sky)
						{
							switch (scene->sky->type)
							{
								case tke::SkyType::atmosphere_scattering:
								{
									static glm::vec2 sun_dir;
									auto as = (tke::SkyAtmosphereScattering*)scene->sky.get();
									if (ImGui::Button("change sun dir"))
									{
										ImGui::OpenPopup("Sun Dir");
										auto euler = as->sun_light->getEuler();
										sun_dir = glm::vec2(euler.x, euler.z);
									}
									if (ImGui::DragFloat("sun power", &as->sun_power, 0.1f, 0.f, 1000.f))
										as->sun_light->setColor(as->sun_color * as->sun_power);
									ImGui::Checkbox("sun shadow", &as->sun_light->shadow);

									if (ImGui::BeginPopupModal("Sun Dir", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
									{
										ImGui::DragFloat("x", &sun_dir[0]);
										ImGui::DragFloat("y", &sun_dir[1]);
										if (ImGui::Button("Ok"))
										{
											scene->setSunDir(sun_dir);
											ImGui::CloseCurrentPopup();
										}
										ImGui::SameLine();
										if (ImGui::Button("Cancel"))
											ImGui::CloseCurrentPopup();

										ImGui::EndPopup();
									}
									break;
								}
								case tke::SkyType::panorama:
								{
									auto pa = (tke::SkyPanorama*)scene->sky.get();
									ImGui::Text("Image:%s", pa->panoImage ? pa->panoImage->filename.c_str() : "Null");
									if (ImGui::BeginDragDropTarget())
									{
										if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("file"))
										{
											static char filename[260];
											strcpy(filename, (char*)payload->Data);
											if (!pa->panoImage || pa->panoImage->filename != filename)
											{
												std::experimental::filesystem::path path(filename);
												auto ext = path.extension();
												if (tke::is_image_file(ext.string()))
												{
													pa->panoImage = tke::getImage(filename);
													scene->needUpdateSky = true;
												}
											}
										}
										ImGui::EndDragDropTarget();
									}
									break;
								}
							}
						}

						break;
					}
					case tke::NodeTypeLight:
					{
						break;
					}
					case tke::NodeTypeObject:
					{
						auto o = (tke::Object*)n;

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
									auto str = tke::translate(936, CP_UTF8, o->model->bones[i]->name);
									if (ImGui::Selectable(str.c_str(), i == boneID))
										boneID = i;
								}

								ImGui::TreePop();
							}
						}

						break;
					}
					case tke::NodeTypeTerrain:
					{
						auto t = (tke::Terrain*)n;

						ImGui::Text("Blend Map:%s", t->blendMap ? t->blendMap->filename.c_str() : "Null");
						show_material(t->materials[0].get());
						//show_material(t->materials[1].get());
						//show_material(t->materials[2].get());
						//show_material(t->materials[3].get());
						ImGui::Text("Height:%f", t->height);
						ImGui::Text("Use Physx:%s", t->use_physx ? "Yse" : "No");

						break;
					}
					case tke::NodeTypeWater:
					{
						break;
					}
				}
			}
			break;
		}
	}

	ImGui::End();
}