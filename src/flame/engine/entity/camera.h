#pragma once

#include <flame/global.h>
#include <flame/engine/entity/component.h>
#include <flame/engine/entity/controller.h>

namespace flame
{
	enum ProjectionType
	{
		ProjectionTypePerspective,
		ProjectionTypeOrtho
	};

	class CameraComponent : public Component
	{
	private:
		ProjectionType proj_type;
		glm::mat4 proj_matrix;
		glm::mat4 proj_matrix_inverse;
		glm::mat4 view_matrix;

		glm::vec3 frustum_points[8];
		glm::vec4 frustum_planes[6];

		long long aux_updated_frame;
	protected:
		virtual bool on_message(Object *sender, Message msg) override;
		virtual void on_update() override;
	public:
		CameraComponent();

		const glm::mat4 &get_proj_matrix() const;
		const glm::mat4 &get_proj_matrix_inverse() const;
		const glm::mat4 &get_view_matrix() const;
		const glm::vec3 *get_frustum_points() const;
		const glm::vec4 *get_frustum_planes() const;

		void set_proj(ProjectionType _proj_type);
		void reset();
	private:
		void update_proj();
		void update_frustum();
	};
}
