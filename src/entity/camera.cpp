#include "node.h"
#include "camera.h"

namespace tke
{
	void CameraComponent::on_update()
	{
		if (aux_matrix_updated_frame < get_parent()->get_transform_dirty_frame())
		{
			view_matrix = glm::inverse(get_parent()->get_matrix());
			update_frustum();

			aux_matrix_updated_frame = total_frame_count;
		}
	}

	CameraComponent::CameraComponent() :
		Component(ComponentTypeCamera),
		target(0.f),
		length(0.f),
		aux_matrix_updated_frame(-1)
	{
		set_proj(ProjectionTypePerspective);
	}

	void CameraComponent::set_proj(ProjectionType proj_type)
	{
		auto vkTrans = glm::mat4(
			glm::vec4(1.f, 0.f, 0.f, 0.f), 
			glm::vec4(0.f, -1.f, 0.f, 0.f),
			glm::vec4(0.f, 0.f, 1.f, 0.f), 
			glm::vec4(0.f, 0.f, 0.f, 1.f)
		);
		switch (proj_type)
		{
			case ProjectionTypePerspective:
				proj_matrix = vkTrans * glm::perspective(glm::radians(fovy), resolution.aspect(), near_plane, far_plane);
				proj_matrix_inverse = glm::inverse(proj_matrix);
				break;
			case ProjectionTypeOrtho:
				proj_matrix = vkTrans * glm::ortho(-1.f, 1.f, -1.f, 1.f, near_plane, far_plane * 2);
				proj_matrix_inverse = glm::inverse(proj_matrix);
				break;
		}
	}

	void CameraComponent::set_length(float _length)
	{
		length = _length;
		look_at_target();
	}

	void CameraComponent::set_target(const glm::vec3 &_target)
	{
		target = _target;
		look_at_target();
	}

	void CameraComponent::reset()
	{
		get_parent()->set_coord(glm::vec3(0.f));
		set_length(0.f);
	}

	void CameraComponent::rotate_by_cursor(float x, float y)
	{
		get_parent()->add_euler(-x * 180.f, -y * 180.f, 0.f);
		look_at_target();
	}

	void CameraComponent::move_by_cursor(float x, float y)
	{
		if (length != 0.f)
		{
			auto l = length / near_plane;
			auto cy = tan(glm::radians(fovy / 2.f)) * near_plane * 2.f;
			target += (-x * cy * res_aspect * l) * get_parent()->get_axis()[0] + (y * cy * l) * 
				get_parent()->get_axis()[1];
			look_at_target();
		}
	}

	void CameraComponent::scroll(float value)
	{
		if (value < 0.f)
			set_length((length + 0.1) * 1.1f);
		else
			set_length(glm::max((length - 0.1f) / 1.1f, 0.f));
	}

	glm::vec3 CameraComponent::get_target() const
	{
		return target;
	}

	float CameraComponent::get_length() const
	{
		return length;
	}

	glm::mat4 CameraComponent::get_proj_matrix() const
	{
		return proj_matrix;
	}

	glm::mat4 CameraComponent::get_proj_matrix_inverse() const
	{
		return proj_matrix_inverse;
	}

	glm::mat4 CameraComponent::get_view_matrix() const
	{
		return view_matrix;
	}

	const glm::vec3 *CameraComponent::get_frustum_points() const
	{
		return frustum_points;
	}

	const glm::vec4 *CameraComponent::get_frustum_planes() const
	{
		return frustum_planes;
	}

	void CameraComponent::look_at_target()
	{
		if (length != 0.f)
			get_parent()->set_coord(target + get_parent()->get_axis()[2] * length);
		else
			get_parent()->set_coord(target);
	}

	void CameraComponent::update_frustum()
	{
		auto tanHfFovy = glm::tan(glm::radians(fovy * 0.5f));

		auto _y1 = near_plane * tanHfFovy;
		auto _z1 = _y1 * res_aspect;
		auto _y2 = far_plane * tanHfFovy;
		auto _z2 = _y2 * res_aspect;
		auto axis = get_parent()->get_axis();
		auto coord = get_parent()->get_coord();
		frustum_points[0] = -_z1 * axis[2] + _y1 * axis[1] + near_plane * axis[0] + coord;
		frustum_points[1] = _z1 * axis[2] + _y1 * axis[1] + near_plane * axis[0] + coord;
		frustum_points[2] = _z1 * axis[2] + -_y1 * axis[1] + near_plane * axis[0] + coord;
		frustum_points[3] = -_z1 * axis[2] + -_y1 * axis[1] + near_plane * axis[0] + coord;
		frustum_points[4] = -_z2 * axis[2] + _y2 * axis[1] + far_plane * axis[0] + coord;
		frustum_points[5] = _z2 * axis[2] + _y2 * axis[1] + far_plane * axis[0] + coord;
		frustum_points[6] = _z2 * axis[2] + -_y2 * axis[1] + far_plane * axis[0] + coord;
		frustum_points[7] = -_z2 * axis[2] + -_y2 * axis[1] + far_plane * axis[0] + coord;
		for (int i = 0; i < 4; i++)
		{
			auto y = frustum_points[i + 4].y;
			if (y < 0.f)
			{
				auto py = frustum_points[i + 4].y - frustum_points[i].y;
				if (py != 0.f)
				{
					frustum_points[i + 4].x -= y * ((frustum_points[i + 4].x - frustum_points[i].x) / py);
					frustum_points[i + 4].z -= y * ((frustum_points[i + 4].z - frustum_points[i].z) / py);
					frustum_points[i + 4].y = 0.f;
				}
			}
		}

		auto vp = proj_matrix * get_parent()->get_matrix();

		frustum_planes[0].x = vp[0].w + vp[0].x;
		frustum_planes[0].y = vp[1].w + vp[1].x;
		frustum_planes[0].z = vp[2].w + vp[2].x;
		frustum_planes[0].w = vp[3].w + vp[3].x;

		frustum_planes[1].x = vp[0].w - vp[0].x;
		frustum_planes[1].y = vp[1].w - vp[1].x;
		frustum_planes[1].z = vp[2].w - vp[2].x;
		frustum_planes[1].w = vp[3].w - vp[3].x;

		frustum_planes[2].x = vp[0].w - vp[0].y;
		frustum_planes[2].y = vp[1].w - vp[1].y;
		frustum_planes[2].z = vp[2].w - vp[2].y;
		frustum_planes[2].w = vp[3].w - vp[3].y;

		frustum_planes[3].x = vp[0].w + vp[0].y;
		frustum_planes[3].y = vp[1].w + vp[1].y;
		frustum_planes[3].z = vp[2].w + vp[2].y;
		frustum_planes[3].w = vp[3].w + vp[3].y;

		frustum_planes[4].x = vp[0].w + vp[0].z;
		frustum_planes[4].y = vp[1].w + vp[1].z;
		frustum_planes[4].z = vp[2].w + vp[2].z;
		frustum_planes[4].w = vp[3].w + vp[3].z;

		frustum_planes[5].x = vp[0].w - vp[0].z;
		frustum_planes[5].y = vp[1].w - vp[1].z;
		frustum_planes[5].z = vp[2].w - vp[2].z;
		frustum_planes[5].w = vp[3].w - vp[3].z;

		for (auto i = 0; i < 6; i++)
			frustum_planes[i] = glm::normalize(frustum_planes[i]);
	}
}
