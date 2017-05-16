#include "camera.h"
#include "core.h"
#include "utils.h"

namespace tke
{
	Camera::Camera()
	{
		baseForwardAng = 90.f;
	}

	void Camera::setMode(Mode mode)
	{
		m_mode = mode;
		m_needUpdateMat = true;
		m_changed = true;
	}

	void Camera::setLength(float length)
	{
		m_length = length;
		m_needUpdateMat = true;
		m_changed = true;
	}

	void Camera::setTarget(glm::vec3 target)
	{
		m_target = target;
		m_needUpdateMat = true;
		m_changed = true;
	}

	void Camera::lookAtTarget()
	{
		if (m_mode == Mode::eTargeting)
		{
			if (m_needUpdateAxis) updateAxis();
			m_coord = m_target + m_axis[2] * m_length;
			m_needUpdateMat = true;
			m_changed = true;
		}
	}


	void Camera::updateFrustum(float aspect, glm::mat4 &matProj)
	{
		auto tanHfFovy = glm::tan(glm::radians(TKE_FOVY * 0.5f));

		auto _y1 = TKE_NEAR * tanHfFovy;
		auto _z1 = _y1 * aspect;
		auto _y2 = TKE_FAR * tanHfFovy;
		auto _z2 = _y2 * aspect;
		m_frustumPoints[0] = -_z1 * m_axis[2] + _y1 * m_axis[1] + TKE_NEAR * m_axis[0] + m_coord;
		m_frustumPoints[1] = _z1 * m_axis[2] + _y1 * m_axis[1] + TKE_NEAR * m_axis[0] + m_coord;
		m_frustumPoints[2] = _z1 * m_axis[2] + -_y1 * m_axis[1] + TKE_NEAR * m_axis[0] + m_coord;
		m_frustumPoints[3] = -_z1 * m_axis[2] + -_y1 * m_axis[1] + TKE_NEAR * m_axis[0] + m_coord;
		m_frustumPoints[4] = -_z2 * m_axis[2] + _y2 * m_axis[1] + TKE_FAR * m_axis[0] + m_coord;
		m_frustumPoints[5] = _z2 * m_axis[2] + _y2 * m_axis[1] + TKE_FAR * m_axis[0] + m_coord;
		m_frustumPoints[6] = _z2 * m_axis[2] + -_y2 * m_axis[1] + TKE_FAR * m_axis[0] + m_coord;
		m_frustumPoints[7] = -_z2 * m_axis[2] + -_y2 * m_axis[1] + TKE_FAR * m_axis[0] + m_coord;
		for (int i = 0; i < 4; i++)
		{
			auto y = m_frustumPoints[i + 4].y;
			if (y < 0.f)
			{
				auto py = m_frustumPoints[i + 4].y - m_frustumPoints[i].y;
				if (py != 0.f)
				{
					m_frustumPoints[i + 4].x -= y * ((m_frustumPoints[i + 4].x - m_frustumPoints[i].x) / py);
					m_frustumPoints[i + 4].z -= y * ((m_frustumPoints[i + 4].z - m_frustumPoints[i].z) / py);
					m_frustumPoints[i + 4].y = 0.f;
				}
			}
		}

		auto matrix = matProj * m_mat;

		m_frustumPlanes[0].x = matrix[0].w + matrix[0].x;
		m_frustumPlanes[0].y = matrix[1].w + matrix[1].x;
		m_frustumPlanes[0].z = matrix[2].w + matrix[2].x;
		m_frustumPlanes[0].w = matrix[3].w + matrix[3].x;

		m_frustumPlanes[1].x = matrix[0].w - matrix[0].x;
		m_frustumPlanes[1].y = matrix[1].w - matrix[1].x;
		m_frustumPlanes[1].z = matrix[2].w - matrix[2].x;
		m_frustumPlanes[1].w = matrix[3].w - matrix[3].x;

		m_frustumPlanes[2].x = matrix[0].w - matrix[0].y;
		m_frustumPlanes[2].y = matrix[1].w - matrix[1].y;
		m_frustumPlanes[2].z = matrix[2].w - matrix[2].y;
		m_frustumPlanes[2].w = matrix[3].w - matrix[3].y;

		m_frustumPlanes[3].x = matrix[0].w + matrix[0].y;
		m_frustumPlanes[3].y = matrix[1].w + matrix[1].y;
		m_frustumPlanes[3].z = matrix[2].w + matrix[2].y;
		m_frustumPlanes[3].w = matrix[3].w + matrix[3].y;

		m_frustumPlanes[4].x = matrix[0].w + matrix[0].z;
		m_frustumPlanes[4].y = matrix[1].w + matrix[1].z;
		m_frustumPlanes[4].z = matrix[2].w + matrix[2].z;
		m_frustumPlanes[4].w = matrix[3].w + matrix[3].z;

		m_frustumPlanes[5].x = matrix[0].w - matrix[0].z;
		m_frustumPlanes[5].y = matrix[1].w - matrix[1].z;
		m_frustumPlanes[5].z = matrix[2].w - matrix[2].z;
		m_frustumPlanes[5].w = matrix[3].w - matrix[3].z;

		for (auto i = 0; i < 6; i++) m_frustumPlanes[i] = glm::normalize(m_frustumPlanes[i]);
	}

	void Camera::reset()
	{
		reset();
		m_coord = glm::vec3(0.f);
		m_length = 1.0f;
		m_needUpdateMat = true;
		m_changed = true;
	}

	void Camera::addAngAccrodingToScreen(float x, float y)
	{
		addEuler(glm::vec3(-x * 180.f, 0.f, -y * 180.f));
	}

	void Camera::scroll(short value)
	{
		if (m_mode == Mode::eTargeting)
		{
			if (value < 0)
				m_length = (m_length + 0.1) * 1.1f;
			else
				m_length = (m_length / 1.1f) - 0.1f;
			if (m_length < 1.f)
			{
				m_length = 1.f;
				m_coord += glm::normalize(m_target - m_coord) * 0.5f;
			}
			m_needUpdateMat = true;
			m_changed = true;
		}
	}

	void Camera::moveAccrodingToScreen(float aspect, float x, float y)
	{
		auto l = m_length / TKE_NEAR;
		auto cy = tan(glm::radians(TKE_FOVY / 2.f)) * TKE_NEAR * 2.f;
		m_target += (-x * cy * aspect * l) * m_axis[0] + (y * cy * l) * m_axis[1];
		lookAtTarget();
	}

	void Camera::move()
	{
		glm::vec3 coord;
		float euler;
		if (!Controller::move(getEuler().x, coord, euler))
			return;
		if (m_mode == Mode::eFree)
			addCoord(coord);
		else if (m_mode == Mode::eTargeting)
			setTarget(m_target + coord);
		addEuler(glm::vec3(euler, 0.f, 0.f));
	}

	void Camera::load(std::ifstream &file)
	{
		glm::vec3 coord;
		file >> coord;
		setCoord(coord);
		glm::vec3 euler;
		file >> euler;
		setEuler(euler);
		glm::vec3 scale;
		file >> scale;
		setScale(scale);
		int mode;
		file >> mode;
		setMode((Mode)mode);
		float length;
		file >> length;
		setLength(length);
	}

	void Camera::save(std::ofstream &file)
	{
		file << getCoord();
		file << getEuler();
		file << getScale();
		int mode = (int)m_mode;
		file << mode;
		file << m_length;
	}
}