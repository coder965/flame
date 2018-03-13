#include <flame/engine/core/core.h>
#include <flame/engine/graphics/renderer.h>
#include <flame/engine/entity/node.h>
#include <flame/engine/entity/camera.h>

namespace flame
{
	bool CameraComponent::on_message(Object *sender, Message msg)
	{
		switch (msg)
		{
			case MessageResolutionChange:
				update_proj();
				update_frustum();
				return true;
		}
	}

	void CameraComponent::on_update()
	{
		if (aux_updated_frame < get_parent()->get_transform_dirty_frame())
		{
			view_matrix = glm::inverse(get_parent()->get_matrix());
			update_frustum();

			aux_updated_frame = total_frame_count;
		}
	}

	CameraComponent::CameraComponent() :
		Component(ComponentTypeCamera),
		proj_type(ProjectionTypePerspective),
		aux_updated_frame(-1)
	{
		follow_to(&resolution);
		update_proj();
	}

	void CameraComponent::set_proj(ProjectionType _proj_type)
	{
		proj_type = _proj_type;
		update_proj();
	}

	void CameraComponent::reset()
	{
		get_parent()->set_coord(glm::vec3(0.f));
	}

	const glm::mat4 &CameraComponent::get_proj_matrix() const
	{
		return proj_matrix;
	}

	const glm::mat4 &CameraComponent::get_proj_matrix_inverse() const
	{
		return proj_matrix_inverse;
	}

	const glm::mat4 &CameraComponent::get_view_matrix() const
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

	void CameraComponent::update_proj()
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

	void CameraComponent::update_frustum()
	{
		auto tanHfFovy = glm::tan(glm::radians(fovy * 0.5f));

		auto _y1 = near_plane * tanHfFovy;
		auto _z1 = _y1 * resolution.aspect();
		auto _y2 = far_plane * tanHfFovy;
		auto _z2 = _y2 * resolution.aspect();
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
