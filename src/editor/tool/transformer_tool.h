#pragma once

#include "../../ui/ui.h"
#include "../../graphics/renderer.h"
#include "../../graphics/display_layer.h"
#include "tool.h"

struct TransformerTool : Tool
{
	tke::DisplayLayer *layer;

	enum Mode
	{
		ModeNull,
		ModeMove,
		ModeRotate,
		ModeScale
	};

	enum TransType
	{
		NONE,
		MOVE_X,
		MOVE_Y,
		MOVE_Z,
		MOVE_XY,
		MOVE_XZ,
		MOVE_YZ,
		MOVE_SCREEN,
		ROTATE_X,
		ROTATE_Y,
		ROTATE_Z,
		ROTATE_SCREEN,
		SCALE_X,
		SCALE_Y,
		SCALE_Z,
		SCALE_XYZ
	};

	Mode mode;
	TransType type;
	tke::Node::Axis selected_axis;

	ImVec2 window_pos;
	ImVec2 window_size;

	tke::Node *target;

	glm::mat4 target_matrix;
	glm::vec3 target_position;
	glm::mat4 proj_view;
	glm::mat4 mvp;

	glm::vec3 camera_position;
	glm::vec3 ray_origin;
	glm::vec3 ray_vector;

	float radius_square_center;
	ImVec2 screen_square_center;
	ImVec2 screen_square_min;
	ImVec2 screen_square_max;

	float screen_factor;

	bool using_;

	bool below_axis_limit[3];
	bool below_plane_limit[3];
	float axis_factor[3];

	ImDrawList *draw_list;

	std::unique_ptr<tke::PlainRenderer> renderer;

	TransformerTool(tke::DisplayLayer *_layer);
	virtual bool TransformerTool::leftDown(int x, int y) override;
	virtual void TransformerTool::mouseMove(int xDisp, int yDisp) override;
	virtual void show(tke::CameraComponent *camera) override;
	tke::PlainRenderer::DrawData getDrawData(int draw_mode);
};
