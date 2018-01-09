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
#include "../select.h"
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

void SceneEditor::on_file_menu()
{
	if (ImGui::MenuItem("Save", "Ctrl+S"))
		scene->save(scene->filename);
	if (ImGui::MenuItem("Close", "Ctrl+S"))
		scene_editor.reset();
}

bool openCreateLightPopup = false;
bool openCreateTerrainPopup = false;
bool openCreateWaterPopup = false;

const char *basic_model_names[] = {
	"triangle",
	"cube",
	"sphere",
	"cylinder",
	"cone",
	"arrow",
	"torus",
	"hammer"
};

void SceneEditor::on_menu_bar()
{
	openCreateLightPopup = false;
	openCreateTerrainPopup = false;
	openCreateWaterPopup = false;

	if (ImGui::BeginMenu_keepalive("Create"))
	{
		if (ImGui::MenuItem("Light"))
			openCreateLightPopup = true;
		if (ImGui::BeginMenu("Object"))
		{
			for (int i = 0; i < TK_ARRAYSIZE(basic_model_names); i++)
			{
				if (ImGui::MenuItem(basic_model_names[i]))
				{
					auto m = tke::getModel(basic_model_names[i]);
					if (m)
					{
						auto o = new tke::Object(m, 0);
						o->setCoord(scene->camera.target);
						scene->addObject(o);
					}
				}
			}
			ImGui::EndMenu();
		}
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
			auto c = scene->camera.get_coord();
			ImGui::Text("%f, %f, %f coord", c.x, c.y, c.z);
		}

		ImGui::EndMenu();
	}
	if (target || follow)
	{
		auto n = selected.get_node();
		if (n && n->type == tke::NodeTypeObject)
			scene->camera.object = (tke::Object*)n;
		else
			scene->camera.object = nullptr;
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
				c = scene->camera.get_coord();
			else
				c = scene->camera.target;
			ImGui::Text("%f %f %f coord", c.x, c.y, c.z);
		}
	};

	if (openCreateLightPopup)
		ImGui::OpenPopup("Create Light");
	if (openCreateTerrainPopup)
		ImGui::OpenPopup("Create Terrain");
	if (openCreateWaterPopup)
		ImGui::OpenPopup("Create Water");
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
			auto t = new tke::Terrain(64, 64, use_physx, tke::get_image(blend_image_name));
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

	glm::vec4 image_rect = tke::fit_rect(glm::vec2(tke::window_cx, tke::window_cy), tke::res_aspect);
	ImGui::SetCursorScreenPos(ImVec2(image_rect.x, image_rect.y));
	ImGui::InvisibleButton("canvas", ImVec2(image_rect.z, image_rect.w));
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	draw_list->AddImage(ImTextureID(tke::get_ui_image_index(layer.image)), ImVec2(image_rect.x, image_rect.y), ImVec2(image_rect.x +
		image_rect.z, image_rect.y + image_rect.w));
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
				auto x = (tke::mouseX - image_rect.x) / image_rect.z * tke::res_cx;
				auto y = (tke::mouseY - image_rect.y) / image_rect.w * tke::res_cy;
				if (!transformerTool->leftDown(x, y))
				{
					tke::PlainRenderer::DrawData draw_data;
					draw_data.mode = tke::PlainRenderer::mode_just_color;
					for (int i = 0; i < scene->objects.size(); i++)
					{
						auto object = scene->objects[i].get();

						tke::PlainRenderer::DrawData::ObjData obj_data;
						obj_data.mat = object->get_matrix();
						obj_data.color = glm::vec4((i + 1) / 255.f, 0.f, 0.f, 0.f);
						obj_data.fill_with_model(object->model.get());
						if (object->model->vertex_skeleton)
							obj_data.bone_buffer = object->animationComponent->bone_buffer.get();
						draw_data.obj_data.push_back(obj_data);
					}
					auto index = tke::pick_up(x, y, std::bind(
						&tke::PlainRenderer::do_render,
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

	{
		auto n = selected.get_node();
		if (n && n->type == tke::NodeTypeObject)
		{
			auto obj = (tke::Object*)n;
			obj->setState(tke::Controller::State::forward, tke::keyStates[VK_UP].pressing);
			obj->setState(tke::Controller::State::backward, tke::keyStates[VK_DOWN].pressing);
			obj->setState(tke::Controller::State::left, tke::keyStates[VK_LEFT].pressing);
			obj->setState(tke::Controller::State::right, tke::keyStates[VK_RIGHT].pressing);
		}
	}

	scene->update();
	if (enableRender)
	{
		defe_renderer->render(scene.get());
		defe_renderer->add_to_drawlist();
	}
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
				vtx_dst[0].position = tke::physx_vec3_to_vec3(line.pos0);
				vtx_dst[0].color = tke::physx_u32_to_vec3(line.color0);
				vtx_dst[1].position = tke::physx_vec3_to_vec3(line.pos1);
				vtx_dst[1].color = tke::physx_u32_to_vec3(line.color1);
				vtx_dst += 2;
			}
			physx_vertex_buffer->unmap();
			tke::LinesRenderer::DrawData data;
			data.vertex_buffer = physx_vertex_buffer.get();
			data.vertex_count = vertex_count;
			lines_renderer->render(layer.framebuffer.get(), false, &scene->camera, &data);
			lines_renderer->add_to_drawlist();
		}
	}

	if (showSelectedWireframe)
	{
		auto n = selected.get_node();
		if (n && n->type == tke::NodeTypeObject)
		{
			auto obj = (tke::Object*)n;
			tke::PlainRenderer::DrawData data;
			data.mode = tke::PlainRenderer::mode_wireframe;
			tke::PlainRenderer::DrawData::ObjData obj_data;
			obj_data.mat = obj->get_matrix();
			obj_data.color = glm::vec4(0.f, 1.f, 0.f, 1.f);
			obj_data.fill_with_model(obj->model.get());
			if (obj->model->vertex_skeleton)
				obj_data.bone_buffer = obj->animationComponent->bone_buffer.get();
			data.obj_data.push_back(obj_data);
			plain_renderer->render(layer.framebuffer.get(), false, &scene->camera, &data);
			plain_renderer->add_to_drawlist();
		}
	}

	{
		auto n = selected.get_node();
		if (n)
		{
			switch (n->type)
			{
			case tke::NodeTypeLight:
				transformerTool->transformer = (tke::Light*)n;
				break;
			case tke::NodeTypeObject:
				transformerTool->transformer = (tke::Object*)n;
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
