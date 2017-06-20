#pragma once

#include <fstream>

#include "transformer.h"
#include "controler.h"

namespace tke
{
	enum CameraMode
	{
		CameraModeFree,
		CameraModeTargeting
	};

	struct Camera : Transformer, Controller
	{
		CameraMode mode = CameraModeFree;
		glm::vec3 target;
		float length = 1.f;

		glm::vec3 frustumPoints[8];
		glm::vec4 frustumPlanes[6];

		Camera();
		void setMode(CameraMode _mode);
		void setLength(float _length);
		void setTarget(const glm::vec3 &_target);
		void lookAtTarget();
		void updateFrustum();
		void reset();
		void addAngAccrodingToScreen(float x, float y);
		void scroll(short value);
		void moveAccrodingToScreen(float x, float y);
		void move();
	};
}
