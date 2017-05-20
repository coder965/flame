#ifndef __TKE_TRANSFORMER__
#define __TKE_TRANSFORMER__

#include "math.h"

namespace tke
{
	struct Transformer
	{
		int m_id;

		enum class Type
		{
			eMove,
			eEulerSet,
			eScale,
			eAsixRotate
		};

		enum class Axis
		{
			eNull = -1,
			eX,
			eY,
			eZ
		};

		glm::vec3 m_coord;
		glm::vec3 m_euler; // (yaw(y), pitch(z), roll(x))
		glm::vec4 m_quat = glm::vec4(0.f, 0.f, 0.f, 1.f);
		glm::vec3 m_scale = glm::vec3(1.f);
		glm::vec3 m_worldScale = glm::vec3(1.f);

		glm::mat3 m_axis;
		glm::mat4 m_mat;
		glm::mat4 m_matInv;

		bool m_needUpdateAxis = false;
		bool m_needUpdateEuler = false;
		bool m_needUpdateQuat = false;
		bool m_needUpdateMat = false;

		bool m_changed = true;

		Transformer();
		Transformer(glm::mat3 &rotation, glm::vec3 coord);

		glm::vec3 getCoord() const;
		glm::vec3 getEuler();
		glm::vec4 getQuat();
		glm::vec3 getScale() const;
		glm::vec3 getWorldScale() const;

		glm::mat3 getAxis();
		glm::mat4 getMat();
		glm::mat4 getMatInv();

		glm::vec3 getTrans(Transformer::Type type);

		void setCoord(const glm::vec3 &coord);
		void addCoord(const glm::vec3 &coord);
		void setEuler(const glm::vec3 &euler);
		void addEuler(const glm::vec3 &euler);
		void setQuat(const glm::vec4 &quat);
		void leftRotate(glm::mat3 left);
		void rightRotate(glm::mat3 right);
		void axisRotate(Axis which, float angle);
		void setScale(const glm::vec3 &scale);
		void addScale(const glm::vec3 &scale);
		void setWorldScale(const glm::vec3 &scale);

		void relate(Transformer *t);
		void scaleRelate(Transformer *t);

		void updateMat();

		void updateAxis();
		void updateEuler();
		void updateQuat();
	};
}

#endif