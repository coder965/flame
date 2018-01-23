#include "../../global.h"
#include "../../ui/ui.h"
#include "../../file_utils.h"
#include "../../graphics/buffer.h"
#include "../../graphics/descriptor.h"
#include "../../model/model.h"
#include "../../model/animation.h"
#include "../../entity/light.h"
#include "../../entity/model_instance.h"
#include "../../entity/terrain.h"
#include "../../entity/water.h"
#include "../../physics/physics.h"
#include "../../pick_up/pick_up.h"
#include "../../input.h"
#include "../../global.h"
#include "../../engine.h"
#include "../select.h"
#include "resource_explorer.h"
#include "scene_editor.h"

void SceneEditor::on_delete()
{
	auto n = selected.get_node();
	if (!n || n == scene)
		return;

	n->get_parent()->remove_child(n);
}

std::unique_ptr<SceneEditor> scene_editor = nullptr;

SceneEditor::SceneEditor(tke::Scene *_scene)
{
	camera_node = new tke::Node(tke::NodeTypeNode);
	camera = new tke::CameraComponent;
	camera_node->add_component(camera);
	tke::root_node->add_child(camera_node);

	plain_renderer = std::make_unique<tke::PlainRenderer>();
	defe_renderer = std::make_unique<tke::DeferredRenderer>(false, layer.image.get());
	defe_renderer->follow_to(tke::root_node);

	scene = _scene;
	tke::root_node->add_child(scene);

	physx_vertex_buffer = std::make_unique<tke::ImmediateVertexBuffer>();
	lines_renderer = std::make_unique<tke::LinesRenderer>();

	transformerTool = std::make_unique<TransformerTool>(layer.image.get());
}

SceneEditor::~SceneEditor()
{
	tke::root_node->remove_child(camera_node);
	tke::root_node->remove_child(scene);
}

void SceneEditor::on_file_menu()
{
	if (ImGui::MenuItem("Save", "Ctrl+S"))
		tke::save_scene(scene);
	if (ImGui::MenuItem("Close"))
		scene_editor.reset();
}

void SceneEditor::on_menu_bar()
{
	if (ImGui::BeginMenu_keepalive("Create"))
	{
		if (ImGui::MenuItem("Empty"))
			;
		if (ImGui::MenuItem("Empty Child"))
			;
		if (ImGui::BeginMenu("Light"))
		{
			tke::LightType light_type = tke::LightType(-1);
			if (ImGui::MenuItem("Parallax"))
				light_type = tke::LightTypeParallax;
			if (ImGui::MenuItem("Point"))
				light_type = tke::LightTypePoint;
			if (ImGui::MenuItem("Spot"))
				light_type = tke::LightTypeSpot;
			if (light_type != -1)
			{
				auto n = new tke::Node(tke::NodeTypeNode);
				n->name = "Light";
				n->set_coord(camera->get_target());
				auto i = new tke::LightComponent;
				i->set_type(light_type);
				n->add_component(i);
				scene->add_child(n);
			}

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("3D"))
		{
			const char *basic_model_names[] = {
				"Triangle",
				"Cube",
				"Sphere",
				"Cylinder",
				"Cone",
				"Arrow",
				"Torus",
				"Hammer"
			};
			for (int i = 0; i < TK_ARRAYSIZE(basic_model_names); i++)
			{
				if (ImGui::MenuItem(basic_model_names[i]))
				{
					auto m = tke::getModel(basic_model_names[i]);
					if (m)
					{
						auto n = new tke::Node(tke::NodeTypeNode);
						n->name = "Object";
						n->set_coord(camera->get_target());
						auto i = new tke::ModelInstanceComponent;
						i->set_model(m);
						n->add_component(i);
						scene->add_child(n);
					}
				}
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Terrain"))
			{
				auto n = new tke::Node(tke::NodeTypeNode);
				n->set_coord(camera->get_target());
				auto t = new tke::TerrainComponent;
				n->add_component(t);
				scene->add_child(n);
			}
			if (ImGui::MenuItem("Water"))
			{
				auto n = new tke::Node(tke::NodeTypeNode);
				n->set_coord(camera->get_target());
				auto w = new tke::WaterComponent;
				n->add_component(w);
				scene->add_child(n);
			}
			ImGui::EndMenu();
		}
		if (ImGui::MenuItem("Camera"))
			;

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
			on_delete();
		ImGui::Separator();
		if (ImGui::MenuItem("Target To Selected"))
			;

		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu_keepalive("Show"))
	{
		ImGui::MenuItem("Enable Render", "", &enableRender);
		ImGui::MenuItem("Show Selected Wire Frame", "", &showSelectedWireframe);
		if (ImGui::MenuItem("View Physx", "", &viewPhysx))
		{
			//if (viewPhysx)
			//{
			//	scene->pxScene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, 1.f);
			//	////scene->setVisualizationParameter(PxVisualizationParameter::eACTOR_AXES, 1.f);
			//	scene->pxScene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_SHAPES, 1.f);
			//	//scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_DYNAMIC, 1.f);
			//	//pxScene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LOCAL_FRAMES, 1.f);
			//	////scene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LIMITS, 1.f);
			//}
			//else
			//	scene->pxScene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, 0.f);
		}

		ImGui::EndMenu();
	}
}

void SceneEditor::do_show()
{
	glm::vec4 image_rect = tke::fit_rect(glm::vec2(tke::window_cx, tke::window_cy), tke::res_aspect);
	ImGui::SetCursorScreenPos(ImVec2(image_rect.x, image_rect.y));
	ImGui::InvisibleButton("canvas", ImVec2(image_rect.z, image_rect.w));
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	draw_list->AddImage(ImGui::ImageID(layer.image), ImVec2(image_rect.x, image_rect.y), ImVec2(image_rect.x +
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
					auto n = new tke::Node(tke::NodeTypeNode);
					n->name = "Object";
					n->set_coord(camera->get_target());
					auto i = new tke::ModelInstanceComponent;
					i->set_model(m);
					n->add_component(i);
					scene->add_child(n);
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
				camera->move_by_cursor(distX, distY);
			else if (tke::keyStates[VK_CONTROL].pressing && tke::mouseMiddle.pressing)
				camera->scroll(distX);
			else if (tke::mouseMiddle.pressing)
				camera->rotate_by_cursor(distX, distY);
			else if (!tke::keyStates[VK_SHIFT].pressing && !tke::keyStates[VK_CONTROL].pressing)
			{
				if (tke::mouseLeft.pressing)
					transformerTool->mouseMove(tke::mouseDispX, tke::mouseDispY);
			}
		}
		if (tke::mouseScroll != 0)
			camera->scroll(tke::mouseScroll);
		if (tke::mouseLeft.justDown)
		{
			if (!tke::keyStates[VK_SHIFT].pressing && !tke::keyStates[VK_CONTROL].pressing)
			{
				auto x = (tke::mouseX - image_rect.x) / image_rect.z * tke::res_cx;
				auto y = (tke::mouseY - image_rect.y) / image_rect.w * tke::res_cy;
				if (!transformerTool->leftDown(x, y))
				{
					//tke::PlainRenderer::DrawData draw_data;
					//draw_data.mode = tke::PlainRenderer::mode_just_color;
					//for (int i = 0; i < scene->objects.size(); i++)
					//{
					//	auto object = scene->objects[i].get();

					//	tke::PlainRenderer::DrawData::ObjData obj_data;
					//	obj_data.mat = object->get_matrix();
					//	obj_data.color = glm::vec4((i + 1) / 255.f, 0.f, 0.f, 0.f);
					//	obj_data.fill_with_model(object->model.get());
					//	if (object->model->vertex_skeleton)
					//		obj_data.bone_buffer = object->animationComponent->bone_buffer.get();
					//	draw_data.obj_data.push_back(obj_data);
					//}
					//auto index = tke::pick_up(x, y, std::bind(
					//	&tke::PlainRenderer::do_render,
					//	plain_renderer.get(), std::placeholders::_1, camera, &draw_data));
					//if (index == 0)
					//	selected.reset();
					//else
					//	selected = scene->objects[index - 1];
				}
			}
		}

		if (ImGui::IsKeyDown(VK_DELETE))
			on_delete();
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

	//{
	//	auto n = selected.get_node();
	//	if (n && n->get_type() == tke::NodeTypeObject)
	//	{
	//		auto obj = (tke::Object*)n;
	//		obj->setState(tke::Controller::State::forward, tke::keyStates[VK_UP].pressing);
	//		obj->setState(tke::Controller::State::backward, tke::keyStates[VK_DOWN].pressing);
	//		obj->setState(tke::Controller::State::left, tke::keyStates[VK_LEFT].pressing);
	//		obj->setState(tke::Controller::State::right, tke::keyStates[VK_RIGHT].pressing);
	//	}
	//}

	if (enableRender)
	{
		defe_renderer->render(scene, camera);
		defe_renderer->add_to_drawlist();
	}

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
		//auto &rb = scene->pxScene->getRenderBuffer();
		//auto lineCount = rb.getNbLines();
		//if (lineCount > 0)
		//{
		//	auto vertex_count = lineCount * 2;
		//	auto size = vertex_count * sizeof(tke::LinesRenderer::Vertex);
		//	auto vtx_dst = (tke::LinesRenderer::Vertex*)physx_vertex_buffer->map(0, size);
		//	for (int i = 0; i < lineCount; i++)
		//	{
		//		auto &line = rb.getLines()[i];
		//		vtx_dst[0].position = tke::physx_vec3_to_vec3(line.pos0);
		//		vtx_dst[0].color = tke::physx_u32_to_vec3(line.color0);
		//		vtx_dst[1].position = tke::physx_vec3_to_vec3(line.pos1);
		//		vtx_dst[1].color = tke::physx_u32_to_vec3(line.color1);
		//		vtx_dst += 2;
		//	}
		//	physx_vertex_buffer->unmap();
		//	tke::LinesRenderer::DrawData data;
		//	data.vertex_buffer = physx_vertex_buffer.get();
		//	data.vertex_count = vertex_count;
		//	lines_renderer->render(layer.framebuffer.get(), false, camera, &data);
		//	lines_renderer->add_to_drawlist();
		//}
	}

	if (showSelectedWireframe)
	{
		//auto n = selected.get_node();
		//if (n && n->get_type() == tke::NodeTypeObject)
		//{
		//	auto obj = (tke::Object*)n;
		//	tke::PlainRenderer::DrawData data;
		//	data.mode = tke::PlainRenderer::mode_wireframe;
		//	tke::PlainRenderer::DrawData::ObjData obj_data;
		//	obj_data.mat = obj->get_matrix();
		//	obj_data.color = glm::vec4(0.f, 1.f, 0.f, 1.f);
		//	obj_data.fill_with_model(obj->model.get());
		//	if (obj->model->vertex_skeleton)
		//		obj_data.bone_buffer = obj->animationComponent->bone_buffer.get();
		//	data.obj_data.push_back(obj_data);
		//	plain_renderer->render(layer.framebuffer.get(), false, camera, &data);
		//	plain_renderer->add_to_drawlist();
		//}
	}

	transformerTool->node = selected.get_node();
	transformerTool->show(camera);
}

void SceneEditor::save(tke::XMLNode *n)
{
	n->add_attribute(new tke::XMLAttribute("filename", scene->get_filename()));
}
