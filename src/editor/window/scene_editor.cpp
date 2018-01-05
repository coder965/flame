#include "../../ui/ui.h"
#include "../../file_utils.h"
#include "../../graphics/buffer.h"
#include "../../graphics/descriptor.h"
#include "../../model/model.h"
#include "../../model/animation.h"
#include "../../entity/light.h"
#include "../../entity/object.h"
#include "../../entity/terrain.h"
#include "../../physics/physics.h"
#include "../../pick_up/pick_up.h"
#include "../../input.h"
#include "../../global.h"
#include "../../application.h"
#include "resource_explorer.h"
#include "scene_editor.h"

std::unique_ptr<SceneEditor> scene_editor = nullptr;

SceneEditor::SceneEditor(std::shared_ptr<tke::Scene> _scene)
	:scene(_scene)
{
	plain_renderer = std::make_unique<tke::PlainRenderer>();
	defe_renderer = std::make_unique<tke::DeferredRenderer>(false, layer.image.get());

	physx_vertex_buffer = std::make_unique<tke::ImmediateVertexBuffer>();
	lines_renderer = std::make_unique<tke::LinesRenderer>();

	transformerTool = std::make_unique<TransformerTool>(layer.image.get());
}

SceneEditor::~SceneEditor()
{
	if (entity_window)
		entity_window->opened = false;
}

void SceneEditor::on_file_menu()
{
	if (ImGui::MenuItem("Save", "Ctrl+S"))
		scene->save(scene->filename);
	if (ImGui::MenuItem("Close", "Ctrl+S"))
		scene_editor.reset();
}

bool openCreateLightPopup = false;
bool openCreateObjectPopup = false;
bool openCreateTerrainPopup = false;
bool openCreateWaterPopup = false;

void SceneEditor::on_menu_bar()
{
	openCreateLightPopup = false;
	openCreateObjectPopup = false;
	openCreateTerrainPopup = false;
	openCreateWaterPopup = false;

	if (ImGui::BeginMenu_keepalive("Create"))
	{
		if (ImGui::MenuItem("Light"))
			openCreateLightPopup = true;
		if (ImGui::MenuItem("Object"))
			openCreateObjectPopup = true;
		if (ImGui::MenuItem("Terrain"))
			openCreateTerrainPopup = true;
		if (ImGui::MenuItem("Water"))
			openCreateWaterPopup = true;

		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu_keepalive("Edit"))
	{
		if (ImGui::MenuItem("Cut", "Ctrl+X"))
			;
		if (ImGui::MenuItem("Copy", "Ctrl+C"))
			;
		if (ImGui::MenuItem("Paste", "Ctrl+V"))
			;
		if (ImGui::MenuItem("Delete", "Del"))
			;

		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu_keepalive("Show"))
	{
		ImGui::MenuItem("Enable Render", "", &enableRender);
		ImGui::MenuItem("Show Selected Wire Frame", "", &showSelectedWireframe);
		if (ImGui::MenuItem("View Physx", "", &viewPhysx))
		{
			if (viewPhysx)
			{
				scene->pxScene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, 1.f);
				////scene->setVisualizationParameter(PxVisualizationParameter::eACTOR_AXES, 1.f);
				scene->pxScene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_SHAPES, 1.f);
				//scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_DYNAMIC, 1.f);
				//pxScene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LOCAL_FRAMES, 1.f);
				////scene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LIMITS, 1.f);
			}
			else
				scene->pxScene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, 0.f);
		}

		ImGui::EndMenu();
	}

	bool target = false;
	if (ImGui::BeginMenu_keepalive("Camera"))
	{
		if (ImGui::MenuItem("Target To Selected"))
			target = true;
		ImGui::MenuItem("Follow", "", &follow);

		if (ImGui::BeginMenu("Mode"))
		{
			if (ImGui::MenuItem("Free", "", scene->camera.mode == tke::CameraMode::free))
				scene->camera.setMode(tke::CameraMode::free);
			if (ImGui::MenuItem("Targeting", "", scene->camera.mode == tke::CameraMode::targeting))
				scene->camera.setMode(tke::CameraMode::targeting);

			ImGui::EndMenu();
		}

		{
			auto c = scene->camera.getCoord();
			ImGui::Text("%f, %f, %f coord", c.x, c.y, c.z);
		}

		ImGui::EndMenu();
	}
	if (target || follow)
	{
		auto s = selected.lock();
		if (s && s->type == tke::NodeTypeObject)
			scene->camera.object = (tke::Object*)s.get();
		else
			scene->camera.object = nullptr;
	}

	if (ImGui::BeginMenu_keepalive("Sky"))
	{
		if (ImGui::BeginMenu("Type"))
		{
			if (ImGui::MenuItem("Null", "", !scene->sky))
			{
				if (scene->sky)
					scene->setSkyType(tke::SkyType::null);
			}
			if (ImGui::MenuItem("Atmosphere Scattering", "", scene->sky && scene->sky->type == tke::SkyType::atmosphere_scattering))
			{
				if (!scene->sky || scene->sky->type != tke::SkyType::atmosphere_scattering)
					scene->setSkyType(tke::SkyType::atmosphere_scattering);
			}
			if (ImGui::MenuItem("Panorama", "", scene->sky && scene->sky->type == tke::SkyType::panorama))
			{
				if (!scene->sky || scene->sky->type != tke::SkyType::panorama)
					scene->setSkyType(tke::SkyType::panorama);
			}

			ImGui::EndMenu();
		}

		auto ambientColor = scene->ambientColor;
		if (ImGui::DragFloat3("Ambient Color", &ambientColor[0], 0.1f, 0.f, 100.f))
			scene->setAmbientColor(ambientColor);
		auto fogColor = scene->fogColor;
		if (ImGui::DragFloat3("Fog Color", &fogColor[0], 0.1f, 0.f, 100.f))
			scene->setFogColor(fogColor);

		ImGui::EndMenu();
	}
}

void SceneEditor::on_view_menu()
{
	if (ImGui::MenuItem("Entity Window"))
	{
		if (!entity_window)
			entity_window = new EntityWindow;
		entity_window->_need_focus = true;
	}
}

void SceneEditor::do_show()
{
	static bool use_camera_position = false;
	static bool use_camera_target_position = false;
	static glm::vec3 coord = glm::vec3(0.f);
	static glm::vec3 euler = glm::vec3(0.f);
	static glm::vec3 scale = glm::vec3(1.f);
	auto funShowCoordUi = [&]() {
		ImGui::Checkbox("Use Camera Position", &use_camera_position);
		if (use_camera_position)
			ImGui::Checkbox("Use Camera Target Position", &use_camera_target_position);
		if (!use_camera_position)
			ImGui::DragFloat3("coord", (float*)&coord[0], 0.5f);
		else
		{
			glm::vec3 c;
			if (!use_camera_target_position)
				c = scene->camera.getCoord();
			else
				c = scene->camera.target;
			ImGui::Text("%f %f %f coord", c.x, c.y, c.z);
		}
	};

	if (openCreateLightPopup)
		ImGui::OpenPopup("Create Light");
	if (openCreateObjectPopup)
		ImGui::OpenPopup("Create Object");
	if (openCreateTerrainPopup)
		ImGui::OpenPopup("Create Terrain");
	if (openCreateWaterPopup)
		ImGui::OpenPopup("Create Water");
	if (ImGui::BeginPopupModal("Create Object", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		static bool basicModel = false;
		static int model_index = 0;
		static char model_filename[260];
		ImGui::Checkbox("Basic Model", &basicModel);
		if (basicModel)
			ImGui::Combo("##Model", &model_index, basic_model_names, TK_ARRAYSIZE(basic_model_names));
		else
			ImGui::InputText("Filename", model_filename, TK_ARRAYSIZE(model_filename));

		if (ImGui::TreeNode("Transform"))
		{
			funShowCoordUi();
			ImGui::DragFloat3("euler", (float*)&euler[0], 0.5f);
			ImGui::DragFloat3("scale", (float*)&scale[0], 0.5f);

			ImGui::TreePop();
		}

		static bool physx_enable = false;
		static bool physx_dynamic = false;
		static bool physx_use_controller = false;
		if (ImGui::TreeNode("Physx"))
		{
			ImGui::Checkbox("enable", &physx_enable);
			if (physx_enable)
			{
				ImGui::Checkbox("dynamic", &physx_dynamic);
				ImGui::Checkbox("use controller", &physx_use_controller);
			}

			ImGui::TreePop();
		}

		if (ImGui::Button("Create"))
		{
			unsigned int _physxType = 0;
			if (physx_enable)
				_physxType |= (int)tke::ObjectPhysicsType::enable;
			if (physx_dynamic)
				_physxType |= (int)tke::ObjectPhysicsType::dynamic;
			if (physx_use_controller)
				_physxType |= (int)tke::ObjectPhysicsType::controller;

			if (basicModel)
				strcpy(model_filename, basic_model_names[model_index]);

			auto m = tke::getModel(model_filename);
			if (m)
			{
				auto o = new tke::Object(m, _physxType);

				glm::vec3 _coord;
				if (use_camera_position)
				{
					if (use_camera_target_position)
						_coord = scene->camera.target;
					else
						_coord = scene->camera.getCoord();
				}
				else
					_coord = coord;
				o->setCoord(_coord);
				o->setEuler(euler);
				o->setScale(scale);

				scene->addObject(o);
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Close"))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}
	if (ImGui::BeginPopupModal("Create Terrain", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (ImGui::TreeNode("Transform"))
		{
			funShowCoordUi();
			ImGui::TreePop();
		}
		static char blend_image_name[260];
		ImGui::InputText("Blend Map", blend_image_name, TK_ARRAYSIZE(blend_image_name));
		static float height = 100.f;
		ImGui::DragFloat("Height", &height);
		static bool use_physx = false;
		ImGui::Checkbox("Use Physx", &use_physx);
		if (ImGui::Button("Create"))
		{
			auto t = new tke::Terrain(use_physx, tke::getImage(blend_image_name));
			t->setCoord(coord);
			t->height = height;
			scene->addTerrain(t);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Close"))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}
	if (ImGui::BeginPopupModal("Create Water", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		//for (int i = 0; i < 3; i++)
		//{
		//	char *strs[] = {"CoordX", "CoordY", "CoordZ"};
		//	ImGui::DragFloat(strs[i], &wcs.coord[i], 0.5f);
		//}
		//ImGui::DragFloat("Height", &wcs.height);
		//if (ImGui::Button("Create"))
		//{
		//	auto w = new tke::Water;
		//	w->setCoord(wcs.coord);
		//	w->height = wcs.height;
		//	scene->addWater(w);
		//}
		//ImGui::SameLine();
		if (ImGui::Button("Close"))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}

	ImVec2 image_pos;
	auto displayCx = tke::window_cx;
	auto displayCy = tke::window_cy;
	ImVec2 image_size;
	if ((float)displayCx / (float)displayCy > tke::res_aspect)
	{
		image_size.y = displayCy;
		image_size.x = tke::res_aspect * image_size.y;
		image_pos.x = (displayCx - image_size.x) * 0.5f;
		image_pos.y = 0;
	}
	else
	{
		image_size.x = displayCx;
		image_size.y = image_size.x / tke::res_aspect;
		image_pos.y = (displayCy - image_size.y) * 0.5f;
		image_pos.x = 0;
	}
	ImGui::SetCursorScreenPos(image_pos);
	ImGui::InvisibleButton("canvas", image_size);
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	draw_list->AddImage(ImTextureID(layer.image->index), image_pos, image_pos + image_size);
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("file"))
		{
			static char filename[260];
			strcpy(filename, (char*)payload->Data);
			std::experimental::filesystem::path path(filename);
			auto ext = path.extension();
			if (tke::is_model_file(ext.string()))
			{
				auto m = tke::getModel(filename);
				if (m)
				{
					auto o = new tke::Object(m, 0);
					scene->addObject(o);
				}
			}
		}
		ImGui::EndDragDropTarget();
	}
	if (ImGui::IsItemHovered())
	{
		if (tke::mouseDispX != 0 || tke::mouseDispY != 0)
		{
			auto distX = (float)tke::mouseDispX / (float)tke::res_cx;
			auto distY = (float)tke::mouseDispY / (float)tke::res_cy;
			if (tke::keyStates[VK_SHIFT].pressing && tke::mouseMiddle.pressing)
				scene->camera.moveByCursor(distX, distY);
			else if (tke::keyStates[VK_CONTROL].pressing && tke::mouseMiddle.pressing)
				scene->camera.scroll(distX);
			else if (tke::mouseMiddle.pressing)
				scene->camera.rotateByCursor(distX, distY);
			else if (!tke::keyStates[VK_SHIFT].pressing && !tke::keyStates[VK_CONTROL].pressing)
			{
				if (tke::mouseLeft.pressing)
					transformerTool->mouseMove(tke::mouseDispX, tke::mouseDispY);
			}
		}
		if (tke::mouseScroll != 0)
			scene->camera.scroll(tke::mouseScroll);
		if (tke::mouseLeft.justDown)
		{
			if (!tke::keyStates[VK_SHIFT].pressing && !tke::keyStates[VK_CONTROL].pressing)
			{
				auto x = (tke::mouseX - image_pos.x) / image_size.x * tke::res_cx;
				auto y = (tke::mouseY - image_pos.y) / image_size.y * tke::res_cy;
				if (!transformerTool->leftDown(x, y))
				{
					tke::PlainRenderer::DrawData draw_data;
					draw_data.mode = tke::PlainRenderer::mode_just_color;
					for (int i = 0; i < scene->objects.size(); i++)
					{
						auto object = scene->objects[i].get();

						tke::PlainRenderer::DrawData::ObjData obj_data;
						obj_data.mat = object->getMat();
						obj_data.color = glm::vec4((i + 1) / 255.f, 0.f, 0.f, 0.f);
						obj_data.fill_with_model(object->model.get());
						if (object->model->vertex_skeleton)
							obj_data.bone_buffer = object->animationComponent->bone_buffer.get();
						draw_data.obj_data.push_back(obj_data);
					}
					auto index = tke::pick_up(x, y, std::bind(
						&tke::PlainRenderer::render_to,
						plain_renderer.get(), std::placeholders::_1, &scene->camera, &draw_data));
					if (index == 0)
						selected.reset();
					else
						selected = scene->objects[index - 1];
				}
			}
		}
	}

	{
		//char *names[] = {
		//	ICON_FA_MOUSE_POINTER, "M", "R", "S"
		//};
		for (int i = 0; i < 4; i++)
		{
			auto needPop = false;
			if (transformerTool->mode == TransformerTool::Mode(i))
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(0.f, 0.6f, 0.6f).Value);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(0.f, 0.7f, 0.7f).Value);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(0.f, 0.8f, 0.8f).Value);
				needPop = true;
			}
			//if (ImGui::Button(names[i])) transformerTool->mode = TransformerTool::Mode(i);
			if (needPop) ImGui::PopStyleColor(3);
			if (i < 3) ImGui::SameLine();
		}
	}

	scene->update();
	if (enableRender)
		defe_renderer->render(nullptr, true, nullptr, scene.get());
	scene->reset();

	{
		//for (int i = 0; i < rb.getNbPoints(); i++)
		//{
		//	p.color = _intToRGB(rb.getPoints()[i].color);
		//	p.coord = _pxVec3ToVec3(rb.getPoints()[i].pos);
		//	tke3_debugBuffer.points.push_back(p);
		//}

		//glNamedBufferSubData(tke_dynamicVertexVBO, 0, sizeof(GLfloat) * 12 * lineCount, tke_dynamicVertexBuffer);
		//glBindVertexArray(tke3_dynamicVertexVAO);
		//glDrawArrays(GL_LINES, 0, lineCount * 2);

		//auto triangleCount = rb.getNbTriangles();
		//for (int i = 0; i < triangleCount; i++)
		//{
		//	auto &triangle = rb.getTriangles()[i];
		//	tke_dynamicVertexBuffer[i * 18 + 0] = triangle.pos0.x;
		//	tke_dynamicVertexBuffer[i * 18 + 1] = triangle.pos0.y;
		//	tke_dynamicVertexBuffer[i * 18 + 2] = triangle.pos0.z;
		//	tke_dynamicVertexBuffer[i * 18 + 3] = triangle.color0 % 256;
		//	tke_dynamicVertexBuffer[i * 18 + 4] = (triangle.color0 / 256) % 256;
		//	tke_dynamicVertexBuffer[i * 18 + 5] = (triangle.color0 / 65536) % 256;
		//	tke_dynamicVertexBuffer[i * 18 + 6] = triangle.pos1.x;
		//	tke_dynamicVertexBuffer[i * 18 + 7] = triangle.pos1.y;
		//	tke_dynamicVertexBuffer[i * 18 + 8] = triangle.pos1.z;
		//	tke_dynamicVertexBuffer[i * 18 + 9] = triangle.color1 % 256;
		//	tke_dynamicVertexBuffer[i * 18 + 10] = (triangle.color1 / 256) % 256;
		//	tke_dynamicVertexBuffer[i * 18 + 11] = (triangle.color1 / 65536) % 256;
		//	tke_dynamicVertexBuffer[i * 18 + 12] = triangle.pos2.x;
		//	tke_dynamicVertexBuffer[i * 18 + 13] = triangle.pos2.y;
		//	tke_dynamicVertexBuffer[i * 18 + 14] = triangle.pos2.z;
		//	tke_dynamicVertexBuffer[i * 18 + 15] = triangle.color2 % 256;
		//	tke_dynamicVertexBuffer[i * 18 + 16] = (triangle.color2 / 256) % 256;
		//	tke_dynamicVertexBuffer[i * 18 + 17] = (triangle.color2 / 65536) % 256;
		//}
		//glNamedBufferSubData(tke_dynamicVertexVBO, 0, sizeof(GLfloat) * 18 * triangleCount, tke_dynamicVertexBuffer);
		//glBindVertexArray(tke3_dynamicVertexVAO);
		//glDrawArrays(GL_TRIANGLES, 0, triangleCount * 3);
	}
	if (viewPhysx)
	{
		auto &rb = scene->pxScene->getRenderBuffer();
		auto lineCount = rb.getNbLines();
		if (lineCount > 0)
		{
			auto vertex_count = lineCount * 2;
			auto size = vertex_count * sizeof(tke::LinesRenderer::Vertex);
			auto vtx_dst = (tke::LinesRenderer::Vertex*)physx_vertex_buffer->map(0, size);
			for (int i = 0; i < lineCount; i++)
			{
				auto &line = rb.getLines()[i];
				vtx_dst[0].position.x = line.pos0.x;
				vtx_dst[0].position.y = line.pos0.y;
				vtx_dst[0].position.z = line.pos0.z;
				vtx_dst[0].color.r = line.color0 % 256;
				vtx_dst[0].color.g = (line.color0 / 256) % 256;
				vtx_dst[0].color.b = (line.color0 / 65536) % 256;
				vtx_dst[1].position.x = line.pos1.x;
				vtx_dst[1].position.y = line.pos1.y;
				vtx_dst[1].position.z = line.pos1.z;
				vtx_dst[1].color.r = line.color1 % 256;
				vtx_dst[1].color.g = (line.color1 / 256) % 256;
				vtx_dst[1].color.b = (line.color1 / 65536) % 256;
				vtx_dst += 2;
			}
			physx_vertex_buffer->unmap();
			tke::LinesRenderer::DrawData data;
			data.vertex_buffer = physx_vertex_buffer.get();
			data.vertex_count = vertex_count;
			lines_renderer->render(layer.framebuffer.get(), false, &scene->camera, &data);
		}
	}

	if (showSelectedWireframe)
	{
		auto s = selected.lock();
		if (s && s->type == tke::NodeTypeObject)
		{
			auto obj = (tke::Object*)s.get();
			tke::PlainRenderer::DrawData data;
			data.mode = tke::PlainRenderer::mode_wireframe;
			tke::PlainRenderer::DrawData::ObjData obj_data;
			obj_data.mat = obj->getMat();
			obj_data.color = glm::vec4(0.f, 1.f, 0.f, 1.f);
			obj_data.fill_with_model(obj->model.get());
			if (obj->model->vertex_skeleton)
				obj_data.bone_buffer = obj->animationComponent->bone_buffer.get();
			data.obj_data.push_back(obj_data);
			plain_renderer->render(layer.framebuffer.get(), false, &scene->camera, &data);
		}
	}

	{
		auto s = selected.lock();
		if (s)
		{
			switch (s->type)
			{
			case tke::NodeTypeLight:
				transformerTool->transformer = (tke::Light*)s.get();
				break;
			case tke::NodeTypeObject:
				transformerTool->transformer = (tke::Object*)s.get();
				break;
			default:
				transformerTool->transformer = nullptr;
			}
		}
		transformerTool->show(&scene->camera);
	}
}

void SceneEditor::save(tke::XMLNode *n)
{
	n->addAttribute("filename", scene->filename);
	n->addAttribute("follow", &follow);
}
