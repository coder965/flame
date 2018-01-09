#pragma once

#include "transformer.h"
#include "controller.h"

namespace tke
{
	struct Object;

	enum class CameraMode
	{
		free = 1 << 0,
		targeting = 1 << 1
	};

	struct Camera : Transformer, Controller
	{
		CameraMode mode = CameraMode::free;
		glm::vec3 target = glm::vec3(0.f);
		float length = 1.f;
		glm::mat4 view_matrix;

		glm::vec3 frustumPoints[8];
		glm::vec4 frustumPlanes[6];

		Object *object = nullptr;

		Camera();
		void setMode(CameraMode _mode);
		void setLength(float _length);
		void setTarget(const glm::vec3 &_target);
		void lookAtTarget();
		void updateFrustum();
		void reset();
		void rotateByCursor(float x, float y);
		void moveByCursor(float x, float y);
		void scroll(float value);
		void move();
		glm::mat4 get_view_matrix();
	};
}
