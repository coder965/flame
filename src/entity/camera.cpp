#include "../model/model.h"
#include "camera.h"
#include "object.h"

namespace tke
{
	Camera::Camera()
		:Node(NodeTypeCamera)
	{
		set_proj(ProjectionTypePerspective);
		ang_offset = 90.f;
	}

	void Camera::set_proj(ProjectionType proj_type)
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
				proj_matrix = vkTrans * glm::perspective(glm::radians(fovy), res_aspect, near_plane, far_plane);
				proj_matrix_inverse = glm::inverse(proj_matrix);
				break;
			case ProjectionTypeOrtho:
				proj_matrix = vkTrans * glm::ortho(-1.f, 1.f, -1.f, 1.f, near_plane, far_plane * 2);
				proj_matrix_inverse = glm::inverse(proj_matrix);
				break;
		}
	}

	void Camera::setMode(CameraMode _mode)
	{
		mode = _mode;
		matrix_dirty = true;
		dirty = true;
	}

	void Camera::setLength(float _length)
	{
		length = _length;
		matrix_dirty = true;
		dirty = true;
	}

	void Camera::setTarget(const glm::vec3 &_target)
	{
		target = _target;
		matrix_dirty = true;
		dirty = true;
	}

	void Camera::lookAtTarget()
	{
		if (mode == CameraMode::targeting)
		{
			if (object)
			{
				target = object->get_coord() + object->model->eye_position * object->get_scale();
				object = nullptr;
			}

			if (axis_dirty) update_axis();
			coord = target + axis[2] * length;
			matrix_dirty = true;
			dirty = true;
		}
	}


	void Camera::updateFrustum()
	{
		auto tanHfFovy = glm::tan(glm::radians(fovy * 0.5f));

		auto _y1 = near_plane * tanHfFovy;
		auto _z1 = _y1 * res_aspect;
		auto _y2 = far_plane * tanHfFovy;
		auto _z2 = _y2 * res_aspect;
		frustumPoints[0] = -_z1 * axis[2] + _y1 * axis[1] + near_plane * axis[0] + coord;
		frustumPoints[1] = _z1 * axis[2] + _y1 * axis[1] + near_plane * axis[0] + coord;
		frustumPoints[2] = _z1 * axis[2] + -_y1 * axis[1] + near_plane * axis[0] + coord;
		frustumPoints[3] = -_z1 * axis[2] + -_y1 * axis[1] + near_plane * axis[0] + coord;
		frustumPoints[4] = -_z2 * axis[2] + _y2 * axis[1] + far_plane * axis[0] + coord;
		frustumPoints[5] = _z2 * axis[2] + _y2 * axis[1] + far_plane * axis[0] + coord;
		frustumPoints[6] = _z2 * axis[2] + -_y2 * axis[1] + far_plane * axis[0] + coord;
		frustumPoints[7] = -_z2 * axis[2] + -_y2 * axis[1] + far_plane * axis[0] + coord;
		for (int i = 0; i < 4; i++)
		{
			auto y = frustumPoints[i + 4].y;
			if (y < 0.f)
			{
				auto py = frustumPoints[i + 4].y - frustumPoints[i].y;
				if (py != 0.f)
				{
					frustumPoints[i + 4].x -= y * ((frustumPoints[i + 4].x - frustumPoints[i].x) / py);
					frustumPoints[i + 4].z -= y * ((frustumPoints[i + 4].z - frustumPoints[i].z) / py);
					frustumPoints[i + 4].y = 0.f;
				}
			}
		}

		auto vp = proj_matrix * matrix;

		frustumPlanes[0].x = vp[0].w + vp[0].x;
		frustumPlanes[0].y = vp[1].w + vp[1].x;
		frustumPlanes[0].z = vp[2].w + vp[2].x;
		frustumPlanes[0].w = vp[3].w + vp[3].x;

		frustumPlanes[1].x = vp[0].w - vp[0].x;
		frustumPlanes[1].y = vp[1].w - vp[1].x;
		frustumPlanes[1].z = vp[2].w - vp[2].x;
		frustumPlanes[1].w = vp[3].w - vp[3].x;

		frustumPlanes[2].x = vp[0].w - vp[0].y;
		frustumPlanes[2].y = vp[1].w - vp[1].y;
		frustumPlanes[2].z = vp[2].w - vp[2].y;
		frustumPlanes[2].w = vp[3].w - vp[3].y;

		frustumPlanes[3].x = vp[0].w + vp[0].y;
		frustumPlanes[3].y = vp[1].w + vp[1].y;
		frustumPlanes[3].z = vp[2].w + vp[2].y;
		frustumPlanes[3].w = vp[3].w + vp[3].y;

		frustumPlanes[4].x = vp[0].w + vp[0].z;
		frustumPlanes[4].y = vp[1].w + vp[1].z;
		frustumPlanes[4].z = vp[2].w + vp[2].z;
		frustumPlanes[4].w = vp[3].w + vp[3].z;

		frustumPlanes[5].x = vp[0].w - vp[0].z;
		frustumPlanes[5].y = vp[1].w - vp[1].z;
		frustumPlanes[5].z = vp[2].w - vp[2].z;
		frustumPlanes[5].w = vp[3].w - vp[3].z;

		for (auto i = 0; i < 6; i++)
			frustumPlanes[i] = glm::normalize(frustumPlanes[i]);
	}

	void Camera::reset()
	{
		Controller::reset();
		coord = glm::vec3(0.f);
		length = 1.0f;
		matrix_dirty = true;
		dirty = true;
	}

	void Camera::rotateByCursor(float x, float y)
	{
		add_euler(-x * 180.f, -y * 180.f, 0.f);
		lookAtTarget();
	}

	void Camera::moveByCursor(float x, float y)
	{
		auto l = length / near_plane;
		auto cy = tan(glm::radians(fovy / 2.f)) * near_plane * 2.f;
		target += (-x * cy * res_aspect * l) * axis[0] + (y * cy * l) * axis[1];
		lookAtTarget();
	}

	void Camera::scroll(float value)
	{
		if (mode == CameraMode::targeting)
		{
			if (value < 0.f)
				length = (length + 0.1) * 1.1f;
			else
				length = (length / 1.1f) - 0.1f;
			if (length < 1.f)
			{
				length = 1.f;
				coord += glm::normalize(target - coord) * 0.5f;
			}
			matrix_dirty = true;
			dirty = true;
		}
	}

	void Camera::move()
	{
		glm::vec3 coord;
		glm::vec3 euler;
		if (!Controller::move(get_euler().x, coord, euler))
			return;
		switch (mode)
		{
		case CameraMode::free:
			add_coord(coord);
			break;
		case CameraMode::targeting:
			setTarget(target + coord);
			break;
		}
		add_euler(euler);
	}

	glm::mat4 Camera::get_view_matrix()
	{
		if (matrix_dirty)
		{
			update_matrix();
			view_matrix = glm::inverse(matrix);
		}
		return view_matrix;
	}
}
