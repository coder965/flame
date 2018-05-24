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

#include <math.h>

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
	const float RAD_ANG     = 180.f / PI;  // rad to angle
	const float ANG_RAD     = PI / 180.f;  // angle to rad
	const float SQRTPI_INV2 = 1.1283791f;  // 2/sqrt(pi)
	const float SQRT2       = 1.4142135f;  // sqrt(2)
	const float SQRT1_2     = 0.7071067f;  // 1/sqrt(2)
	// == 

	const float EPS = 0.000001f;

	inline float get_inf()
	{
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

	inline float fract(float v)
	{
		return v - floor(v);
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

	Ivec2 mod(int a, int b);
	float mix(float v0, float v1, float q);

	float dot(const Vec2 &lhs, const Vec2 &rhs);
	float dot(const Vec3 &lhs, const Vec3 &rhs);
	float dot(const Vec4 &lhs, const Vec4 &rhs);

	Vec3 cross(const Vec3 &lhs, const Vec3 &rhs);

	Rect get_fit_rect(const Vec2 &desired_size, float xy_aspect);
	Rect get_fit_rect_no_zoom_in(const Vec2 &desired_size, const Vec2 &size);

	float rand(const Vec2 &v);
	float noise(const Vec2 &v);
	float fbm(const Vec2 &v);

	struct Vec2
	{
		float x;
		float y;

		Vec2();

		explicit Vec2(float v);
		Vec2(float _x, float _y);
		Vec2(const Vec2 &v);
		explicit Vec2(const Vec3 &v);
		explicit Vec2(const Vec4 &v);
		explicit Vec2(const Ivec2 &v);
		explicit Vec2(const Ivec3 &v);
		explicit Vec2(const Ivec4 &v);
		float &operator[](int i);
		float const&operator[](int i) const;
		Vec2 &operator=(const Vec2 &v);
		Vec2 &operator=(const Vec3 &v);
		Vec2 &operator=(const Vec4 &v);
		Vec2 &operator=(const Ivec2 &v);
		Vec2 &operator=(const Ivec3 &v);
		Vec2 &operator=(const Ivec4 &v);
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

	bool operator==(const Vec2 &lhs, const Vec2 &rhs);
	bool operator!=(const Vec2 &lhs, const Vec2 &rhs);
	Vec2 operator+(const Vec2 &lhs, const Vec2 &rhs);
	Vec2 operator-(const Vec2 &lhs, const Vec2 &rhs);
	Vec2 operator*(const Vec2 &lhs, const Vec2 &rhs);
	Vec2 operator/(const Vec2 &lhs, const Vec2 &rhs);
	Vec2 operator+(const Vec2 &lhs, const Ivec2 &rhs);
	Vec2 operator-(const Vec2 &lhs, const Ivec2 &rhs);
	Vec2 operator*(const Vec2 &lhs, const Ivec2 &rhs);
	Vec2 operator/(const Vec2 &lhs, const Ivec2 &rhs);
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
		explicit Vec3(float v);
		Vec3(float _x, float _y, float _z);
		Vec3(const Vec2 &v, float _z);
		Vec3(const Vec3 &v);
		explicit Vec3(const Vec4 &v);
		Vec3(const Ivec2 &v, float _z);
		explicit Vec3(const Ivec3 &v);
		explicit Vec3(const Ivec4 &v);
		float &operator[](int i);
		float const&operator[](int i) const;
		Vec3 &operator=(const Vec3 &v);
		Vec3 &operator=(const Vec4 &v);
		Vec3 &operator=(const Ivec3 &v);
		Vec3 &operator=(const Ivec4 &v);
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

	bool operator==(const Vec3 &lhs, const Vec3 &rhs);
	bool operator!=(const Vec3 &lhs, const Vec3 &rhs);
	Vec3 operator+(const Vec3 &lhs, const Vec3 &rhs);
	Vec3 operator-(const Vec3 &lhs, const Vec3 &rhs);
	Vec3 operator*(const Vec3 &lhs, const Vec3 &rhs);
	Vec3 operator/(const Vec3 &lhs, const Vec3 &rhs);
	Vec3 operator+(const Vec3 &lhs, const Ivec3 &rhs);
	Vec3 operator-(const Vec3 &lhs, const Ivec3 &rhs);
	Vec3 operator*(const Vec3 &lhs, const Ivec3 &rhs);
	Vec3 operator/(const Vec3 &lhs, const Ivec3 &rhs);
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
		explicit Vec4(float v);
		Vec4(float _x, float _y, float _z, float _w);
		Vec4(const Vec2 &v, float _z, float _w);
		Vec4(const Vec3 &v, float _w);
		Vec4(const Vec4 &v);
		Vec4(const Ivec2 &v, float _z, float _w);
		Vec4(const Ivec3 &v, float _w);
		explicit Vec4(const Ivec4 &v);
		float &operator[](int i);
		float const&operator[](int i) const;
		Vec4 &operator=(const Vec4 &v);
		Vec4 &operator=(const Ivec4 &v);
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

	bool operator==(const Vec4 &lhs, const Vec4 &rhs);
	bool operator!=(const Vec4 &lhs, const Vec4 &rhs);
	Vec4 operator+(const Vec4 &lhs, const Vec4 &rhs);
	Vec4 operator-(const Vec4 &lhs, const Vec4 &rhs);
	Vec4 operator*(const Vec4 &lhs, const Vec4 &rhs);
	Vec4 operator/(const Vec4 &lhs, const Vec4 &rhs);
	Vec4 operator+(const Vec4 &lhs, const Ivec4 &rhs);
	Vec4 operator-(const Vec4 &lhs, const Ivec4 &rhs);
	Vec4 operator*(const Vec4 &lhs, const Ivec4 &rhs);
	Vec4 operator/(const Vec4 &lhs, const Ivec4 &rhs);
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
		explicit Ivec2(int v);
		Ivec2(int _x, int _y);
		explicit Ivec2(const Vec2 &v);
		explicit Ivec2(const Vec3 &v);
		explicit Ivec2(const Vec4 &v);
		Ivec2(const Ivec2 &v);
		explicit Ivec2(const Ivec3 &v);
		explicit Ivec2(const Ivec4 &v);
		int &operator[](int i);
		int const&operator[](int i) const;
		Ivec2 &operator=(const Vec2 &v);
		Ivec2 &operator=(const Vec3 &v);
		Ivec2 &operator=(const Vec4 &v);
		Ivec2 &operator=(const Ivec2 &v);
		Ivec2 &operator=(const Ivec3 &v);
		Ivec2 &operator=(const Ivec4 &v);
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

	bool operator==(const Ivec2 &lhs, const Ivec2 &rhs);
	bool operator!=(const Ivec2 &lhs, const Ivec2 &rhs);
	Ivec2 operator+(const Ivec2 &lhs, const Vec2 &rhs);
	Ivec2 operator-(const Ivec2 &lhs, const Vec2 &rhs);
	Ivec2 operator*(const Ivec2 &lhs, const Vec2 &rhs);
	Ivec2 operator/(const Ivec2 &lhs, const Vec2 &rhs);
	Ivec2 operator+(const Ivec2 &lhs, const Ivec2 &rhs);
	Ivec2 operator-(const Ivec2 &lhs, const Ivec2 &rhs);
	Ivec2 operator*(const Ivec2 &lhs, const Ivec2 &rhs);
	Ivec2 operator/(const Ivec2 &lhs, const Ivec2 &rhs);
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
		explicit Ivec3(int v);
		Ivec3(int _x, int _y, int _z);
		Ivec3(const Vec2 &v, int _z);
		explicit Ivec3(const Vec3 &v);
		explicit Ivec3(const Vec4 &v);
		Ivec3(const Ivec2 &v, int _z);
		Ivec3(const Ivec3 &v);
		explicit Ivec3(const Ivec4 &v);
		int &operator[](int i);
		int const&operator[](int i) const;
		Ivec3 &operator=(const Vec3 &v);
		Ivec3 &operator=(const Vec4 &v);
		Ivec3 &operator=(const Ivec3 &v);
		Ivec3 &operator=(const Ivec4 &v);
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

	bool operator==(const Ivec3 &lhs, const Ivec3 &rhs);
	bool operator!=(const Ivec3 &lhs, const Ivec3 &rhs);
	Ivec3 operator+(const Ivec3 &lhs, const Vec3 &rhs);
	Ivec3 operator-(const Ivec3 &lhs, const Vec3 &rhs);
	Ivec3 operator*(const Ivec3 &lhs, const Vec3 &rhs);
	Ivec3 operator/(const Ivec3 &lhs, const Vec3 &rhs);
	Ivec3 operator+(const Ivec3 &lhs, const Ivec3 &rhs);
	Ivec3 operator-(const Ivec3 &lhs, const Ivec3 &rhs);
	Ivec3 operator*(const Ivec3 &lhs, const Ivec3 &rhs);
	Ivec3 operator/(const Ivec3 &lhs, const Ivec3 &rhs);
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
		explicit Ivec4(int v);
		Ivec4(int _x, int _y, int _z, int _w);
		Ivec4(const Vec2 &v, int _z, int _w);
		Ivec4(const Vec3 &v, int _w);
		explicit Ivec4(const Vec4 &v);
		Ivec4(const Ivec2 &v, int _z, int _w);
		Ivec4(const Ivec3 &v, int _w);
		Ivec4(const Ivec4 &v);
		int &operator[](int i);
		int const&operator[](int i) const;
		Ivec4 &operator=(const Vec4 &v);
		Ivec4 &operator=(const Ivec4 &v);
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

	bool operator==(const Ivec4 &lhs, const Ivec4 &rhs);
	bool operator!=(const Ivec4 &lhs, const Ivec4 &rhs);
	Ivec4 operator+(const Ivec4 &lhs, const Vec4 &rhs);
	Ivec4 operator-(const Ivec4 &lhs, const Vec4 &rhs);
	Ivec4 operator*(const Ivec4 &lhs, const Vec4 &rhs);
	Ivec4 operator/(const Ivec4 &lhs, const Vec4 &rhs);
	Ivec4 operator+(const Ivec4 &lhs, const Ivec4 &rhs);
	Ivec4 operator-(const Ivec4 &lhs, const Ivec4 &rhs);
	Ivec4 operator*(const Ivec4 &lhs, const Ivec4 &rhs);
	Ivec4 operator/(const Ivec4 &lhs, const Ivec4 &rhs);
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
		explicit Mat2(float diagonal);
		Mat2(float Xx, float Xy, 
			float Yx, float Yy);
		Mat2(const Vec2 &v0, const Vec2 &v1);
		Mat2(const Mat2 &v);
		explicit Mat2(const Mat3 &v);
		explicit Mat2(const Mat4 &v);
		Vec2 &operator[](int i);
		Vec2 const &operator[](int i) const;
		Mat2 &operator=(const Mat2 &v);
		Mat2 &operator=(const Mat3 &v);
		Mat2 &operator=(const Mat4 &v);
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

	Mat2 operator+(const Mat2 &lhs, const Mat2 &rhs);
	Mat2 operator-(const Mat2 &lhs, const Mat2 &rhs);
	Mat2 operator*(const Mat2 &lhs, const Mat2 &rhs);
	Vec2 operator*(const Mat2 &lhs, const Vec2 &rhs);
	Mat2 operator/(const Mat2 &lhs, const Mat2 &rhs);
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
		explicit Mat3(float diagonal /* scale */);
		Mat3(float Xx, float Xy, float Xz,
			float Yx, float Yy, float Yz,
			float Zx, float Zy, float Zz);
		Mat3(const Vec3 &v0, const Vec3 &v1, const Vec3 &v2);
		explicit Mat3(const EulerYawPitchRoll &e);
		explicit Mat3(const Quat &q);
		Mat3(const Vec3 &axis, float rad);
		Mat3(const Mat3 &v);
		explicit Mat3(const Mat4 &v);
		Vec3 &operator[](int i);
		Vec3 const &operator[](int i) const;
		Mat3 &operator=(const EulerYawPitchRoll &e);
		Mat3 &operator=(const Quat &q);
		Mat3 &operator=(const Mat3 &v);
		Mat3 &operator=(const Mat4 &v);
		Mat3 &operator+=(const Mat3 &v);
		Mat3 &operator-=(const Mat3 &v);
		Mat3 &operator*=(const Mat3 &v);
		Mat3 &operator/=(const Mat3 &v);
		Mat3 &operator+=(float v);
		Mat3 &operator-=(float v);
		Mat3 &operator*=(float v);
		Mat3 &operator/=(float v);
		void normalize();
		void transpose();
		Mat3 get_transpose() const;
		void inverse();
		Mat3 get_inverse() const;
	};

	Mat3 operator+(const Mat3 &lhs, const Mat3 &rhs);
	Mat3 operator-(const Mat3 &lhs, const Mat3 &rhs);
	Mat3 operator*(const Mat3 &lhs, const Mat3 &rhs);
	Vec3 operator*(const Mat3 &lhs, const Vec3 &rhs);
	Mat3 operator/(const Mat3 &lhs, const Mat3 &rhs);
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
		explicit Mat4(float diagonal);
		Mat4(float Xx, float Xy, float Xz, float Xw,
			float Yx, float Yy, float Yz, float Yw,
			float Zx, float Zy, float Zz, float Zw,
			float Wx, float Wy, float Wz, float Ww);
		Mat4(const Vec4 &v0, const Vec4 &v1, const Vec4 &v2, const Vec4 &v3);
		Mat4(const Mat4 &v);
		Mat4(const Mat3 &mat3, const Vec3 &coord);
		Mat4(const Vec3 &x_axis, const Vec3 &y_axis, const Vec3 &coord);
		Vec4 &operator[](int i);
		Vec4 const &operator[](int i) const;
		Mat4 &operator=(const Mat4 &v);
		Mat4 &operator+=(const Mat4 &v);
		Mat4 &operator-=(const Mat4 &v);
		Mat4 &operator*=(const Mat4 &v);
		Mat4 &operator/=(const Mat4 &v);
		Mat4 &operator+=(float v);
		Mat4 &operator-=(float v);
		Mat4 &operator*=(float v);
		Mat4 &operator/=(float v);
		void normalize_33();
		void transpose();
		Mat4 get_transpose() const;
		void inverse();
		Mat4 get_inverse() const;
	};

	Mat4 operator+(const Mat4 &lhs, const Mat4 &rhs);
	Mat4 operator-(const Mat4 &lhs, const Mat4 &rhs);
	Mat4 operator*(const Mat4 &lhs, const Mat4 &rhs);
	Vec4 operator*(const Mat4 &lhs, const Vec4 &rhs);
	Vec3 operator*(const Mat4 &lhs, const Vec3 &rhs);
	Mat4 operator/(const Mat4 &lhs, const Mat4 &rhs);
	Mat4 operator+(const Mat4 &lhs, float rhs);
	Mat4 operator-(const Mat4 &lhs, float rhs);
	Mat4 operator*(const Mat4 &lhs, float rhs);
	Mat4 operator/(const Mat4 &lhs, float rhs);
	Mat4 operator+(float lhs, const Mat4 &rhs);
	Mat4 operator-(float lhs, const Mat4 &rhs);
	Mat4 operator*(float lhs, const Mat4 &rhs);
	Mat4 operator/(float lhs, const Mat4 &rhs);

	// in angle
	struct EulerYawPitchRoll
	{
		float yaw;
		float pitch;
		float roll;

		EulerYawPitchRoll();
		explicit EulerYawPitchRoll(const Quat &q);
	};

	struct Quat
	{
		float x;
		float y;
		float z;
		float w;

		Quat();
		Quat(float _x, float _y, float _z, float _w);
		void normalize();
	};
	
	Quat operator*(const Quat &lhs, const Quat &rhs);

	struct Rect
	{
		Vec2 min;
		Vec2 max;

		enum Side
		{
			SideN,
			SideS,
			SideW,
			SideE,
			SideNW,
			SideNE,
			SideSW,
			SideSE,
			Outside,
			Inside
		};

		Rect();
		Rect(const Vec2 &_min, const Vec2 &_max);
		Rect(float min_x, float min_y, float max_x, float max_y);
		Rect(const Rect &v);
		Rect &operator+=(const Vec2 &v);
		Rect &operator-=(const Vec2 &v);
		float width() const;
		float height() const;
		void expand(float length);
		Rect get_expanded(float length);
		bool contains(const Vec2 &p);
		Side calc_side(const Vec2 &p, float threshold);
	};

	Vec2 get_side_dir();

	Rect operator+(const Rect &r, const Vec2 &off);
	Rect operator-(const Rect &r, const Vec2 &off);

	struct Plane 
	{
		Vec3 normal;
		float d;

		Plane();
		Plane(const Vec3 &n, float _d);
		Plane(const Vec3 &n, const Vec3 &p);
		float intersect(const Vec3 &origin, const Vec3 &dir);
	};

	namespace math_detail
	{
		void rotate(const Vec3 &axis, float rad, Mat3 &m);
		void mat3_to_quat(const Mat3 &m, Quat &q);
		void euler_to_mat3(const EulerYawPitchRoll &e, Mat3 &m);
		void quat_to_euler(const Quat &q, EulerYawPitchRoll &e);
		void quat_to_mat3(const Quat &q, Mat3 &m);
	}

	inline Ivec2 mod(int a, int b)
	{
		return Ivec2(a / b, a % b);
	}

	inline float mix(float v0, float v1, float q)
	{
		return v0 + q * (v1 - v0);
	}

	inline float dot(const Vec2 &lhs, const Vec2 &rhs)
	{
		return lhs.x * rhs.x + lhs.y * rhs.y;
	}

	inline float dot(const Vec3 &lhs, const Vec3 &rhs)
	{
		return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
	}

	inline float dot(const Vec4 &lhs, const Vec4 &rhs)
	{
		return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
	}

	inline float rand(const Vec2 &v)
	{
		return fract(cos(v.x * (12.9898) + v.y * (4.1414)) * 43758.5453);
	}

	inline Rect get_fit_rect(const Vec2 &desired_size, float xy_aspect)
	{
		if (desired_size.x <= 0.f || desired_size.y <= 0.f)
			return Rect(0.f, 0.f, 1.f, 1.f);
		Rect ret;
		if (desired_size.x / desired_size.y > xy_aspect)
		{
			ret.max.x = xy_aspect * desired_size.y;
			ret.max.y = desired_size.y;
			ret.min.x = (desired_size.x - ret.max.x) * 0.5f;
			ret.min.y = 0;
			ret.max += ret.min;
		}
		else
		{
			ret.max.x = desired_size.x;
			ret.max.y = desired_size.x / xy_aspect;
			ret.min.x = 0;
			ret.min.y = (desired_size.y - ret.max.y) * 0.5f;
			ret.max += ret.min;
		}
		return ret;
	}

	inline Rect get_fit_rect_no_zoom_in(const Vec2 &desired_size, const Vec2 &size)
	{
		if (desired_size.x <= 0.f || desired_size.y <= 0.f)
			return Rect(0.f, 0.f, 1.f, 1.f);
		if (size.x <= desired_size.x && size.y <= desired_size.y)
		{
			Rect ret;
			ret.max.x = size.x;
			ret.max.y = size.y;
			ret.min.x = (desired_size.x - size.x) * 0.5f;
			ret.min.y = (desired_size.y - size.y) * 0.5f;
			ret.max += ret.min;
			return ret;
		}
		else
			return get_fit_rect(desired_size, size.x / size.y);
	}

	inline float noise(const Vec2 &_v)
	{
		const auto SC = 250;

		auto v = _v / SC;
		Vec2 vf(fract(v.x), fract(v.y));
		Vec2 vi(floor(v.x), floor(v.y));

		auto r0 = rand(vi);
		auto r1 = rand(vi + Vec2(1.f, 0.f));
		auto r2 = rand(vi + Vec2(0.f, 1.f));
		auto r3 = rand(vi + Vec2(1.f, 1.f));

		auto vs = 3.f * vf * vf - 2.f * vf * vf * vf;

		return mix(
			mix(r0, r1, vs.x),
			mix(r2, r3, vs.x),
			vs.y);
	}

	inline float fbm(const Vec2 &_v)
	{
		auto v = _v;
		auto r = 0.f;

		auto a = 1.f / 3.f;
		for (auto i = 0; i < 4; i++)
		{
			r += noise(v) * a;
			a /= 3.f;
			v *= 3.f;
		}

		return r;
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

	inline bool operator==(const Vec2 &lhs, const Vec2 &rhs)
	{
		return equals(lhs.x, rhs.x) && equals(lhs.y, rhs.y);
	}

	inline bool operator!=(const Vec2 &lhs, const Vec2 &rhs)
	{
		return !(lhs == rhs);
	}

	inline Vec2 operator+(const Vec2 &lhs, const Vec2 &rhs)
	{
		Vec2 ret(lhs);
		ret += rhs;
		return ret;
	}

	inline Vec2 operator-(const Vec2 &lhs, const Vec2 &rhs)
	{
		Vec2 ret(lhs);
		ret -= rhs;
		return ret;
	}

	inline Vec2 operator*(const Vec2 &lhs, const Vec2 &rhs)
	{
		Vec2 ret(lhs);
		ret *= rhs;
		return ret;
	}

	inline Vec2 operator/(const Vec2 &lhs, const Vec2 &rhs)
	{
		Vec2 ret(lhs);
		ret /= rhs;
		return ret;
	}

	inline Vec2 operator+(const Vec2 &lhs, const Ivec2 &rhs)
	{
		Vec2 ret(lhs);
		ret += rhs;
		return ret;
	}

	inline Vec2 operator-(const Vec2 &lhs, const Ivec2 &rhs)
	{
		Vec2 ret(lhs);
		ret -= rhs;
		return ret;
	}

	inline Vec2 operator*(const Vec2 &lhs, const Ivec2 &rhs)
	{
		Vec2 ret(lhs);
		ret *= rhs;
		return ret;
	}

	inline Vec2 operator/(const Vec2 &lhs, const Ivec2 &rhs)
	{
		Vec2 ret(lhs);
		ret /= rhs;
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

	inline bool operator==(const Vec3 &lhs, const Vec3 &rhs)
	{
		return equals(lhs.x, rhs.x) && equals(lhs.y, rhs.y) && equals(lhs.z, rhs.z);
	}

	inline bool operator!=(const Vec3 &lhs, const Vec3 &rhs)
	{
		return !(lhs == rhs);
	}

	inline Vec3 operator+(const Vec3 &lhs, const Vec3 &rhs)
	{
		Vec3 ret(lhs);
		ret += rhs;
		return ret;
	}

	inline Vec3 operator-(const Vec3 &lhs, const Vec3 &rhs)
	{
		Vec3 ret(lhs);
		ret -= rhs;
		return ret;
	}

	inline Vec3 operator*(const Vec3 &lhs, const Vec3 &rhs)
	{
		Vec3 ret(lhs);
		ret *= rhs;
		return ret;
	}

	inline Vec3 operator/(const Vec3 &lhs, const Vec3 &rhs)
	{
		Vec3 ret(lhs);
		ret /= rhs;
		return ret;
	}

	inline Vec3 operator+(const Vec3 &lhs, const Ivec3 &rhs)
	{
		Vec3 ret(lhs);
		ret += rhs;
		return ret;
	}

	inline Vec3 operator-(const Vec3 &lhs, const Ivec3 &rhs)
	{
		Vec3 ret(lhs);
		ret -= rhs;
		return ret;
	}

	inline Vec3 operator*(const Vec3 &lhs, const Ivec3 &rhs)
	{
		Vec3 ret(lhs);
		ret *= rhs;
		return ret;
	}

	inline Vec3 operator/(const Vec3 &lhs, const Ivec3 &rhs)
	{
		Vec3 ret(lhs);
		ret /= rhs;
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

	inline bool operator==(const Vec4 &lhs, const Vec4 &rhs)
	{
		return equals(lhs.x, rhs.x) && equals(lhs.y, rhs.y) && equals(lhs.z, rhs.z) && equals(lhs.w, rhs.w);
	}

	inline bool operator!=(const Vec4 &lhs, const Vec4 &rhs)
	{
		return !(lhs == rhs);
	}

	inline Vec4 operator+(const Vec4 &lhs, const Vec4 &rhs)
	{
		Vec4 ret(lhs);
		ret += rhs;
		return ret;
	}

	inline Vec4 operator-(const Vec4 &lhs, const Vec4 &rhs)
	{
		Vec4 ret(lhs);
		ret -= rhs;
		return ret;
	}

	inline Vec4 operator*(const Vec4 &lhs, const Vec4 &rhs)
	{
		Vec4 ret(lhs);
		ret *= rhs;
		return ret;
	}

	inline Vec4 operator/(const Vec4 &lhs, const Vec4 &rhs)
	{
		Vec4 ret(lhs);
		ret /= rhs;
		return ret;
	}

	inline Vec4 operator+(const Vec4 &lhs, const Ivec4 &rhs)
	{
		Vec4 ret(lhs);
		ret += rhs;
		return ret;
	}

	inline Vec4 operator-(const Vec4 &lhs, const Ivec4 &rhs)
	{
		Vec4 ret(lhs);
		ret -= rhs;
		return ret;
	}

	inline Vec4 operator*(const Vec4 &lhs, const Ivec4 &rhs)
	{
		Vec4 ret(lhs);
		ret *= rhs;
		return ret;
	}

	inline Vec4 operator/(const Vec4 &lhs, const Ivec4 &rhs)
	{
		Vec4 ret(lhs);
		ret /= rhs;
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

	inline bool operator==(const Ivec2 &lhs, const Ivec2 &rhs)
	{
		return lhs.x == rhs.x && lhs.y == rhs.y;
	}

	inline bool operator!=(const Ivec2 &lhs, const Ivec2 &rhs)
	{
		return !(lhs == rhs);
	}

	inline Ivec2 operator+(const Ivec2 &lhs, const Vec2 &rhs)
	{
		Ivec2 ret(lhs);
		ret += rhs;
		return ret;
	}

	inline Ivec2 operator-(const Ivec2 &lhs, const Vec2 &rhs)
	{
		Ivec2 ret(lhs);
		ret -= rhs;
		return ret;
	}

	inline Ivec2 operator*(const Ivec2 &lhs, const Vec2 &rhs)
	{
		Ivec2 ret(lhs);
		ret *= rhs;
		return ret;
	}

	inline Ivec2 operator/(const Ivec2 &lhs, const Vec2 &rhs)
	{
		Ivec2 ret(lhs);
		ret /= rhs;
		return ret;
	}

	inline Ivec2 operator+(const Ivec2 &lhs, const Ivec2 &rhs)
	{
		Ivec2 ret(lhs);
		ret += rhs;
		return ret;
	}

	inline Ivec2 operator-(const Ivec2 &lhs, const Ivec2 &rhs)
	{
		Ivec2 ret(lhs);
		ret -= rhs;
		return ret;
	}

	inline Ivec2 operator*(const Ivec2 &lhs, const Ivec2 &rhs)
	{
		Ivec2 ret(lhs);
		ret *= rhs;
		return ret;
	}

	inline Ivec2 operator/(const Ivec2 &lhs, const Ivec2 &rhs)
	{
		Ivec2 ret(lhs);
		ret /= rhs;
		return ret;
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

	inline bool operator==(const Ivec3 &lhs, const Ivec3 &rhs)
	{
		return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
	}

	inline bool operator!=(const Ivec3 &lhs, const Ivec3 &rhs)
	{
		return !(lhs == rhs);
	}

	inline Ivec3 operator+(const Ivec3 &lhs, const Vec3 &rhs)
	{
		Ivec3 ret(lhs);
		ret += rhs;
		return ret;
	}

	inline Ivec3 operator-(const Ivec3 &lhs, const Vec3 &rhs)
	{
		Ivec3 ret(lhs);
		ret -= rhs;
		return ret;
	}

	inline Ivec3 operator*(const Ivec3 &lhs, const Vec3 &rhs)
	{
		Ivec3 ret(lhs);
		ret *= rhs;
		return ret;
	}

	inline Ivec3 operator/(const Ivec3 &lhs, const Vec3 &rhs)
	{
		Ivec3 ret(lhs);
		ret /= rhs;
		return ret;
	}

	inline Ivec3 operator+(const Ivec3 &lhs, const Ivec3 &rhs)
	{
		Ivec3 ret(lhs);
		ret += rhs;
		return ret;
	}

	inline Ivec3 operator-(const Ivec3 &lhs, const Ivec3 &rhs)
	{
		Ivec3 ret(lhs);
		ret -= rhs;
		return ret;
	}

	inline Ivec3 operator*(const Ivec3 &lhs, const Ivec3 &rhs)
	{
		Ivec3 ret(lhs);
		ret *= rhs;
		return ret;
	}

	inline Ivec3 operator/(const Ivec3 &lhs, const Ivec3 &rhs)
	{
		Ivec3 ret(lhs);
		ret /= rhs;
		return ret;
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

	inline bool operator==(const Ivec4 &lhs, const Ivec4 &rhs)
	{
		return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.w == rhs.w;
	}

	inline bool operator!=(const Ivec4 &lhs, const Ivec4 &rhs)
	{
		return !(lhs == rhs);
	}

	inline Ivec4 operator+(const Ivec4 &lhs, const Vec4 &rhs)
	{
		Ivec4 ret(lhs);
		ret += rhs;
		return ret;
	}

	inline Ivec4 operator-(const Ivec4 &lhs, const Vec4 &rhs)
	{
		Ivec4 ret(lhs);
		ret -= rhs;
		return ret;
	}

	inline Ivec4 operator*(const Ivec4 &lhs, const Vec4 &rhs)
	{
		Ivec4 ret(lhs);
		ret *= rhs;
		return ret;
	}

	inline Ivec4 operator/(const Ivec4 &lhs, const Vec4 &rhs)
	{
		Ivec4 ret(lhs);
		ret /= rhs;
		return ret;
	}

	inline Ivec4 operator+(const Ivec4 &lhs, const Ivec4 &rhs)
	{
		Ivec4 ret(lhs);
		ret += rhs;
		return ret;
	}

	inline Ivec4 operator-(const Ivec4 &lhs, const Ivec4 &rhs)
	{
		Ivec4 ret(lhs);
		ret -= rhs;
		return ret;
	}

	inline Ivec4 operator*(const Ivec4 &lhs, const Ivec4 &rhs)
	{
		Ivec4 ret(lhs);
		ret *= rhs;
		return ret;
	}

	inline Ivec4 operator/(const Ivec4 &lhs, const Ivec4 &rhs)
	{
		Ivec4 ret(lhs);
		ret /= rhs;
		return ret;
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
		(*this)[0][0] = diagonal; (*this)[1][0] = 0.f;
		(*this)[0][1] = 0.f;      (*this)[1][1] = diagonal;
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
		auto det_inv = 1.f / (
			(*this)[0][0] * (*this)[1][1] - 
			(*this)[1][0] * (*this)[0][1]);

		Mat2 ret(
			+(*this)[1][1] * det_inv,
			-(*this)[0][1] * det_inv,
			-(*this)[1][0] * det_inv,
			+(*this)[0][0] * det_inv);

		return ret;
	}

	inline Mat2 operator+(const Mat2 &lhs, const Mat2 &rhs)
	{
		Mat2 ret(lhs);
		ret += rhs;
		return ret;
	}

	inline Mat2 operator-(const Mat2 &lhs, const Mat2 &rhs)
	{
		Mat2 ret(lhs);
		ret -= rhs;
		return ret;
	}

	inline Mat2 operator*(const Mat2 &lhs, const Mat2 &rhs)
	{
		Mat2 ret;
		ret[0][0] = lhs[0][0] * rhs[0][0] + lhs[1][0] * rhs[0][1];
		ret[0][1] = lhs[0][1] * rhs[0][0] + lhs[1][1] * rhs[0][1];
		ret[1][0] = lhs[0][0] * rhs[1][0] + lhs[1][0] * rhs[1][1];
		ret[1][1] = lhs[0][1] * rhs[1][0] + lhs[1][1] * rhs[1][1];
		return ret;
	}

	inline Vec2 operator*(const Mat2 &lhs, const Vec2 &rhs)
	{
		Vec2 ret;
		ret.x = lhs[0][0] * rhs.x + lhs[1][0] * rhs.y;
		ret.y = lhs[0][1] * rhs.x + lhs[1][1] * rhs.y;
		return ret;
	}

	inline Mat2 operator/(const Mat2 &lhs, const Mat2 &rhs)
	{
		Mat2 ret(lhs);
		ret /= rhs;
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

	inline Mat3::Mat3()
	{
	}

	inline Mat3::Mat3(float diagonal)
	{
		(*this)[0][0] = diagonal; (*this)[1][0] = 0.f;      (*this)[2][0] = 0.f;
		(*this)[0][1] = 0.f;      (*this)[1][1] = diagonal; (*this)[2][1] = 0.f;
		(*this)[0][2] = 0.f;      (*this)[1][2] = 0.f;      (*this)[2][2] = diagonal;
	}

	inline Mat3::Mat3(float Xx, float Xy, float Xz,
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

	inline Vec3 &Mat3::operator[](int i)
	{
		return (*this)[i];
	}

	inline Vec3 const&Mat3::operator[](int i) const
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

	inline Mat3 &Mat3::operator+=(const Mat3 &v)
	{
		(*this)[0] += v[0];
		(*this)[1] += v[1];
		(*this)[2] += v[2];
		return *this;
	}

	inline Mat3 &Mat3::operator-=(const Mat3 &v)
	{
		(*this)[0] -= v[0];
		(*this)[1] -= v[1];
		(*this)[2] -= v[2];
		return *this;
	}

	inline Mat3 &Mat3::operator*=(const Mat3 &v)
	{
		return (*this = *this * v);
	}

	inline Mat3 &Mat3::operator/=(const Mat3 &v)
	{
		return *this *= v.get_inverse();
	}

	inline Mat3 &Mat3::operator+=(float v)
	{
		(*this)[0] += v;
		(*this)[1] += v;
		(*this)[2] += v;
		return *this;
	}

	inline Mat3 &Mat3::operator-=(float v)
	{
		(*this)[0] -= v;
		(*this)[1] -= v;
		(*this)[2] -= v;
		return *this;
	}

	inline Mat3 &Mat3::operator*=(float v)
	{
		(*this)[0] *= v;
		(*this)[1] *= v;
		(*this)[2] *= v;
		return *this;
	}

	inline Mat3 &Mat3::operator/=(float v)
	{
		(*this)[0] /= v;
		(*this)[1] /= v;
		(*this)[2] /= v;
		return *this;
	}

	inline void Mat3::normalize()
	{
		(*this)[0].normalize();
		(*this)[1].normalize();
		(*this)[2].normalize();
	}

	inline void Mat3::transpose()
	{
		*this = get_transpose();
	}

	inline Mat3 Mat3::get_transpose() const
	{
		Mat3 ret;
		ret[0][0] = (*this)[0][0];
		ret[0][1] = (*this)[1][0];
		ret[0][2] = (*this)[2][0];
		ret[1][0] = (*this)[0][1];
		ret[1][1] = (*this)[1][1];
		ret[1][2] = (*this)[2][1];
		ret[2][0] = (*this)[0][2];
		ret[2][1] = (*this)[1][2];
		ret[2][2] = (*this)[2][2];
		return ret;
	}

	inline void Mat3::inverse()
	{
		*this = get_inverse();
	}

	inline Mat3 Mat3::get_inverse() const
	{
		auto det_inv = 1.f / (
			+(*this)[0][0] * ((*this)[1][1] * (*this)[2][2] - (*this)[2][1] * (*this)[1][2])
			-(*this)[1][0] * ((*this)[0][1] * (*this)[2][2] - (*this)[2][1] * (*this)[0][2])
			+(*this)[2][0] * ((*this)[0][1] * (*this)[1][2] - (*this)[1][1] * (*this)[0][2]));

		Mat3 ret;;
		ret[0][0] = +((*this)[1][1] * (*this)[2][2] - (*this)[2][1] * (*this)[1][2]) * det_inv;
		ret[1][0] = -((*this)[1][0] * (*this)[2][2] - (*this)[2][0] * (*this)[1][2]) * det_inv;
		ret[2][0] = +((*this)[1][0] * (*this)[2][1] - (*this)[2][0] * (*this)[1][1]) * det_inv;
		ret[0][1] = -((*this)[0][1] * (*this)[2][2] - (*this)[2][1] * (*this)[0][2]) * det_inv;
		ret[1][1] = +((*this)[0][0] * (*this)[2][2] - (*this)[2][0] * (*this)[0][2]) * det_inv;
		ret[2][1] = -((*this)[0][0] * (*this)[2][1] - (*this)[2][0] * (*this)[0][1]) * det_inv;
		ret[0][2] = +((*this)[0][1] * (*this)[1][2] - (*this)[1][1] * (*this)[0][2]) * det_inv;
		ret[1][2] = -((*this)[0][0] * (*this)[1][2] - (*this)[1][0] * (*this)[0][2]) * det_inv;
		ret[2][2] = +((*this)[0][0] * (*this)[1][1] - (*this)[1][0] * (*this)[0][1]) * det_inv;

		return ret;
	}

	inline Mat3 operator+(const Mat3 &lhs, const Mat3 &rhs)
	{
		Mat3 ret(lhs);
		ret += rhs;
		return ret;
	}

	inline Mat3 operator-(const Mat3 &lhs, const Mat3 &rhs)
	{
		Mat3 ret(lhs);
		ret -= rhs;
		return ret;
	}

	inline Mat3 operator*(const Mat3 &lhs, const Mat3 &rhs)
	{
		Mat3 ret;
		ret[0][0] = lhs[0][0] * rhs[0][0] + lhs[1][0] * rhs[0][1] + lhs[2][0] * rhs[0][2];
		ret[0][1] = lhs[0][1] * rhs[0][0] + lhs[1][1] * rhs[0][1] + lhs[2][1] * rhs[0][2];
		ret[0][2] = lhs[0][2] * rhs[0][0] + lhs[1][2] * rhs[0][1] + lhs[2][2] * rhs[0][2];
		ret[1][0] = lhs[0][0] * rhs[1][0] + lhs[1][0] * rhs[1][1] + lhs[2][0] * rhs[1][2];
		ret[1][1] = lhs[0][1] * rhs[1][0] + lhs[1][1] * rhs[1][1] + lhs[2][1] * rhs[1][2];
		ret[1][2] = lhs[0][2] * rhs[1][0] + lhs[1][2] * rhs[1][1] + lhs[2][2] * rhs[1][2];
		ret[2][0] = lhs[0][0] * rhs[2][0] + lhs[1][0] * rhs[2][1] + lhs[2][0] * rhs[2][2];
		ret[2][1] = lhs[0][1] * rhs[2][0] + lhs[1][1] * rhs[2][1] + lhs[2][1] * rhs[2][2];
		ret[2][2] = lhs[0][2] * rhs[2][0] + lhs[1][2] * rhs[2][1] + lhs[2][2] * rhs[2][2];
		return ret;
	}

	inline Vec3 operator*(const Mat3 &lhs, const Vec3 &rhs)
	{
		Vec3 ret;
		ret.x = lhs[0][0] * rhs.x + lhs[1][0] * rhs.y + lhs[2][0] * rhs.z;
		ret.y = lhs[0][1] * rhs.x + lhs[1][1] * rhs.y + lhs[2][1] * rhs.z;
		ret.z = lhs[0][2] * rhs.x + lhs[1][2] * rhs.y + lhs[2][2] * rhs.z;
		return ret;
	}

	inline Mat3 operator/(const Mat3 &lhs, const Mat3 &rhs)
	{
		Mat3 ret(lhs);
		ret /= rhs;
		return ret;
	}

	inline Mat3 operator+(const Mat3 &lhs, float rhs)
	{
		Mat3 ret(lhs);
		ret[0] += rhs;
		ret[1] += rhs;
		ret[2] += rhs;
		return ret;
	}

	inline Mat3 operator-(const Mat3 &lhs, float rhs)
	{
		Mat3 ret(lhs);
		ret[0] -= rhs;
		ret[1] -= rhs;
		ret[2] -= rhs;
		return ret;
	}

	inline Mat3 operator*(const Mat3 &lhs, float rhs)
	{
		Mat3 ret(lhs);
		ret[0] *= rhs;
		ret[1] *= rhs;
		ret[2] *= rhs;
		return ret;
	}

	inline Mat3 operator/(const Mat3 &lhs, float rhs)
	{
		Mat3 ret(lhs);
		ret[0] /= rhs;
		ret[1] /= rhs;
		ret[2] /= rhs;
		return ret;
	}

	inline Mat3 operator+(float lhs, const Mat3 &rhs)
	{
		Mat3 ret(rhs);
		ret[0] += lhs;
		ret[1] += lhs;
		ret[2] += lhs;
		return ret;
	}

	inline Mat3 operator-(float lhs, const Mat3 &rhs)
	{
		Mat3 ret(rhs);
		ret[0] -= lhs;
		ret[1] -= lhs;
		ret[2] -= lhs;
		return ret;
	}

	inline Mat3 operator*(float lhs, const Mat3 &rhs)
	{
		Mat3 ret(rhs);
		ret[0] *= lhs;
		ret[1] *= lhs;
		ret[2] *= lhs;
		return ret;
	}

	inline Mat3 operator/(float lhs, const Mat3 &rhs)
	{
		Mat3 ret(rhs);
		ret[0] /= lhs;
		ret[1] /= lhs;
		ret[2] /= lhs;
		return ret;
	}

	inline Mat4::Mat4()
	{
	}

	inline Mat4::Mat4(float diagonal)
	{
		(*this)[0][0] = diagonal; (*this)[1][0] = 0.f;      (*this)[2][0] = 0.f;      (*this)[3][0] = 0.f;
		(*this)[0][1] = 0.f;      (*this)[1][1] = diagonal; (*this)[2][1] = 0.f;      (*this)[3][1] = 0.f;
		(*this)[0][2] = 0.f;      (*this)[1][2] = 0.f;      (*this)[2][2] = diagonal; (*this)[3][2] = 0.f;
		(*this)[0][3] = 0.f;      (*this)[1][3] = 0.f;      (*this)[2][3] = 0.f;      (*this)[3][3] = diagonal;
	}

	inline Mat4::Mat4(float Xx, float Xy, float Xz, float Xw,
		float Yx, float Yy, float Yz, float Yw,
		float Zx, float Zy, float Zz, float Zw,
		float Wx, float Wy, float Wz, float Ww)
	{
		(*this)[0][0] = Xx; (*this)[1][0] = Yx; (*this)[2][0] = Zx; (*this)[3][0] = Wx;
		(*this)[0][1] = Xy; (*this)[1][1] = Yy; (*this)[2][1] = Zy; (*this)[3][1] = Wy;
		(*this)[0][2] = Xz; (*this)[1][2] = Yz; (*this)[2][2] = Zz; (*this)[3][2] = Wz;
		(*this)[0][3] = Xw; (*this)[1][3] = Yw; (*this)[2][3] = Zw; (*this)[3][3] = Ww;
	}

	inline Mat4::Mat4(const Vec4 &v0, const Vec4 &v1, const Vec4 &v2, const Vec4 &v3)
	{
		(*this)[0] = v0;
		(*this)[1] = v1;
		(*this)[2] = v2;
		(*this)[3] = v3;
	}

	inline Mat4::Mat4(const Mat4 &v)
	{
		(*this)[0] = v[0];
		(*this)[1] = v[1];
		(*this)[2] = v[2];
		(*this)[3] = v[3];
	}

	inline Mat4::Mat4(const Mat3 &mat3, const Vec3 &coord)
	{
		(*this)[0] = Vec4(mat3[0], 0.f);
		(*this)[1] = Vec4(mat3[1], 0.f);
		(*this)[2] = Vec4(mat3[2], 0.f);
		(*this)[3] = Vec4(coord, 1.f);
	}

	inline Mat4::Mat4(const Vec3 &x_axis, const Vec3 &y_axis, const Vec3 &coord)
	{
		(*this)[0] = Vec4(x_axis, 0.f);
		(*this)[1] = Vec4(y_axis, 0.f);
		(*this)[2] = Vec4(cross(x_axis, y_axis), 0.f);
		(*this)[3] = Vec4(coord, 1.f);
	}

	inline Vec4 &Mat4::operator[](int i)
	{
		return (*this)[i];
	}

	inline Vec4 const&Mat4::operator[](int i) const
	{
		return (*this)[i];
	}

	inline Mat4 &Mat4::operator=(const Mat4 &v)
	{
		(*this)[0] = v[0];
		(*this)[1] = v[1];
		(*this)[2] = v[2];
		(*this)[3] = v[3];
		return *this;
	}

	inline Mat4 &Mat4::operator+=(const Mat4 &v)
	{
		(*this)[0] += v[0];
		(*this)[1] += v[1];
		(*this)[2] += v[2];
		(*this)[3] += v[3];
		return *this;
	}

	inline Mat4 &Mat4::operator-=(const Mat4 &v)
	{
		(*this)[0] -= v[0];
		(*this)[1] -= v[1];
		(*this)[2] -= v[2];
		(*this)[3] -= v[3];
		return *this;
	}

	inline Mat4 &Mat4::operator*=(const Mat4 &v)
	{
		return (*this = *this * v);
	}

	inline Mat4 &Mat4::operator/=(const Mat4 &v)
	{
		return *this *= v.get_inverse();
	}

	inline Mat4 &Mat4::operator+=(float v)
	{
		(*this)[0] += v;
		(*this)[1] += v;
		(*this)[2] += v;
		(*this)[3] += v;
		return *this;
	}

	inline Mat4 &Mat4::operator-=(float v)
	{
		(*this)[0] -= v;
		(*this)[1] -= v;
		(*this)[2] -= v;
		(*this)[3] -= v;
		return *this;
	}

	inline Mat4 &Mat4::operator*=(float v)
	{
		(*this)[0] *= v;
		(*this)[1] *= v;
		(*this)[2] *= v;
		(*this)[3] *= v;
		return *this;
	}

	inline Mat4 &Mat4::operator/=(float v)
	{
		(*this)[0] /= v;
		(*this)[1] /= v;
		(*this)[2] /= v;
		(*this)[3] /= v;
		return *this;
	}

	inline void Mat4::normalize_33()
	{
		(*this)[0] = Vec4(Vec3((*this)[0]).get_normalize(), 0.f);
		(*this)[1] = Vec4(Vec3((*this)[1]).get_normalize(), 0.f);
		(*this)[2] = Vec4(Vec3((*this)[2]).get_normalize(), 0.f);
	}

	inline void Mat4::transpose()
	{
		*this = get_transpose();
	}

	inline Mat4 Mat4::get_transpose() const
	{
		Mat4 ret;
		ret[0][0] = (*this)[0][0];
		ret[0][1] = (*this)[1][0];
		ret[0][2] = (*this)[2][0];
		ret[0][3] = (*this)[3][0];
		ret[1][0] = (*this)[0][1];
		ret[1][1] = (*this)[1][1];
		ret[1][2] = (*this)[2][1];
		ret[1][3] = (*this)[3][1];
		ret[2][0] = (*this)[0][2];
		ret[2][1] = (*this)[1][2];
		ret[2][2] = (*this)[2][2];
		ret[2][3] = (*this)[3][2];
		ret[3][0] = (*this)[0][3];
		ret[3][1] = (*this)[1][3];
		ret[3][2] = (*this)[2][3];
		ret[3][3] = (*this)[3][3];
		return ret;
	}

	inline void Mat4::inverse()
	{
		*this = get_inverse();
	}

	inline Mat4 Mat4::get_inverse() const
	{
		auto coef00 = (*this)[2][2] * (*this)[3][3] - (*this)[3][2] * (*this)[2][3];
		auto coef02 = (*this)[1][2] * (*this)[3][3] - (*this)[3][2] * (*this)[1][3];
		auto coef03 = (*this)[1][2] * (*this)[2][3] - (*this)[2][2] * (*this)[1][3];

		auto coef04 = (*this)[2][1] * (*this)[3][3] - (*this)[3][1] * (*this)[2][3];
		auto coef06 = (*this)[1][1] * (*this)[3][3] - (*this)[3][1] * (*this)[1][3];
		auto coef07 = (*this)[1][1] * (*this)[2][3] - (*this)[2][1] * (*this)[1][3];

		auto coef08 = (*this)[2][1] * (*this)[3][2] - (*this)[3][1] * (*this)[2][2];
		auto coef10 = (*this)[1][1] * (*this)[3][2] - (*this)[3][1] * (*this)[1][2];
		auto coef11 = (*this)[1][1] * (*this)[2][2] - (*this)[2][1] * (*this)[1][2];

		auto coef12 = (*this)[2][0] * (*this)[3][3] - (*this)[3][0] * (*this)[2][3];
		auto coef14 = (*this)[1][0] * (*this)[3][3] - (*this)[3][0] * (*this)[1][3];
		auto coef15 = (*this)[1][0] * (*this)[2][3] - (*this)[2][0] * (*this)[1][3];

		auto coef16 = (*this)[2][0] * (*this)[3][2] - (*this)[3][0] * (*this)[2][2];
		auto coef18 = (*this)[1][0] * (*this)[3][2] - (*this)[3][0] * (*this)[1][2];
		auto coef19 = (*this)[1][0] * (*this)[2][2] - (*this)[2][0] * (*this)[1][2];

		auto coef20 = (*this)[2][0] * (*this)[3][1] - (*this)[3][0] * (*this)[2][1];
		auto coef22 = (*this)[1][0] * (*this)[3][1] - (*this)[3][0] * (*this)[1][1];
		auto coef23 = (*this)[1][0] * (*this)[2][1] - (*this)[2][0] * (*this)[1][1];

		Vec4 fac0(coef00, coef00, coef02, coef03);
		Vec4 fac1(coef04, coef04, coef06, coef07);
		Vec4 fac2(coef08, coef08, coef10, coef11);
		Vec4 fac3(coef12, coef12, coef14, coef15);
		Vec4 fac4(coef16, coef16, coef18, coef19);
		Vec4 fac5(coef20, coef20, coef22, coef23);

		Vec4 vec0((*this)[1][0], (*this)[0][0], (*this)[0][0], (*this)[0][0]);
		Vec4 vec1((*this)[1][1], (*this)[0][1], (*this)[0][1], (*this)[0][1]);
		Vec4 vec2((*this)[1][2], (*this)[0][2], (*this)[0][2], (*this)[0][2]);
		Vec4 vec3((*this)[1][3], (*this)[0][3], (*this)[0][3], (*this)[0][3]);

		Vec4 inv0(vec1 * fac0 - vec2 * fac1 + vec3 * fac2);
		Vec4 inv1(vec0 * fac0 - vec2 * fac3 + vec3 * fac4);
		Vec4 inv2(vec0 * fac1 - vec1 * fac3 + vec3 * fac5);
		Vec4 inv3(vec0 * fac2 - vec1 * fac4 + vec2 * fac5);

		Vec4 signA(+1, -1, +1, -1);
		Vec4 signB(-1, +1, -1, +1);
		Mat4 inverse(inv0 * signA, inv1 * signB, inv2 * signA, inv3 * signB);

		Vec4 row0(inverse[0][0], inverse[1][0], inverse[2][0], inverse[3][0]);

		Vec4 dot0((*this)[0] * row0);
		auto dot1 = (dot0.x + dot0.y) + (dot0.z + dot0.w);

		auto det_inv = 1.f / dot1;

		return inverse * det_inv;
	}

	inline Mat4 operator+(const Mat4 &lhs, const Mat4 &rhs)
	{
		Mat4 ret(lhs);
		ret += rhs;
		return ret;
	}

	inline Mat4 operator-(const Mat4 &lhs, const Mat4 &rhs)
	{
		Mat4 ret(lhs);
		ret -= rhs;
		return ret;
	}

	Mat4 operator*(const Mat4 &lhs, const Mat4 &rhs)
	{
		Mat4 ret;
		ret[0][0] = lhs[0][0] * rhs[0][0] + lhs[1][0] * rhs[0][1] + lhs[2][0] * rhs[0][2] + lhs[3][0] * rhs[0][3];
		ret[0][1] = lhs[0][1] * rhs[0][0] + lhs[1][1] * rhs[0][1] + lhs[2][1] * rhs[0][2] + lhs[3][1] * rhs[0][3];
		ret[0][2] = lhs[0][2] * rhs[0][0] + lhs[1][2] * rhs[0][1] + lhs[2][2] * rhs[0][2] + lhs[3][2] * rhs[0][3];
		ret[0][3] = lhs[0][3] * rhs[0][0] + lhs[1][3] * rhs[0][1] + lhs[2][3] * rhs[0][2] + lhs[3][3] * rhs[0][3];
		ret[1][0] = lhs[0][0] * rhs[1][0] + lhs[1][0] * rhs[1][1] + lhs[2][0] * rhs[1][2] + lhs[3][0] * rhs[1][3];
		ret[1][1] = lhs[0][1] * rhs[1][0] + lhs[1][1] * rhs[1][1] + lhs[2][1] * rhs[1][2] + lhs[3][1] * rhs[1][3];
		ret[1][2] = lhs[0][2] * rhs[1][0] + lhs[1][2] * rhs[1][1] + lhs[2][2] * rhs[1][2] + lhs[3][2] * rhs[1][3];
		ret[1][3] = lhs[0][3] * rhs[1][0] + lhs[1][3] * rhs[1][1] + lhs[2][3] * rhs[1][2] + lhs[3][3] * rhs[1][3];
		ret[2][0] = lhs[0][0] * rhs[2][0] + lhs[1][0] * rhs[2][1] + lhs[2][0] * rhs[2][2] + lhs[3][0] * rhs[2][3];
		ret[2][1] = lhs[0][1] * rhs[2][0] + lhs[1][1] * rhs[2][1] + lhs[2][1] * rhs[2][2] + lhs[3][1] * rhs[2][3];
		ret[2][2] = lhs[0][2] * rhs[2][0] + lhs[1][2] * rhs[2][1] + lhs[2][2] * rhs[2][2] + lhs[3][2] * rhs[2][3];
		ret[2][3] = lhs[0][3] * rhs[2][0] + lhs[1][3] * rhs[2][1] + lhs[2][3] * rhs[2][2] + lhs[3][3] * rhs[2][3];
		ret[3][0] = lhs[0][0] * rhs[3][0] + lhs[1][0] * rhs[3][1] + lhs[2][0] * rhs[3][2] + lhs[3][0] * rhs[3][3];
		ret[3][1] = lhs[0][1] * rhs[3][0] + lhs[1][1] * rhs[3][1] + lhs[2][1] * rhs[3][2] + lhs[3][1] * rhs[3][3];
		ret[3][2] = lhs[0][2] * rhs[3][0] + lhs[1][2] * rhs[3][1] + lhs[2][2] * rhs[3][2] + lhs[3][2] * rhs[3][3];
		ret[2][3] = lhs[0][3] * rhs[3][0] + lhs[1][3] * rhs[3][1] + lhs[2][3] * rhs[3][2] + lhs[3][3] * rhs[3][3];
		return ret;
	}

	inline Vec4 operator*(const Mat4 &lhs, const Vec4 &rhs)
	{
		Vec4 ret;
		ret.x = lhs[0][0] * rhs.x + lhs[1][0] * rhs.y + lhs[2][0] * rhs.z + lhs[3][0] * rhs.w;
		ret.y = lhs[0][1] * rhs.x + lhs[1][1] * rhs.y + lhs[2][1] * rhs.z + lhs[3][1] * rhs.w;
		ret.z = lhs[0][2] * rhs.x + lhs[1][2] * rhs.y + lhs[2][2] * rhs.z + lhs[3][2] * rhs.w;
		ret.w = lhs[0][3] * rhs.x + lhs[1][3] * rhs.y + lhs[2][3] * rhs.z + lhs[3][3] * rhs.w;
		return ret;
	}

	inline Vec3 operator*(const Mat4 &lhs, const Vec3 &rhs)
	{
		Vec3 ret;
		ret.x = lhs[0][0] * rhs.x + lhs[1][0] * rhs.y + lhs[2][0] * rhs.z + lhs[3][0];
		ret.y = lhs[0][1] * rhs.x + lhs[1][1] * rhs.y + lhs[2][1] * rhs.z + lhs[3][1];
		ret.z = lhs[0][2] * rhs.x + lhs[1][2] * rhs.y + lhs[2][2] * rhs.z + lhs[3][2];
		return ret;
	}

	inline Mat4 operator/(const Mat4 &lhs, const Mat4 &rhs)
	{
		Mat4 ret(lhs);
		ret /= rhs;
		return ret;
	}

	inline Mat4 operator+(const Mat4 &lhs, float rhs)
	{
		Mat4 ret(lhs);
		ret[0] += rhs;
		ret[1] += rhs;
		ret[2] += rhs;
		ret[3] += rhs;
		return ret;
	}

	inline Mat4 operator-(const Mat4 &lhs, float rhs)
	{
		Mat4 ret(lhs);
		ret[0] -= rhs;
		ret[1] -= rhs;
		ret[2] -= rhs;
		ret[3] -= rhs;
		return ret;
	}

	inline Mat4 operator*(const Mat4 &lhs, float rhs)
	{
		Mat4 ret(lhs);
		ret[0] *= rhs;
		ret[1] *= rhs;
		ret[2] *= rhs;
		ret[3] *= rhs;
		return ret;
	}

	inline Mat4 operator/(const Mat4 &lhs, float rhs)
	{
		Mat4 ret(lhs);
		ret[0] /= rhs;
		ret[1] /= rhs;
		ret[2] /= rhs;
		ret[3] /= rhs;
		return ret;
	}

	inline Mat4 operator+(float lhs, const Mat4 &rhs)
	{
		Mat4 ret(rhs);
		ret[0] += lhs;
		ret[1] += lhs;
		ret[2] += lhs;
		ret[3] += lhs;
		return ret;
	}

	inline Mat4 operator-(float lhs, const Mat4 &rhs)
	{
		Mat4 ret(rhs);
		ret[0] -= lhs;
		ret[1] -= lhs;
		ret[2] -= lhs;
		ret[3] -= lhs;
		return ret;
	}

	inline Mat4 operator*(float lhs, const Mat4 &rhs)
	{
		Mat4 ret(rhs);
		ret[0] *= lhs;
		ret[1] *= lhs;
		ret[2] *= lhs;
		ret[3] *= lhs;
		return ret;
	}

	inline Mat4 operator/(float lhs, const Mat4 &rhs)
	{
		Mat4 ret(rhs);
		ret[0] /= lhs;
		ret[1] /= lhs;
		ret[2] /= lhs;
		ret[3] /= lhs;
		return ret;
	}

	inline Vec3 cross(const Vec3 &lhs, const Vec3 &rhs)
	{
		return Vec3(
			lhs.y * rhs.z - rhs.y * lhs.z,
			lhs.z * rhs.x - rhs.z * lhs.x,
			lhs.x * rhs.y - rhs.x * lhs.y);
	}

	inline EulerYawPitchRoll::EulerYawPitchRoll()
	{
	}

	inline EulerYawPitchRoll::EulerYawPitchRoll(const Quat &q)
	{
		math_detail::quat_to_euler(q, *this);
	}

	inline Quat::Quat()
	{
	}

	inline Quat::Quat(float _x, float _y, float _z, float _w) :
		x(_x),
		y(_y),
		z(_z),
		w(_w)
	{
	}

	inline void Quat::normalize()
	{
		auto l = sqrt(x * x + y * y + z * z + w * w);
		x /= l;
		y /= l;
		z /= l;
		w /= l;
	}

	inline Quat operator*(const Quat &lhs, const Quat &rhs)
	{
		Quat ret;
		ret.x = lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y;
		ret.y = lhs.w * rhs.y - lhs.x * rhs.z + lhs.y * rhs.w + lhs.z * rhs.x;
		ret.z = lhs.w * rhs.z + lhs.x * rhs.y + lhs.y * rhs.w + lhs.z * rhs.x;
		ret.w = lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z;
		return ret;
	}

	inline Rect::Rect()
	{
	}

	inline Rect::Rect(const Vec2 &_min, const Vec2 &_max) :
		min(_min),
		max(_max)
	{
	}

	inline Rect::Rect(float min_x, float min_y, float max_x, float max_y) :
		min(min_x, min_y),
		max(max_x, max_y)
	{
	}

	inline Rect::Rect(const Rect &v) :
		min(v.min),
		max(v.max)
	{
	}
	
	inline Rect &Rect::operator+=(const Vec2 &v)
	{
		min += v;
		max += v;
		return *this;
	}

	inline Rect &Rect::operator-=(const Vec2 &v)
	{
		min -= v;
		max -= v;
		return *this;
	}

	inline float Rect::width() const
	{
		return max.x - min.x;
	}

	inline float Rect::height() const
	{
		return max.y - min.y;
	}

	inline void Rect::expand(float length)
	{
		min.x -= length;
		min.y -= length;
		max.x += length;
		max.y += length;
	}

	inline Rect Rect::get_expanded(float length)
	{
		Rect ret(*this);
		ret.expand(length);
		return ret;
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

	inline Vec2 get_side_dir(Rect::Side s)
	{
		switch (s)
		{
		case Rect::SideN:
			return Vec2(0.f, -1.f);
		case Rect::SideS:
			return Vec2(0.f, 1.f);
		case Rect::SideW:
			return Vec2(-1.f, 0.f);
		case Rect::SideE:
			return Vec2(1.f, 0.f);
		case Rect::SideNW:
			return Vec2(-1.f, -1.f);
		case Rect::SideNE:
			return Vec2(1.f, -1.f);
		case Rect::SideSW:
			return Vec2(-1.f, 1.f);
		case Rect::SideSE:
			return Vec2(1.f, 1.f);
		case Rect::Outside:
			return Vec2(2.f);
		case Rect::Inside:
			return Vec2(0.f);
		}
	}

	inline Rect operator+(const Rect &r, const Vec2 &off)
	{
		Rect ret(r);
		ret += off;
		return ret;
	}

	inline Rect operator-(const Rect &r, const Vec2 &off)
	{
		Rect ret(r);
		ret -= off;
		return ret;
	}

	inline Plane::Plane()
	{
	}

	inline Plane::Plane(const Vec3 &n, float _d) :
		normal(n),
		d(_d)
	{
	}

	inline Plane::Plane(const Vec3 &n, const Vec3 &p) :
		normal(n),
		d(dot(normal, p))
	{
	}

	inline float Plane::intersect(const Vec3 &origin, const Vec3 &dir)
	{
		auto numer = dot(normal, origin) - d;
		auto denom = dot(normal, dir);

		if (abs(denom) < EPS)
			return -1.f;

		return -(numer / denom);
	}

	namespace math_detail
	{
		inline void rotate(const Vec3 &axis, float rad, Mat3 &m)
		{
			const auto c = cos(rad);
			const auto s = sin(rad);

			Vec3 temp((1.f - c) * axis);

			m[0][0] = c + temp[0] * axis[0];
			m[0][1] = 0 + temp[0] * axis[1] + s * axis[2];
			m[0][2] = 0 + temp[0] * axis[2] - s * axis[1];

			m[1][0] = 0 + temp[1] * axis[0] - s * axis[2];
			m[1][1] = c + temp[1] * axis[1];
			m[1][2] = 0 + temp[1] * axis[2] + s * axis[0];

			m[2][0] = 0 + temp[2] * axis[0] + s * axis[1];
			m[2][1] = 0 + temp[2] * axis[1] - s * axis[0];
			m[2][2] = c + temp[2] * axis[2];
		}

		inline void mat3_to_quat(const Mat3 &m, Quat &q)
		{
			float s;
			float tq[4];
			int   i, j;
			// Use tq to store the largest trace
			tq[0] = 1.f + m[0][0] + m[1][1] + m[2][2];
			tq[1] = 1.f + m[0][0] - m[1][1] - m[2][2];
			tq[2] = 1.f - m[0][0] + m[1][1] - m[2][2];
			tq[3] = 1.f - m[0][0] - m[1][1] + m[2][2];
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
				q.x = m[1][2] - m[2][1];
				q.y = m[2][0] - m[0][2];
				q.z = m[0][1] - m[1][0];
			}
			else if (j == 1)
			{
				q.w = m[1][2] - m[2][1];
				q.x = tq[1];
				q.y = m[0][1] + m[1][0];
				q.z = m[2][0] + m[0][2];
			}
			else if (j == 2)
			{
				q.w = m[2][0] - m[0][2];
				q.x = m[0][1] + m[1][0];
				q.y = tq[2];
				q.z = m[1][2] + m[2][1];
			}
			else /* if (j==3) */
			{
				q.w = m[0][1] - m[1][0];
				q.x = m[2][0] + m[0][2];
				q.y = m[1][2] + m[2][1];
				q.z = tq[3];
			}
			s = sqrt(0.25f / tq[j]);
			q.w *= s;
			q.x *= s;
			q.y *= s;
			q.z *= s;
			q.normalize();
		}

		inline void euler_to_mat3(const EulerYawPitchRoll &e, Mat3 &m)
		{
			m[0] = Vec3(1.f, 0.f, 0.f);
			m[1] = Vec3(0.f, 1.f, 0.f);
			m[2] = Vec3(0.f, 0.f, 1.f);
			Mat3 mat_yaw(Vec3(m[1]), e.yaw * ANG_RAD);
			m[0] = mat_yaw * m[0];
			m[2] = mat_yaw * m[2];
			Mat3 mat_pitch(Vec3(m[0]), e.pitch * ANG_RAD);
			m[2] = mat_pitch * m[2];
			m[1] = mat_pitch * m[1];
			Mat3 mat_roll(Vec3(m[2]), e.roll * ANG_RAD);
			m[1] = mat_roll * m[1];
			m[0] = mat_roll * m[0];
		}

		inline void quat_to_euler(const Quat &q, EulerYawPitchRoll &e)
		{
			auto sqw = q.w * q.w;
			auto sqx = q.x * q.x;
			auto sqy = q.y * q.y;
			auto sqz = q.z * q.z;

			auto unit = sqx + sqy + sqz + sqw; // if normalised is one, otherwise is correction factor
			auto test = q.x * q.y + q.z * q.w;
			if (test > 0.499f * unit)
			{ // singularity at north pole
				e.yaw = 2.f * atan2(q.x, q.w);
				e.pitch = PI / 2.f;
				e.roll = 0;
				return;
			}
			if (test < -0.499f * unit)
			{ // singularity at south pole
				e.yaw = -2.f * atan2(q.x, q.w);
				e.pitch = -PI / 2.f;
				e.roll = 0;
				return;
			}

			e.yaw = atan2(2.f * q.y * q.w - 2.f * q.x * q.z, sqx - sqy - sqz + sqw) * RAD_ANG;
			e.pitch = asin(2.f * test / unit) * RAD_ANG;
			e.roll = atan2(2.f * q.x * q.w - 2.f * q.y * q.z, -sqx + sqy - sqz + sqw) * RAD_ANG;
		}

		inline void quat_to_mat3(const Quat &q, Mat3 &m)
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

			m[0][0] = 1.f - (yy + zz);
			m[1][0] = xy - wz;
			m[2][0] = xz + wy;

			m[0][1] = xy + wz;
			m[1][1] = 1.f - (xx + zz);
			m[2][1] = yz - wx;

			m[0][2] = xz - wy;
			m[1][2] = yz + wx;
			m[2][2] = 1.f - (xx + yy);
		}
	}
}
