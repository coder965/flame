#include <flame/common/math.h>

namespace flame
{
	int digit_num(int a)
	{
		auto d = 0;
		do
		{
			d++;
			a /= 10;
		} while (a);
		return d;
	}

	float linear_depth_ortho(float z, float depth_near, float depth_far)
	{
		z = z * 0.5 + 0.5;
		return z * (depth_far - depth_near) + depth_near;
	}

	float linear_depth_perspective(float z, float depth_near, float depth_far)
	{
		float a = (1.0 - depth_far / depth_near) * 0.5 / depth_far;
		float b = (1.0 + depth_far / depth_near) * 0.5 / depth_far;
		return 1.0 / (a * z + b);
	}

	glm::vec3 transform(const glm::vec3 &v, const glm::mat4 &mat)
	{
		auto v_ = mat * glm::vec4(v, 1.f);
		v_ /= v_.w;
		return v_;
	}

	glm::vec4 plane(const glm::vec3 &p, const glm::vec3 &normal)
	{
		return glm::vec4(normal, glm::dot(normal, p));
	}

	void ortho_normalize(glm::mat3 &mat)
	{
		for (auto i = 0; i < 3; i++)
			mat[i] = glm::normalize(mat[i]);
	}

	void ortho_normalize(glm::mat4 &mat)
	{
		for (auto i = 0; i < 3; i++)
			mat[i] = glm::vec4(glm::normalize(glm::vec3(mat[i])), 0.f);
	}

	float ray_intersect_plane(const glm::vec3 &origin, const glm::vec3 &vector, const glm::vec4 &plane)
	{
		auto numer = glm::dot(glm::vec3(plane), origin) - plane.w;
		auto denom = glm::dot(glm::vec3(plane), vector);

		if (glm::abs(denom) < FLT_EPSILON)
			return -1.0f;

		return -(numer / denom);
	}

	glm::mat3 quaternion_to_mat3(glm::vec4 &q)
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

		glm::mat3 mat;
		mat[0][0] = 1.f - (yy + zz);
		mat[1][0] = xy - wz;
		mat[2][0] = xz + wy;

		mat[0][1] = xy + wz;
		mat[1][1] = 1.f - (xx + zz);
		mat[2][1] = yz - wx;

		mat[0][2] = xz - wy;
		mat[1][2] = yz + wx;
		mat[2][2] = 1.f - (xx + yy);
		return mat;
	}

	glm::vec4 mat3_to_quaternion(glm::mat3 &mat)
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
		glm::vec4 q;
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
		return q;
	}

	glm::vec3 quaternion_to_euler(glm::vec4 &q)
	{
		float yaw, pitch, roll;

		auto sqw = q.w * q.w;
		auto sqx = q.x * q.x;
		auto sqy = q.y * q.y;
		auto sqz = q.z * q.z;

		auto unit = sqx + sqy + sqz + sqw; // if normalised is one, otherwise is correction factor
		auto test = q.x * q.y + q.z * q.w;
		if (test > 0.499f * unit)
		{ // singularity at north pole
			yaw = 2.f * atan2(q.x, q.w);
			pitch = M_PI / 2.f;
			roll = 0;
			return glm::vec3(0.f);
		}
		if (test < -0.499f * unit)
		{ // singularity at south pole
			yaw = -2.f * atan2(q.x, q.w);
			pitch = -M_PI / 2.f;
			roll = 0;
			return glm::vec3(0.f);
		}

		yaw = atan2(2.f * q.y * q.w - 2.f * q.x * q.z, sqx - sqy - sqz + sqw);
		pitch = asin(2.f * test / unit);
		roll = atan2(2.f * q.x * q.w - 2.f * q.y * q.z, -sqx + sqy - sqz + sqw);

		return glm::vec3(
			glm::degrees(yaw),
			glm::degrees(pitch),
			glm::degrees(roll)
		);
	}

	void quaternion_rotate(glm::vec4 &q, glm::vec3 &v)
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

	glm::mat3 euler_to_mat3(float x, float y, float z)
	{
		return euler_to_mat3(glm::vec3(x, y, z));
	}

	glm::mat3 euler_to_mat3(glm::vec3 &e)
	{
		using namespace glm;
		auto x = vec3(1.f, 0.f, 0.f), y = vec3(0.f, 1.f, 0.f), z = vec3(0.f, 0.f, 1.f);
		auto matYaw = mat3(glm::rotate(glm::radians(e.x), y));
		x = matYaw * x;
		z = matYaw * z;
		auto matPitch = mat3(glm::rotate(glm::radians(e.y), x));
		z = matPitch * z;
		y = matPitch * y;
		auto matRoll = mat3(glm::rotate(glm::radians(e.z), z));
		y = matRoll * y;
		x = matRoll * x;
		return glm::mat3(x, y, z);
	}

	glm::mat4 make_matrix(const glm::mat3 &rotation, const glm::vec3 coord)
	{
		return glm::mat4(
			glm::vec4(rotation[0], 0.f), 
			glm::vec4(rotation[1], 0.f), 
			glm::vec4(rotation[2], 0.f), 
			glm::vec4(coord, 1.f)
		);
	}

	glm::mat4 make_matrix(const glm::vec3 &x, const glm::vec3 &y, const glm::vec3 coord)
	{
		return glm::mat4(
			glm::vec4(x, 0.f),
			glm::vec4(y, 0.f),
			glm::vec4(glm::cross(x, y), 0.f),
			glm::vec4(coord, 1.f)
		);
	}

	float rand2d(const glm::vec2 &v)
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

	glm::vec4 fit_rect(const glm::vec2 &desired_size, float xy_aspect)
	{
		if (desired_size.x <= 0.f || desired_size.y <= 0.f)
			return glm::vec4(0.f, 0.f, 1.f, 1.f);
		glm::vec4 ret;
		if (desired_size.x / desired_size.y > xy_aspect)
		{
			ret.z = xy_aspect * desired_size.y;
			ret.w = desired_size.y;
			ret.x = (desired_size.x - ret.z) * 0.5f;
			ret.y = 0;
		}
		else
		{
			ret.z = desired_size.x;
			ret.w = ret.z / xy_aspect;
			ret.x = 0;
			ret.y = (desired_size.y - ret.w) * 0.5f;
		}
		return ret;
	}

	glm::vec4 fit_rect_no_zoom_in(const glm::vec2 &desired_size, const glm::vec2 &size)
	{
		if (desired_size.x <= 0.f || desired_size.y <= 0.f)
			return glm::vec4(0.f, 0.f, 1.f, 1.f);
		if (size.x <= desired_size.x && size.y <= desired_size.y)
		{
			glm::vec4 ret;
			ret.z = size.x;
			ret.w = size.y;
			ret.x = (desired_size.x - ret.z) * 0.5f;
			ret.y = (desired_size.y - ret.w) * 0.5f;
			return ret;
		}
		else
			return fit_rect(desired_size, size.x / size.y);
	}
}