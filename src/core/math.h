#ifndef __TK_MATH__
#define __TK_MATH__

#include <fstream>

#include "D:\glm\glm.hpp"
#include "D:\glm\gtc\matrix_transform.hpp"
#include "D:\glm\gtx\transform2.hpp"

#define EPS (0.000001f)
#define PI (3.141592653f)

bool inline operator < (glm::vec3 &a, glm::vec3 &b)
{
	if (a.x < b.x && a.y < b.y && a.z < b.z)
		return true;
	return false;
}
bool inline operator > (glm::vec3 &a, glm::vec3 &b)
{
	if (a.x > b.x && a.y > b.y && a.z > b.z)
		return true;
	return false;
}
bool inline operator <= (glm::vec3 &a, glm::vec3 &b)
{
	if (a.x <= b.x && a.y <= b.y && a.z <= b.z)
		return true;
	return false;
}
bool inline operator >= (glm::vec3 &a, glm::vec3 &b)
{
	if (a.x >= b.x && a.y >= b.y && a.z >= b.z)
		return true;
	return false;
}

bool inline operator < (float a, glm::vec3 &b)
{
	if (a < b.x && a < b.y && a < b.z)
		return true;
	return false;
}
bool inline operator > (float a, glm::vec3 &b)
{
	if (a > b.x && a > b.y && a > b.z)
		return true;
	return false;
}
bool inline operator <= (float a, glm::vec3 &b)
{
	if (a <= b.x && a <= b.y && a <= b.z)
		return true;
	return false;
}
bool inline operator >= (float a, glm::vec3 &b)
{
	if (a >= b.x && a >= b.y && a >= b.z)
		return true;
	return false;
}

bool inline operator < (glm::vec3 a, float b)
{
	if (a.x < b && a.y < b && a.z < b)
		return true;
	return false;
}
bool inline operator > (glm::vec3 a, float b)
{
	if (a.x > b && a.y > b && a.z > b)
		return true;
	return false;
}
bool inline operator <= (glm::vec3 a, float b)
{
	if (a.x <= b && a.y <= b && a.z <= b)
		return true;
	return false;
}
bool inline operator >= (glm::vec3 a, float b)
{
	if (a.x >= b && a.y >= b && a.z >= b)
		return true;
	return false;
}

inline std::ifstream& operator>>(std::ifstream &file, glm::vec2 &v)
{
	file.read((char*)&v, sizeof(glm::vec2));
	return file;
}

inline std::ofstream& operator<<(std::ofstream &file, glm::vec2 &v)
{
	file.write((char*)&v, sizeof(glm::vec2));
	return file;
}

inline std::ifstream& operator>>(std::ifstream &file, glm::vec3 &v)
{
	file.read((char*)&v, sizeof(glm::vec3));
	return file;
}

inline std::ofstream& operator<<(std::ofstream &file, glm::vec3 &v)
{
	file.write((char*)&v, sizeof(glm::vec3));
	return file;
}

inline std::ifstream& operator>>(std::ifstream &file, glm::vec4 &v)
{
	file.read((char*)&v, sizeof(glm::vec4));
	return file;
}

inline std::ofstream& operator<<(std::ofstream &file, glm::vec4 &v)
{
	file.write((char*)&v, sizeof(glm::vec4));
	return file;
}

namespace tke
{
	void quaternionToMatrix(glm::vec4 &q, glm::mat3 &mat);
	void matrixToQuaternion(glm::mat3 &mat, glm::vec4 &q);
	void quaternionRotate(glm::vec4 &q, glm::vec3 v);
	void eulerYzxToMatrix(glm::vec3 &e, glm::mat3 &mat);
	void eulerYxzToMatrix(glm::vec3 &e, glm::mat3 &mat);
	glm::mat4 makeMatrix(glm::mat3 &rotation, glm::vec3 coord);

	float rand2d(glm::vec2 v);
	float noise2d(glm::vec2 v);
	float fbm2d(glm::vec2 v);
}

#endif