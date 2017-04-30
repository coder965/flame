#include "transformer.h"

namespace tke
{
	Transformer::Transformer() {}

	Transformer::Transformer(glm::mat3 &rotation, glm::vec3 coord)
	{
		m_axis = rotation;
		m_coord = coord;

		m_needUpdateQuat = true;
		m_needUpdateEuler = true;
		m_needUpdateMat = true;
	}

	void Transformer::updateAxis()
	{
		if (!m_needUpdateQuat)
			quaternionToMatrix(m_quat, m_axis);// update by quat
		else
			eulerYzxToMatrix(m_euler, m_axis);// update by euler
		m_needUpdateAxis = false;
	}

	void Transformer::updateEuler()
	{
		if (m_needUpdateQuat) updateQuat();
		// updata by quat
		float heading, attitude, bank;

		auto sqw = m_quat.w * m_quat.w;
		auto sqx = m_quat.x * m_quat.x;
		auto sqy = m_quat.y * m_quat.y;
		auto sqz = m_quat.z * m_quat.z;

		auto unit = sqx + sqy + sqz + sqw; // if normalised is one, otherwise is correction factor
		auto test = m_quat.x * m_quat.y + m_quat.z * m_quat.w;
		if (test > 0.499f * unit)
		{ // singularity at north pole
			heading = 2.f * atan2(m_quat.x, m_quat.w);
			attitude = PI / 2.f;
			bank = 0;
			return;
		}
		if (test < -0.499f * unit)
		{ // singularity at south pole
			heading = -2.f * atan2(m_quat.x, m_quat.w);
			attitude = -PI / 2.f;
			bank = 0;
			return;
		}

		heading = atan2(2.f * m_quat.y * m_quat.w - 2.f * m_quat.x * m_quat.z, sqx - sqy - sqz + sqw);
		attitude = asin(2.f * test / unit);
		bank = atan2(2.f * m_quat.x * m_quat.w - 2.f * m_quat.y * m_quat.z, -sqx + sqy - sqz + sqw);

		m_euler.x = glm::degrees(heading);
		m_euler.y = glm::degrees(attitude);
		m_euler.z = glm::degrees(bank);
		m_needUpdateEuler = false;
	}

	void Transformer::updateQuat()
	{
		if (m_needUpdateAxis) updateAxis();
		// update by axis
		matrixToQuaternion(m_axis, m_quat);
		m_needUpdateQuat = false;
	}

	void Transformer::updateMat()
	{
		if (m_needUpdateAxis) updateAxis();
		m_mat = glm::translate(m_coord * m_worldScale) * glm::mat4(m_axis) * glm::scale(m_scale * m_worldScale);
		m_matInv = glm::inverse(m_mat);
		m_needUpdateMat = false;
	}

	glm::vec3 Transformer::getCoord() const
	{
		return m_coord;
	}

	glm::mat3 Transformer::getAxis()
	{
		if (m_needUpdateAxis) updateAxis();
		return m_axis;
	}

	glm::vec3 Transformer::getScale() const
	{
		return m_scale;
	}

	glm::vec3 Transformer::getWorldScale() const
	{
		return m_worldScale;
	}

	glm::vec3 Transformer::getEuler()
	{
		if (m_needUpdateEuler) updateEuler(); // Y -> Z -> X
		return m_euler;
	}

	glm::vec4 Transformer::getQuat()
	{
		if (m_needUpdateQuat)
			updateQuat();
		return m_quat;
	}

	glm::mat4 Transformer::getMat()
	{
		if (m_needUpdateMat) updateMat();
		return m_mat;
	}

	glm::mat4 Transformer::getMatInv()
	{
		if (m_needUpdateMat) updateMat();
		return m_matInv;
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

	void Transformer::setCoord(const glm::vec3 &coord)
	{
		m_coord = coord;
		m_needUpdateMat = true;
		m_changed = true;
	}

	void Transformer::addCoord(const glm::vec3 &coord)
	{
		setCoord(m_coord + coord);
	}

	void Transformer::setScale(const glm::vec3 &scale)
	{
		m_scale = scale;
		m_needUpdateMat = true;

		m_changed = true;
	}

	void Transformer::addScale(const glm::vec3 &scale)
	{
		setScale(m_scale + scale);
	}

	void Transformer::setWorldScale(const glm::vec3 &scale)
	{
		m_worldScale = scale;
		m_needUpdateMat = true;

		m_changed = true;
	}

	void Transformer::setEuler(const glm::vec3 &euler)
	{
		m_euler = glm::mod(euler, 360.f);

		m_needUpdateAxis = true;
		m_needUpdateEuler = false;
		m_needUpdateQuat = true;
		m_needUpdateMat = true;

		m_changed = true;
	}

	void Transformer::addEuler(const glm::vec3 &euler)
	{
		setEuler(getEuler() + euler);
	}

	void Transformer::setQuat(const glm::vec4 &quat)
	{
		m_quat = quat;
		m_needUpdateAxis = true;
		m_needUpdateEuler = true;
		m_needUpdateQuat = false;
		m_needUpdateMat = true;

		m_changed = true;
	}

	void Transformer::axisRotate(Axis which, float angle)
	{
		using namespace glm;
		switch (which)
		{
		case Axis::eX:
		{
			auto m = mat3(rotate(angle, m_axis[0]));
			m_axis[1] = normalize(m * m_axis[1]);
			m_axis[2] = normalize(m * m_axis[2]);
		}
		break;
		case Axis::eY:
		{
			auto m = mat3(rotate(angle, m_axis[1]));
			m_axis[0] = normalize(m * m_axis[0]);
			m_axis[2] = normalize(m * m_axis[2]);
		}
		break;
		case Axis::eZ:
		{
			auto m = mat3(rotate(angle, m_axis[2]));
			m_axis[1] = normalize(m * m_axis[1]);
			m_axis[0] = normalize(m * m_axis[0]);
		}
		break;
		}

		m_needUpdateAxis = false;
		m_needUpdateEuler = true;
		m_needUpdateQuat = true;
		m_needUpdateMat = true;

		m_changed = true;
	}

	void Transformer::leftRotate(glm::mat3 left)
	{
		m_axis = left * m_axis;

		m_needUpdateAxis = false;
		m_needUpdateEuler = true;
		m_needUpdateQuat = true;
		m_needUpdateMat = true;

		m_changed = true;
	}

	void Transformer::rightRotate(glm::mat3 right)
	{
		m_axis = m_axis * right;

		m_needUpdateAxis = false;
		m_needUpdateEuler = true;
		m_needUpdateQuat = true;
		m_needUpdateMat = true;

		m_changed = true;
	}

	void Transformer::scaleRelate(Transformer *t)
	{
		m_coord *= t->m_scale;
		m_coord *= t->m_scale;
		m_axis = t->m_axis * m_axis;
		m_coord = t->m_axis * m_coord;
		m_coord += t->m_coord;
		m_needUpdateMat = true;

		m_changed = true;
	}

	void Transformer::relate(Transformer *t)
	{
		m_coord -= t->m_coord;
		m_axis *= glm::transpose(t->m_axis);
		m_needUpdateMat = true;

		m_changed = true;
	}
}