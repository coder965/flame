#include "../../global.h"
#include "../../string_utils.h"
#include "../../ui/ui.h"
#include "../../graphics/image.h"
#include "../../model/model.h"
#include "../../entity/scene.h"
#include "../../entity/model_instance.h"
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
				switch (n->get_type())
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
							if (scene->sky->type == tke::SkyTypeDebug)
								sky_type_index = 1;
							else if (scene->sky->type == tke::SkyTypeAtmosphereScattering)
								sky_type_index = 2;
							else if (scene->sky->type == tke::SkyTypePanorama)
								sky_type_index = 3;
						}
						const char *sky_type_names[] = {
							"Null",
							"Debug",
							"Atmosphere Scattering",
							"Panorama"
						};
						if (ImGui::Combo("sky type", &sky_type_index, sky_type_names, TK_ARRAYSIZE(sky_type_names)))
						{
							switch (sky_type_index)
							{
								case 0:
									scene->setSkyType(tke::SkyTypeNull);
									break;
								case 1:
									scene->setSkyType(tke::SkyTypeDebug);
									break;
								case 2:
									scene->setSkyType(tke::SkyTypeAtmosphereScattering);
									break;
								case 3:
									scene->setSkyType(tke::SkyTypePanorama);
									break;
							}
						}

						if (scene->sky)
						{
							switch (scene->sky->type)
							{
								case tke::SkyTypeAtmosphereScattering:
								{
									static glm::vec2 sun_dir;
									auto as = (tke::SkyAtmosphereScattering*)scene->sky.get();
									if (ImGui::Button("change sun dir"))
									{
										ImGui::OpenPopup("Sun Dir");
										auto euler = as->node->get_euler();
										sun_dir = glm::vec2(euler.x, euler.z);
									}
									if (ImGui::DragFloat("sun power", &as->sun_power, 0.1f, 0.f, 1000.f))
										as->sun_light->set_color(as->sun_color * as->sun_power);
									bool shadow = as->sun_light->is_enable_shadow();
									if (ImGui::Checkbox("sun shadow", &shadow))
										as->sun_light->set_enable_shadow(shadow);

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
								case tke::SkyTypePanorama:
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
													scene->set_pano_sky_image(tke::get_image(filename));
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
					case tke::NodeTypeNode:
					{
						auto coord = n->get_coord();
						if (ImGui::DragFloat3("coord", &coord[0]))
							n->set_coord(coord);
						auto euler = n->get_euler();
						if (ImGui::DragFloat3("euler", &euler[0]))
							n->set_euler(euler);
						auto scale = n->get_scale();
						if (ImGui::DragFloat3("scale", &scale[0]))
							n->set_scale(scale);

						for (auto &c : n->get_components())
						{
							switch (c->get_type())
							{
								case tke::ComponentTypeController:
								{
									if (ImGui::TreeNode("Controller"))
									{

										//	ImGui::DragFloat("ang offset", &o->ang_offset);
										//	ImGui::DragFloat("speed", &o->speed);
										//	ImGui::DragFloat("turn speed", &o->turn_speed);


										ImGui::TreePop();
									}
									break;
								}
								case tke::ComponentTypeCamera:
								{
									if (ImGui::TreeNode("Camera"))
									{
										ImGui::TreePop();
									}
									break;
								}
								case tke::ComponentTypeLight:
								{
									if (ImGui::TreeNode("Light"))
									{
										ImGui::TreePop();
									}
									break;
								}
								case tke::ComponentTypeModelInstance:
								{
									if (ImGui::TreeNode("Model Instance"))
									{
										auto i = (tke::ModelInstanceComponent*)c.get();
										auto m = i->get_model();
										ImGui::Text("model:%s", m->filename.c_str());
										if (ImGui::TreeNode("Bake"))
										{
											auto triangle_count = m->indice_count / 3;

											ImGui::TextUnformatted("UV:");

											ImDrawList* draw_list = ImGui::GetWindowDrawList();
											ImVec2 pos = ImGui::GetCursorScreenPos();
											ImVec2 size = ImVec2(256.f, 256.f);
											draw_list->AddRect(pos - ImVec2(2.f, 2.f), pos + size + ImVec2(3.f, 3.f), ImColor(255, 255, 255));
											ImGui::InvisibleButton("canvas", size);

											auto aux = m->geometry_aux.get();
											if (aux)
											{
												draw_list->PushClipRect(pos - ImVec2(1.f, 1.f), pos + size + ImVec2(1.f, 1.f), true);
												for (int i = 0; i < triangle_count; i++)
												{
													glm::vec2 uv[3];
													for (int j = 0; j < 3; j++)
														uv[j] = aux->triangles[i].bake_uv[j];

													draw_list->AddLine(
														pos + ImVec2(uv[0].x, uv[0].y) * size,
														pos + ImVec2(uv[1].x, uv[1].y) * size,
														IM_COL32(255, 255, 0, 255)
													);
													draw_list->AddLine(
														pos + ImVec2(uv[1].x, uv[1].y) * size,
														pos + ImVec2(uv[2].x, uv[2].y) * size,
														IM_COL32(255, 255, 0, 255)
													);
													draw_list->AddLine(
														pos + ImVec2(uv[2].x, uv[2].y) * size,
														pos + ImVec2(uv[0].x, uv[0].y) * size,
														IM_COL32(255, 255, 0, 255)
													);

												}
												draw_list->PopClipRect();
											}

											if (ImGui::Button("Create UV"))
												m->create_uv();

											ImGui::TreePop();
										}
										if (ImGui::TreeNode("Geometries"))
										{
											static int geo_index = -1;
											ImGui::Combo("##geometry", &geo_index, [](void *data, int idx, const char **out_text) {
												auto m = (tke::Model*)data;
												*out_text = m->geometries[idx]->name.c_str();
												return true;
											}, m, m->geometries.size());
											if (geo_index >= 0 && geo_index < m->geometries.size())
											{
												auto g = m->geometries[geo_index].get();
												ImGui::Text("Indice Base:%d", g->indiceBase);
												ImGui::Text("Indice Count:%d", g->indiceCount);
												ImGui::Separator();
												ImGui::Text("Material:%s", g->material->name.c_str());
											}

											ImGui::TreePop();
										}

										//	if (o->model->vertex_skeleton)
										//	{
										//		static int boneID = -1;
										//		if (boneID >= o->model->bones.size()) boneID = -1;

										//		if (ImGui::TreeNode("Bones Motion"))
										//		{
										//			for (int i = 0; i < o->model->bones.size(); i++)
										//			{
										//				auto str = tke::translate(936, CP_UTF8, o->model->bones[i]->name);
										//				if (ImGui::Selectable(str.c_str(), i == boneID))
										//					boneID = i;
										//			}

										//			ImGui::TreePop();
										//		}
										//	}

										ImGui::TreePop();
									}
									break;
								}
								case tke::ComponentTypeTerrain:
								{
									if (ImGui::TreeNode("Terrain"))
									{
										auto t = (tke::TerrainComponent*)c.get();

										//	ImGui::Text("Blend Map:%s", t->blend_image ? t->blend_image->filename.c_str() : "Null");
										//	show_material(t->materials[0].get());
										//	//show_material(t->materials[1].get());
										//	//show_material(t->materials[2].get());
										//	//show_material(t->materials[3].get());
										//	ImGui::Text("Height:%f", t->height);
										//	ImGui::Text("Use Physx:%s", t->enable_physics ? "Yse" : "No");

										ImGui::TreePop();
									}
									break;
								}
								case tke::ComponentTypeWater:
								{
									if (ImGui::TreeNode("Water"))
									{
										ImGui::TreePop();
									}
									break;
								}
							}
						}

						break;
					}
				}
			}
			break;
		}
	}

	ImGui::End();
}