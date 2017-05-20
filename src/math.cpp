#include "math.h"

namespace tke
{
	void quaternionToMatrix(glm::vec4 &q, glm::mat3 &mat)
	{
		float wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;

		x2 = 2.f * q.x;
		y2 = 2.f * q.y;
		z2 = 2.f * q.z;

		xx = q.x * x2;
		xy = q.x * y2;
		xz = q.x * z2;

		yy = q.y * y2;
		yz = q.y * z2;
		zz = q.z * z2;

		wx = q.w * x2;
		wy = q.w * y2;
		wz = q.w * z2;

		mat[0][0] = 1.f - (yy + zz);
		mat[1][0] = xy - wz;
		mat[2][0] = xz + wy;

		mat[0][1] = xy + wz;
		mat[1][1] = 1.f - (xx + zz);
		mat[2][1] = yz - wx;

		mat[0][2] = xz - wy;
		mat[1][2] = yz + wx;
		mat[2][2] = 1.f - (xx + yy);
	}

	void matrixToQuaternion(glm::mat3 &mat, glm::vec4 &q)
	{
		float s;
		float tq[4];
		int    i, j;
		// Use tq to store the largest trace
		tq[0] = 1.f + mat[0][0] + mat[1][1] + mat[2][2];
		tq[1] = 1.f + mat[0][0] - mat[1][1] - mat[2][2];
		tq[2] = 1.f - mat[0][0] + mat[1][1] - mat[2][2];
		tq[3] = 1.f - mat[0][0] - mat[1][1] + mat[2][2];
		// Find the maximum (could also use stacked if's later)
		j = 0;
		for (i = 1; i < 4; i++)
		{
			j = (tq[i] > tq[j]) ? i : j;
		}
		// check the diagonal
		if (j == 0)
		{
			/* perform instant calculation */
			q.w = tq[0];
			q.x = mat[1][2] - mat[2][1];
			q.y = mat[2][0] - mat[0][2];
			q.z = mat[0][1] - mat[1][0];
		}
		else if (j == 1)
		{
			q.w = mat[1][2] - mat[2][1];
			q.x = tq[1];
			q.y = mat[0][1] + mat[1][0];
			q.z = mat[2][0] + mat[0][2];
		}
		else if (j == 2)
		{
			q.w = mat[2][0] - mat[0][2];
			q.x = mat[0][1] + mat[1][0];
			q.y = tq[2];
			q.z = mat[1][2] + mat[2][1];
		}
		else /* if (j==3) */
		{
			q.w = mat[0][1] - mat[1][0];
			q.x = mat[2][0] + mat[0][2];
			q.y = mat[1][2] + mat[2][1];
			q.z = tq[3];
		}
		s = sqrt(0.25f / tq[j]);
		q.w *= s;
		q.x *= s;
		q.y *= s;
		q.z *= s;
		q = glm::normalize(q);
	}

	void quaternionRotate(glm::vec4 &q, glm::vec3 v)
	{
		auto s = -q.x * v.x - q.y * v.y - q.z * v.z;
		auto i = q.w * v.x + q.y * v.z - q.z * v.y;
		auto j = q.w * v.y + q.z * v.x - q.x * v.z;
		auto k = q.w * v.z + q.x * v.y - q.y * v.x;

		auto w = q.s;
		auto x = -q.x;
		auto y = -q.y;
		auto z = -q.z;

		v.x = s * x + j * z - k * y;
		v.y = s * y + k * x - i * z;
		v.z = s * z + i * y - j * x;
	}

	void eulerYzxToMatrix(glm::vec3 &e, glm::mat3 &mat)
	{
		using namespace glm;
		auto x = vec3(1.f, 0.f, 0.f), y = vec3(0.f, 1.f, 0.f), z = vec3(0.f, 0.f, 1.f);
		auto matYaw = mat3(glm::rotate(e.x, y));
		x = matYaw * x;
		z = matYaw * z;
		auto matPitch = mat3(glm::rotate(e.y, z));
		x = matPitch * x;
		y = matPitch * y;
		auto matRoll = mat3(glm::rotate(e.z, x));
		y = matRoll * y;
		z = matRoll * z;
		mat[0] = x;
		mat[1] = y;
		mat[2] = z;
	}

	void eulerYxzToMatrix(glm::vec3 &e, glm::mat3 &mat)
	{
		using namespace glm;
		auto x = vec3(1.f, 0.f, 0.f), y = vec3(0.f, 1.f, 0.f), z = vec3(0.f, 0.f, 1.f);
		auto matYaw = mat3(glm::rotate(e.x, y));
		x = matYaw * x;
		z = matYaw * z;
		auto matPitch = mat3(glm::rotate(e.y, x));
		z = matPitch * z;
		y = matPitch * y;
		auto matRoll = mat3(glm::rotate(e.z, z));
		y = matRoll * y;
		x = matRoll * x;
		mat[0] = x;
		mat[1] = y;
		mat[2] = z;
	}

	glm::mat4 makeMatrix(glm::mat3 &rotation, glm::vec3 coord)
	{
		return glm::mat4(glm::vec4(rotation[0], 0.f), glm::vec4(rotation[1], 0.f), glm::vec4(rotation[2], 0.f), glm::vec4(coord, 1.f));
	}

	float rand2d(glm::vec2 v)
	{
		return glm::fract(cos(v.x * (12.9898) + v.y * (4.1414)) * 43758.5453);
	}

	float noise2d(glm::vec2 v)
	{
		const auto SC = 250;

		v /= SC;
		auto vf = glm::fract(v);
		auto vi = glm::floor(v);

		auto r0 = rand2d(vi);
		auto r1 = rand2d(vi + glm::vec2(1.f, 0.f));
		auto r2 = rand2d(vi + glm::vec2(0.f, 1.f));
		auto r3 = rand2d(vi + glm::vec2(1.f, 1.f));

		auto vs = 3.f * vf * vf - 2.f * vf * vf * vf;

		return glm::mix(glm::mix(r0, r1, vs.x),
			glm::mix(r2, r3, vs.x),
			vs.y);
	}

	float fbm2d(glm::vec2 v)
	{
		auto r = 0.f;

		auto a = 1.f / 3.f;
		for (auto i = 0; i < 4; i++)
		{
			r += noise2d(v) * a;
			a /= 3.f;
			v *= 3.f;
		}

		return r;
	}
}