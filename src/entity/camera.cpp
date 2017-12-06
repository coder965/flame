#include "camera.h"
#include "object.h"
#include "../core.h"

namespace tke
{
	Camera::Camera()
	{
		ang_offset = 90.f;
	}

	void Camera::setMode(CameraMode _mode)
	{
		mode = _mode;
		needUpdateMat = true;
		changed = true;
	}

	void Camera::setLength(float _length)
	{
		length = _length;
		needUpdateMat = true;
		changed = true;
	}

	void Camera::setTarget(const glm::vec3 &_target)
	{
		target = _target;
		needUpdateMat = true;
		changed = true;
	}

	void Camera::lookAtTarget()
	{
		if (mode == CameraMode::targeting)
		{
			if (object)
			{
				target = object->getCoord() + object->model->eye_position * object->getScale();
				object = nullptr;
			}

			if (needUpdateAxis) updateAxis();
			coord = target + axis[2] * length;
			needUpdateMat = true;
			changed = true;
		}
	}


	void Camera::updateFrustum()
	{
		auto tanHfFovy = glm::tan(glm::radians(fovy * 0.5f));

		auto _y1 = near_plane * tanHfFovy;
		auto _z1 = _y1 * screenAspect;
		auto _y2 = far_plane * tanHfFovy;
		auto _z2 = _y2 * screenAspect;
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

		auto matrix = matPerspective * mat;

		frustumPlanes[0].x = matrix[0].w + matrix[0].x;
		frustumPlanes[0].y = matrix[1].w + matrix[1].x;
		frustumPlanes[0].z = matrix[2].w + matrix[2].x;
		frustumPlanes[0].w = matrix[3].w + matrix[3].x;

		frustumPlanes[1].x = matrix[0].w - matrix[0].x;
		frustumPlanes[1].y = matrix[1].w - matrix[1].x;
		frustumPlanes[1].z = matrix[2].w - matrix[2].x;
		frustumPlanes[1].w = matrix[3].w - matrix[3].x;

		frustumPlanes[2].x = matrix[0].w - matrix[0].y;
		frustumPlanes[2].y = matrix[1].w - matrix[1].y;
		frustumPlanes[2].z = matrix[2].w - matrix[2].y;
		frustumPlanes[2].w = matrix[3].w - matrix[3].y;

		frustumPlanes[3].x = matrix[0].w + matrix[0].y;
		frustumPlanes[3].y = matrix[1].w + matrix[1].y;
		frustumPlanes[3].z = matrix[2].w + matrix[2].y;
		frustumPlanes[3].w = matrix[3].w + matrix[3].y;

		frustumPlanes[4].x = matrix[0].w + matrix[0].z;
		frustumPlanes[4].y = matrix[1].w + matrix[1].z;
		frustumPlanes[4].z = matrix[2].w + matrix[2].z;
		frustumPlanes[4].w = matrix[3].w + matrix[3].z;

		frustumPlanes[5].x = matrix[0].w - matrix[0].z;
		frustumPlanes[5].y = matrix[1].w - matrix[1].z;
		frustumPlanes[5].z = matrix[2].w - matrix[2].z;
		frustumPlanes[5].w = matrix[3].w - matrix[3].z;

		for (auto i = 0; i < 6; i++)
			frustumPlanes[i] = glm::normalize(frustumPlanes[i]);
	}

	void Camera::reset()
	{
		Controller::reset();
		coord = glm::vec3(0.f);
		length = 1.0f;
		needUpdateMat = true;
		changed = true;
	}

	void Camera::rotateByCursor(float x, float y)
	{
		addEuler(glm::vec3(-x * 180.f, 0.f, -y * 180.f));
		lookAtTarget();
	}

	void Camera::moveByCursor(float x, float y)
	{
		auto l = length / near_plane;
		auto cy = tan(glm::radians(fovy / 2.f)) * near_plane * 2.f;
		target += (-x * cy * screenAspect * l) * axis[0] + (y * cy * l) * axis[1];
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
			needUpdateMat = true;
			changed = true;
		}
	}

	void Camera::move()
	{
		glm::vec3 coord;
		glm::vec3 euler;
		if (!Controller::move(getEuler().x, coord, euler))
			return;
		switch (mode)
		{
		case CameraMode::free:
			addCoord(coord);
			break;
		case CameraMode::targeting:
			setTarget(target + coord);
			break;
		}
		addEuler(euler);
	}
}
