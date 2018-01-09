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

	void Transformer::updateAxis()
	{
		if (!quat_dirty)
			axis = quaternion_to_mat3(quat);// update by quat
		else
			axis = euler_to_mat3(euler);// update by euler
		axis_dirty = false;
	}

	void Transformer::updateEuler()
	{
		if (quat_dirty) 
			updateQuat(); // updata by quat
		
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

	void Transformer::updateQuat()
	{
		if (axis_dirty) 
			updateAxis(); // update by axis
		quat = mat3_to_quaternion(axis);
		quat_dirty = false;
	}

	void Transformer::updateMat()
	{
		if (axis_dirty)
			updateAxis();
		matrix = glm::translate(coord) * glm::mat4(axis) * glm::scale(scale);
		matrix_dirty = false;
	}

	glm::vec3 Transformer::get_coord() const
	{
		return coord;
	}

	glm::mat3 Transformer::get_axis()
	{
		if (axis_dirty) 
			updateAxis();
		return axis;
	}

	glm::vec3 Transformer::get_scale() const
	{
		return scale;
	}

	glm::vec3 Transformer::get_euler()
	{
		if (euler_dirty) 
			updateEuler(); // Y -> Z -> X
		return euler;
	}

	glm::vec4 Transformer::get_quat()
	{
		if (quat_dirty)
			updateQuat();
		return quat;
	}

	glm::mat4 Transformer::get_matrix()
	{
		if (matrix_dirty) 
			updateMat();
		return matrix;
	}

	void Transformer::set_coord(const glm::vec3 &_coord)
	{
		coord = _coord;
		matrix_dirty = true;
		dirty = true;
	}

	void Transformer::addCoord(const glm::vec3 &_coord)
	{
		set_coord(coord + _coord);
	}

	void Transformer::setEuler(const glm::vec3 &_euler)
	{
		euler = glm::mod(_euler, 360.f);

		axis_dirty = true;
		euler_dirty = false;
		quat_dirty = true;
		matrix_dirty = true;

		dirty = true;
	}

	void Transformer::addEuler(const glm::vec3 &_euler)
	{
		setEuler(get_euler() + _euler);
	}

	void Transformer::setQuat(const glm::vec4 &_quat)
	{
		quat = _quat;
		axis_dirty = true;
		euler_dirty = true;
		quat_dirty = false;
		matrix_dirty = true;

		dirty = true;
	}

	void Transformer::axisRotate(Axis which, float angle)
	{
		switch (which)
		{
		case AxisX:
		{
			auto m = glm::mat3(glm::rotate(glm::radians(angle), axis[0]));
			axis[1] = glm::normalize(m * axis[1]);
			axis[2] = glm::normalize(m * axis[2]);
		}
			break;
		case AxisY:
		{
			auto m = glm::mat3(glm::rotate(glm::radians(angle), axis[1]));
			axis[0] = glm::normalize(m * axis[0]);
			axis[2] = glm::normalize(m * axis[2]);
		}
			break;
		case AxisZ:
		{
			auto m = glm::mat3(glm::rotate(glm::radians(angle), axis[2]));
			axis[1] = glm::normalize(m * axis[1]);
			axis[0] = glm::normalize(m * axis[0]);
		}
			break;
		}

		axis_dirty = false;
		euler_dirty = true;
		quat_dirty = true;
		matrix_dirty = true;

		dirty = true;
	}

	void Transformer::leftRotate(const glm::mat3 &left)
	{
		axis = left * axis;

		axis_dirty = false;
		euler_dirty = true;
		quat_dirty = true;
		matrix_dirty = true;

		dirty = true;
	}

	void Transformer::rightRotate(const glm::mat3 &right)
	{
		axis = axis * right;

		axis_dirty = false;
		euler_dirty = true;
		quat_dirty = true;
		matrix_dirty = true;

		dirty = true;
	}

	void Transformer::setScale(const glm::vec3 &_scale)
	{
		scale = _scale;
		matrix_dirty = true;

		dirty = true;
	}

	void Transformer::addScale(const glm::vec3 &_scale)
	{
		setScale(scale + _scale);
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
}