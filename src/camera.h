#ifndef __TKE_CAMERA__
#define __TKE_CAMERA__

#include <fstream>

#include "transformer.h"
#include "controler.h"

namespace tke
{
	struct Camera : Transformer, Controller
	{
		enum class Mode : int
		{
			eFree,
			eTargeting
		};

		Mode mode = Mode::eFree;
		glm::vec3 target;
		float length = 1.f;

		glm::vec3 frustumPoints[8];
		glm::vec4 frustumPlanes[6];

		Camera();
		void setMode(Mode _mode);
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

#endif