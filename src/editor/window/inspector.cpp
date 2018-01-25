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

InspectorWindow::InspectorWindow() :
	Window("Inspector")
{
}

InspectorWindow::~InspectorWindow()
{
	inspector_window = nullptr;
}

void InspectorWindow::on_show()
{
	switch (selected.type)
	{
		case SelectTypeFile:
		{
			static std::string filename;
			static bool is_folder;
			static long long file_size;
			static std::string file_size_text;
			static tke::FileType file_type;
			static glm::ivec2 image_file_resolution;
			if (selected.get_filename() != filename)
			{
				filename = selected.get_filename();
				std::fs::path path(filename);
				is_folder = std::fs::is_directory(path);
				if (!is_folder)
				{
					std::error_code e;
					file_size = std::fs::file_size(path, e);
					if (file_size < 1024)
						file_size_text = "size: " + std::to_string(file_size) + " Byte";
					else if (file_size < 1024 * 1024)
						file_size_text = "size: " + std::_Floating_to_string("%.2f", float(file_size) / 1024.f) + " KB";
					else if (file_size < 1024 * 1024 * 1024)
						file_size_text = "size: " + std::_Floating_to_string("%.2f", float(file_size) / 1024.f / 1024.f) + " MB";
					else
						file_size_text = "size: " + std::_Floating_to_string("%.2f", float(file_size) / 1024.f / 1024.f / 1024.f) + " GB";

					auto ext = path.extension().string();
					file_type = tke::get_file_type(ext);

					switch (file_type)
					{
						case tke::FileTypeImage:
						{
							auto i = tke::get_image(filename);
							if (i)
							{
								image_file_resolution.x = i->levels[0].cx;
								image_file_resolution.y = i->levels[0].cy;
							}
							break;
						}
					}
				}
			}
			ImGui::Text("filename: %s", selected.get_filename().c_str());
			if (!is_folder)
			{
				ImGui::TextUnformatted(file_size_text.c_str());

				switch (file_type)
				{
					case tke::FileTypeImage:
					{
						ImGui::Text("resolution: %d x %d", image_file_resolution.x, image_file_resolution.y);
						break;
					}
				}
			}
			break;
		}
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

						auto bg_color = scene->get_bg_color();
						if (ImGui::ColorEdit3("background color", &bg_color[0], ImGuiColorEditFlags_NoInputs))
							scene->set_bg_color(bg_color);
						auto ambient_color = scene->get_ambient_color();
						if (ImGui::ColorEdit3("ambient color", &ambient_color[0], ImGuiColorEditFlags_NoInputs))
							scene->set_ambient_color(ambient_color);
						auto fog_color = scene->get_fog_color();
						if (ImGui::ColorEdit3("fog color", &fog_color[0], ImGuiColorEditFlags_NoInputs))
							scene->set_fog_color(fog_color);

						int sky_type = scene->get_sky_type();
						const char *sky_type_names[] = {
							"null",
							"debug",
							"atmosphere scattering",
							"panorama"
						};
						if (ImGui::Combo("sky type", &sky_type, sky_type_names, TK_ARRAYSIZE(sky_type_names)))
							scene->set_sky_type((tke::SkyType)sky_type);

						switch (scene->get_sky_type())
						{
							case tke::SkyTypeAtmosphereScattering:
							{
								static glm::vec2 sun_dir;
								auto as = (tke::SkyAtmosphereScattering*)scene->get_sky();
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
								auto pa = (tke::SkyPanorama*)scene->get_sky();
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

						break;
					}
					case tke::NodeTypeNode:
					{
						auto coord = n->get_coord();
						if (ImGui::DragFloat3("coord", &coord[0], 0.05f))
							n->set_coord(coord);
						auto euler = n->get_euler();
						if (ImGui::DragFloat3("euler", &euler[0], 0.05f))
							n->set_euler(euler);
						auto scale = n->get_scale();
						if (ImGui::DragFloat3("scale", &scale[0], 0.05f))
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
										if (ImGui::TreeNode("UV"))
										{
											static int uv_index = 0;
											tke::Model::UV *uv = nullptr;
											ImGui::Combo("uv", &uv_index, [](void *data, int idx, const char **out_text) {
												auto m = (tke::Model*)data;
												*out_text = m->uvs[idx]->name.c_str();
												return true;
											}, m, m->uvs.size());
											if (uv_index < m->uvs.size())
												uv = m->uvs[uv_index].get();
											int use_index = -1;
											if (uv)
											{
												if (m->geometry_uv == uv && m->bake_uv == uv)
													use_index = 3;
												else if (m->geometry_uv == uv)
													use_index = 1;
												else if (m->bake_uv == uv)
													use_index = 2;
												else
													use_index = 0;
											}
											static const char *use_names[] = {
												"null",
												"geometry",
												"bake",
												"geometry and bake"
											};
											if (ImGui::Combo("use as", &use_index, use_names, TK_ARRAYSIZE(use_names)))
											{
												if (uv)
												{
													switch (use_index)
													{
														case 0:
															if (m->geometry_uv == uv)
																m->assign_uv_to_geometry(nullptr);
															if (m->bake_uv == uv)
																m->assign_uv_to_bake(nullptr);
															break;
														case 1:
															m->assign_uv_to_geometry(uv);
															if (m->bake_uv == uv)
																m->assign_uv_to_bake(nullptr);
															break;
														case 2:
															if (m->geometry_uv == uv)
																m->assign_uv_to_geometry(nullptr);
															m->assign_uv_to_bake(uv);
															break;
														case 3:
															m->assign_uv_to_geometry(uv);
															m->assign_uv_to_bake(uv);
															break;
													}
												}
											}

											if (ImGui::Button("Create"))
												m->create_uv();
											ImGui::SameLine();
											if (ImGui::Button("Remove"))
											{
												if (uv)
												{
													m->remove_uv(uv);
													uv = nullptr;
												}
											}

											auto triangle_count = m->indices.size() / 3;

											ImDrawList* draw_list = ImGui::GetWindowDrawList();
											ImVec2 pos = ImGui::GetCursorScreenPos();
											ImVec2 size = ImVec2(256.f, 256.f);
											draw_list->AddRect(pos - ImVec2(2.f, 2.f), pos + size + ImVec2(3.f, 3.f), ImColor(255, 255, 255));
											ImGui::InvisibleButton("canvas", size);

											static bool overlap_bake_grid = false;
											ImGui::Checkbox("overlap bake grid", &overlap_bake_grid);

											draw_list->PushClipRect(pos - ImVec2(1.f, 1.f), pos + size + ImVec2(1.f, 1.f), true);
											if (uv)
											{
												for (int i = 0; i < triangle_count; i++)
												{
													glm::vec2 p[3];
													for (int j = 0; j < 3; j++)
														p[j] = uv->unique[uv->indices[i * 3 + j]];
													draw_list->AddLine(
														pos + ImVec2(p[0].x, p[0].y) * size,
														pos + ImVec2(p[1].x, p[1].y) * size,
														IM_COL32(255, 255, 255, 255)
													);
													draw_list->AddLine(
														pos + ImVec2(p[1].x, p[1].y) * size,
														pos + ImVec2(p[2].x, p[2].y) * size,
														IM_COL32(255, 255, 255, 255)
													);
													draw_list->AddLine(
														pos + ImVec2(p[2].x, p[2].y) * size,
														pos + ImVec2(p[0].x, p[0].y) * size,
														IM_COL32(255, 255, 255, 255)
													);
												}
											}
											if (overlap_bake_grid)
											{
												for (int x = 0; x <= 256; x += m->bake_grid_pixel_size)
												{
													draw_list->AddLine(
														pos + ImVec2(x, 0),
														pos + ImVec2(x, 256),
														IM_COL32(255, 255, 0, 128)
													);
												}
												for (int y = 0; y <= 256; y += m->bake_grid_pixel_size)
												{
													draw_list->AddLine(
														pos + ImVec2(0, y),
														pos + ImVec2(256, y),
														IM_COL32(255, 255, 0, 128)
													);
												}
											}
											draw_list->PopClipRect();

											if (ImGui::TreeNode("Bake"))
											{

												ImGui::TreePop();
											}

											ImGui::TreePop();
										}
										if (ImGui::TreeNode("Geometries"))
										{
											static int geo_index = 0;
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
												ImGui::Text("Material:%s", g->material->get_name().c_str());
												show_material(g->material.get());
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
}