#pragma once

#include <memory>
#include <vector>

#include <PxPhysicsAPI.h>

#include <flame/math/math.h>

namespace flame
{
	glm::vec3 physx_u32_to_vec3(const physx::PxU32 &src);
	glm::vec3 physx_vec3_to_vec3(const physx::PxVec3 &src);
	physx::PxVec3 vec3_to_physx_vec3(const glm::vec3 &src);
	physx::PxMat33 mat3_to_physx_mat3(const glm::mat3 &src);
	physx::PxTransform get_physx_trans(const glm::vec3 &coord, const glm::vec4 &quat);
	physx::PxTransform get_physx_trans(const glm::vec3 &coord, const glm::mat3 &axis);

	struct CollisionGroup
	{
		int originalID;
		unsigned int originalmask;
	};

	enum class ShapeType
	{
		box = 1 << 0,
		sphere = 1 << 1,
		capsule = 1 << 2, // scale: 0 - radius, scale 1 - length
		plane = 1 << 3,
		convex_mesh = 1 << 4,
		triangle_mesh = 1 << 5,
		height_field = 1 << 6
	};

	std::string shapeTypeName(ShapeType t);

	struct Shape
	{
		glm::vec3 coord = glm::vec3(0.f);
		glm::vec4 quat = glm::vec4(0.f, 0.f, 0.f, 1.f);
		glm::vec3 scale = glm::vec3(0.f);
		ShapeType type = ShapeType::box;

		Shape();
		Shape(ShapeType _type);
		float getVolume() const;
	};

	enum class RigidbodyType
	{
		static_r = 1 << 0,
		dynamic = 1 << 1,
		dynamic_but_location = 1 << 2 // special for pmd/pmx 
	};

	struct Rigidbody
	{
		glm::vec3 coord = glm::vec3(0.f);
		glm::vec4 quat = glm::vec4(0.f, 0.f, 0.f, 1.f);
		std::string name;
		RigidbodyType type = RigidbodyType::static_r;
		int boneID = -1;
		int originCollisionGroupID;
		int originCollisionFreeFlag;
		int collisionGroupID = -1;
		float density = 10.f;
		float velocityAttenuation;
		float rotationAttenuation;
		float bounce;
		float friction;
		std::vector<std::unique_ptr<Shape>> shapes;

		Rigidbody();
		Rigidbody(RigidbodyType _type);
		Shape *new_shape();
		void remove_shape(Shape *s);
	};

	struct Joint
	{
		glm::vec3 coord = glm::vec3(0.f);
		glm::vec4 quat = glm::vec4(0.f, 0.f, 0.f, 1.f);
		std::string name;
		int rigid0ID;
		int rigid1ID;
		glm::vec3 maxCoord = glm::vec3(0.f);
		glm::vec3 minCoord = glm::vec3(0.f);
		glm::vec3 maxRotation = glm::vec3(0.f);
		glm::vec3 minRotation = glm::vec3(0.f);
		glm::vec3 springConstant = glm::vec3(0.f);
		glm::vec3 sprintRotationConstant = glm::vec3(0.f);
	};

	extern physx::PxFoundation *pxFoundation;
	extern physx::PxPhysics *pxPhysics;
	extern physx::PxMaterial *pxDefaultMaterial;

	physx::PxRigidActor *createStaticRigidActor(physx::PxTransform &trans);
	physx::PxRigidActor *createDynamicRigidActor(physx::PxTransform &trans, bool kinematic, float density);

	void init_physics();
}
