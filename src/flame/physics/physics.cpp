#include <flame/physics/physics.h>

namespace tke
{
	glm::vec3 physx_u32_to_vec3(const physx::PxU32 &src)
	{
		unsigned int t = src;
		float r = (t % 256) / 255.f;
		t /= 256;
		float g = (t % 256) / 255.f;
		t /= 256;
		float b = (t % 256) / 255.f;
		return glm::vec3(r, g, b);
	}

	glm::vec3 physx_vec3_to_vec3(const physx::PxVec3 &src)
	{
		return glm::vec3(src.x, src.y, src.z);
	}

	physx::PxVec3 vec3_to_physx_vec3(const glm::vec3 &src)
	{
		return physx::PxVec3(src.x, src.y, src.z);
	}

	physx::PxMat33 mat3_to_physx_mat3(const glm::mat3 &src)
	{
		return physx::PxMat33(
			physx::PxVec3(src[0][0], src[0][1], src[0][2]),
			physx::PxVec3(src[1][0], src[1][1], src[1][2]),
			physx::PxVec3(src[2][0], src[2][1], src[2][2])
		);
	}

	physx::PxTransform get_physx_trans(const glm::vec3 &coord, const glm::vec4 &quat)
	{
		return physx::PxTransform(vec3_to_physx_vec3(coord), physx::PxQuat(quat.x, quat.y, quat.z, quat.w));
	}

	physx::PxTransform get_physx_trans(const glm::vec3 &coord, const glm::mat3 &axis)
	{
		return physx::PxTransform(vec3_to_physx_vec3(coord), physx::PxQuat(mat3_to_physx_mat3(axis)));
	}

	std::string shapeTypeName(ShapeType t)
	{
		char *names[] = {
			"Box",
			"Sphere",
			"Capsule",
			"Plane",
			"Convex Mesh",
			"Triangle Mesh",
			"Height Field"
		};
		return names[(int)t];
	}

	Shape::Shape()
	{
	}

	Shape::Shape(ShapeType _type)
		:type(_type)
	{
	}

	float Shape::getVolume() const
	{
		switch (type)
		{
		case ShapeType::box:
			return scale.x * scale.y * scale.z * 8.f;
		case ShapeType::sphere:
			return 4.f * scale.x * scale.x * scale.x * M_PI / 3.f;
		case ShapeType::capsule:
			return 4.f * scale.x * scale.x * scale.x * M_PI / 3.f + M_PI * scale.x * scale.x * scale.y;
		}
		return 0.f;
	}

	Rigidbody::Rigidbody()
	{
	}

	Rigidbody::Rigidbody(RigidbodyType _type)
		:type(_type)
	{
	}

	Shape *Rigidbody::new_shape()
	{
		auto s = new Shape;
		shapes.emplace_back(s);
		return s;
	}

	void Rigidbody::remove_shape(Shape *s)
	{
		for (auto it = shapes.begin(); it != shapes.end(); it++)
		{
			if (it->get() == s)
			{
				shapes.erase(it);
				return;
			}
		}
	}

	physx::PxFoundation *pxFoundation = nullptr;
	physx::PxPhysics *pxPhysics = nullptr;
	physx::PxMaterial *pxDefaultMaterial = nullptr;

	physx::PxRigidActor *createStaticRigidActor(physx::PxTransform &trans)
	{
		auto body = pxPhysics->createRigidStatic(trans);
		return body;
	}

	physx::PxRigidActor *createDynamicRigidActor(physx::PxTransform &trans, bool kinematic, float density)
	{
		auto body = pxPhysics->createRigidDynamic(trans);
		physx::PxRigidBodyExt::updateMassAndInertia(*body, density);
		if (kinematic)
			body->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);
		return body;
	}

	void createPhysicsScene()
	{
		//	auto group1ID = 1;
		//	for (auto g : scene->pCollisionGroups)
		//	{
		//		for (int i = 0; i < 16; i++)
		//		{
		//			if ((g->originalmask & (1 << i)) == 0)
		//			{
		//				auto group2ID = 1;
		//				for (auto h : scene->pCollisionGroups)
		//				{
		//					if (h->originalID == i)
		//					{
		//						PxSetGroupCollisionFlag(group1ID, group2ID, false);
		//					}
		//					group2ID++;
		//				}
		//			}
		//		}
		//		group1ID++;
		//	}

		//	//PxSetGroupCollisionFlag(3, 3, false);
		//	//PxSetGroupCollisionFlag(3, 4, false);
		//	//PxSetGroupCollisionFlag(4, 4, false);
		//	//for (int i = 0; i < 100; i++)
		//	//	for (int j = 0; j < 100; j++)
		//	//		PxSetGroupCollisionFlag(i + 1, j + 1, false);
		//}
	}

	//void destoryPhysicsScene()
	//{
	//	pxControllerManager->release();
	//	pxScene->release();
	//}

	void initPhysics()
	{
		//static auto allocator = physx::PxDefaultAllocator();
		//static auto errorCallBack = physx::PxDefaultErrorCallback();
		//pxFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, allocator, errorCallBack);
		//pxPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *pxFoundation, physx::PxTolerancesScale());
		//pxDefaultMaterial = pxPhysics->createMaterial(0.5f, 0.5f, 0.6f);
	}
}
