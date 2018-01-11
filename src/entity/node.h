#pragma once

#include <vector>
#include <memory>

#include "../math/math.h"
#include "../_object.h"
#include "component.h"

namespace tke
{
	enum NodeType
	{
		NodeTypeNode,
		NodeTypeScene,
		NodeTypeCamera,
		NodeTypeLight,
		NodeTypeObject,
		NodeTypeTerrain,
		NodeTypeWater
	};

	struct Scene;
	struct Camera;
	struct Water;

	 class Node : public _Object
	{
	public:
		enum Axis
		{
			AxisNull = -1,
			AxisX,
			AxisY,
			AxisZ
		};
	private:
		glm::vec3 coord;
		glm::vec3 euler; // (yaw, pitch, roll)
		glm::vec4 quat;
		glm::vec3 scale;

		glm::mat3 axis;
		glm::mat4 matrix;

		bool axis_dirty;
		bool euler_dirty;
		bool quat_dirty;
		bool matrix_dirty;
	protected:
		bool transform_dirty;
		bool attribute_dirty;
		bool image_dirty;
	private:
		NodeType type;
		Node *parent;
		std::vector<std::unique_ptr<Node>> children; 
	public:
		virtual bool on_message(_Object*, Message) { return false; }

		bool broadcast(Node *src, Message msg);

		Node(NodeType _type);
		virtual ~Node();

		glm::vec3 get_coord() const;
		glm::vec3 get_euler();
		glm::vec4 get_quat();
		glm::vec3 get_scale() const;

		glm::mat3 get_axis();
		glm::mat4 get_matrix();
		glm::mat4 get_world_matrix();
		glm::vec3 get_world_coord();

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
		void scale_relate(Node *t);

		bool is_transform_dirty();
		bool is_attribute_dirty();
		bool is_image_dirty();

		NodeType get_type() const;
		Node *get_parent() const;
		const std::vector<std::unique_ptr<Node>> &get_children() const;
		
		Camera *new_camera();
		Water *new_water();
		void add_child(Node *n);
		void remove_child(Node *n);

		void update();
		void clear();
	protected:
		virtual void on_update() {};
		virtual void on_clear() {};

	private:
		void update_matrix();
		void update_axis();
		void update_euler();
		void update_quat();

		void mark_coord_setted();
		void mark_euler_setted();
		void mark_quat_setted();
		void mark_axis_setted();
		void mark_scale_setted();
	};
}
