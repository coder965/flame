#ifndef __TKE_TRANSFORMER__
#define __TKE_TRANSFORMER__

#include "math.h"

namespace tke
{
	struct Transformer
	{
		int id;

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

		glm::vec3 coord;
		glm::vec3 euler; // (yaw(y), pitch(z), roll(x))
		glm::vec4 quat = glm::vec4(0.f, 0.f, 0.f, 1.f);
		glm::vec3 scale = glm::vec3(1.f);
		glm::vec3 worldScale = glm::vec3(1.f);

		glm::mat3 axis;
		glm::mat4 mat;
		glm::mat4 matInv;

		bool needUpdateAxis = false;
		bool needUpdateEuler = false;
		bool needUpdateQuat = false;
		bool needUpdateMat = false;

		bool changed = true;

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

		void setCoord(const glm::vec3 &_coord);
		void addCoord(const glm::vec3 &_coord);
		void setEuler(const glm::vec3 &_euler);
		void addEuler(const glm::vec3 &_euler);
		void setQuat(const glm::vec4 &_quat);
		void leftRotate(const glm::mat3 &left);
		void rightRotate(const glm::mat3 &right);
		void axisRotate(Axis which, float angle);
		void setScale(const glm::vec3 &_scale);
		void addScale(const glm::vec3 &_scale);
		void setWorldScale(const glm::vec3 &_scale);

		void relate(Transformer *t);
		void scaleRelate(Transformer *t);

		void updateMat();

		void updateAxis();
		void updateEuler();
		void updateQuat();
	};
}

#endif