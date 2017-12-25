#pragma once

#include <memory>

#include "../../PhysX-3.3/PhysXSDK/Include/PxPhysicsAPI.h"

#include "../entity/transformer.h"

namespace tke
{
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

	struct Shape : Transformer
	{
		int id;

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

	struct Rigidbody : Transformer
	{
		int id;

		RigidbodyType type = RigidbodyType::static_r;

		std::string name;
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
		void addShape(Shape *s);
		Shape *removeShape(Shape *s);
	};

	struct Joint : Transformer
	{
		int id;

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

	void initPhysics();
}
