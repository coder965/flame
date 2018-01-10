#pragma once

#include <vector>
#include <memory>

#include "../refl.h"
#include "../math/math.h"
#include "../_object.h"
#include "component.h"

namespace tke
{
	enum NodeType
	{
		NodeTypeScene,
		NodeTypeCamera,
		NodeTypeLight,
		NodeTypeObject,
		NodeTypeTerrain,
		NodeTypeWater
	};

	struct Water;

	REFLECTABLE struct Node : _Object
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

		NodeType type;
		Node *parent = nullptr;
		std::vector<std::unique_ptr<Node>> children; 

		virtual bool on_message(_Object*, Message) { return false; }

		Node(NodeType _type);
		Node(NodeType _type, const glm::vec3 &_coord, const glm::mat3 &rotation);

		glm::vec3 get_coord() const;
		glm::vec3 get_euler();
		glm::vec4 get_quat();
		glm::vec3 get_scale() const;

		glm::mat3 get_axis();
		glm::mat4 get_matrix();

		void set_coord(const glm::vec3 &_coord);
		void set_coord(float x, float y, float z);
		void set_coord(float v);
		void set_coord_x(float v);
		void set_coord_y(float v);
		void set_coord_z(float v);
		void add_coord(const glm::vec3 &_coord);
		void add_coord(float x, float y, float z);
		void add_coord(float v);
		void add_coord_x(float v);
		void add_coord_y(float v);
		void add_coord_z(float v);
		void set_euler(const glm::vec3 &_euler);
		void set_euler(float x, float y, float z);
		void set_euler(float v);
		void set_euler_x(float v);
		void set_euler_y(float v);
		void set_euler_z(float v);
		void add_euler(const glm::vec3 &_euler);
		void add_euler(float x, float y, float z);
		void add_euler(float v);
		void add_euler_x(float v);
		void add_euler_y(float v);
		void add_euler_z(float v);
		void set_quat(const glm::vec4 &_quat);
		void set_quat(float x, float y, float z, float w);
		void left_rotate(const glm::mat3 &left);
		void right_rotate(const glm::mat3 &right);
		void axis_rotate(Axis which, float angle);
		void set_scale(const glm::vec3 &_scale);
		void set_scale(float x, float y, float z);
		void set_scale(float v);
		void set_scale_x(float v);
		void set_scale_y(float v);
		void set_scale_z(float v);
		void add_scale(const glm::vec3 &_scale);
		void add_scale(float x, float y, float z);
		void add_scale(float v);
		void add_scale_x(float v);
		void add_scale_y(float v);
		void add_scale_z(float v);

		void relate(Node *t);
		void scaleRelate(Node *t);

		void update_matrix();
		void update_axis();
		void update_euler();
		void update_quat();

		Water *new_water();
		void remove_child(Node *n);

		bool broadcast(Node *src, Message msg);

	private:
		void mark_coord_setted();
		void mark_euler_setted();
		void mark_quat_setted();
		void mark_axis_setted();
		void mark_scale_setted();
	};
}
