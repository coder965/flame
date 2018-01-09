#pragma once

#include "../refl.h"
#include "../math/math.h"

namespace tke
{
	REFLECTABLE struct Transformer
	{
		REFL_BANK;

		enum Axis
		{
			AxisNull = -1,
			AxisX,
			AxisY,
			AxisZ
		};

		REFLv glm::vec3 coord = glm::vec3(0.f);
		REFLv glm::vec3 euler = glm::vec3(0.f); // (yaw, pitch, roll)
		glm::vec4 quat = glm::vec4(0.f, 0.f, 0.f, 1.f);
		REFLv glm::vec3 scale = glm::vec3(1.f);

		glm::mat3 axis = glm::mat3(1.f);
		glm::mat4 matrix = glm::mat4(1.f);

		bool axis_dirty = false;
		bool euler_dirty = false;
		bool quat_dirty = false;
		bool matrix_dirty = false;

		bool dirty = true;

		Transformer();
		Transformer(glm::mat3 &rotation, glm::vec3 coord);

		glm::vec3 get_coord() const;
		glm::vec3 get_euler();
		glm::vec4 get_quat();
		glm::vec3 get_scale() const;

		glm::mat3 get_axis();
		glm::mat4 get_matrix();

		void set_coord(const glm::vec3 &_coord);
		inline void set_coord(float x, float y, float z) { set_coord(glm::vec3(x, y, z)); }
		inline void set_coord(float v) { set_coord(glm::vec3(v)); }
		inline void setCoordX(float v) { auto p = get_coord();  set_coord(glm::vec3(v, p.y, p.z)); }
		inline void setCoordY(float v) { auto p = get_coord();  set_coord(glm::vec3(p.x, v, p.z)); }
		inline void setCoordZ(float v) { auto p = get_coord();  set_coord(glm::vec3(p.x, p.y, v)); }
		void addCoord(const glm::vec3 &_coord);
		inline void addCoord(float x, float y, float z) { addCoord(glm::vec3(x, y, z)); }
		inline void addCoord(float v) { addCoord(glm::vec3(v)); }
		inline void addCoordX(float v) { addCoord(glm::vec3(v, 0.f, 0.f)); }
		inline void addCoordY(float v) { addCoord(glm::vec3(0.f, v, 0.f)); }
		inline void addCoordZ(float v) { addCoord(glm::vec3(0.f, 0.f, v)); }
		void setEuler(const glm::vec3 &_euler);
		inline void setEuler(float x, float y, float z) { setEuler(glm::vec3(x, y, z)); }
		inline void setEuler(float v) { setEuler(glm::vec3(v)); }
		inline void setEulerX(float v) { auto p = get_euler();  set_coord(glm::vec3(v, p.y, p.z)); }
		inline void setEulerY(float v) { auto p = get_euler();  set_coord(glm::vec3(p.x, v, p.z)); }
		inline void setEulerZ(float v) { auto p = get_euler();  set_coord(glm::vec3(p.x, p.y, v)); }
		void addEuler(const glm::vec3 &_euler);
		inline void addEuler(float x, float y, float z) { addEuler(glm::vec3(x, y, z)); }
		inline void addEuler(float v) { addEuler(glm::vec3(v)); }
		inline void addEulerX(float v) { addEuler(glm::vec3(v, 0.f, 0.f)); }
		inline void addEulerY(float v) { addEuler(glm::vec3(0.f, v, 0.f)); }
		inline void addEulerZ(float v) { addEuler(glm::vec3(0.f, 0.f, v)); }
		void setQuat(const glm::vec4 &_quat);
		inline void setQuat(float x, float y, float z, float w) { setQuat(glm::vec4(x, y, z, w)); }
		void leftRotate(const glm::mat3 &left);
		void rightRotate(const glm::mat3 &right);
		void axisRotate(Axis which, float angle);
		void setScale(const glm::vec3 &_scale);
		inline void setScale(float x, float y, float z) { setScale(glm::vec3(x, y, z)); }
		inline void setScale(float v) { setScale(glm::vec3(v)); }
		inline void setScaleX(float v) { auto p = get_scale();  setScale(glm::vec3(v, p.y, p.z)); }
		inline void setScaleY(float v) { auto p = get_scale();  setScale(glm::vec3(p.x, v, p.z)); }
		inline void setScaleZ(float v) { auto p = get_scale();  setScale(glm::vec3(p.x, p.y, v)); }
		void addScale(const glm::vec3 &_scale);
		inline void addScale(float x, float y, float z) { addScale(glm::vec3(x, y, z)); }
		inline void addScale(float v) { addScale(glm::vec3(v)); }
		inline void addScaleX(float v) { addScale(glm::vec3(v, 0.f, 0.f)); }
		inline void addScaleY(float v) { addScale(glm::vec3(0.f, v, 0.f)); }
		inline void addScaleZ(float v) { addScale(glm::vec3(0.f, 0.f, v)); }

		void relate(Transformer *t);
		void scaleRelate(Transformer *t);

		void updateMat();

		void updateAxis();
		void updateEuler();
		void updateQuat();
	};
}
