#include "node.h"
#include "camera.h"
#include "water.h"

namespace tke
{
	Node::Node(NodeType _type) :
		coord(0.f), 
		euler(0.f), 
		quat(0.f, 0.f, 0.f, 1.f), 
		scale(1.f),
		axis(1.f),
		matrix(1.f),
		axis_dirty(false),
		euler_dirty(false),
		quat_dirty(false),
		matrix_dirty(false),
		transform_dirty(true),
		attribute_dirty(true),
		image_dirty(true),
		type(_type),
		parent(nullptr)
	{
	}

	Node::~Node()
	{
		if (parent)
			parent->remove_child(this);
	}

	glm::vec3 Node::get_coord() const
	{
		return coord;
	}

	glm::mat3 Node::get_axis()
	{
		if (axis_dirty)
			update_axis();
		return axis;
	}

	glm::vec3 Node::get_scale() const
	{
		return scale;
	}

	glm::vec3 Node::get_euler()
	{
		if (euler_dirty)
			update_euler();
		return euler;
	}

	glm::vec4 Node::get_quat()
	{
		if (quat_dirty)
			update_quat();
		return quat;
	}

	glm::mat4 Node::get_matrix()
	{
		if (matrix_dirty)
			update_matrix();
		return matrix;
	}

	glm::mat4 Node::get_world_matrix()
	{
		auto m = get_matrix();
		if (parent)
			m = parent->get_world_matrix() * m;
		return m;
	}

	glm::vec3 Node::get_world_coord()
	{
		return glm::vec3(get_world_matrix()[3]);
	}

	void Node::set_coord(const glm::vec3 &_coord)
	{
		coord = _coord;
		mark_coord_setted();
	}

	void Node::set_coord(float x, float y, float z)
	{
		coord.x = x;
		coord.y = y;
		coord.z = z;
		mark_coord_setted();
	}

	void Node::set_coord(float v)
	{
		coord.x = v;
		coord.y = v;
		coord.z = v;
		mark_coord_setted();
	}

	void Node::set_coord_x(float v)
	{
		coord.x = v;
		mark_coord_setted();
	}

	void Node::set_coord_y(float v)
	{
		coord.y = v;
		mark_coord_setted();
	}

	void Node::set_coord_z(float v)
	{
		coord.z = v;
		mark_coord_setted();
	}

	void Node::add_coord(const glm::vec3 &_coord)
	{
		coord += _coord;
		mark_coord_setted();
	}

	void Node::add_coord(float x, float y, float z)
	{
		coord.x += x;
		coord.y += y;
		coord.z += z;
		mark_coord_setted();
	}

	void Node::add_coord(float v)
	{
		coord.x += v;
		coord.y += v;
		coord.z += v;
		mark_coord_setted();
	}

	void Node::add_coord_x(float v)
	{
		coord.x += v;
		mark_coord_setted();
	}

	void Node::add_coord_y(float v)
	{
		coord.y += v;
		mark_coord_setted();
	}

	void Node::add_coord_z(float v)
	{
		coord.z += v;
		mark_coord_setted();
	}

	void Node::set_euler(const glm::vec3 &_euler)
	{
		euler = glm::mod(_euler, 360.f);
		mark_euler_setted();
	}

	void Node::set_euler(float x, float y, float z)
	{
		euler.x = glm::mod(x, 360.f);
		euler.y = glm::mod(y, 360.f);
		euler.z = glm::mod(z, 360.f);
		mark_euler_setted();
	}

	void Node::set_euler(float v)
	{
		v = glm::mod(v, 360.f);
		euler.x = v;
		euler.y = v;
		euler.z = v;
		mark_euler_setted();
	}

	void Node::set_euler_x(float v)
	{
		if (euler_dirty)
			update_euler();
		euler.x = glm::mod(v, 360.f);
		mark_euler_setted();
	}

	void Node::set_euler_y(float v)
	{
		if (euler_dirty)
			update_euler();
		euler.y = glm::mod(v, 360.f);
		mark_euler_setted();
	}

	void Node::set_euler_z(float v)
	{
		if (euler_dirty)
			update_euler();
		euler.z = glm::mod(v, 360.f);
		mark_euler_setted();
	}

	void Node::add_euler(const glm::vec3 &_euler)
	{
		if (euler_dirty)
			update_euler();
		euler += _euler;
		euler = glm::mod(euler, 360.f);
		mark_euler_setted();
	}

	void Node::add_euler(float x, float y, float z)
	{
		if (euler_dirty)
			update_euler();
		euler.x += x;
		euler.y += y;
		euler.z += z;
		euler = glm::mod(euler, 360.f);
		mark_euler_setted();
	}

	void Node::add_euler(float v)
	{
		if (euler_dirty)
			update_euler();
		euler.x += v;
		euler.y += v;
		euler.z += v;
		euler = glm::mod(euler, 360.f);
		mark_euler_setted();
	}

	void Node::add_euler_x(float v)
	{
		if (euler_dirty)
			update_euler();
		euler.x += v;
		euler = glm::mod(euler, 360.f);
		mark_euler_setted();
	}

	void Node::add_euler_y(float v)
	{
		if (euler_dirty)
			update_euler();
		euler.y += v;
		euler = glm::mod(euler, 360.f);
		mark_euler_setted();
	}

	void Node::add_euler_z(float v)
	{
		if (euler_dirty)
			update_euler();
		euler.z += v;
		euler = glm::mod(euler, 360.f);
		mark_euler_setted();
	}

	void Node::set_quat(const glm::vec4 &_quat)
	{
		quat = _quat;
		mark_quat_setted();
	}

	void Node::set_quat(float x, float y, float z, float w)
	{
		quat.x = x;
		quat.y = y;
		quat.z = z;
		quat.w = w;
		mark_quat_setted();
	}

	void Node::left_rotate(const glm::mat3 &left)
	{
		axis = left * axis;
		mark_axis_setted();
	}

	void Node::right_rotate(const glm::mat3 &right)
	{
		axis = axis * right;
		mark_axis_setted();
	}

	void Node::axis_rotate(Axis which, float angle)
	{
		switch (which)
		{
			case AxisX:
			{
				auto m = glm::mat3(glm::rotate(glm::radians(angle), axis[0]));
				axis[1] = glm::normalize(m * axis[1]);
				axis[2] = glm::normalize(m * axis[2]);
				break;
			}
			case AxisY:
			{
				auto m = glm::mat3(glm::rotate(glm::radians(angle), axis[1]));
				axis[0] = glm::normalize(m * axis[0]);
				axis[2] = glm::normalize(m * axis[2]);
				break;
			}
			case AxisZ:
			{
				auto m = glm::mat3(glm::rotate(glm::radians(angle), axis[2]));
				axis[1] = glm::normalize(m * axis[1]);
				axis[0] = glm::normalize(m * axis[0]);
				break;
			}
		}
		mark_axis_setted();
	}

	void Node::set_scale(const glm::vec3 &_scale)
	{
		scale = _scale;
		mark_scale_setted();
	}

	void Node::set_scale(float x, float y, float z)
	{
		scale.x = x;
		scale.y = y;
		scale.z = z;
		mark_scale_setted();
	}

	void Node::set_scale(float v)
	{
		scale.x = v;
		scale.y = v;
		scale.z = v;
		mark_scale_setted();
	}

	void Node::set_scale_x(float v)
	{
		scale.x = v;
		mark_scale_setted();
	}

	void Node::set_scale_y(float v)
	{
		scale.y = v;
		mark_scale_setted();
	}

	void Node::set_scale_z(float v)
	{
		scale.z = v;
		mark_scale_setted();
	}

	void Node::add_scale(const glm::vec3 &_scale)
	{
		scale += _scale;
		mark_scale_setted();
	}

	void Node::add_scale(float x, float y, float z)
	{
		scale.x += x;
		scale.y += y;
		scale.z += z;
		mark_scale_setted();
	}

	void Node::add_scale(float v)
	{
		scale.x += v;
		scale.y += v;
		scale.z += v;
		mark_scale_setted();
	}

	void Node::add_scale_x(float v)
	{
		scale.x += v;
		mark_scale_setted();
	}

	void Node::add_scale_y(float v)
	{
		scale.y += v;
		mark_scale_setted();
	}

	void Node::add_scale_z(float v)
	{
		scale.z += v;
		mark_scale_setted();
	}

	void Node::scale_relate(Node *t)
	{
		coord *= t->scale;
		coord *= t->scale;
		axis = t->axis * axis;
		coord = t->axis * coord;
		coord += t->coord;
		matrix_dirty = true;

		transform_dirty = true;
	}

	void Node::relate(Node *t)
	{
		coord -= t->coord;
		axis *= glm::transpose(t->axis);
		matrix_dirty = true;

		transform_dirty = true;
	}

	void Node::update_matrix()
	{
		if (axis_dirty)
			update_axis();
		matrix = glm::translate(coord) * glm::mat4(axis) * glm::scale(scale);
		matrix_dirty = false;
	}

	void Node::update_axis()
	{
		if (!quat_dirty)
			axis = quaternion_to_mat3(quat);// update by quat
		else
			axis = euler_to_mat3(euler);// update by euler
		axis_dirty = false;
	}

	void Node::update_euler()
	{
		if (quat_dirty)
			update_quat(); // updata by quat

		euler = quaternion_to_euler(quat);
		euler_dirty = false;
	}

	void Node::update_quat()
	{
		if (axis_dirty)
			update_axis(); // update by axis
		quat = mat3_to_quaternion(axis);
		quat_dirty = false;
	}

	bool Node::is_transform_dirty()
	{
		return transform_dirty;
	}

	bool Node::is_attribute_dirty()
	{
		return attribute_dirty;
	}

	bool Node::is_image_dirty()
	{
		return image_dirty;
	}

	void Node::update()
	{
		on_update();
		transform_dirty = false;
		attribute_dirty = false;
		image_dirty = false;
		for (auto &c : children)
			c->update();
	}

	NodeType Node::get_type() const
	{
		return type;
	}

	Node *Node::get_parent() const
	{
		return parent;
	}

	const std::vector<std::unique_ptr<Node>> &Node::get_children() const
	{
		return children;
	}

	Camera *Node::new_camera()
	{
		auto c = new Camera;
		c->parent = this;
		c->name = "camera";
		children.emplace_back(c);
		return c;
	}

	Water *Node::new_water()
	{
		auto w = new Water;
		w->parent = this;
		w->name = "water";
		broadcast(w, MessageWaterAdd);
		children.emplace_back(w);
		return w;
	}

	void Node::add_child(Node *n)
	{
		n->parent = this;
		children.emplace_back(n);
	}

	void Node::remove_child(Node *n)
	{
		for (auto it = children.begin(); it != children.end(); it++)
		{
			if (it->get() == n)
			{
				switch (n->type)
				{
					case NodeTypeWater:
						broadcast(n, MessageWaterRemove);
						break;
				}
				children.erase(it);
				return;
			}
		}
	}

	bool Node::broadcast(Node *src, Message msg)
	{
		if (_Object::broadcast(src, msg))
			return true;
		if (parent)
			return parent->broadcast(src, msg);
		return false;
	}

	void Node::mark_coord_setted()
	{
		matrix_dirty = true;
		transform_dirty = true;
	}

	void Node::mark_euler_setted()
	{
		axis_dirty = true;
		euler_dirty = false;
		quat_dirty = true;
		matrix_dirty = true;
		transform_dirty = true;
	}

	void Node::mark_quat_setted()
	{
		axis_dirty = true;
		euler_dirty = true;
		quat_dirty = false;
		matrix_dirty = true;
		transform_dirty = true;
	}

	void Node::mark_axis_setted()
	{
		axis_dirty = false;
		euler_dirty = true;
		quat_dirty = true;
		matrix_dirty = true;
		transform_dirty = true;
	}

	void Node::mark_scale_setted()
	{
		matrix_dirty = true;
		transform_dirty = true;
	}
}
