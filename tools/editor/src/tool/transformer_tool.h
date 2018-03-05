#pragma once

#include <flame/ui/ui.h>
#include "tool.h"

struct TransformerTool : Tool
{
	enum Operation
	{
		TRANSLATE,
		ROTATE,
		SCALE
	};

	enum Mode
	{
		LOCAL,
		WORLD
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

	Operation operation;
	Mode mode;
	TransType type;
	bool enable_snap;
	glm::vec3 translate_snap;
	float rotate_snap;
	float scale_snap;

	ImVec2 window_pos;
	ImVec2 window_size;

	flame::Node *target;

	glm::mat4 model_matrix;
	glm::mat4 model_matrix_inverse;
	glm::mat4 model_source;
	glm::vec3 model_position;
	glm::mat4 proj_view;
	glm::mat4 mvp;

	glm::vec3 camera_position;
	glm::vec3 camera_dir;
	glm::vec3 ray_origin;
	glm::vec3 ray_vector;

	float radius_square_center;
	ImVec2 screen_square_center;
	ImVec2 screen_square_min;
	ImVec2 screen_square_max;

	float screen_factor;
	glm::vec3 relative_origin;

	bool enable;
	bool using_;

	glm::vec4 translation_plane;
	glm::vec3 translation_plane_origin;
	glm::vec3 matrix_origin;

	glm::vec3 scale;
	glm::vec3 scale_value_origin;
	float save_mouse_pos_x;

	glm::vec3 rotation_vector_source;
	float rotation_angle;
	float rotation_angle_origin;

	bool below_axis_limit[3];
	bool below_plane_limit[3];
	float axis_factor[3];

	ImDrawList *draw_list;

	TransformerTool();
	virtual void show(glm::vec2 _window_pos, glm::vec2 _window_size, flame::CameraComponent *camera) override;
	bool is_over();
private:
	ImVec2 world_to_screen(const glm::vec3 &coord, const glm::mat4 &mat);
	void compute_tripod_axis_and_visibility(int axis_index, glm::vec3 &dir_plane_x, glm::vec3 &dir_plane_y, bool &below_axis_limit, bool &below_plane_limit);
	float compute_angle_on_plan();
	void compute_colors(ImU32 *colors, int type);
	int get_move_type(glm::vec3 *hit_proportion);
	int get_rotate_type();
	int get_scale_type();
	void draw_hatched_axis(const glm::vec3 &axis);
};
