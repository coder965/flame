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
											{
												m->create_geometry_aux();

												auto aux = m->geometry_aux.get();

												float min_x = 0.f, max_x = 0.f, min_z = 0.f, max_z = 0.f;

												std::vector<int> remain_triangles;
												remain_triangles.resize(triangle_count - 1);
												for (int i = 0; i < triangle_count - 1; i++)
													remain_triangles[i] = i + 1;

												static const auto up_dir = glm::vec3(0.f, 1.f, 0.f);

												static std::function<void(int tri_idx, glm::ivec3 swizzle, glm::vec4 base)> fProcessTri;

												auto count = 0;
												fProcessTri = [&](int tri_idx, glm::ivec3 swizzle, glm::vec4 base) {
													if (tri_idx >= 4 && tri_idx <= 5)
														int cut = 1;

													int indices[3];
													glm::vec3 positions[3];
													for (int i = 0; i < 3; i++)
													{
														indices[i] = aux->triangles[tri_idx].indices[i];
														positions[i] = aux->unique_vertex[indices[i]];
													}
													auto v0 = glm::normalize(positions[swizzle[0]] - positions[swizzle[1]]);
													auto v1 = glm::normalize(positions[swizzle[2]] - positions[swizzle[1]]);
													auto n = glm::normalize(glm::cross(v1, v0));
													auto b = glm::cross(v0, n);
													auto src_mat = tke::make_matrix(glm::mat3(v0, n, b), positions[swizzle[1]]);
													auto src_mat_inv = glm::inverse(src_mat);
													glm::vec2 uv[3];
													auto base_dir = glm::vec3(base.x, 0.f, base.y);
													auto dst_mat = tke::make_matrix(glm::mat3(base_dir, up_dir, glm::cross(base_dir, up_dir)), glm::vec3(base.z, 0.f, base.w));
													for (int i = 0; i < 3; i++)
													{
														auto p = src_mat_inv * glm::vec4(positions[i], 1.f);
														p = dst_mat * p;
														uv[i].x = p.x;
														uv[i].y = p.z;
														if (p.x < min_x)
															min_x = p.x;
														if (p.x > max_x)
															max_x = p.x;
														if (p.z < min_z)
															min_z = p.z;
														if (p.z > max_z)
															max_z = p.z;
														aux->triangles[tri_idx].bake_uv[i] = uv[i];
													}

													for (int i = 0; i < 3; i++)
													{
														auto adj_idx = aux->triangles[tri_idx].adjacency[i];
														if (adj_idx.first != -1)
														{
															auto it = std::find(remain_triangles.begin(), remain_triangles.end(), adj_idx.first);
															if (it != remain_triangles.end())
															{
																remain_triangles.erase(it);
																glm::ivec3 swizzle;
																glm::vec4 base;
																switch (adj_idx.second)
																{
																	case 0:
																		swizzle = glm::ivec3(1, 2, 0);
																		break;
																	case 1:
																		swizzle = glm::ivec3(2, 0, 1);
																		break;
																	case 2:
																		swizzle = glm::ivec3(0, 1, 2);
																		break;
																}
																switch (i)
																{
																	case 0:
																		base = glm::vec4(glm::normalize(uv[1] - uv[0]), uv[0]);
																		break;
																	case 1:
																		base = glm::vec4(glm::normalize(uv[2] - uv[1]), uv[1]);
																		break;
																	case 2:
																		base = glm::vec4(glm::normalize(uv[0] - uv[2]), uv[2]);
																		break;
																}
																fProcessTri(adj_idx.first, swizzle, base);
															}
														}
													}
												};

												fProcessTri(0, glm::ivec3(0, 1, 2), glm::vec4(1.f, 0.f, 0.f, 0.f));

												auto cx = max_x - min_x;
												auto cz = max_z - min_z;
												for (int i = 0; i < triangle_count; i++)
												{
													for (int j = 0; j < 3; j++)
													{
														auto &uv = aux->triangles[i].bake_uv[j];
														uv.x -= min_x;
														uv.y -= min_z;
														uv.x /= cx;
														uv.y /= cz;
														int cut = 1;
													}
												}
											}

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

										ImGui::TreePop();
									}
									break;
								}
								case tke::ComponentTypeTerrain:
								{
									if (ImGui::TreeNode("Terrain"))
									{
										auto t = (tke::TerrainComponent*)c.get();

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
					//case tke::NodeTypeObject:
					//{
					//	ImGui::DragFloat("ang offset", &o->ang_offset);
					//	ImGui::DragFloat("speed", &o->speed);
					//	ImGui::DragFloat("turn speed", &o->turn_speed);

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

					//	break;
					//}
					//case tke::NodeTypeTerrain:
					//{
					//	auto t = (tke::Terrain*)n;

					//	ImGui::Text("Blend Map:%s", t->blend_image ? t->blend_image->filename.c_str() : "Null");
					//	show_material(t->materials[0].get());
					//	//show_material(t->materials[1].get());
					//	//show_material(t->materials[2].get());
					//	//show_material(t->materials[3].get());
					//	ImGui::Text("Height:%f", t->height);
					//	ImGui::Text("Use Physx:%s", t->enable_physics ? "Yse" : "No");

					//	break;
					//}
				}
			}
			break;
		}
	}

	ImGui::End();
}