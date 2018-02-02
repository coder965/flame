#include "../../global.h"
#include "../../graphics/command_buffer.h"
#include "../../entity/node.h"
#include "../../model/model.h"
#include "../../pick_up/pick_up.h"
#include "transformer_tool.h"

TransformerTool::TransformerTool(tke::DisplayLayer *_layer) :
	layer(_layer)
{
	renderer = std::make_unique<tke::PlainRenderer>();
}

static tke::CameraComponent *currentCamera = nullptr;
tke::PlainRenderer::DrawData TransformerTool::getDrawData(int draw_mode)
{
	tke::PlainRenderer::DrawData draw_data;

	if (!node || mode == TransformerTool::ModeNull)
		return draw_data;

	auto camera_coord = currentCamera->get_parent()->get_world_coord();

	auto dir = node->get_world_coord() - camera_coord;
	if (glm::length(dir) <= 0.f)
		return draw_data;

	dir = glm::normalize(dir);
	auto coord = camera_coord + dir * 5.f;

	auto model = mode == TransformerTool::ModeMove ? tke::arrowModel :
		(mode == TransformerTool::ModeRotate ? tke::torusModel : tke::hamerModel);

	draw_data.obj_data.resize(3);

	draw_data.obj_data[0].mat = glm::translate(coord);
	draw_data.obj_data[0].color = draw_mode == 0 ? (selectedAxis == 0 ? glm::vec4(1.f, 1.f, 0.f, 1.f) : glm::vec4(1.f, 0.f, 0.f, 1.f)) : glm::vec4(1.f / 255.f, 0.f, 0.f, 0.f);
	draw_data.obj_data[0].fill_with_model(model.get());

	draw_data.obj_data[1].mat = glm::translate(coord) * glm::rotate(90.f, glm::vec3(0, 0, 1));
	draw_data.obj_data[1].color = draw_mode == 0 ? (selectedAxis == 1 ? glm::vec4(1.f, 1.f, 0.f, 1.f) : glm::vec4(0.f, 1.f, 0.f, 1.f)) : glm::vec4(2.f / 255.f, 0.f, 0.f, 0.f);
	draw_data.obj_data[1].fill_with_model(model.get());

	draw_data.obj_data[2].mat = glm::translate(coord) * glm::rotate(-90.f, glm::vec3(0, 1, 0));
	draw_data.obj_data[2].color = draw_mode == 0 ? (selectedAxis == 2 ? glm::vec4(1.f, 1.f, 0.f, 1.f) : glm::vec4(0.f, 0.f, 1.f, 1.f)) : glm::vec4(3.f / 255.f, 0.f, 0.f, 0.f);
	draw_data.obj_data[2].fill_with_model(model.get());

	return draw_data;
}

bool TransformerTool::leftDown(int x, int y)
{
	if (!node)
		return false;

	auto draw_data = getDrawData(1);
	if (!draw_data.obj_data.empty())
	{
		draw_data.mode = tke::PlainRenderer::mode_just_color;
		auto index = tke::pick_up(x, y, std::bind(&tke::PlainRenderer::do_render,
			renderer.get(), std::placeholders::_1, currentCamera, &draw_data));
		selectedAxis = tke::Node::Axis(index - 1);
		return index != 0;
	}
	return false;
}

void TransformerTool::mouseMove(int _xDisp, int _yDisp)
{
	if (!node || mode == TransformerTool::ModeNull || selectedAxis == -1)
		return;

	switch (mode)
	{
		case ModeMove:
		{
			auto xDisp = (float)_xDisp / (float)tke::resolution.x();
			auto yDisp = (float)_yDisp / (float)tke::resolution.y();

			auto p = currentCamera->get_proj_matrix() * currentCamera->get_view_matrix() * glm::vec4(node->get_coord(), 1.f);
			p = p / p.w;
			p.x += xDisp;
			p.y += yDisp;
			p = currentCamera->get_proj_matrix_inverse() * p;
			p = p / p.w;
			p = currentCamera->get_parent()->get_world_matrix() * p;

			switch (selectedAxis)
			{
				case tke::Node::AxisX:
					node->set_coord_x(p.x);
					break;
				case tke::Node::AxisY:
					node->set_coord_y(p.y);
					break;
				case tke::Node::AxisZ:
					node->set_coord_z(p.z);
					break;
			}
		}
		break;
	}
}

void TransformerTool::show(tke::CameraComponent *camera)
{
	currentCamera = camera;
	auto draw_data = getDrawData(0);
	if (!draw_data.obj_data.empty())
	{
		draw_data.mode = tke::PlainRenderer::mode_color_and_front_light;
		renderer->render(layer->framebuffer.get(), false, camera, &draw_data);
		renderer->add_to_drawlist();
	}
}
