//MIT License
//
//Copyright (c) 2018 wjs
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#pragma once

#ifdef _FLAME_MATH_EXPORTS
#define FLAME_MATH_EXPORTS __declspec(dllexport)
#else
#define FLAME_MATH_EXPORTS __declspec(dllimport)
#endif

#include <math.h>

//#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtx/transform2.hpp>

namespace flame
{
	// == copy from math.h of C ==
	const float E           = 2.7182818f;  // e
	const float LOG2E       = 1.4426950f;  // log2(e)
	const float LOG10E      = 0.4342944f;  // log10(e)
	const float LN2         = 0.6931471f;  // ln(2)
	const float LN10        = 2.3025850f;  // ln(10)
	const float PI          = 3.1415926f;  // pi
	const float PI_2        = 1.5707963f;  // pi/2
	const float PI_4        = 0.7853981f;  // pi/4
	const float PI_INV      = 0.3183098f;  // 1/pi
	const float PI_INV2     = 0.6366197f;  // 2/pi
	const float SQRTPI_INV2 = 1.1283791f;  // 2/sqrt(pi)
	const float SQRT2       = 1.4142135f;  // sqrt(2)
	const float SQRT1_2     = 0.7071067f;  // 1/sqrt(2)
	// == 

	const float EPS = 0.000001f;

	inline float get_inf()
	{
		glm::mat4 a, b;
		glm::mat2 c(1, 2, 3, 4);
		a /= b;
		a[0][0];
		glm::transpose(a);
		auto zero = 0.f;
		return 1.f / zero;
	}

	inline bool is_inf(float v)
	{
		return (1.f + v) == v;
	}

	inline float abs(float v)
	{
		return v > 0.f ? v : -v;
	}

	inline float min(float a, float b)
	{
		return a < b ? a : b;
	}

	inline float max(float a, float b)
	{
		return a > b ? a : b;
	}

	inline float min(int a, int b)
	{
		return a < b ? a : b;
	}

	inline float max(int a, int b)
	{
		return a > b ? a : b;
	}

	inline float min(float a, int b)
	{
		return a < b ? a : b;
	}

	inline float max(float a, int b)
	{
		return a > b ? a : b;
	}

	inline float min(int a, float b)
	{
		return a < b ? a : b;
	}

	inline float max(int a, float b)
	{
		return a > b ? a : b;
	}

	inline bool equals(float a, float b)
	{
		return abs(a - b) < EPS;
	}

	inline int count_digit(int a)
	{
		auto d = 0;
		do
		{
			d++;
			a /= 10;
		} while (a);
		return d;
	}

	inline float ortho_depth_to_linear(float z, float depth_near, float depth_far)
	{
		z = z * 0.5 + 0.5;
		return z * (depth_far - depth_near) + depth_near;
	}

	inline float perspective_depth_to_linear(float z, float depth_near, float depth_far)
	{
		float a = (1.f - depth_far / depth_near) * 0.5f / depth_far;
		float b = (1.f + depth_far / depth_near) * 0.5f / depth_far;
		return 1.f / (a * z + b);
	}

	struct Vec2;
	struct Vec3;
	struct Vec4;
	struct Ivec2;
	struct Ivec3;
	struct Ivec4;
	struct Mat2;
	struct Mat3;
	struct Mat4;
	struct EulerYawPitchRoll;
	struct Quat;
	struct Rect;
	struct Plane;

	struct Vec2
	{
		float x;
		float y;

		Vec2();

		Vec2(float v);
		Vec2(float _x, float _y);
		Vec2(const Vec2 &v);
		Vec2(const Vec3 &v);
		Vec2(const Vec4 &v);
		Vec2(const Ivec2 &v);
		Vec2(const Ivec3 &v);
		Vec2(const Ivec4 &v);
		float &operator[](int i);
		float const&operator[](int i) const;
		Vec2 &operator=(const Vec2 &v);
		Vec2 &operator=(const Vec3 &v);
		Vec2 &operator=(const Vec4 &v);
		Vec2 &operator=(const Ivec2 &v);
		Vec2 &operator=(const Ivec3 &v);
		Vec2 &operator=(const Ivec4 &v);
		bool operator==(const Vec2 &v);
		bool operator!=(const Vec2 &v);
		Vec2 operator+(const Vec2 &v);
		Vec2 operator-(const Vec2 &v);
		Vec2 operator*(const Vec2 &v);
		Vec2 operator/(const Vec2 &v);
		Vec2 operator+(const Ivec2 &v);
		Vec2 operator-(const Ivec2 &v);
		Vec2 operator*(const Ivec2 &v);
		Vec2 operator/(const Ivec2 &v);
		Vec2 &operator+=(const Vec2 &v);
		Vec2 &operator-=(const Vec2 &v);
		Vec2 &operator*=(const Vec2 &v);
		Vec2 &operator/=(const Vec2 &v);
		Vec2 &operator+=(const Ivec2 &v);
		Vec2 &operator-=(const Ivec2 &v);
		Vec2 &operator*=(const Ivec2 &v);
		Vec2 &operator/=(const Ivec2 &v);
		Vec2 &operator+=(float v);
		Vec2 &operator-=(float v);
		Vec2 &operator*=(float v);
		Vec2 &operator/=(float v);
		float length() const;
		void normalize();
		Vec2 get_normalize() const;
	};

	Vec2 operator+(const Vec2 &lhs, float rhs);
	Vec2 operator-(const Vec2 &lhs, float rhs);
	Vec2 operator*(const Vec2 &lhs, float rhs);
	Vec2 operator/(const Vec2 &lhs, float rhs);
	Vec2 operator+(float lhs, const Vec2 &rhs);
	Vec2 operator-(float lhs, const Vec2 &rhs);
	Vec2 operator*(float lhs, const Vec2 &rhs);
	Vec2 operator/(float lhs, const Vec2 &rhs);

	struct Vec3
	{
		float x;
		float y;
		float z;

		Vec3();
		Vec3(float v);
		Vec3(float _x, float _y, float _z);
		Vec3(const Vec2 &v, float _z);
		Vec3(const Vec3 &v);
		Vec3(const Vec4 &v);
		Vec3(const Ivec2 &v, float _z);
		Vec3(const Ivec3 &v);
		Vec3(const Ivec4 &v);
		float &operator[](int i);
		float const&operator[](int i) const;
		Vec3 &operator=(const Vec3 &v);
		Vec3 &operator=(const Vec4 &v);
		Vec3 &operator=(const Ivec3 &v);
		Vec3 &operator=(const Ivec4 &v);
		bool operator==(const Vec3 &v);
		bool operator!=(const Vec3 &v);
		Vec3 operator+(const Vec3 &v);
		Vec3 operator-(const Vec3 &v);
		Vec3 operator*(const Vec3 &v);
		Vec3 operator/(const Vec3 &v);
		Vec3 operator+(const Ivec3 &v);
		Vec3 operator-(const Ivec3 &v);
		Vec3 operator*(const Ivec3 &v);
		Vec3 operator/(const Ivec3 &v);
		Vec3 &operator+=(const Vec3 &v);
		Vec3 &operator-=(const Vec3 &v);
		Vec3 &operator*=(const Vec3 &v);
		Vec3 &operator/=(const Vec3 &v);
		Vec3 &operator+=(const Ivec3 &v);
		Vec3 &operator-=(const Ivec3 &v);
		Vec3 &operator*=(const Ivec3 &v);
		Vec3 &operator/=(const Ivec3 &v);
		Vec3 &operator+=(float v);
		Vec3 &operator-=(float v);
		Vec3 &operator*=(float v);
		Vec3 &operator/=(float v);
		float length() const;
		void normalize();
		Vec3 get_normalize() const;
	};

	Vec3 operator+(const Vec3 &lhs, float rhs);
	Vec3 operator-(const Vec3 &lhs, float rhs);
	Vec3 operator*(const Vec3 &lhs, float rhs);
	Vec3 operator/(const Vec3 &lhs, float rhs);
	Vec3 operator+(float lhs, const Vec3 &rhs);
	Vec3 operator-(float lhs, const Vec3 &rhs);
	Vec3 operator*(float lhs, const Vec3 &rhs);
	Vec3 operator/(float lhs, const Vec3 &rhs);

	struct Vec4
	{
		float x;
		float y;
		float z;
		float w;

		Vec4();
		Vec4(float v);
		Vec4(float _x, float _y, float _z, float _w);
		Vec4(const Vec2 &v, float _z, float _w);
		Vec4(const Vec3 &v, float _w);
		Vec4(const Vec4 &v);
		Vec4(const Ivec2 &v, float _z, float _w);
		Vec4(const Ivec3 &v, float _w);
		Vec4(const Ivec4 &v);
		float &operator[](int i);
		float const&operator[](int i) const;
		Vec4 &operator=(const Vec4 &v);
		Vec4 &operator=(const Ivec4 &v);
		bool operator==(const Vec4 &v);
		bool operator!=(const Vec4 &v);
		Vec4 operator+(const Vec4 &v);
		Vec4 operator-(const Vec4 &v);
		Vec4 operator*(const Vec4 &v);
		Vec4 operator/(const Vec4 &v);
		Vec4 operator+(const Ivec4 &v);
		Vec4 operator-(const Ivec4 &v);
		Vec4 operator*(const Ivec4 &v);
		Vec4 operator/(const Ivec4 &v);
		Vec4 &operator+=(const Vec4 &v);
		Vec4 &operator-=(const Vec4 &v);
		Vec4 &operator*=(const Vec4 &v);
		Vec4 &operator/=(const Vec4 &v);
		Vec4 &operator+=(const Ivec4 &v);
		Vec4 &operator-=(const Ivec4 &v);
		Vec4 &operator*=(const Ivec4 &v);
		Vec4 &operator/=(const Ivec4 &v);
		Vec4 &operator+=(float v);
		Vec4 &operator-=(float v);
		Vec4 &operator*=(float v);
		Vec4 &operator/=(float v);
		float length() const;
		void normalize();
		Vec4 get_normalize() const;
	};

	Vec4 operator+(const Vec4 &lhs, float rhs);
	Vec4 operator-(const Vec4 &lhs, float rhs);
	Vec4 operator*(const Vec4 &lhs, float rhs);
	Vec4 operator/(const Vec4 &lhs, float rhs);
	Vec4 operator+(float lhs, const Vec4 &rhs);
	Vec4 operator-(float lhs, const Vec4 &rhs);
	Vec4 operator*(float lhs, const Vec4 &rhs);
	Vec4 operator/(float lhs, const Vec4 &rhs);

	struct Ivec2
	{
		int x;
		int y;

		Ivec2();
		Ivec2(int v);
		Ivec2(int _x, int _y);
		Ivec2(const Vec2 &v);
		Ivec2(const Vec3 &v);
		Ivec2(const Vec4 &v);
		Ivec2(const Ivec2 &v);
		Ivec2(const Ivec3 &v);
		Ivec2(const Ivec4 &v);
		int &operator[](int i);
		int const&operator[](int i) const;
		Ivec2 &operator=(const Vec2 &v);
		Ivec2 &operator=(const Vec3 &v);
		Ivec2 &operator=(const Vec4 &v);
		Ivec2 &operator=(const Ivec2 &v);
		Ivec2 &operator=(const Ivec3 &v);
		Ivec2 &operator=(const Ivec4 &v);
		bool operator==(const Ivec2 &v);
		bool operator!=(const Ivec2 &v);
		Ivec2 operator+(const Vec2 &v);
		Ivec2 operator-(const Vec2 &v);
		Ivec2 operator*(const Vec2 &v);
		Ivec2 operator/(const Vec2 &v);
		Ivec2 operator+(const Ivec2 &v);
		Ivec2 operator-(const Ivec2 &v);
		Ivec2 operator*(const Ivec2 &v);
		Ivec2 operator/(const Ivec2 &v);
		Ivec2 &operator+=(const Vec2 &v);
		Ivec2 &operator-=(const Vec2 &v);
		Ivec2 &operator*=(const Vec2 &v);
		Ivec2 &operator/=(const Vec2 &v);
		Ivec2 &operator+=(const Ivec2 &v);
		Ivec2 &operator-=(const Ivec2 &v);
		Ivec2 &operator*=(const Ivec2 &v);
		Ivec2 &operator/=(const Ivec2 &v);
		Ivec2 &operator+=(int v);
		Ivec2 &operator-=(int v);
		Ivec2 &operator*=(int v);
		Ivec2 &operator/=(int v);
	};

	Ivec2 operator+(const Ivec2 &lhs, int rhs);
	Ivec2 operator-(const Ivec2 &lhs, int rhs);
	Ivec2 operator*(const Ivec2 &lhs, int rhs);
	Ivec2 operator/(const Ivec2 &lhs, int rhs);
	Ivec2 operator+(int lhs, const Ivec2 &rhs);
	Ivec2 operator-(int lhs, const Ivec2 &rhs);
	Ivec2 operator*(int lhs, const Ivec2 &rhs);
	Ivec2 operator/(int lhs, const Ivec2 &rhs);

	struct Ivec3
	{
		int x;
		int y;
		int z;

		Ivec3();
		Ivec3(int v);
		Ivec3(int _x, int _y, int _z);
		Ivec3(const Vec2 &v, int _z);
		Ivec3(const Vec3 &v);
		Ivec3(const Vec4 &v);
		Ivec3(const Ivec2 &v, int _z);
		Ivec3(const Ivec3 &v);
		Ivec3(const Ivec4 &v);
		int &operator[](int i);
		int const&operator[](int i) const;
		Ivec3 &operator=(const Vec3 &v);
		Ivec3 &operator=(const Vec4 &v);
		Ivec3 &operator=(const Ivec3 &v);
		Ivec3 &operator=(const Ivec4 &v);
		bool operator==(const Ivec3 &v);
		bool operator!=(const Ivec3 &v);
		Ivec3 operator+(const Vec3 &v);
		Ivec3 operator-(const Vec3 &v);
		Ivec3 operator*(const Vec3 &v);
		Ivec3 operator/(const Vec3 &v);
		Ivec3 operator+(const Ivec3 &v);
		Ivec3 operator-(const Ivec3 &v);
		Ivec3 operator*(const Ivec3 &v);
		Ivec3 operator/(const Ivec3 &v);
		Ivec3 &operator+=(const Vec3 &v);
		Ivec3 &operator-=(const Vec3 &v);
		Ivec3 &operator*=(const Vec3 &v);
		Ivec3 &operator/=(const Vec3 &v);
		Ivec3 &operator+=(const Ivec3 &v);
		Ivec3 &operator-=(const Ivec3 &v);
		Ivec3 &operator*=(const Ivec3 &v);
		Ivec3 &operator/=(const Ivec3 &v);
		Ivec3 &operator+=(int v);
		Ivec3 &operator-=(int v);
		Ivec3 &operator*=(int v);
		Ivec3 &operator/=(int v);
	};

	Ivec3 operator+(const Ivec3 &lhs, int rhs);
	Ivec3 operator-(const Ivec3 &lhs, int rhs);
	Ivec3 operator*(const Ivec3 &lhs, int rhs);
	Ivec3 operator/(const Ivec3 &lhs, int rhs);
	Ivec3 operator+(int lhs, const Ivec3 &rhs);
	Ivec3 operator-(int lhs, const Ivec3 &rhs);
	Ivec3 operator*(int lhs, const Ivec3 &rhs);
	Ivec3 operator/(int lhs, const Ivec3 &rhs);

	struct Ivec4
	{
		int x;
		int y;
		int z;
		int w;

		Ivec4();
		Ivec4(int v);
		Ivec4(int _x, int _y, int _z, int _w);
		Ivec4(const Vec2 &v, int _z, int _w);
		Ivec4(const Vec3 &v, int _w);
		Ivec4(const Vec4 &v);
		Ivec4(const Ivec2 &v, int _z, int _w);
		Ivec4(const Ivec3 &v, int _w);
		Ivec4(const Ivec4 &v);
		int &operator[](int i);
		int const&operator[](int i) const;
		Ivec4 &operator=(const Vec4 &v);
		Ivec4 &operator=(const Ivec4 &v);
		bool operator==(const Ivec4 &v);
		bool operator!=(const Ivec4 &v);
		Ivec4 operator+(const Vec4 &v);
		Ivec4 operator-(const Vec4 &v);
		Ivec4 operator*(const Vec4 &v);
		Ivec4 operator/(const Vec4 &v);
		Ivec4 operator+(const Ivec4 &v);
		Ivec4 operator-(const Ivec4 &v);
		Ivec4 operator*(const Ivec4 &v);
		Ivec4 operator/(const Ivec4 &v);
		Ivec4 &operator+=(const Vec4 &v);
		Ivec4 &operator-=(const Vec4 &v);
		Ivec4 &operator*=(const Vec4 &v);
		Ivec4 &operator/=(const Vec4 &v);
		Ivec4 &operator+=(const Ivec4 &v);
		Ivec4 &operator-=(const Ivec4 &v);
		Ivec4 &operator*=(const Ivec4 &v);
		Ivec4 &operator/=(const Ivec4 &v);
		Ivec4 &operator+=(int v);
		Ivec4 &operator-=(int v);
		Ivec4 &operator*=(int v);
		Ivec4 &operator/=(int v);
	};

	Ivec4 operator+(const Ivec4 &lhs, int rhs);
	Ivec4 operator-(const Ivec4 &lhs, int rhs);
	Ivec4 operator*(const Ivec4 &lhs, int rhs);
	Ivec4 operator/(const Ivec4 &lhs, int rhs);
	Ivec4 operator+(int lhs, const Ivec4 &rhs);
	Ivec4 operator-(int lhs, const Ivec4 &rhs);
	Ivec4 operator*(int lhs, const Ivec4 &rhs);
	Ivec4 operator/(int lhs, const Ivec4 &rhs);

	struct Mat2
	{
		Vec2 cols[2];

		Mat2();
		Mat2(float diagonal);
		Mat2(float Xx, float Xy, 
			float Yx, float Yy);
		Mat2(const Vec2 &v0, const Vec2 &v1);
		Mat2(const Mat2 &v);
		Mat2(const Mat3 &v);
		Mat2(const Mat4 &v);
		Vec2 &operator[](int i);
		Vec2 const &operator[](int i) const;
		Mat2 &operator=(const Mat2 &v);
		Mat2 &operator=(const Mat3 &v);
		Mat2 &operator=(const Mat4 &v);
		Mat2 operator+(const Mat2 &v);
		Mat2 operator-(const Mat2 &v);
		Mat2 operator*(const Mat2 &v);
		Vec2 operator*(const Vec2 &v);
		Mat2 operator/(const Mat2 &v);
		Mat2 &operator+=(const Mat2 &v);
		Mat2 &operator-=(const Mat2 &v);
		Mat2 &operator*=(const Mat2 &v);
		Mat2 &operator/=(const Mat2 &v);
		Mat2 &operator+=(float v);
		Mat2 &operator-=(float v);
		Mat2 &operator*=(float v);
		Mat2 &operator/=(float v);
		void transpose();
		Mat2 get_transpose() const;
		void inverse();
		Mat2 get_inverse() const;
	};

	Mat2 operator+(const Mat2 &lhs, float rhs);
	Mat2 operator-(const Mat2 &lhs, float rhs);
	Mat2 operator*(const Mat2 &lhs, float rhs);
	Mat2 operator/(const Mat2 &lhs, float rhs);
	Mat2 operator+(float lhs, const Mat2 &rhs);
	Mat2 operator-(float lhs, const Mat2 &rhs);
	Mat2 operator*(float lhs, const Mat2 &rhs);
	Mat2 operator/(float lhs, const Mat2 &rhs);

	struct Mat3
	{
		Vec3 cols[3];

		Mat3();
		Mat3(float diagonal);
		Mat3(float Xx, float Xy, float Xz,
			float Yx, float Yy, float Yz,
			float Zx, float Zy, float Zz);
		Mat3(const Vec3 &v0, const Vec3 &v1, const Vec3 &v2);
		Mat3(const Mat3 &v);
		Mat3(const Mat4 &v);
		Vec3 &operator[](int i);
		Vec3 const &operator[](int i) const;
		Mat3 &operator=(const Mat3 &v);
		Mat3 &operator=(const Mat4 &v);
		Mat3 operator+(const Mat3 &v);
		Mat3 operator-(const Mat3 &v);
		Mat3 operator*(const Mat3 &v);
		Vec3 operator*(const Vec3 &v);
		Mat3 operator/(const Mat3 &v);
		Mat3 &operator+=(const Mat3 &v);
		Mat3 &operator-=(const Mat3 &v);
		Mat3 &operator*=(const Mat3 &v);
		Mat3 &operator/=(const Mat3 &v);
		Mat3 &operator+=(float v);
		Mat3 &operator-=(float v);
		Mat3 &operator*=(float v);
		Mat3 &operator/=(float v);
		void transpose();
		Mat3 get_transpose();
		void inverse();
		Mat3 get_inverse();
	};

	Mat3 operator+(const Mat3 &lhs, float rhs);
	Mat3 operator-(const Mat3 &lhs, float rhs);
	Mat3 operator*(const Mat3 &lhs, float rhs);
	Mat3 operator/(const Mat3 &lhs, float rhs);
	Mat3 operator+(float lhs, const Mat3 &rhs);
	Mat3 operator-(float lhs, const Mat3 &rhs);
	Mat3 operator*(float lhs, const Mat3 &rhs);
	Mat3 operator/(float lhs, const Mat3 &rhs);

	struct Mat4
	{
		Vec4 cols[4];

		Mat4();
		Mat4(float diagonal);
		Mat4(float Xx, float Xy, float Xz, float Xw,
			float Yx, float Yy, float Yz, float Yw,
			float Zx, float Zy, float Zz, float Zw,
			float Wx, float Wy, float Wz, float Ww);
		Mat4(const Vec4 &v0, const Vec4 &v1, const Vec4 &v2, const Vec4 &v3);
		Mat4(const Mat4 &v);
		Vec4 &operator[](int i);
		Vec4 const &operator[](int i) const;
		Mat4 &operator=(const Mat4 &v);
		Mat4 operator+(const Mat4 &v);
		Mat4 operator-(const Mat4 &v);
		Mat4 operator*(const Mat4 &v);
		Vec4 operator*(const Vec4 &v);
		Vec3 operator*(const Vec3 &v);
		Mat4 operator/(const Mat4 &v);
		Mat4 &operator+=(const Mat4 &v);
		Mat4 &operator-=(const Mat4 &v);
		Mat4 &operator*=(const Mat4 &v);
		Mat4 &operator/=(const Mat4 &v);
		Mat4 &operator+=(float v);
		Mat4 &operator-=(float v);
		Mat4 &operator*=(float v);
		Mat4 &operator/=(float v);
		void transpose();
		Mat4 get_transpose();
		void inverse();
		Mat4 get_inverse();
	};

	Mat4 operator+(const Mat4 &lhs, float rhs);
	Mat4 operator-(const Mat4 &lhs, float rhs);
	Mat4 operator*(const Mat4 &lhs, float rhs);
	Mat4 operator/(const Mat4 &lhs, float rhs);
	Mat4 operator+(float lhs, const Mat4 &rhs);
	Mat4 operator-(float lhs, const Mat4 &rhs);
	Mat4 operator*(float lhs, const Mat4 &rhs);
	Mat4 operator/(float lhs, const Mat4 &rhs);

	struct EulerYawPitchRoll
	{
		float yaw;
		float pitch;
		float roll;
	};

	struct Quat
	{
		float x;
		float y;
		float z;
		float w;
	};

	struct Rect
	{
		Vec2 min;
		Vec2 max;

		enum Side
		{
			Outside,
			SideN,
			SideS,
			SideE,
			SideW,
			SideNE,
			SideNW,
			SideSE,
			SideSW,
			Inside
		};

		void expand();
		Rect get_expanded();
		bool contains(const Vec2 &p);
		Side calc_side(const Vec2 &p, float threshold);
	};

	struct Plane 
	{
		Vec3 normal;
		float d;

		Plane();
		Plane(const Vec3 &n, float d);
		Plane(const Vec3 &n, const Vec3 &p);
	};

	inline Ivec2 mod(int a, int b)
	{
		return Ivec2(a / b, a % b);
	}

	inline Vec2::Vec2() 
	{
	}

	inline Vec2::Vec2(float v) :
		x(v),
		y(v)
	{
	}

	inline Vec2::Vec2(float _x, float _y) :
		x(_x),
		y(_y)
	{
	}

	inline Vec2::Vec2(const Vec2 &v) :
		x(v.x),
		y(v.y)
	{
	}

	inline Vec2::Vec2(const Vec3 &v) :
		x(v.x),
		y(v.y)
	{
	}

	inline Vec2::Vec2(const Vec4 &v) :
		x(v.x),
		y(v.y)
	{
	}

	inline Vec2::Vec2(const Ivec2 &v) :
		x(v.x),
		y(v.y)
	{
	}

	inline Vec2::Vec2(const Ivec3 &v) :
		x(v.x),
		y(v.y)
	{
	}

	inline Vec2::Vec2(const Ivec4 &v) :
		x(v.x),
		y(v.y)
	{
	}

	inline float &Vec2::operator[](int i)
	{
		return *(&x + i);
	}

	inline float const&Vec2::operator[](int i) const
	{
		return *(&x + i);
	}

	inline Vec2 &Vec2::operator=(const Vec2 &v)
	{
		x = v.x;
		y = v.y;
		return *this;
	}

	inline Vec2 &Vec2::operator=(const Vec3 &v)
	{
		x = v.x;
		y = v.y;
		return *this;
	}

	inline Vec2 &Vec2::operator=(const Vec4 &v)
	{
		x = v.x;
		y = v.y;
		return *this;
	}

	inline Vec2 &Vec2::operator=(const Ivec2 &v)
	{
		x = v.x;
		y = v.y;
		return *this;
	}

	inline Vec2 &Vec2::operator=(const Ivec3 &v)
	{
		x = v.x;
		y = v.y;
		return *this;
	}

	inline Vec2 &Vec2::operator=(const Ivec4 &v)
	{
		x = v.x;
		y = v.y;
		return *this;
	}

	inline bool Vec2::operator==(const Vec2 &v)
	{
		return equals(x, v.x) && equals(y, v.y);
	}

	inline bool Vec2::operator!=(const Vec2 &v)
	{
		return !(*this == v);
	}

	inline Vec2 Vec2::operator+(const Vec2 &v)
	{
		Vec2 ret(*this);
		ret += v;
		return ret;
	}

	inline Vec2 Vec2::operator-(const Vec2 &v)
	{
		Vec2 ret(*this);
		ret -= v;
		return ret;
	}

	inline Vec2 Vec2::operator*(const Vec2 &v)
	{
		Vec2 ret(*this);
		ret *= v;
		return ret;
	}

	inline Vec2 Vec2::operator/(const Vec2 &v)
	{
		Vec2 ret(*this);
		ret /= v;
		return ret;
	}

	inline Vec2 Vec2::operator+(const Ivec2 &v)
	{
		Vec2 ret(*this);
		ret += v;
		return ret;
	}

	inline Vec2 Vec2::operator-(const Ivec2 &v)
	{
		Vec2 ret(*this);
		ret -= v;
		return ret;
	}

	inline Vec2 Vec2::operator*(const Ivec2 &v)
	{
		Vec2 ret(*this);
		ret *= v;
		return ret;
	}

	inline Vec2 Vec2::operator/(const Ivec2 &v)
	{
		Vec2 ret(*this);
		ret /= v;
		return ret;
	}

	inline Vec2 &Vec2::operator+=(const Vec2 &v)
	{
		x += v.x;
		y += v.y;
		return *this;
	}

	inline Vec2 &Vec2::operator-=(const Vec2 &v)
	{
		x -= v.x;
		y -= v.y;
		return *this;
	}

	inline Vec2 &Vec2::operator*=(const Vec2 &v)
	{
		x *= v.x;
		y *= v.y;
		return *this;
	}

	inline Vec2 &Vec2::operator/=(const Vec2 &v)
	{
		x /= v.x;
		y /= v.y;
		return *this;
	}

	inline Vec2 &Vec2::operator+=(const Ivec2 &v)
	{
		x += v.x;
		y += v.y;
		return *this;
	}

	inline Vec2 &Vec2::operator-=(const Ivec2 &v)
	{
		x -= v.x;
		y -= v.y;
		return *this;
	}

	inline Vec2 &Vec2::operator*=(const Ivec2 &v)
	{
		x *= v.x;
		y *= v.y;
		return *this;
	}

	inline Vec2 &Vec2::operator/=(const Ivec2 &v)
	{
		x /= v.x;
		y /= v.y;
		return *this;
	}

	inline Vec2 &Vec2::operator+=(float v)
	{
		x += v;
		y += v;
		return *this;
	}

	inline Vec2 &Vec2::operator-=(float v)
	{
		x -= v;
		y -= v;
		return *this;
	}

	inline Vec2 &Vec2::operator*=(float v)
	{
		x *= v;
		y *= v;
		return *this;
	}

	inline Vec2 &Vec2::operator/=(float v)
	{
		x /= v;
		y /= v;
		return *this;
	}

	inline float Vec2::length() const
	{
		return sqrt(x * x + y * y);
	}

	inline void Vec2::normalize()
	{
		auto l = length();
		x /= l;
		y /= l;
	}

	inline Vec2 Vec2::get_normalize() const
	{
		Vec2 ret(*this);
		ret.normalize();
		return ret;
	}

	inline Vec2 operator+(const Vec2 &lhs, float rhs)
	{
		Vec2 ret(lhs);
		ret.x += rhs;
		ret.y += rhs;
		return ret;
	}

	inline Vec2 operator-(const Vec2 &lhs, float rhs)
	{
		Vec2 ret(lhs);
		ret.x -= rhs;
		ret.y -= rhs;
		return ret;
	}

	inline Vec2 operator*(const Vec2 &lhs, float rhs)
	{
		Vec2 ret(lhs);
		ret.x *= rhs;
		ret.y *= rhs;
		return ret;
	}

	inline Vec2 operator/(const Vec2 &lhs, float rhs)
	{
		Vec2 ret(lhs);
		ret.x /= rhs;
		ret.y /= rhs;
		return ret;
	}

	inline Vec2 operator+(float lhs, const Vec2 &rhs)
	{
		Vec2 ret(rhs);
		ret.x += lhs;
		ret.y += lhs;
		return ret;
	}

	inline Vec2 operator-(float lhs, const Vec2 &rhs)
	{
		Vec2 ret(rhs);
		ret.x -= lhs;
		ret.y -= lhs;
		return ret;
	}

	inline Vec2 operator*(float lhs, const Vec2 &rhs)
	{
		Vec2 ret(rhs);
		ret.x *= lhs;
		ret.y *= lhs;
		return ret;
	}

	inline Vec2 operator/(float lhs, const Vec2 &rhs)
	{
		Vec2 ret(rhs);
		ret.x /= lhs;
		ret.y /= lhs;
		return ret;
	}

	inline Vec3::Vec3()
	{
	}

	inline Vec3::Vec3(float v) :
		x(v),
		y(v),
		z(v)
	{
	}

	inline Vec3::Vec3(float _x, float _y, float _z) :
		x(_x),
		y(_y),
		z(_z)
	{
	}

	inline Vec3::Vec3(const Vec2 &v, float _z) :
		x(v.x),
		y(v.y),
		z(_z)
	{
	}

	inline Vec3::Vec3(const Vec3 &v) :
		x(v.x),
		y(v.y),
		z(v.z)
	{
	}

	inline Vec3::Vec3(const Vec4 &v) :
		x(v.x),
		y(v.y),
		z(v.z)
	{
	}

	inline Vec3::Vec3(const Ivec2 &v, float _z) :
		x(v.x),
		y(v.y),
		z(_z)
	{
	}

	inline Vec3::Vec3(const Ivec3 &v) :
		x(v.x),
		y(v.y),
		z(v.z)
	{
	}

	inline Vec3::Vec3(const Ivec4 &v) :
		x(v.x),
		y(v.y),
		z(v.z)
	{
	}

	inline float &Vec3::operator[](int i)
	{
		return *(&x + i);
	}

	inline float const&Vec3::operator[](int i) const
	{
		return *(&x + i);
	}

	inline Vec3 &Vec3::operator=(const Vec3 &v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		return *this;
	}

	inline Vec3 &Vec3::operator=(const Vec4 &v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		return *this;
	}

	inline Vec3 &Vec3::operator=(const Ivec3 &v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		return *this;
	}

	inline Vec3 &Vec3::operator=(const Ivec4 &v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		return *this;
	}

	inline bool Vec3::operator==(const Vec3 &v)
	{
		return equals(x, v.x) && equals(y, v.y) && equals(z, v.z);
	}

	inline bool Vec3::operator!=(const Vec3 &v)
	{
		return !(*this == v);
	}

	inline Vec3 Vec3::operator+(const Vec3 &v)
	{
		Vec3 ret(*this);
		ret += v;
		return ret;
	}

	inline Vec3 Vec3::operator-(const Vec3 &v)
	{
		Vec3 ret(*this);
		ret -= v;
		return ret;
	}

	inline Vec3 Vec3::operator*(const Vec3 &v)
	{
		Vec3 ret(*this);
		ret *= v;
		return ret;
	}

	inline Vec3 Vec3::operator/(const Vec3 &v)
	{
		Vec3 ret(*this);
		ret /= v;
		return ret;
	}

	inline Vec3 Vec3::operator+(const Ivec3 &v)
	{
		Vec3 ret(*this);
		ret += v;
		return ret;
	}

	inline Vec3 Vec3::operator-(const Ivec3 &v)
	{
		Vec3 ret(*this);
		ret -= v;
		return ret;
	}

	inline Vec3 Vec3::operator*(const Ivec3 &v)
	{
		Vec3 ret(*this);
		ret *= v;
		return ret;
	}

	inline Vec3 Vec3::operator/(const Ivec3 &v)
	{
		Vec3 ret(*this);
		ret /= v;
		return ret;
	}

	inline Vec3 &Vec3::operator+=(const Vec3 &v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	inline Vec3 &Vec3::operator-=(const Vec3 &v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	inline Vec3 &Vec3::operator*=(const Vec3 &v)
	{
		x *= v.x;
		y *= v.y;
		z *= v.z;
		return *this;
	}

	inline Vec3 &Vec3::operator/=(const Vec3 &v)
	{
		x /= v.x;
		y /= v.y;
		z /= v.z;
		return *this;
	}

	inline Vec3 &Vec3::operator+=(const Ivec3 &v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	inline Vec3 &Vec3::operator-=(const Ivec3 &v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	inline Vec3 &Vec3::operator*=(const Ivec3 &v)
	{
		x *= v.x;
		y *= v.y;
		z *= v.z;
		return *this;
	}

	inline Vec3 &Vec3::operator/=(const Ivec3 &v)
	{
		x /= v.x;
		y /= v.y;
		z /= v.z;
		return *this;
	}

	inline Vec3 &Vec3::operator+=(float v)
	{
		x += v;
		y += v;
		z += v;
		return *this;
	}

	inline Vec3 &Vec3::operator-=(float v)
	{
		x -= v;
		y -= v;
		z -= v;
		return *this;
	}

	inline Vec3 &Vec3::operator*=(float v)
	{
		x *= v;
		y *= v;
		z *= v;
		return *this;
	}

	inline Vec3 &Vec3::operator/=(float v)
	{
		x /= v;
		y /= v;
		z /= v;
		return *this;
	}

	inline float Vec3::length() const
	{
		return sqrt(x * x + y * y + z * z);
	}

	inline void Vec3::normalize()
	{
		auto l = length();
		x /= l;
		y /= l;
		z /= l;
	}

	inline Vec3 Vec3::get_normalize() const
	{
		Vec3 ret(*this);
		ret.normalize();
		return ret;
	}

	inline Vec3 operator+(const Vec3 &lhs, float rhs)
	{
		Vec3 ret(lhs);
		ret.x += rhs;
		ret.y += rhs;
		ret.z += rhs;
		return ret;
	}

	inline Vec3 operator-(const Vec3 &lhs, float rhs)
	{
		Vec3 ret(lhs);
		ret.x -= rhs;
		ret.y -= rhs;
		ret.z -= rhs;
		return ret;
	}

	inline Vec3 operator*(const Vec3 &lhs, float rhs)
	{
		Vec3 ret(lhs);
		ret.x *= rhs;
		ret.y *= rhs;
		ret.z *= rhs;
		return ret;
	}

	inline Vec3 operator/(const Vec3 &lhs, float rhs)
	{
		Vec3 ret(lhs);
		ret.x /= rhs;
		ret.y /= rhs;
		ret.z /= rhs;
		return ret;
	}

	inline Vec3 operator+(float lhs, const Vec3 &rhs)
	{
		Vec3 ret(rhs);
		ret.x += lhs;
		ret.y += lhs;
		ret.z += lhs;
		return ret;
	}

	inline Vec3 operator-(float lhs, const Vec3 &rhs)
	{
		Vec3 ret(rhs);
		ret.x -= lhs;
		ret.y -= lhs;
		ret.z -= lhs;
		return ret;
	}

	inline Vec3 operator*(float lhs, const Vec3 &rhs)
	{
		Vec3 ret(rhs);
		ret.x *= lhs;
		ret.y *= lhs;
		ret.z *= lhs;
		return ret;
	}

	inline Vec3 operator/(float lhs, const Vec3 &rhs)
	{
		Vec3 ret(rhs);
		ret.x /= lhs;
		ret.y /= lhs;
		ret.z /= lhs;
		return ret;
	}

	inline Vec4::Vec4()
	{
	}

	inline Vec4::Vec4(float v) :
		x(v),
		y(v),
		z(v),
		w(v)
	{
	}

	inline Vec4::Vec4(float _x, float _y, float _z, float _w) :
		x(_x),
		y(_y),
		z(_z),
		w(_w)
	{
	}

	inline Vec4::Vec4(const Vec2 &v, float _z, float _w) :
		x(v.x),
		y(v.y),
		z(_z),
		w(_w)
	{
	}

	inline Vec4::Vec4(const Vec3 &v, float _w) :
		x(v.x),
		y(v.y),
		z(v.z),
		w(_w)
	{
	}

	inline Vec4::Vec4(const Vec4 &v) :
		x(v.x),
		y(v.y),
		z(v.z),
		w(v.w)
	{
	}

	inline Vec4::Vec4(const Ivec2 &v, float _z, float _w) :
		x(v.x),
		y(v.y),
		z(_z),
		w(_w)
	{
	}

	inline Vec4::Vec4(const Ivec3 &v, float _w) :
		x(v.x),
		y(v.y),
		z(v.z),
		w(_w)
	{
	}

	inline Vec4::Vec4(const Ivec4 &v) :
		x(v.x),
		y(v.y),
		z(v.z),
		w(v.w)
	{
	}

	inline float &Vec4::operator[](int i)
	{
		return *(&x + i);
	}

	inline float const&Vec4::operator[](int i) const
	{
		return *(&x + i);
	}

	inline Vec4 &Vec4::operator=(const Vec4 &v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		w = v.w;
		return *this;
	}

	inline Vec4 &Vec4::operator=(const Ivec4 &v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		w = v.w;
		return *this;
	}

	inline bool Vec4::operator==(const Vec4 &v)
	{
		return equals(x, v.x) && equals(y, v.y) && equals(z, v.z) && equals(w, v.w);
	}

	inline bool Vec4::operator!=(const Vec4 &v)
	{
		return !(*this == v);
	}

	inline Vec4 Vec4::operator+(const Vec4 &v)
	{
		Vec4 ret(*this);
		ret += v;
		return ret;
	}

	inline Vec4 Vec4::operator-(const Vec4 &v)
	{
		Vec4 ret(*this);
		ret -= v;
		return ret;
	}

	inline Vec4 Vec4::operator*(const Vec4 &v)
	{
		Vec4 ret(*this);
		ret *= v;
		return ret;
	}

	inline Vec4 Vec4::operator/(const Vec4 &v)
	{
		Vec4 ret(*this);
		ret /= v;
		return ret;
	}

	inline Vec4 Vec4::operator+(const Ivec4 &v)
	{
		Vec4 ret(*this);
		ret += v;
		return ret;
	}

	inline Vec4 Vec4::operator-(const Ivec4 &v)
	{
		Vec4 ret(*this);
		ret -= v;
		return ret;
	}

	inline Vec4 Vec4::operator*(const Ivec4 &v)
	{
		Vec4 ret(*this);
		ret *= v;
		return ret;
	}

	inline Vec4 Vec4::operator/(const Ivec4 &v)
	{
		Vec4 ret(*this);
		ret /= v;
		return ret;
	}

	inline Vec4 &Vec4::operator+=(const Vec4 &v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		w += v.w;
		return *this;
	}

	inline Vec4 &Vec4::operator-=(const Vec4 &v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		w -= v.w;
		return *this;
	}

	inline Vec4 &Vec4::operator*=(const Vec4 &v)
	{
		x *= v.x;
		y *= v.y;
		z *= v.z;
		w *= v.w;
		return *this;
	}

	inline Vec4 &Vec4::operator/=(const Vec4 &v)
	{
		x /= v.x;
		y /= v.y;
		z /= v.z;
		w /= v.w;
		return *this;
	}

	inline Vec4 &Vec4::operator+=(const Ivec4 &v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		w += v.w;
		return *this;
	}

	inline Vec4 &Vec4::operator-=(const Ivec4 &v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		w -= v.w;
		return *this;
	}

	inline Vec4 &Vec4::operator*=(const Ivec4 &v)
	{
		x *= v.x;
		y *= v.y;
		z *= v.z;
		w *= v.w;
		return *this;
	}

	inline Vec4 &Vec4::operator/=(const Ivec4 &v)
	{
		x /= v.x;
		y /= v.y;
		z /= v.z;
		w /= v.w;
		return *this;
	}

	inline Vec4 &Vec4::operator+=(float v)
	{
		x += v;
		y += v;
		z += v;
		w += v;
		return *this;
	}

	inline Vec4 &Vec4::operator-=(float v)
	{
		x -= v;
		y -= v;
		z -= v;
		w -= v;
		return *this;
	}

	inline Vec4 &Vec4::operator*=(float v)
	{
		x *= v;
		y *= v;
		z *= v;
		w *= v;
		return *this;
	}

	inline Vec4 &Vec4::operator/=(float v)
	{
		x /= v;
		y /= v;
		z /= v;
		w /= v;
		return *this;
	}

	inline float Vec4::length() const
	{
		return sqrt(x * x + y * y + z * z + w * w);
	}

	inline void Vec4::normalize()
	{
		auto l = length();
		x /= l;
		y /= l;
		z /= l;
		w /= l;
	}

	inline Vec4 Vec4::get_normalize() const
	{
		Vec4 ret(*this);
		ret.normalize();
		return ret;
	}

	inline Vec4 operator+(const Vec4 &lhs, float rhs)
	{
		Vec4 ret(lhs);
		ret.x += rhs;
		ret.y += rhs;
		ret.z += rhs;
		ret.w += rhs;
		return ret;
	}

	inline Vec4 operator-(const Vec4 &lhs, float rhs)
	{
		Vec4 ret(lhs);
		ret.x -= rhs;
		ret.y -= rhs;
		ret.z -= rhs;
		ret.w -= rhs;
		return ret;
	}

	inline Vec4 operator*(const Vec4 &lhs, float rhs)
	{
		Vec4 ret(lhs);
		ret.x *= rhs;
		ret.y *= rhs;
		ret.z *= rhs;
		ret.w *= rhs;
		return ret;
	}

	inline Vec4 operator/(const Vec4 &lhs, float rhs)
	{
		Vec4 ret(lhs);
		ret.x /= rhs;
		ret.y /= rhs;
		ret.z /= rhs;
		ret.w /= rhs;
		return ret;
	}

	inline Vec4 operator+(float lhs, const Vec4 &rhs)
	{
		Vec4 ret(rhs);
		ret.x += lhs;
		ret.y += lhs;
		ret.z += lhs;
		ret.w += lhs;
		return ret;
	}

	inline Vec4 operator-(float lhs, const Vec4 &rhs)
	{
		Vec4 ret(rhs);
		ret.x -= lhs;
		ret.y -= lhs;
		ret.z -= lhs;
		ret.w -= lhs;
		return ret;
	}

	inline Vec4 operator*(float lhs, const Vec4 &rhs)
	{
		Vec4 ret(rhs);
		ret.x *= lhs;
		ret.y *= lhs;
		ret.z *= lhs;
		ret.w *= lhs;
		return ret;
	}

	inline Vec4 operator/(float lhs, const Vec4 &rhs)
	{
		Vec4 ret(rhs);
		ret.x /= lhs;
		ret.y /= lhs;
		ret.z /= lhs;
		ret.w /= lhs;
		return ret;
	}

	inline Ivec2::Ivec2()
	{
	}

	inline Ivec2::Ivec2(int v) :
		x(v),
		y(v)
	{
	}

	inline Ivec2::Ivec2(int _x, int _y) :
		x(_x),
		y(_y)
	{
	}

	inline Ivec2::Ivec2(const Vec2 &v) :
		x(v.x),
		y(v.y)
	{
	}

	inline Ivec2::Ivec2(const Vec3 &v) :
		x(v.x),
		y(v.y)
	{
	}

	inline Ivec2::Ivec2(const Vec4 &v) :
		x(v.x),
		y(v.y)
	{
	}

	inline Ivec2::Ivec2(const Ivec2 &v) :
		x(v.x),
		y(v.y)
	{
	}

	inline Ivec2::Ivec2(const Ivec3 &v) :
		x(v.x),
		y(v.y)
	{
	}

	inline Ivec2::Ivec2(const Ivec4 &v) :
		x(v.x),
		y(v.y)
	{
	}

	inline int &Ivec2::operator[](int i)
	{
		return *(&x + i);
	}

	inline int const&Ivec2::operator[](int i) const
	{
		return *(&x + i);
	}

	inline Ivec2 &Ivec2::operator=(const Vec2 &v)
	{
		x = v.x;
		y = v.y;
		return *this;
	}

	inline Ivec2 &Ivec2::operator=(const Vec3 &v)
	{
		x = v.x;
		y = v.y;
		return *this;
	}

	inline Ivec2 &Ivec2::operator=(const Vec4 &v)
	{
		x = v.x;
		y = v.y;
		return *this;
	}

	inline Ivec2 &Ivec2::operator=(const Ivec2 &v)
	{
		x = v.x;
		y = v.y;
		return *this;
	}

	inline Ivec2 &Ivec2::operator=(const Ivec3 &v)
	{
		x = v.x;
		y = v.y;
		return *this;
	}

	inline Ivec2 &Ivec2::operator=(const Ivec4 &v)
	{
		x = v.x;
		y = v.y;
		return *this;
	}

	inline bool Ivec2::operator==(const Ivec2 &v)
	{
		return x == v.x && y == v.y;
	}

	inline bool Ivec2::operator!=(const Ivec2 &v)
	{
		return !(*this == v);
	}

	inline Ivec2 Ivec2::operator+(const Vec2 &v)
	{
		Ivec2 ret(*this);
		ret += v;
		return ret;
	}

	inline Ivec2 Ivec2::operator-(const Vec2 &v)
	{
		Ivec2 ret(*this);
		ret -= v;
		return ret;
	}

	inline Ivec2 Ivec2::operator*(const Vec2 &v)
	{
		Ivec2 ret(*this);
		ret *= v;
		return ret;
	}

	inline Ivec2 Ivec2::operator/(const Vec2 &v)
	{
		Ivec2 ret(*this);
		ret /= v;
		return ret;
	}

	inline Ivec2 Ivec2::operator+(const Ivec2 &v)
	{
		Ivec2 ret(*this);
		ret += v;
		return ret;
	}

	inline Ivec2 Ivec2::operator-(const Ivec2 &v)
	{
		Ivec2 ret(*this);
		ret -= v;
		return ret;
	}

	inline Ivec2 Ivec2::operator*(const Ivec2 &v)
	{
		Ivec2 ret(*this);
		ret *= v;
		return ret;
	}

	inline Ivec2 Ivec2::operator/(const Ivec2 &v)
	{
		Ivec2 ret(*this);
		ret /= v;
		return ret;
	}

	inline Ivec2 &Ivec2::operator+=(const Vec2 &v)
	{
		x += v.x;
		y += v.y;
		return *this;
	}

	inline Ivec2 &Ivec2::operator-=(const Vec2 &v)
	{
		x -= v.x;
		y -= v.y;
		return *this;
	}

	inline Ivec2 &Ivec2::operator*=(const Vec2 &v)
	{
		x *= v.x;
		y *= v.y;
		return *this;
	}

	inline Ivec2 &Ivec2::operator/=(const Vec2 &v)
	{
		x /= v.x;
		y /= v.y;
		return *this;
	}

	inline Ivec2 &Ivec2::operator+=(const Ivec2 &v)
	{
		x += v.x;
		y += v.y;
		return *this;
	}

	inline Ivec2 &Ivec2::operator-=(const Ivec2 &v)
	{
		x -= v.x;
		y -= v.y;
		return *this;
	}

	inline Ivec2 &Ivec2::operator*=(const Ivec2 &v)
	{
		x *= v.x;
		y *= v.y;
		return *this;
	}

	inline Ivec2 &Ivec2::operator/=(const Ivec2 &v)
	{
		x /= v.x;
		y /= v.y;
		return *this;
	}

	inline Ivec2 &Ivec2::operator+=(int v)
	{
		x += v;
		y += v;
		return *this;
	}

	inline Ivec2 &Ivec2::operator-=(int v)
	{
		x -= v;
		y -= v;
		return *this;
	}

	inline Ivec2 &Ivec2::operator*=(int v)
	{
		x *= v;
		y *= v;
		return *this;
	}

	inline Ivec2 &Ivec2::operator/=(int v)
	{
		x /= v;
		y /= v;
		return *this;
	}

	inline Ivec2 operator+(const Ivec2 &lhs, int rhs)
	{
		Ivec2 ret(lhs);
		ret.x += rhs;
		ret.y += rhs;
		return ret;
	}

	inline Ivec2 operator-(const Ivec2 &lhs, int rhs)
	{
		Ivec2 ret(lhs);
		ret.x -= rhs;
		ret.y -= rhs;
		return ret;
	}

	inline Ivec2 operator*(const Ivec2 &lhs, int rhs)
	{
		Ivec2 ret(lhs);
		ret.x *= rhs;
		ret.y *= rhs;
		return ret;
	}

	inline Ivec2 operator/(const Ivec2 &lhs, int rhs)
	{
		Ivec2 ret(lhs);
		ret.x /= rhs;
		ret.y /= rhs;
		return ret;
	}

	inline Ivec2 operator+(int lhs, const Ivec2 &rhs)
	{
		Ivec2 ret(rhs);
		ret.x += lhs;
		ret.y += lhs;
		return ret;
	}

	inline Ivec2 operator-(int lhs, const Ivec2 &rhs)
	{
		Ivec2 ret(rhs);
		ret.x -= lhs;
		ret.y -= lhs;
		return ret;
	}

	inline Ivec2 operator*(int lhs, const Ivec2 &rhs)
	{
		Ivec2 ret(rhs);
		ret.x *= lhs;
		ret.y *= lhs;
		return ret;
	}

	inline Ivec2 operator/(int lhs, const Ivec2 &rhs)
	{
		Ivec2 ret(rhs);
		ret.x /= lhs;
		ret.y /= lhs;
		return ret;
	}

	inline Ivec3::Ivec3()
	{
	}

	inline Ivec3::Ivec3(int v) :
		x(v),
		y(v),
		z(v)
	{
	}

	inline Ivec3::Ivec3(int _x, int _y, int _z) :
		x(_x),
		y(_y),
		z(_z)
	{
	}

	inline Ivec3::Ivec3(const Vec2 &v, int _z) :
		x(v.x),
		y(v.y),
		z(_z)
	{
	}

	inline Ivec3::Ivec3(const Vec3 &v) :
		x(v.x),
		y(v.y),
		z(v.z)
	{
	}

	inline Ivec3::Ivec3(const Vec4 &v) :
		x(v.x),
		y(v.y),
		z(v.z)
	{
	}

	inline Ivec3::Ivec3(const Ivec2 &v, int _z) :
		x(v.x),
		y(v.y),
		z(_z)
	{
	}

	inline Ivec3::Ivec3(const Ivec3 &v) :
		x(v.x),
		y(v.y),
		z(v.z)
	{
	}

	inline Ivec3::Ivec3(const Ivec4 &v) :
		x(v.x),
		y(v.y),
		z(v.z)
	{
	}

	inline int &Ivec3::operator[](int i)
	{
		return *(&x + i);
	}

	inline int const&Ivec3::operator[](int i) const
	{
		return *(&x + i);
	}

	inline Ivec3 &Ivec3::operator=(const Vec3 &v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		return *this;
	}

	inline Ivec3 &Ivec3::operator=(const Vec4 &v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		return *this;
	}

	inline Ivec3 &Ivec3::operator=(const Ivec3 &v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		return *this;
	}

	inline Ivec3 &Ivec3::operator=(const Ivec4 &v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		return *this;
	}

	inline bool Ivec3::operator==(const Ivec3 &v)
	{
		return x == v.x && y == v.y && z == v.z;
	}

	inline bool Ivec3::operator!=(const Ivec3 &v)
	{
		return !(*this == v);
	}

	inline Ivec3 Ivec3::operator+(const Vec3 &v)
	{
		Ivec3 ret(*this);
		ret += v;
		return ret;
	}

	inline Ivec3 Ivec3::operator-(const Vec3 &v)
	{
		Ivec3 ret(*this);
		ret -= v;
		return ret;
	}

	inline Ivec3 Ivec3::operator*(const Vec3 &v)
	{
		Ivec3 ret(*this);
		ret *= v;
		return ret;
	}

	inline Ivec3 Ivec3::operator/(const Vec3 &v)
	{
		Ivec3 ret(*this);
		ret /= v;
		return ret;
	}

	inline Ivec3 Ivec3::operator+(const Ivec3 &v)
	{
		Ivec3 ret(*this);
		ret += v;
		return ret;
	}

	inline Ivec3 Ivec3::operator-(const Ivec3 &v)
	{
		Ivec3 ret(*this);
		ret -= v;
		return ret;
	}

	inline Ivec3 Ivec3::operator*(const Ivec3 &v)
	{
		Ivec3 ret(*this);
		ret *= v;
		return ret;
	}

	inline Ivec3 Ivec3::operator/(const Ivec3 &v)
	{
		Ivec3 ret(*this);
		ret /= v;
		return ret;
	}

	inline Ivec3 &Ivec3::operator+=(const Vec3 &v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	inline Ivec3 &Ivec3::operator-=(const Vec3 &v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	inline Ivec3 &Ivec3::operator*=(const Vec3 &v)
	{
		x *= v.x;
		y *= v.y;
		z *= v.z;
		return *this;
	}

	inline Ivec3 &Ivec3::operator/=(const Vec3 &v)
	{
		x /= v.x;
		y /= v.y;
		z /= v.z;
		return *this;
	}

	inline Ivec3 &Ivec3::operator+=(const Ivec3 &v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	inline Ivec3 &Ivec3::operator-=(const Ivec3 &v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	inline Ivec3 &Ivec3::operator*=(const Ivec3 &v)
	{
		x *= v.x;
		y *= v.y;
		z *= v.z;
		return *this;
	}

	inline Ivec3 &Ivec3::operator/=(const Ivec3 &v)
	{
		x /= v.x;
		y /= v.y;
		z /= v.z;
		return *this;
	}

	inline Ivec3 &Ivec3::operator+=(int v)
	{
		x += v;
		y += v;
		z += v;
		return *this;
	}

	inline Ivec3 &Ivec3::operator-=(int v)
	{
		x -= v;
		y -= v;
		z -= v;
		return *this;
	}

	inline Ivec3 &Ivec3::operator*=(int v)
	{
		x *= v;
		y *= v;
		z *= v;
		return *this;
	}

	inline Ivec3 &Ivec3::operator/=(int v)
	{
		x /= v;
		y /= v;
		z /= v;
		return *this;
	}

	inline Ivec3 operator+(const Ivec3 &lhs, int rhs)
	{
		Ivec3 ret(lhs);
		ret.x += rhs;
		ret.y += rhs;
		ret.z += rhs;
		return ret;
	}

	inline Ivec3 operator-(const Ivec3 &lhs, int rhs)
	{
		Ivec3 ret(lhs);
		ret.x -= rhs;
		ret.y -= rhs;
		ret.z -= rhs;
		return ret;
	}

	inline Ivec3 operator*(const Ivec3 &lhs, int rhs)
	{
		Ivec3 ret(lhs);
		ret.x *= rhs;
		ret.y *= rhs;
		ret.z *= rhs;
		return ret;
	}

	inline Ivec3 operator/(const Ivec3 &lhs, int rhs)
	{
		Ivec3 ret(lhs);
		ret.x /= rhs;
		ret.y /= rhs;
		ret.z /= rhs;
		return ret;
	}

	inline Ivec3 operator+(int lhs, const Ivec3 &rhs)
	{
		Ivec3 ret(rhs);
		ret.x += lhs;
		ret.y += lhs;
		ret.z += lhs;
		return ret;
	}

	inline Ivec3 operator-(int lhs, const Ivec3 &rhs)
	{
		Ivec3 ret(rhs);
		ret.x -= lhs;
		ret.y -= lhs;
		ret.z -= lhs;
		return ret;
	}

	inline Ivec3 operator*(int lhs, const Ivec3 &rhs)
	{
		Ivec3 ret(rhs);
		ret.x *= lhs;
		ret.y *= lhs;
		ret.z *= lhs;
		return ret;
	}

	inline Ivec3 operator/(int lhs, const Ivec3 &rhs)
	{
		Ivec3 ret(rhs);
		ret.x /= lhs;
		ret.y /= lhs;
		ret.z /= lhs;
		return ret;
	}

	inline Ivec4::Ivec4()
	{
	}

	inline Ivec4::Ivec4(int v) :
		x(v),
		y(v),
		z(v),
		w(v)
	{
	}

	inline Ivec4::Ivec4(int _x, int _y, int _z, int _w) :
		x(_x),
		y(_y),
		z(_z),
		w(_w)
	{
	}

	inline Ivec4::Ivec4(const Vec2 &v, int _z, int _w) :
		x(v.x),
		y(v.y),
		z(_z),
		w(_w)
	{
	}

	inline Ivec4::Ivec4(const Vec3 &v, int _w) :
		x(v.x),
		y(v.y),
		z(v.z),
		w(_w)
	{
	}

	inline Ivec4::Ivec4(const Vec4 &v) :
		x(v.x),
		y(v.y),
		z(v.z),
		w(v.w)
	{
	}

	inline Ivec4::Ivec4(const Ivec2 &v, int _z, int _w) :
		x(v.x),
		y(v.y),
		z(_z),
		w(_w)
	{
	}

	inline Ivec4::Ivec4(const Ivec3 &v, int _w) :
		x(v.x),
		y(v.y),
		z(v.z),
		w(_w)
	{
	}

	inline Ivec4::Ivec4(const Ivec4 &v) :
		x(v.x),
		y(v.y),
		z(v.z),
		w(v.w)
	{
	}

	inline int &Ivec4::operator[](int i)
	{
		return *(&x + i);
	}

	inline int const&Ivec4::operator[](int i) const
	{
		return *(&x + i);
	}

	inline Ivec4 &Ivec4::operator=(const Vec4 &v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		w = v.w;
		return *this;
	}

	inline Ivec4 &Ivec4::operator=(const Ivec4 &v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		w = v.w;
		return *this;
	}

	inline bool Ivec4::operator==(const Ivec4 &v)
	{
		return x == v.x && y == v.y && z== v.z && w == v.w;
	}

	inline bool Ivec4::operator!=(const Ivec4 &v)
	{
		return !(*this == v);
	}

	inline Ivec4 Ivec4::operator+(const Vec4 &v)
	{
		Ivec4 ret(*this);
		ret += v;
		return ret;
	}

	inline Ivec4 Ivec4::operator-(const Vec4 &v)
	{
		Ivec4 ret(*this);
		ret -= v;
		return ret;
	}

	inline Ivec4 Ivec4::operator*(const Vec4 &v)
	{
		Ivec4 ret(*this);
		ret *= v;
		return ret;
	}

	inline Ivec4 Ivec4::operator/(const Vec4 &v)
	{
		Ivec4 ret(*this);
		ret /= v;
		return ret;
	}

	inline Ivec4 Ivec4::operator+(const Ivec4 &v)
	{
		Ivec4 ret(*this);
		ret += v;
		return ret;
	}

	inline Ivec4 Ivec4::operator-(const Ivec4 &v)
	{
		Ivec4 ret(*this);
		ret -= v;
		return ret;
	}

	inline Ivec4 Ivec4::operator*(const Ivec4 &v)
	{
		Ivec4 ret(*this);
		ret *= v;
		return ret;
	}

	inline Ivec4 Ivec4::operator/(const Ivec4 &v)
	{
		Ivec4 ret(*this);
		ret /= v;
		return ret;
	}

	inline Ivec4 &Ivec4::operator+=(const Vec4 &v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		w += v.w;
		return *this;
	}

	inline Ivec4 &Ivec4::operator-=(const Vec4 &v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		w -= v.w;
		return *this;
	}

	inline Ivec4 &Ivec4::operator*=(const Vec4 &v)
	{
		x *= v.x;
		y *= v.y;
		z *= v.z;
		w *= v.w;
		return *this;
	}

	inline Ivec4 &Ivec4::operator/=(const Vec4 &v)
	{
		x /= v.x;
		y /= v.y;
		z /= v.z;
		w /= v.w;
		return *this;
	}

	inline Ivec4 &Ivec4::operator+=(const Ivec4 &v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		w += v.w;
		return *this;
	}

	inline Ivec4 &Ivec4::operator-=(const Ivec4 &v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		w -= v.w;
		return *this;
	}

	inline Ivec4 &Ivec4::operator*=(const Ivec4 &v)
	{
		x *= v.x;
		y *= v.y;
		z *= v.z;
		w *= v.w;
		return *this;
	}

	inline Ivec4 &Ivec4::operator/=(const Ivec4 &v)
	{
		x /= v.x;
		y /= v.y;
		z /= v.z;
		w /= v.w;
		return *this;
	}

	inline Ivec4 &Ivec4::operator+=(int v)
	{
		x += v;
		y += v;
		z += v;
		w += v;
		return *this;
	}

	inline Ivec4 &Ivec4::operator-=(int v)
	{
		x -= v;
		y -= v;
		z -= v;
		w -= v;
		return *this;
	}

	inline Ivec4 &Ivec4::operator*=(int v)
	{
		x *= v;
		y *= v;
		z *= v;
		w *= v;
		return *this;
	}

	inline Ivec4 &Ivec4::operator/=(int v)
	{
		x /= v;
		y /= v;
		z /= v;
		w /= v;
		return *this;
	}

	inline Ivec4 operator+(const Ivec4 &lhs, int rhs)
	{
		Ivec4 ret(lhs);
		ret.x += rhs;
		ret.y += rhs;
		ret.z += rhs;
		ret.w += rhs;
		return ret;
	}

	inline Ivec4 operator-(const Ivec4 &lhs, int rhs)
	{
		Ivec4 ret(lhs);
		ret.x -= rhs;
		ret.y -= rhs;
		ret.z -= rhs;
		ret.w -= rhs;
		return ret;
	}

	inline Ivec4 operator*(const Ivec4 &lhs, int rhs)
	{
		Ivec4 ret(lhs);
		ret.x *= rhs;
		ret.y *= rhs;
		ret.z *= rhs;
		ret.w *= rhs;
		return ret;
	}

	inline Ivec4 operator/(const Ivec4 &lhs, int rhs)
	{
		Ivec4 ret(lhs);
		ret.x /= rhs;
		ret.y /= rhs;
		ret.z /= rhs;
		ret.w /= rhs;
		return ret;
	}

	inline Ivec4 operator+(int lhs, const Ivec4 &rhs)
	{
		Ivec4 ret(rhs);
		ret.x += lhs;
		ret.y += lhs;
		ret.z += lhs;
		ret.w += lhs;
		return ret;
	}

	inline Ivec4 operator-(int lhs, const Ivec4 &rhs)
	{
		Ivec4 ret(rhs);
		ret.x -= lhs;
		ret.y -= lhs;
		ret.z -= lhs;
		ret.w -= lhs;
		return ret;
	}

	inline Ivec4 operator*(int lhs, const Ivec4 &rhs)
	{
		Ivec4 ret(rhs);
		ret.x *= lhs;
		ret.y *= lhs;
		ret.z *= lhs;
		ret.w *= lhs;
		return ret;
	}

	inline Ivec4 operator/(int lhs, const Ivec4 &rhs)
	{
		Ivec4 ret(rhs);
		ret.x /= lhs;
		ret.y /= lhs;
		ret.z /= lhs;
		ret.w /= lhs;
		return ret;
	}

	inline Mat2::Mat2()
	{
	}

	inline Mat2::Mat2(float diagonal)
	{
		(*this)[0][0] = diagonal;  (*this)[1][0] = 0.f;
		(*this)[0][1] = 0.f;       (*this)[1][1] = diagonal;
	}

	inline Mat2::Mat2(float Xx, float Xy,
		float Yx, float Yy)
	{
		(*this)[0][0] = Xx;  (*this)[1][0] = Yx;
		(*this)[0][1] = Xy;  (*this)[1][1] = Yy;
	}

	inline Mat2::Mat2(const Vec2 &v0, const Vec2 &v1)
	{
		(*this)[0] = v0;
		(*this)[1] = v1;
	}

	inline Mat2::Mat2(const Mat2 &v)
	{
		(*this)[0] = v[0];
		(*this)[1] = v[1];
	}

	inline Mat2::Mat2(const Mat3 &v)
	{
		(*this)[0] = v[0];
		(*this)[1] = v[1];
	}

	inline Mat2::Mat2(const Mat4 &v)
	{
		(*this)[0] = v[0];
		(*this)[1] = v[1];
	}

	inline Vec2 &Mat2::operator[](int i)
	{
		return (*this)[i];
	}

	inline Vec2 const&Mat2::operator[](int i) const
	{
		return (*this)[i];
	}

	inline Mat2 &Mat2::operator=(const Mat2 &v)
	{
		(*this)[0] = v[0];
		(*this)[1] = v[1];
		return *this;
	}

	inline Mat2 &Mat2::operator=(const Mat3 &v)
	{
		(*this)[0] = v[0];
		(*this)[1] = v[1];
		return *this;
	}

	inline Mat2 &Mat2::operator=(const Mat4 &v)
	{
		(*this)[0] = v[0];
		(*this)[1] = v[1];
		return *this;
	}

	inline Mat2 Mat2::operator+(const Mat2 &v)
	{
		Mat2 ret(*this);
		ret += v;
		return ret;
	}

	inline Mat2 Mat2::operator-(const Mat2 &v)
	{
		Mat2 ret(*this);
		ret -= v;
		return ret;
	}

	inline Mat2 Mat2::operator*(const Mat2 &v)
	{
		Mat2 ret;
		ret[0][0] = (*this)[0][0] * v[0][0] + (*this)[1][0] * v[0][1];
		ret[0][1] = (*this)[0][1] * v[0][0] + (*this)[1][1] * v[0][1];
		ret[1][0] = (*this)[0][0] * v[1][0] + (*this)[1][0] * v[1][1];
		ret[1][1] = (*this)[0][1] * v[1][0] + (*this)[1][1] * v[1][1];
		return ret;
	}

	inline Vec2 Mat2::operator*(const Vec2 &v)
	{
		Vec2 ret;
		ret.x = (*this)[0][0] * v.x + (*this)[1][0] * v.y;
		ret.y = (*this)[0][1] * v.x + (*this)[1][1] * v.y;
		return ret;
	}

	inline Mat2 Mat2::operator/(const Mat2 &v)
	{
		Mat2 ret(*this);
		ret /= v;
		return ret;
	}

	inline Mat2 &Mat2::operator+=(const Mat2 &v)
	{
		(*this)[0] += v[0];
		(*this)[1] += v[1];
		return *this;
	}

	inline Mat2 &Mat2::operator-=(const Mat2 &v)
	{
		(*this)[0] -= v[0];
		(*this)[1] -= v[1];
		return *this;
	}

	inline Mat2 &Mat2::operator*=(const Mat2 &v)
	{
		return (*this = *this * v);
	}

	inline Mat2 &Mat2::operator/=(const Mat2 &v)
	{
		return *this *= v.get_inverse();
	}

	inline Mat2 &Mat2::operator+=(float v)
	{
		(*this)[0] += v;
		(*this)[1] += v;
		return *this;
	}

	inline Mat2 &Mat2::operator-=(float v)
	{
		(*this)[0] -= v;
		(*this)[1] -= v;
		return *this;
	}

	inline Mat2 &Mat2::operator*=(float v)
	{
		(*this)[0] *= v;
		(*this)[1] *= v;
		return *this;
	}

	inline Mat2 &Mat2::operator/=(float v)
	{
		(*this)[0] /= v;
		(*this)[1] /= v;
		return *this;
	}

	inline void Mat2::transpose()
	{
		*this = get_transpose();
	}

	inline Mat2 Mat2::get_transpose() const
	{
		Mat2 ret;
		ret[0][0] = (*this)[0][0];
		ret[0][1] = (*this)[1][0];
		ret[1][0] = (*this)[0][1];
		ret[1][1] = (*this)[1][1];
		return ret;
	}

	inline void Mat2::inverse()
	{
		*this = get_inverse();
	}

	inline Mat2 Mat2::get_inverse() const
	{
		float det_inv = 1.f / (
			(*this)[0][0] * (*this)[1][1] - 
			(*this)[1][0] * (*this)[0][1]);

		Mat2 ret(
			+(*this)[1][1] * det_inv,
			-(*this)[0][1] * det_inv,
			-(*this)[1][0] * det_inv,
			+(*this)[0][0] * det_inv);

		return ret;
	}

	inline Mat2 operator+(const Mat2 &lhs, float rhs)
	{
		Mat2 ret(lhs);
		ret[0] += rhs;
		ret[1] += rhs;
		return ret;
	}

	inline Mat2 operator-(const Mat2 &lhs, float rhs)
	{
		Mat2 ret(lhs);
		ret[0] -= rhs;
		ret[1] -= rhs;
		return ret;
	}

	inline Mat2 operator*(const Mat2 &lhs, float rhs)
	{
		Mat2 ret(lhs);
		ret[0] *= rhs;
		ret[1] *= rhs;
		return ret;
	}

	inline Mat2 operator/(const Mat2 &lhs, float rhs)
	{
		Mat2 ret(lhs);
		ret[0] /= rhs;
		ret[1] /= rhs;
		return ret;
	}

	inline Mat2 operator+(float lhs, const Mat2 &rhs)
	{
		Mat2 ret(rhs);
		ret[0] += lhs;
		ret[1] += lhs;
		return ret;
	}

	inline Mat2 operator-(float lhs, const Mat2 &rhs)
	{
		Mat2 ret(rhs);
		ret[0] -= lhs;
		ret[1] -= lhs;
		return ret;
	}

	inline Mat2 operator*(float lhs, const Mat2 &rhs)
	{
		Mat2 ret(rhs);
		ret[0] *= lhs;
		ret[1] *= lhs;
		return ret;
	}

	inline Mat2 operator/(float lhs, const Mat2 &rhs)
	{
		Mat2 ret(rhs);
		ret[0] /= lhs;
		ret[1] /= lhs;
		return ret;
	}

	Mat3::Mat3()
	{
	}

	Mat3::Mat3(float diagonal)
	{
		(*this)[0][0] = diagonal;  (*this)[1][0] = 0.f;      (*this)[2][0] = 0.f;
		(*this)[0][1] = 0.f;       (*this)[1][1] = diagonal; (*this)[2][1] = 0.f;
		(*this)[0][2] = 0.f;       (*this)[1][2] = 0.f;      (*this)[2][2] = diagonal;
	}

	Mat3::Mat3(float Xx, float Xy, float Xz,
		float Yx, float Yy, float Yz,
		float Zx, float Zy, float Zz)
	{
		(*this)[0][0] = Xx; (*this)[1][0] = Yx; (*this)[2][0] = Zx;
		(*this)[0][1] = Xy; (*this)[1][1] = Yy; (*this)[2][1] = Zy;
		(*this)[0][2] = Xz; (*this)[1][2] = Yz; (*this)[2][2] = Zz;
	}

	inline Mat3::Mat3(const Vec3 &v0, const Vec3 &v1, const Vec3 &v2)
	{
		(*this)[0] = v0;
		(*this)[1] = v1;
		(*this)[2] = v2;
	}

	inline Mat3::Mat3(const Mat3 &v)
	{
		(*this)[0] = v[0];
		(*this)[1] = v[1];
		(*this)[2] = v[2];
	}

	inline Mat3::Mat3(const Mat4 &v)
	{
		(*this)[0] = v[0];
		(*this)[1] = v[1];
		(*this)[2] = v[2];
	}

	inline Mat3 &Mat3::operator[](int i)
	{
		return (*this)[i];
	}

	inline Mat3 const&Mat3::operator[](int i) const
	{
		return (*this)[i];
	}

	inline Mat3 &Mat3::operator=(const Mat3 &v)
	{
		(*this)[0] = v[0];
		(*this)[1] = v[1];
		(*this)[2] = v[2];
		return *this;
	}

	inline Mat3 &Mat3::operator=(const Mat4 &v)
	{
		(*this)[0] = v[0];
		(*this)[1] = v[1];
		(*this)[2] = v[2];
		return *this;
	}

	inline bool Rect::contains(const Vec2 &p)
	{
		return p.x > min.x && p.x < max.x &&
			p.y > min.y && p.y < max.y;
	}

	inline Rect::Side Rect::calc_side(const Vec2 &p, float threshold)
	{
		if (p.x < max.x + threshold && p.x > max.x &&
			p.y > min.y - threshold && p.y < min.y)
			return SideNE;
		if (p.x > min.x - threshold && p.x < min.x &&
			p.y > min.y - threshold && p.y < min.y)
			return SideNW;
		if (p.x < max.x + threshold && p.x > max.x &&
			p.y < max.y + threshold && p.y > max.y)
			return SideSE;
		if (p.x > min.x - threshold && p.x < min.x &&
			p.y < max.y + threshold && p.y > max.y)
			return SideSW;
		if (p.y > min.y - threshold && p.y < min.y &&
			p.x > min.x && p.x < max.x)
			return SideN;
		if (p.y < max.y + threshold && p.y > max.y &&
			p.x > min.x && p.x < max.x)
			return SideS;
		if (p.x < max.x + threshold && p.x > max.x &&
			p.y > min.y && p.y < max.y)
			return SideE;
		if (p.x > min.x - threshold && p.x < min.x &&
			p.y > min.y && p.y < max.y)
			return SideW;
		if (contains(p))
			return Inside;
		return Outside;
	}

	//inline glm::vec3 transform(const glm::vec3 &v, const glm::mat4 &mat)
	//{
	//	auto v_ = mat * glm::vec4(v, 1.f);
	//	v_ /= v_.w;
	//	return glm::vec3(v_);
	//}

	//inline glm::vec4 plane(const glm::vec3 &p, const glm::vec3 &normal)
	//{
	//	return glm::vec4(normal, glm::dot(normal, p));
	//}

	//inline void expand_rect(glm::vec4 &rect, float length)
	//{
	//	rect.x -= length;
	//	rect.y -= length;
	//	rect.z += length;
	//	rect.w += length;
	//}

	//inline glm::vec4 get_expand_rect(glm::vec4 &rect, float length)
	//{
	//	glm::vec4 new_rect;
	//	new_rect.x = rect.x - length;
	//	new_rect.y = rect.y - length;
	//	new_rect.z = rect.z + length;
	//	new_rect.w = rect.w + length;
	//	return new_rect;
	//}

	//inline void ortho_normalize(glm::mat3 &mat)
	//{
	//	for (auto i = 0; i < 3; i++)
	//		mat[i] = glm::normalize(mat[i]);
	//}

	//inline void ortho_normalize(glm::mat4 &mat)
	//{
	//	for (auto i = 0; i < 3; i++)
	//		mat[i] = glm::vec4(glm::normalize(glm::vec3(mat[i])), 0.f);
	//}

	//FLAME_MATH_EXPORTS float ray_intersect_plane(const glm::vec3 &origin, const glm::vec3 &vector, const glm::vec4 &plane);

	//FLAME_MATH_EXPORTS glm::mat3 quat_to_mat3(glm::vec4 &q);

	//FLAME_MATH_EXPORTS glm::vec4 mat3_to_quat(glm::mat3 &mat);

	//FLAME_MATH_EXPORTS glm::vec3 quat_to_euler(glm::vec4 &q);

	//FLAME_MATH_EXPORTS void quat_rotate(glm::vec4 &q, glm::vec3 &v);

	//FLAME_MATH_EXPORTS glm::mat3 euler_to_mat3(glm::vec3 &e);

	//inline glm::mat3 euler_to_mat3(float x, float y, float z)
	//{
	//	return euler_to_mat3(glm::vec3(x, y, z));
	//}

	//inline glm::mat4 make_matrix(const glm::mat3 &rotation, const glm::vec3 coord)
	//{
	//	return glm::mat4(
	//		glm::vec4(rotation[0], 0.f),
	//		glm::vec4(rotation[1], 0.f),
	//		glm::vec4(rotation[2], 0.f),
	//		glm::vec4(coord, 1.f)
	//	);
	//}

	//inline glm::mat4 make_matrix(const glm::vec3 &x, const glm::vec3 &y, const glm::vec3 coord)
	//{
	//	return glm::mat4(
	//		glm::vec4(x, 0.f),
	//		glm::vec4(y, 0.f),
	//		glm::vec4(glm::cross(x, y), 0.f),
	//		glm::vec4(coord, 1.f)
	//	);
	//}

	//FLAME_MATH_EXPORTS float rand2d(const glm::vec2 &v);
	//FLAME_MATH_EXPORTS float noise2d(glm::vec2 v);
	//FLAME_MATH_EXPORTS float fbm2d(glm::vec2 v);

	//FLAME_MATH_EXPORTS glm::vec4 fit_rect(const glm::vec2 &desired_size, float xy_aspect);
	//FLAME_MATH_EXPORTS glm::vec4 fit_rect_no_zoom_in(const glm::vec2 &desired_size, const glm::vec2 &size);
}
