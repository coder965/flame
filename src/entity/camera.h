#pragma once

#include "../global.h"
#include "component.h"
#include "controller.h"

namespace tke
{
	enum ProjectionType
	{
		ProjectionTypePerspective,
		ProjectionTypeOrtho
	};

	class CameraComponent : public Component
	{
	private:
		glm::vec3 target;
		float length;
		glm::mat4 proj_matrix;
		glm::mat4 proj_matrix_inverse;
		glm::mat4 view_matrix;

		glm::vec3 frustum_points[8];
		glm::vec4 frustum_planes[6];

		long long aux_matrix_updated_frame;
	protected:
		virtual void on_update() override;
	public:
		CameraComponent();

		glm::vec3 get_target() const;
		float get_length() const;
		glm::mat4 get_proj_matrix() const;
		glm::mat4 get_proj_matrix_inverse() const;
		glm::mat4 get_view_matrix() const;
		const glm::vec3 *get_frustum_points() const;
		const glm::vec4 *get_frustum_planes() const;

		void set_proj(ProjectionType proj_type);
		void set_length(float _length);
		void set_target(const glm::vec3 &_target);
		void rotate_by_cursor(float x, float y);
		void move_by_cursor(float x, float y);
		void scroll(float value);
		void reset();
	private:
		void look_at_target();
		void update_frustum();
	};
}
