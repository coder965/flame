#include "../../global.h"
#include "../../graphics/command_buffer.h"
#include "../../entity/node.h"
#include "../../model/model.h"
#include "../../pick_up/pick_up.h"
#include "transformer_tool.h"

TransformerTool::TransformerTool(tke::DisplayLayer *_layer) :
	layer(_layer),
	mode(ModeNull),
	type(NONE),
	selected_axis(tke::Node::AxisNull),
	target(nullptr),
	using_(false)
{
	renderer = std::make_unique<tke::PlainRenderer>();
}

static tke::CameraComponent *currentCamera = nullptr;
tke::PlainRenderer::DrawData TransformerTool::getDrawData(int draw_mode)
{
	tke::PlainRenderer::DrawData draw_data;

	if (!target || mode == TransformerTool::ModeNull)
		return draw_data;

	auto camera_coord = currentCamera->get_parent()->get_world_coord();

	auto dir = target->get_world_coord() - camera_coord;
	if (glm::length(dir) <= 0.f)
		return draw_data;

	dir = glm::normalize(dir);
	auto coord = camera_coord + dir * 5.f;

	auto model = mode == TransformerTool::ModeMove ? tke::arrowModel :
		(mode == TransformerTool::ModeRotate ? tke::torusModel : tke::hamerModel);

	draw_data.obj_data.resize(3);

	draw_data.obj_data[0].mat = glm::translate(coord);
	draw_data.obj_data[0].color = draw_mode == 0 ? (selected_axis == 0 ? glm::vec4(1.f, 1.f, 0.f, 1.f) : glm::vec4(1.f, 0.f, 0.f, 1.f)) : glm::vec4(1.f / 255.f, 0.f, 0.f, 0.f);
	draw_data.obj_data[0].fill_with_model(model.get());

	draw_data.obj_data[1].mat = glm::translate(coord) * glm::rotate(90.f, glm::vec3(0, 0, 1));
	draw_data.obj_data[1].color = draw_mode == 0 ? (selected_axis == 1 ? glm::vec4(1.f, 1.f, 0.f, 1.f) : glm::vec4(0.f, 1.f, 0.f, 1.f)) : glm::vec4(2.f / 255.f, 0.f, 0.f, 0.f);
	draw_data.obj_data[1].fill_with_model(model.get());

	draw_data.obj_data[2].mat = glm::translate(coord) * glm::rotate(-90.f, glm::vec3(0, 1, 0));
	draw_data.obj_data[2].color = draw_mode == 0 ? (selected_axis == 2 ? glm::vec4(1.f, 1.f, 0.f, 1.f) : glm::vec4(0.f, 0.f, 1.f, 1.f)) : glm::vec4(3.f / 255.f, 0.f, 0.f, 0.f);
	draw_data.obj_data[2].fill_with_model(model.get());

	return draw_data;
}

bool TransformerTool::leftDown(int x, int y)
{
	if (!target)
		return false;

	auto draw_data = getDrawData(1);
	if (!draw_data.obj_data.empty())
	{
		draw_data.mode = tke::PlainRenderer::mode_just_color;
		auto index = tke::pick_up(x, y, std::bind(&tke::PlainRenderer::do_render,
			renderer.get(), std::placeholders::_1, currentCamera, &draw_data));
		selected_axis = tke::Node::Axis(index - 1);
		return index != 0;
	}
	return false;
}

void TransformerTool::mouseMove(int _xDisp, int _yDisp)
{
	if (!target || mode == TransformerTool::ModeNull || selected_axis == -1)
		return;

	switch (mode)
	{
		case ModeMove:
		{
			auto xDisp = (float)_xDisp / (float)tke::resolution.x();
			auto yDisp = (float)_yDisp / (float)tke::resolution.y();

			auto p = currentCamera->get_proj_matrix() * currentCamera->get_view_matrix() * glm::vec4(target->get_coord(), 1.f);
			p = p / p.w;
			p.x += xDisp;
			p.y += yDisp;
			p = currentCamera->get_proj_matrix_inverse() * p;
			p = p / p.w;
			p = currentCamera->get_parent()->get_world_matrix() * p;

			switch (selected_axis)
			{
				case tke::Node::AxisX:
					target->set_coord_x(p.x);
					break;
				case tke::Node::AxisY:
					target->set_coord_y(p.y);
					break;
				case tke::Node::AxisZ:
					target->set_coord_z(p.z);
					break;
			}
		}
		break;
	}
}

static const float angle_limit = 0.96f;
static const float plane_limit = 0.2f;

static const glm::vec3 direction_unary[] = { glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 1.f) };
static const ImU32 direction_color[] = { 0xFF0000AA, 0xFF00AA00, 0xFFAA0000 };

static const ImU32 plane_border_color[] = { 0xFFAA0000, 0xFF0000AA, 0xFF00AA00 };
static const ImU32 plane_color[] = { 0x610000AA, 0x6100AA00, 0x61AA0000 };
static const ImU32 selection_color = 0x8A1080FF;
static const ImU32 inactive_color = 0x99999999;
static const ImU32 translationLineColor = 0xAAAAAAAA;
static const float quad_min = 0.5f;
static const float quad_max = 0.8f;
static const float quad_uv[8] = { quad_min, quad_min, quad_min, quad_max, quad_max, quad_max, quad_max, quad_min };

static void ComputeColors(ImU32 *colors, TransformerTool::TransType type, TransformerTool::Mode mode)
{
	switch (mode)
	{
		case TransformerTool::ModeMove:
			colors[0] = (type == TransformerTool::MOVE_SCREEN) ? selection_color : 0xFFFFFFFF;
			for (int i = 0; i < 3; i++)
			{
				int index = (i + 2) % 3;
				colors[i + 1] = (type == TransformerTool::MOVE_X + i) ? selection_color : direction_color[i];
				colors[i + 4] = (type == TransformerTool::MOVE_XY + i) ? selection_color : plane_color[index];
				colors[i + 4] = (type == TransformerTool::MOVE_SCREEN) ? selection_color : colors[i + 4];
			}
			break;
		case TransformerTool::ModeRotate:
			break;
		case TransformerTool::ModeScale:
			break;
		default:
			for (int i = 0; i < 7; i++)
				colors[i] = inactive_color;
	}
}

static ImVec2 _world_to_screen(TransformerTool *tool, const glm::vec3 &coord, const glm::mat4 &mat)
{
	auto v = tke::transform(coord, mat);
	return tool->window_pos + (ImVec2(v.x, v.y) / 2.f + ImVec2(0.5f, 0.5f)) * tool->window_size;
}

static float _intersect_ray_plane(const glm::vec3 &origin, const glm::vec3 &vector, const glm::vec4 &plane)
{
	float numer = glm::dot(glm::vec3(plane), origin) - plane.w;
	float denom = glm::dot(glm::vec3(plane), vector);

	if (fabsf(denom) < FLT_EPSILON)
		return -1.0f;

	return -(numer / denom);
}

static void _compute_tripod_axis_and_visibility(TransformerTool *tool, int axis_index, glm::vec3 &dir_plane_x, glm::vec3 &dir_plane_y, bool &below_axis_limit, bool &below_plane_limit)
{
	const auto plan_normal = (axis_index + 2) % 3;
	dir_plane_x = direction_unary[axis_index];
	dir_plane_y = direction_unary[(axis_index + 1) % 3];

	auto dir_plane_normal_world = glm::normalize(glm::vec3(tool->target_matrix * glm::vec4(direction_unary[plan_normal], 1.f)));
	auto dir_plane_x_World = glm::normalize(glm::vec3(tool->target_matrix * glm::vec4(dir_plane_x, 1.f)));
	auto dir_plane_y_World = glm::normalize(glm::vec3(tool->target_matrix * glm::vec4(dir_plane_y, 1.f)));

	auto camera_eye_to_tool = glm::normalize(tool->target_position - tool->camera_position);
	auto dot_camera_dir_x = glm::dot(camera_eye_to_tool, dir_plane_x_World);
	auto dot_camera_dir_y = glm::dot(camera_eye_to_tool, dir_plane_y_World);

	auto mul_axis_x = (dot_camera_dir_x > 0.f) ? -1.f : 1.f;
	auto mul_axis_y = (dot_camera_dir_y > 0.f) ? -1.f : 1.f;
	dir_plane_x *= mul_axis_x;
	dir_plane_y *= mul_axis_y;

	below_axis_limit = glm::abs(dot_camera_dir_x) < angle_limit;
	below_plane_limit = (glm::abs(glm::dot(camera_eye_to_tool, dir_plane_normal_world)) > plane_limit);

	tool->axis_factor[axis_index] = mul_axis_x;
	tool->axis_factor[(axis_index + 1) % 3] = mul_axis_y;
	tool->below_axis_limit[axis_index] = below_axis_limit;
	tool->below_plane_limit[axis_index] = below_plane_limit;
}

static int _get_move_type(TransformerTool *tool, glm::vec3 *hit_proportion)
{
	ImGuiIO& io = ImGui::GetIO();
	int type = TransformerTool::NONE;

	if (io.MousePos.x >= tool->screen_square_min.x && io.MousePos.x <= tool->screen_square_max.x &&
		io.MousePos.y >= tool->screen_square_min.y && io.MousePos.y <= tool->screen_square_max.y)
		type = TransformerTool::MOVE_SCREEN;

	const glm::vec3 direction[3] = { tool->target_matrix[0], tool->target_matrix[1], tool->target_matrix[2] };

	for (auto i = 0; i < 3 && type == TransformerTool::NONE; i++)
	{
		glm::vec3 dir_plane_x, dir_plane_y;
		auto below_axis_limit = false;
		auto below_plane_limit = false;
		_compute_tripod_axis_and_visibility(tool, i, dir_plane_x, dir_plane_y, below_axis_limit, below_plane_limit);
		dir_plane_x = tool->target_matrix * glm::vec4(dir_plane_x, 0.f);
		dir_plane_y = tool->target_matrix * glm::vec4(dir_plane_y, 0.f);

		const auto plan_normal = (i + 2) % 3;

		const auto len = _intersect_ray_plane(tool->ray_origin, tool->ray_vector, tke::plane(tool->target_position, direction[plan_normal]));
		auto pos_on_plane = tool->ray_origin + tool->ray_vector * len;

		const auto dx = glm::dot(dir_plane_x, (pos_on_plane - tool->target_position) / tool->screen_factor);
		const auto dy = glm::dot(dir_plane_y, (pos_on_plane - tool->target_position) / tool->screen_factor);
		if (below_axis_limit && dy > -0.1f && dy < 0.1f && dx > 0.1f  && dx < 1.f)
			type = TransformerTool::MOVE_X + i;

		if (below_plane_limit && dx >= quad_uv[0] && dx <= quad_uv[4] && dy >= quad_uv[1] && dy <= quad_uv[3])
			type = TransformerTool::MOVE_XY + i;

		if (hit_proportion)
			*hit_proportion = glm::vec3(dx, dy, 0.f);
	}
	return type;
}

static void _draw_hatched_axis(TransformerTool *tool, const glm::vec3 &axis)
{
	for (int j = 1; j < 10; j++)
	{
		ImVec2 baseSSpace2 = _world_to_screen(tool, axis * 0.05f * (float)(j * 2) * tool->screen_factor, tool->mvp);
		ImVec2 worldDirSSpace2 = _world_to_screen(tool, axis * 0.05f * (float)(j * 2 + 1) * tool->screen_factor, tool->mvp);
		tool->draw_list->AddLine(baseSSpace2, worldDirSSpace2, 0x80000000, 6.f);
	}
}

void TransformerTool::show(tke::CameraComponent *camera)
{
	draw_list = ImGui::GetWindowDrawList();
	if (!draw_list)
		return;

	if (!target || mode == TransformerTool::ModeNull)
		return;

	ImU32 colors[7];
	ComputeColors(colors, type, ModeMove);

	window_pos = ImGui::GetWindowPos();
	window_size = ImGui::GetWindowSize();

	target_matrix = target->get_world_matrix();
	target_position = target->get_world_coord();
	proj_view = camera->get_proj_matrix() * camera->get_view_matrix();
	mvp = proj_view * target_matrix;

	camera_position = camera->get_parent()->get_world_coord();
	{
		ImGuiIO& io = ImGui::GetIO();

		auto proj_view_inverse = glm::inverse(proj_view);

		ImVec2 mo = ImVec2((io.MousePos - window_pos) / window_size) * 2.f - ImVec2(1.f, 1.f);

		ray_origin = tke::transform(glm::vec3(mo.x, mo.y, -1.f), proj_view_inverse);
		ray_vector = glm::normalize(tke::transform(glm::vec3(mo.x, mo.y, 1.f), proj_view_inverse) - ray_origin);
	}

	screen_factor = 0.1f * (proj_view * glm::vec4(target_position, 1.f)).w;
	screen_square_center = _world_to_screen(this, glm::vec3(0.f), mvp);
	screen_square_min = ImVec2(screen_square_center.x - 10.f, screen_square_center.y - 10.f);
	screen_square_max = ImVec2(screen_square_center.x + 10.f, screen_square_center.y + 10.f);

	if ((mvp * glm::vec4(0.f, 0.f, 0.f, 1.f)).z < 0.001f)
		return;

	switch (mode)
	{
		case ModeMove:
		{
			ImGuiIO &io = ImGui::GetIO();

			if (using_)
			{

			}
			else
			{
				glm::vec3 hit_proportion;
				type = (TransType)_get_move_type(this, &hit_proportion);
				if (ImGui::IsMouseClicked(0))
				{

				}
			}
			break;
		}
	}

	switch (mode)
	{
		case ModeMove:
		{
			const auto origin = _world_to_screen(this, target_position, proj_view);

			auto below_axis_limit = false;
			auto below_plane_limit = false;
			for (auto i = 0; i < 3; ++i)
			{
				glm::vec3 dir_plane_x, dir_plane_y;
				_compute_tripod_axis_and_visibility(this, i, dir_plane_x, dir_plane_y, below_axis_limit, below_plane_limit);

				if (below_axis_limit)
				{
					auto base_s_space = _world_to_screen(this, dir_plane_x * 0.1f * screen_factor, mvp);
					auto world_dir_s_space = _world_to_screen(this, dir_plane_x * screen_factor, mvp);

					draw_list->AddLine(base_s_space, world_dir_s_space, colors[i + 1], 3.f);

					auto dir(origin - world_dir_s_space);

					auto d = glm::sqrt(ImLengthSqr(dir));
					dir /= d;
					dir *= 6.f;

					ImVec2 ortogonalDir(dir.y, -dir.x);
					auto a(world_dir_s_space + dir);
					draw_list->AddTriangleFilled(world_dir_s_space - dir, a + ortogonalDir, a - ortogonalDir, colors[i + 1]);

					if (axis_factor[i] < 0.f)
						_draw_hatched_axis(this, dir_plane_x);
				}

				if (below_plane_limit)
				{
					ImVec2 screen_quad_pts[4];
					for (int j = 0; j < 4; ++j)
					{
						glm::vec3 corner_world_pos = (dir_plane_x * quad_uv[j * 2] + dir_plane_y * quad_uv[j * 2 + 1]) * screen_factor;
						screen_quad_pts[j] = _world_to_screen(this, corner_world_pos, mvp);
					}
					draw_list->AddPolyline(screen_quad_pts, 4, plane_border_color[i], true, 1.0f);
					draw_list->AddConvexPolyFilled(screen_quad_pts, 4, colors[i + 4]);
				}
			}

			draw_list->AddCircleFilled(screen_square_center, 6.f, colors[0], 16);

			break;
		}
	}

	//currentCamera = camera;
	//auto draw_data = getDrawData(0);
	//if (!draw_data.obj_data.empty())
	//{
	//	draw_data.mode = tke::PlainRenderer::mode_color_and_front_light;
	//	renderer->render(layer->framebuffer.get(), false, camera, &draw_data);
	//	renderer->add_to_drawlist();
	//}
}
