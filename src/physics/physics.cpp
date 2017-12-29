#include "physics.h"

namespace tke
{
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
		auto size = getScale();
		switch (type)
		{
		case ShapeType::box:
			return size.x * size.y * size.z * 8.f;
		case ShapeType::sphere:
			return 4.f * size.x * size.x * size.x * M_PI / 3.f;
		case ShapeType::capsule:
			return 4.f * size.x * size.x * size.x * M_PI / 3.f + M_PI * size.x * size.x * size.y;
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
		static auto magicNumber = 0;
		auto s = std::make_unique<Shape>();
		s->id = magicNumber++;
		shapes.push_back(std::move(s));
		return s.get();
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
		static auto allocator = physx::PxDefaultAllocator();
		static auto errorCallBack = physx::PxDefaultErrorCallback();
		pxFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, allocator, errorCallBack);
		pxPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *pxFoundation, physx::PxTolerancesScale());
		pxDefaultMaterial = pxPhysics->createMaterial(0.5f, 0.5f, 0.6f);
	}
}
