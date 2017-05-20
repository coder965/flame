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

		Mode m_mode = Mode::eFree;
		glm::vec3 m_target;
		float m_length = 1.f;

		glm::vec3 m_frustumPoints[8];
		glm::vec4 m_frustumPlanes[6];

		Camera();
		void setMode(Mode mode);
		void setLength(float length);
		void setTarget(glm::vec3 target);
		void lookAtTarget();
		void updateFrustum(float aspect, glm::mat4 &matProj);
		void reset();
		void addAngAccrodingToScreen(float x, float y);
		void scroll(short value);
		void moveAccrodingToScreen(float aspect, float x, float y);
		void move();
		void load(std::ifstream &file);
		void save(std::ofstream &file);
	};
}

#endif