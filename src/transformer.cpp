#include "transformer.h"

namespace tke
{
	Transformer::Transformer() {}

	Transformer::Transformer(glm::mat3 &rotation, glm::vec3 coord)
	{
		axis = rotation;
		coord = coord;

		needUpdateQuat = true;
		needUpdateEuler = true;
		needUpdateMat = true;
	}

	void Transformer::updateAxis()
	{
		if (!needUpdateQuat)
			quaternionToMatrix(quat, axis);// update by quat
		else
			eulerYzxToMatrix(euler, axis);// update by euler
		needUpdateAxis = false;
	}

	void Transformer::updateEuler()
	{
		if (needUpdateQuat) updateQuat();
		// updata by quat
		float heading, attitude, bank;

		auto sqw = quat.w * quat.w;
		auto sqx = quat.x * quat.x;
		auto sqy = quat.y * quat.y;
		auto sqz = quat.z * quat.z;

		auto unit = sqx + sqy + sqz + sqw; // if normalised is one, otherwise is correction factor
		auto test = quat.x * quat.y + quat.z * quat.w;
		if (test > 0.499f * unit)
		{ // singularity at north pole
			heading = 2.f * atan2(quat.x, quat.w);
			attitude = M_PI / 2.f;
			bank = 0;
			return;
		}
		if (test < -0.499f * unit)
		{ // singularity at south pole
			heading = -2.f * atan2(quat.x, quat.w);
			attitude = -M_PI / 2.f;
			bank = 0;
			return;
		}

		heading = atan2(2.f * quat.y * quat.w - 2.f * quat.x * quat.z, sqx - sqy - sqz + sqw);
		attitude = asin(2.f * test / unit);
		bank = atan2(2.f * quat.x * quat.w - 2.f * quat.y * quat.z, -sqx + sqy - sqz + sqw);

		euler.x = glm::degrees(heading);
		euler.y = glm::degrees(attitude);
		euler.z = glm::degrees(bank);
		needUpdateEuler = false;
	}

	void Transformer::updateQuat()
	{
		if (needUpdateAxis) updateAxis();
		// update by axis
		matrixToQuaternion(axis, quat);
		needUpdateQuat = false;
	}

	void Transformer::updateMat()
	{
		if (needUpdateAxis) updateAxis();
		mat = glm::translate(coord * worldScale) * glm::mat4(axis) * glm::scale(scale * worldScale);
		matInv = glm::inverse(mat);
		needUpdateMat = false;
	}

	glm::vec3 Transformer::getCoord() const
	{
		return coord;
	}

	glm::mat3 Transformer::getAxis()
	{
		if (needUpdateAxis) updateAxis();
		return axis;
	}

	glm::vec3 Transformer::getScale() const
	{
		return scale;
	}

	glm::vec3 Transformer::getWorldScale() const
	{
		return worldScale;
	}

	glm::vec3 Transformer::getEuler()
	{
		if (needUpdateEuler) updateEuler(); // Y -> Z -> X
		return euler;
	}

	glm::vec4 Transformer::getQuat()
	{
		if (needUpdateQuat)
			updateQuat();
		return quat;
	}

	glm::mat4 Transformer::getMat()
	{
		if (needUpdateMat) updateMat();
		return mat;
	}

	glm::mat4 Transformer::getMatInv()
	{
		if (needUpdateMat) updateMat();
		return matInv;
	}

	glm::vec3 Transformer::getTrans(Transformer::Type type)
	{
		switch (type)
		{
		case Type::eMove:
			return getCoord();
		case Type::eEulerSet:
			return getEuler();
		case Type::eScale:
			return getScale();
		}
		return glm::vec3(0);
	}

	void Transformer::setCoord(const glm::vec3 &_coord)
	{
		coord = _coord;
		needUpdateMat = true;
		changed = true;
	}

	void Transformer::addCoord(const glm::vec3 &_coord)
	{
		setCoord(coord + _coord);
	}

	void Transformer::setScale(const glm::vec3 &_scale)
	{
		scale = _scale;
		needUpdateMat = true;

		changed = true;
	}

	void Transformer::addScale(const glm::vec3 &_scale)
	{
		setScale(scale + _scale);
	}

	void Transformer::setWorldScale(const glm::vec3 &_scale)
	{
		worldScale = _scale;
		needUpdateMat = true;

		changed = true;
	}

	void Transformer::setEuler(const glm::vec3 &_euler)
	{
		euler = glm::mod(_euler, 360.f);

		needUpdateAxis = true;
		needUpdateEuler = false;
		needUpdateQuat = true;
		needUpdateMat = true;

		changed = true;
	}

	void Transformer::addEuler(const glm::vec3 &_euler)
	{
		setEuler(getEuler() + _euler);
	}

	void Transformer::setQuat(const glm::vec4 &_quat)
	{
		quat = _quat;
		needUpdateAxis = true;
		needUpdateEuler = true;
		needUpdateQuat = false;
		needUpdateMat = true;

		changed = true;
	}

	void Transformer::axisRotate(Axis which, float angle)
	{
		using namespace glm;
		switch (which)
		{
		case Axis::eX:
		{
			auto m = mat3(rotate(angle, axis[0]));
			axis[1] = normalize(m * axis[1]);
			axis[2] = normalize(m * axis[2]);
		}
		break;
		case Axis::eY:
		{
			auto m = mat3(rotate(angle, axis[1]));
			axis[0] = normalize(m * axis[0]);
			axis[2] = normalize(m * axis[2]);
		}
		break;
		case Axis::eZ:
		{
			auto m = mat3(rotate(angle, axis[2]));
			axis[1] = normalize(m * axis[1]);
			axis[0] = normalize(m * axis[0]);
		}
		break;
		}

		needUpdateAxis = false;
		needUpdateEuler = true;
		needUpdateQuat = true;
		needUpdateMat = true;

		changed = true;
	}

	void Transformer::leftRotate(const glm::mat3 &left)
	{
		axis = left * axis;

		needUpdateAxis = false;
		needUpdateEuler = true;
		needUpdateQuat = true;
		needUpdateMat = true;

		changed = true;
	}

	void Transformer::rightRotate(const glm::mat3 &right)
	{
		axis = axis * right;

		needUpdateAxis = false;
		needUpdateEuler = true;
		needUpdateQuat = true;
		needUpdateMat = true;

		changed = true;
	}

	void Transformer::scaleRelate(Transformer *t)
	{
		coord *= t->scale;
		coord *= t->scale;
		axis = t->axis * axis;
		coord = t->axis * coord;
		coord += t->coord;
		needUpdateMat = true;

		changed = true;
	}

	void Transformer::relate(Transformer *t)
	{
		coord -= t->coord;
		axis *= glm::transpose(t->axis);
		needUpdateMat = true;

		changed = true;
	}
}