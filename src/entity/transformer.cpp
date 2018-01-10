#include "transformer.h"

namespace tke
{
	Transformer::Transformer() {}

	Transformer::Transformer(glm::mat3 &rotation, glm::vec3 coord)
	{
		axis = rotation;
		coord = coord;

		quat_dirty = true;
		euler_dirty = true;
		matrix_dirty = true;
	}

	glm::vec3 Transformer::get_coord() const
	{
		return coord;
	}

	glm::mat3 Transformer::get_axis()
	{
		if (axis_dirty)
			update_axis();
		return axis;
	}

	glm::vec3 Transformer::get_scale() const
	{
		return scale;
	}

	glm::vec3 Transformer::get_euler()
	{
		if (euler_dirty)
			update_euler();
		return euler;
	}

	glm::vec4 Transformer::get_quat()
	{
		if (quat_dirty)
			update_quat();
		return quat;
	}

	glm::mat4 Transformer::get_matrix()
	{
		if (matrix_dirty)
			update_matrix();
		return matrix;
	}

	void Transformer::set_coord(const glm::vec3 &_coord)
	{
		coord = _coord;
		mark_coord_setted();
	}

	void Transformer::set_coord(float x, float y, float z)
	{
		coord.x = x;
		coord.y = y;
		coord.z = z;
		mark_coord_setted();
	}

	void Transformer::set_coord(float v)
	{
		coord.x = v;
		coord.y = v;
		coord.z = v;
		mark_coord_setted();
	}

	void Transformer::set_coord_x(float v)
	{
		coord.x = v;
		mark_coord_setted();
	}

	void Transformer::set_coord_y(float v)
	{
		coord.y = v;
		mark_coord_setted();
	}

	void Transformer::set_coord_z(float v)
	{
		coord.z = v;
		mark_coord_setted();
	}

	void Transformer::add_coord(const glm::vec3 &_coord)
	{
		coord += _coord;
		mark_coord_setted();
	}

	void Transformer::add_coord(float x, float y, float z)
	{
		coord.x += x;
		coord.y += y;
		coord.z += z;
		mark_coord_setted();
	}

	void Transformer::add_coord(float v)
	{
		coord.x += v;
		coord.y += v;
		coord.z += v;
		mark_coord_setted();
	}

	void Transformer::add_coord_x(float v)
	{
		coord.x += v;
		mark_coord_setted();
	}

	void Transformer::add_coord_y(float v)
	{
		coord.y += v;
		mark_coord_setted();
	}

	void Transformer::add_coord_z(float v)
	{
		coord.z += v;
		mark_coord_setted();
	}

	void Transformer::set_euler(const glm::vec3 &_euler)
	{
		euler = glm::mod(_euler, 360.f);
		mark_euler_setted();
	}

	void Transformer::set_euler(float x, float y, float z)
	{
		euler.x = glm::mod(x, 360.f);
		euler.y = glm::mod(y, 360.f);
		euler.z = glm::mod(z, 360.f);
		mark_euler_setted();
	}

	void Transformer::set_euler(float v)
	{
		v = glm::mod(v, 360.f);
		euler.x = v;
		euler.y = v;
		euler.z = v;
		mark_euler_setted();
	}

	void Transformer::set_euler_x(float v)
	{
		if (euler_dirty)
			update_euler();
		euler.x = glm::mod(v, 360.f);
		mark_euler_setted();
	}

	void Transformer::set_euler_y(float v)
	{
		if (euler_dirty)
			update_euler();
		euler.y = glm::mod(v, 360.f);
		mark_euler_setted();
	}

	void Transformer::set_euler_z(float v)
	{
		if (euler_dirty)
			update_euler();
		euler.z = glm::mod(v, 360.f);
		mark_euler_setted();
	}

	void Transformer::add_euler(const glm::vec3 &_euler)
	{
		if (euler_dirty)
			update_euler();
		euler += _euler;
		euler = glm::mod(euler, 360.f);
		mark_euler_setted();
	}

	void Transformer::add_euler(float x, float y, float z)
	{
		if (euler_dirty)
			update_euler();
		euler.x += x;
		euler.y += y;
		euler.z += z;
		euler = glm::mod(euler, 360.f);
		mark_euler_setted();
	}

	void Transformer::add_euler(float v)
	{
		if (euler_dirty)
			update_euler();
		euler.x += v;
		euler.y += v;
		euler.z += v;
		euler = glm::mod(euler, 360.f);
		mark_euler_setted();
	}

	void Transformer::add_euler_x(float v)
	{
		if (euler_dirty)
			update_euler();
		euler.x += v;
		euler = glm::mod(euler, 360.f);
		mark_euler_setted();
	}

	void Transformer::add_euler_y(float v)
	{
		if (euler_dirty)
			update_euler();
		euler.y += v;
		euler = glm::mod(euler, 360.f);
		mark_euler_setted();
	}

	void Transformer::add_euler_z(float v)
	{
		if (euler_dirty)
			update_euler();
		euler.z += v;
		euler = glm::mod(euler, 360.f);
		mark_euler_setted();
	}

	void Transformer::set_quat(const glm::vec4 &_quat)
	{
		quat = _quat;
		mark_quat_setted();
	}

	void Transformer::set_quat(float x, float y, float z, float w)
	{
		quat.x = x;
		quat.y = y;
		quat.z = z;
		quat.w = w;
		mark_quat_setted();
	}

	void Transformer::left_rotate(const glm::mat3 &left)
	{
		axis = left * axis;
		mark_axis_setted();
	}

	void Transformer::right_rotate(const glm::mat3 &right)
	{
		axis = axis * right;
		mark_axis_setted();
	}

	void Transformer::axis_rotate(Axis which, float angle)
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

	void Transformer::set_scale(const glm::vec3 &_scale)
	{
		scale = _scale;
		mark_scale_setted();
	}

	void Transformer::set_scale(float x, float y, float z) 
	{
		scale.x = x;
		scale.y = y;
		scale.z = z;
		mark_scale_setted();
	}

	void Transformer::set_scale(float v) 
	{
		scale.x = v;
		scale.y = v;
		scale.z = v;
		mark_scale_setted();
	}

	void Transformer::set_scale_x(float v) 
	{
		scale.x = v;
		mark_scale_setted();
	}

	void Transformer::set_scale_y(float v) 
	{
		scale.y = v;
		mark_scale_setted();
	}

	void Transformer::set_scale_z(float v) 
	{
		scale.z = v;
		mark_scale_setted();
	}

	void Transformer::add_scale(const glm::vec3 &_scale)
	{
		scale += _scale;
		mark_scale_setted();
	}

	void Transformer::add_scale(float x, float y, float z) 
	{
		scale.x += x;
		scale.y += y;
		scale.z += z;
		mark_scale_setted();
	}

	void Transformer::add_scale(float v) 
	{
		scale.x += v;
		scale.y += v;
		scale.z += v;
		mark_scale_setted();
	}

	void Transformer::add_scale_x(float v) 
	{
		scale.x += v;
		mark_scale_setted();
	}

	void Transformer::add_scale_y(float v) 
	{
		scale.y += v;
		mark_scale_setted();
	}

	void Transformer::add_scale_z(float v) 
	{
		scale.z += v;
		mark_scale_setted();
	}

	void Transformer::scaleRelate(Transformer *t)
	{
		coord *= t->scale;
		coord *= t->scale;
		axis = t->axis * axis;
		coord = t->axis * coord;
		coord += t->coord;
		matrix_dirty = true;

		dirty = true;
	}

	void Transformer::relate(Transformer *t)
	{
		coord -= t->coord;
		axis *= glm::transpose(t->axis);
		matrix_dirty = true;

		dirty = true;
	}

	void Transformer::update_matrix()
	{
		if (axis_dirty)
			update_axis();
		matrix = glm::translate(coord) * glm::mat4(axis) * glm::scale(scale);
		matrix_dirty = false;
	}

	void Transformer::update_axis()
	{
		if (!quat_dirty)
			axis = quaternion_to_mat3(quat);// update by quat
		else
			axis = euler_to_mat3(euler);// update by euler
		axis_dirty = false;
	}

	void Transformer::update_euler()
	{
		if (quat_dirty)
			update_quat(); // updata by quat

		float yaw, pitch, roll;

		auto sqw = quat.w * quat.w;
		auto sqx = quat.x * quat.x;
		auto sqy = quat.y * quat.y;
		auto sqz = quat.z * quat.z;

		auto unit = sqx + sqy + sqz + sqw; // if normalised is one, otherwise is correction factor
		auto test = quat.x * quat.y + quat.z * quat.w;
		if (test > 0.499f * unit)
		{ // singularity at north pole
			yaw = 2.f * atan2(quat.x, quat.w);
			pitch = M_PI / 2.f;
			roll = 0;
			return;
		}
		if (test < -0.499f * unit)
		{ // singularity at south pole
			yaw = -2.f * atan2(quat.x, quat.w);
			pitch = -M_PI / 2.f;
			roll = 0;
			return;
		}

		yaw = atan2(2.f * quat.y * quat.w - 2.f * quat.x * quat.z, sqx - sqy - sqz + sqw);
		pitch = asin(2.f * test / unit);
		roll = atan2(2.f * quat.x * quat.w - 2.f * quat.y * quat.z, -sqx + sqy - sqz + sqw);

		euler.x = glm::degrees(yaw);
		euler.y = glm::degrees(pitch);
		euler.z = glm::degrees(roll);
		euler_dirty = false;
	}

	void Transformer::update_quat()
	{
		if (axis_dirty)
			update_axis(); // update by axis
		quat = mat3_to_quaternion(axis);
		quat_dirty = false;
	}

	void Transformer::mark_coord_setted()
	{
		matrix_dirty = true;
		dirty = true;
	}

	void Transformer::mark_euler_setted()
	{
		axis_dirty = true;
		euler_dirty = false;
		quat_dirty = true;
		matrix_dirty = true;
		dirty = true;
	}

	void Transformer::mark_quat_setted()
	{
		axis_dirty = true;
		euler_dirty = true;
		quat_dirty = false;
		matrix_dirty = true;
		dirty = true;
	}

	void Transformer::mark_axis_setted()
	{
		axis_dirty = false;
		euler_dirty = true;
		quat_dirty = true;
		matrix_dirty = true;
		dirty = true;
	}

	void Transformer::mark_scale_setted()
	{
		matrix_dirty = true;
		dirty = true;
	}
}