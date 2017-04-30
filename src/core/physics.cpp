#include "core.h"

namespace tke
{
	void iniPhysics()
	{
		//static auto allocator = PxDefaultAllocator();
		//static auto errorCallBack = PxDefaultErrorCallback();
		//pxFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, allocator, errorCallBack);
		//pxPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *pxFoundation, PxTolerancesScale());
		//pxMaterial = pxPhysics->createMaterial(0.5f, 0.5f, 0.6f);
	}

	//static PxRigidActor *_createRigidActor(PxTransform &trans, bool dynamic, float d)
	//{
	//	auto body = pxPhysics->createRigidDynamic(trans);
	//	PxRigidBodyExt::updateMassAndInertia(*body, d);
	//	if (!dynamic)
	//		body->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
	//	return body;
	//}

	void createPhysicsScene()
	{
		//PxSceneDesc sceneDesc(pxPhysics->getTolerancesScale());
		//sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
		////sceneDesc.gravity = PxVec3(0.0f, 0.f, 0.0f);
		//sceneDesc.cpuDispatcher = PxDefaultCpuDispatcherCreate(2);
		//sceneDesc.filterShader = PxDefaultSimulationFilterShader;
		//pxScene = pxPhysics->createScene(sceneDesc);
		//pxControllerManager = PxCreateControllerManager(*pxScene);
		//pxScene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.f);
		////scene->setVisualizationParameter(PxVisualizationParameter::eACTOR_AXES, 1.f);
		//pxScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.f);
		////scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_DYNAMIC, 1.f);
		//pxScene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LOCAL_FRAMES, 1.f);
		////scene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LIMITS, 1.f);

		//auto pO0 = tke3_scene->pObjects[0];
		//auto pO1 = tke3_scene->pObjects[1];

		//auto coord0 = pO0->getCoord();
		//auto coord1 = pO1->getCoord();

		//testAct0 = physics->createRigidStatic(PxTransform(coord0.x, coord0.y, coord0.z, PxQuat(PxMat33(
		//	PxVec3(pO0->getAxis()[0][0], pO0->getAxis()[0][1], pO0->getAxis()[0][2]),
		//	PxVec3(pO0->getAxis()[1][0], pO0->getAxis()[1][1], pO0->getAxis()[1][2]),
		//	PxVec3(pO0->getAxis()[2][0], pO0->getAxis()[2][1], pO0->getAxis()[2][2])))));
		//testAct0->createShape(PxBoxGeometry(0.5f * pO0->getScale().x, 0.5f * pO0->getScale().y, 0.5f * pO0->getScale().z), *material, PxTransform(PxVec3(0, 0, 0)));
		//PxRigidDynamic *body = physics->createRigidDynamic(PxTransform(coord1.x, coord1.y, coord1.z, PxQuat(PxMat33(
		//	PxVec3(pO1->getAxis()[0][0], pO1->getAxis()[0][1], pO1->getAxis()[0][2]),
		//	PxVec3(pO1->getAxis()[1][0], pO1->getAxis()[1][1], pO1->getAxis()[1][2]),
		//	PxVec3(pO1->getAxis()[2][0], pO1->getAxis()[2][1], pO1->getAxis()[2][2])))));
		//PxRigidBodyExt::updateMassAndInertia(*body, 10.f);
		//testAct1 = body;
		//auto t =  PxTransform(PxVec3(0, 0, 0));
		//testAct1->createShape(PxBoxGeometry(0.5f * pO1->getScale().x, 0.5f * pO1->getScale().y, 0.5f * pO1->getScale().z), *material, t);

		////PxSetGroupCollisionFlag(1, 2, false);

		////PxSetGroup(*testAct0, 1);
		////PxSetGroup(*testAct1, 2);

		//scene->addActor(*testAct0);
		//scene->addActor(*testAct1);

		////auto j = PxDistanceJointCreate(*physics, testAct0, PxTransform(coord0.x, coord0.y, coord0.z), testAct1, PxTransform(-coord1.x, -coord1.y, -coord1.z));
		////auto j = PxSphericalJointCreate(*physics, testAct0, PxTransform(-coord0.x, -coord0.y, -coord0.z), testAct1, PxTransform(-coord1.x, -coord1.y, -coord1.z));
		////auto j = PxD6JointCreate(*physics, testAct0, PxTransform(-coord0.x, -coord0.y, -coord0.z, PxQuat(PxPi / 2, PxVec3(0, 1, 0))), testAct1, PxTransform(-coord1.x, -coord1.y, -coord1.z, PxQuat(PxPi / 2, PxVec3(0, 1, 0))));
		////auto j = PxD6JointCreate(*physics, testAct0, PxTransform(-coord0.x, -coord0.y, -coord0.z, PxQuat(PxPi / 2, PxVec3(0, 1, 0))), testAct1, PxTransform(-coord1.x, -coord1.y, -coord1.z, PxQuat(PxPi / 2, PxVec3(0, 1, 0))));
		//auto theAxis0 = pO1->getAxis();
		//auto theTrans0 = PxTransform(PxQuat(PxMat33(
		//	PxVec3(theAxis0[0][0], theAxis0[0][1], theAxis0[0][2]),
		//	PxVec3(theAxis0[1][0], theAxis0[1][1], theAxis0[1][2]),
		//	PxVec3(theAxis0[2][0], theAxis0[2][1], theAxis0[2][2]))));
		//auto theAxis1 = glm::transpose(pO1->getAxis());
		//auto theTrans1 = PxTransform(PxQuat(PxMat33(
		//	PxVec3(theAxis1[0][0], theAxis1[0][1], theAxis1[0][2]),
		//	PxVec3(theAxis1[1][0], theAxis1[1][1], theAxis1[1][2]),
		//	PxVec3(theAxis1[2][0], theAxis1[2][1], theAxis1[2][2]))));
		////auto j = PxD6JointCreate(*physics, testAct0, theTrans0 * PxTransform(-coord0.x, -coord0.y, -coord0.z), testAct1, theTrans1 * PxTransform(-coord1.x, -coord1.y, -coord1.z));
		//auto j = PxD6JointCreate(*physics, testAct0, theTrans0 * PxTransform(-coord0.x, -coord0.y, -coord0.z), testAct1, theTrans1 * PxTransform(-coord1.x, -coord1.y, -coord1.z));
		////j->setMotion(PxD6Axis::eTWIST, PxD6Motion::eFREE);
		//j->setMotion(PxD6Axis::eSWING2, PxD6Motion::eFREE);
		////j->setMotion(PxD6Axis::eSWING1, PxD6Motion::eFREE);
		////j->setTwistLimit(PxJointAngularLimitPair(-5, 15));
		////j->setSwingLimit(PxJointLimitCone(PxPi / 1, PxPi / 10));
		////j->setTwistLimit(PxJointAngularLimitPair(-PxPi / 2, -PxPi / 4));
		////j->setLimitCone(PxJointLimitCone(PxPi / 2, PxPi));
		////j->setSphericalJointFlag(PxSphericalJointFlag::eLIMIT_ENABLED, true);
		////j->setMaxDistance(10.0f);
		////j->setDistanceJointFlag(PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, true);
		//j->setConstraintFlag(PxConstraintFlag::eVISUALIZATION, true);

		//return;


		//for (int i = 0; i < 100; i++)
		//	for (int j = 0; j < 100; j++)
		//		PxSetGroupCollisionFlag(i, j, false);

		//	if (engine->controllingObject)
		//	{
		//		auto pObject = engine->controllingObject;

		//		auto pModel = pObject->pModel;

		//		pObject->floatingTime = 0.f;

		//		PxCapsuleControllerDesc capsuleDesc;
		//		capsuleDesc.height = pModel->controllerHeight;
		//		capsuleDesc.radius = pModel->controllerRadius;
		//		capsuleDesc.climbingMode = PxCapsuleClimbingMode::eCONSTRAINED;
		//		capsuleDesc.material = pxMaterial;
		//		PxExtendedVec3 v;
		//		v.x = pModel->controllerPosition.x * pObject->getScale().x + pObject->getCoord().x;
		//		v.y = pModel->controllerPosition.y * pObject->getScale().y + pObject->getCoord().y;
		//		v.z = pModel->controllerPosition.z * pObject->getScale().z + pObject->getCoord().z;
		//		capsuleDesc.position = v;
		//		capsuleDesc.stepOffset = capsuleDesc.radius;

		//		pxController = pxControllerManager->createController(capsuleDesc);
		//	}

		//	for (auto pObject : scene->pObjects)
		//	{
		//		//if (pObject == tke3_controlingObject)
		//		//{
		//		//	continue;
		//		//}

		//		auto pModel = pObject->pModel;

		//		if (pModel->rigidbodies.size() == 0)
		//			continue;

		//		auto objScale = pObject->getScale();
		//		auto objCoord = pObject->getCoord();
		//		auto objAxis = pObject->getAxis();
		//		PxTransform trans(objCoord.x, objCoord.y, objCoord.z, PxQuat(PxMat33(
		//			PxVec3(objAxis[0][0], objAxis[0][1], objAxis[0][2]),
		//			PxVec3(objAxis[1][0], objAxis[1][1], objAxis[1][2]),
		//			PxVec3(objAxis[2][0], objAxis[2][1], objAxis[2][2]))));
		//		int id = 0;
		//		for (auto r : pModel->rigidbodies)
		//		{
		//			//if (/*r->mode != tke3RigidBody::Mode::eStatic &&*/ (id != 0 && (id < 6 || id > 19)))
		//			//{
		//			//	id++;
		//			//	continue;
		//			//}

		//			//if (r->mode == tke3RigidBody::Mode::eStatic/* && id != pModel->joints[0]->rigid1ID*/)
		//			//	continue;

		//			r->collisionGroupID = scene->getCollisionGroupID(r->originCollisionGroupID, r->originCollisionFreeFlag);

		//			auto rigidCoord = r->getCoord();
		//			if (r->boneID != -1) rigidCoord += pModel->bones[r->boneID].rootCoord;
		//			rigidCoord *= objScale;
		//			auto rigidAxis = r->getAxis();

		//			pObject->rigidDatas[id].rotation = objAxis * rigidAxis;
		//			pObject->rigidDatas[id].coord = objCoord + objAxis * rigidCoord;
		//			auto actor = _createRigidActor(trans * PxTransform(rigidCoord.x, rigidCoord.y, rigidCoord.z, PxQuat(PxMat33(
		//				PxVec3(rigidAxis[0][0], rigidAxis[0][1], rigidAxis[0][2]),
		//				PxVec3(rigidAxis[1][0], rigidAxis[1][1], rigidAxis[1][2]),
		//				PxVec3(rigidAxis[2][0], rigidAxis[2][1], rigidAxis[2][2])))), r->mode != Rigidbody::Mode::eStatic, r->density);
		//			r->phyActor = actor;
		//			for (auto s : r->shapes)
		//			{
		//				//if (r->mode == tke3RigidBody::Mode::eStatic)
		//				//	break;

		//				glm::vec3 coord = s->getCoord() * objScale;
		//				glm::mat3 axis = s->getAxis();
		//				glm::vec3 scale = s->getScale() * objScale;
		//				PxTransform trans(coord.x, coord.y, coord.z, PxQuat(PxMat33(
		//					PxVec3(axis[0][0], axis[0][1], axis[0][2]),
		//					PxVec3(axis[1][0], axis[1][1], axis[1][2]),
		//					PxVec3(axis[2][0], axis[2][1], axis[2][2]))));
		//				switch (s->type)
		//				{
		//				case Shape::Type::eBox:
		//					actor->createShape(PxBoxGeometry(scale[0], scale[1], scale[2]), *pxMaterial, trans);
		//					break;
		//				case Shape::Type::eSphere:
		//					actor->createShape(PxSphereGeometry(scale[0]), *pxMaterial, trans);
		//					break;
		//				case Shape::Type::eCapsule:
		//					actor->createShape(PxCapsuleGeometry(scale[0], scale[1]), *pxMaterial, trans * PxTransform(PxQuat(PxHalfPi, PxVec3(0, 0, 1))));
		//					break;
		//				}
		//			}
		//			pxScene->addActor(*actor);
		//			PxSetGroup(*actor, r->collisionGroupID + 1);

		//			//if (r->mode != tke3RigidBody::Mode::eStatic)
		//			//	break;

		//			id++;
		//		}

		//		int jID = 0;
		//		for (auto j : pModel->joints)
		//		{
		//			if (j->rigid0ID == j->rigid1ID)
		//			{
		//				jID++;
		//				continue;
		//			}
		//			//if ((j->rigid0ID != 0 && (j->rigid0ID < 6 || j->rigid0ID > 19)) || (j->rigid1ID != 0 && (j->rigid1ID < 6 || j->rigid1ID > 19)))
		//			//{
		//			//	jID++;
		//			//	continue;
		//			//}

		//			auto pR0 = pModel->rigidbodies[j->rigid0ID];
		//			auto pR1 = pModel->rigidbodies[j->rigid1ID];
		//			auto coord0 = (pR0->getCoord() + pModel->bones[pR0->boneID].rootCoord);
		//			coord0 = j->getCoord() - coord0;
		//			//coord0 = - coord0;
		//			auto coord1 = (pR1->getCoord() + pModel->bones[pR1->boneID].rootCoord);
		//			coord1 = j->getCoord() - coord1;
		//			//coord1 =  - coord1;
		//			auto axis = j->getAxis();
		//			auto axis0 = glm::transpose(pR0->getAxis());
		//			auto axis1 = glm::transpose(pR1->getAxis());
		//			auto t = PxTransform(PxQuat(PxMat33(
		//				PxVec3(axis[0][0], axis[0][1], axis[0][2]),
		//				PxVec3(axis[1][0], axis[1][1], axis[1][2]),
		//				PxVec3(axis[2][0], axis[2][1], axis[2][2]))));
		//			auto t0 = PxTransform(PxQuat(PxMat33(
		//				PxVec3(axis0[0][0], axis0[0][1], axis0[0][2]),
		//				PxVec3(axis0[1][0], axis0[1][1], axis0[1][2]),
		//				PxVec3(axis0[2][0], axis0[2][1], axis0[2][2]))));
		//			auto t1 = PxTransform(PxQuat(PxMat33(
		//				PxVec3(axis1[0][0], axis1[0][1], axis1[0][2]),
		//				PxVec3(axis1[1][0], axis1[1][1], axis1[1][2]),
		//				PxVec3(axis1[2][0], axis1[2][1], axis1[2][2]))));
		//			//auto j = PxSphericalJointCreate(*physics, (PxRigidActor*)pR0->phyActor, t * PxTransform(PxVec3(coord0.x, coord0.y, coord0.z)),
		//			//	(PxRigidActor*)pR1->phyActor, t * PxTransform(PxVec3(coord1.x, coord1.y, coord1.z)));
		//			auto p = PxD6JointCreate(*pxPhysics, (PxRigidActor*)pR0->phyActor, t0 * PxTransform(PxVec3(coord0.x, coord0.y, coord0.z)) * t,
		//				(PxRigidActor*)pR1->phyActor, t1 * PxTransform(PxVec3(coord1.x, coord1.y, coord1.z)) * t);
		//			p->setConstraintFlag(PxConstraintFlag::Enum::eCOLLISION_ENABLED, true);
		//			p->setSwingLimit(PxJointLimitCone(PxPi / 4, PxPi / 4));
		//			p->setMotion(PxD6Axis::eSWING1, PxD6Motion::eLIMITED);
		//			p->setMotion(PxD6Axis::eSWING1, PxD6Motion::eLIMITED);
		//			//auto p = PxSphericalJointCreate(*physics, (PxRigidActor*)pR0->phyActor, PxTransform(PxVec3(coord0.x, coord0.y, coord0.z), PxQuat(PxMat33(
		//			//	PxVec3(axis1[0][0], axis1[0][1], axis1[0][2]),
		//			//	PxVec3(axis1[1][0], axis1[1][1], axis1[1][2]),
		//			//	PxVec3(axis1[2][0], axis1[2][1], axis1[2][2])))),
		//			//	(PxRigidActor*)pR1->phyActor, PxTransform(PxVec3(coord1.x, coord1.y, coord1.z), PxQuat(PxMat33(
		//			//		PxVec3(axis1[0][0], axis1[0][1], axis1[0][2]),
		//			//		PxVec3(axis1[1][0], axis1[1][1], axis1[1][2]),
		//			//		PxVec3(axis1[2][0], axis1[2][1], axis1[2][2])))));

		//			//break;
		//			//if (jID == 0)
		//			//p->setConstraintFlag(PxConstraintFlag::eVISUALIZATION, true);
		//			jID++;
		//		}

		//		//pAcotr->setName();
		//	}

		//	for (auto pTerrain : scene->pTerrains)
		//	{
		//		if (pTerrain->heightMap)
		//		{
		//			auto m = pTerrain->heightMap;

		//			PxU32 numVerts = pTerrain->blockCx * pTerrain->blockCy;

		//			auto samples = new PxHeightFieldSample[numVerts];
		//			memset(samples, 0, numVerts * sizeof(PxHeightFieldSample));

		//			int xOffset = m->m_width / pTerrain->blockCx;
		//			int yOffset = m->m_height / pTerrain->blockCy;

		//			for (PxU32 x = 0; x < pTerrain->blockCx; x++)
		//			{
		//				for (PxU32 y = 0; y < pTerrain->blockCy; y++)
		//					samples[x + y * pTerrain->blockCx].height = m->getPixel(y * yOffset, x * xOffset, 0);
		//			}

		//			PxHeightFieldDesc hfDesc;
		//			hfDesc.format = PxHeightFieldFormat::eS16_TM;
		//			hfDesc.nbColumns = pTerrain->blockCx;
		//			hfDesc.nbRows = pTerrain->blockCy;
		//			hfDesc.samples.data = samples;
		//			hfDesc.samples.stride = sizeof(PxHeightFieldSample);

		//			auto heightField = pxPhysics->createHeightField(hfDesc);

		//			auto hfActor = pxPhysics->createRigidStatic(PxTransform(PxIdentity));

		//			PxHeightFieldGeometry hfGeom(heightField, PxMeshGeometryFlags(), pTerrain->height / 255.f, pTerrain->blockSize, pTerrain->blockSize);
		//			hfActor->createShape(hfGeom, *pxMaterial);

		//			pxScene->addActor(*hfActor);

		//			delete[]samples;
		//		}
		//	}

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

		//void destoryPhysicsScene()
		//{
		//	pxControllerManager->release();
		//	pxScene->release();
		//}

		//void syncPhysics()
		//{
		//	//{
		//	//	scene->simulate(1.f / 60.f);
		//	//	scene->fetchResults(true);

		//	//	auto trans = testAct1->getGlobalPose();
		//	//	auto coord = glm::vec3(trans.p.x, trans.p.y, trans.p.z);
		//	//	auto quat = glm::vec4(trans.q.x, trans.q.y, trans.q.z, trans.q.w);
		//	//	tke3_scene->pObjects[1]->setCoord(coord);
		//	//	tke3_scene->pObjects[1]->setQuat(quat);
		//	//}

		//	//return;

		//	if (engine->controllingObject)
		//	{
		//		auto pObject = engine->controllingObject;

		//		PxVec3 disp;
		//		disp.y = -0.001f * pObject->floatingTime * pObject->floatingTime;
		//		pObject->floatingTime += 1.f;
		//		disp.y = -0.1f;

		//		auto e = pObject->getEuler();
		//		auto v = pObject->controller.move(e.x);

		//		pObject->addEuler(glm::vec3(e.x, 0.f, 0.f));

		//		disp.x = v.x;
		//		disp.z = v.z;

		//		if (pxController->move(disp, 0.f, 1.f / 60.f, nullptr)& PxControllerCollisionFlag::eCOLLISION_DOWN)
		//			pObject->floatingTime = 0.f;
		//	}

		//	pxScene->simulate(1.f / 60.f);
		//	pxScene->fetchResults(true);

		//	if (true)
		//	{
		//		for (auto pObject : scene->pObjects)
		//		{
		//			auto objScale = pObject->getScale();
		//			auto objCoord = pObject->getCoord();
		//			auto objAxis = pObject->getAxis();
		//			PxTransform objTrans(objCoord.x, objCoord.y, objCoord.z, PxQuat(PxMat33(
		//				PxVec3(objAxis[0][0], objAxis[0][1], objAxis[0][2]),
		//				PxVec3(objAxis[1][0], objAxis[1][1], objAxis[1][2]),
		//				PxVec3(objAxis[2][0], objAxis[2][1], objAxis[2][2]))));
		//			auto pModel = pObject->pModel;
		//			int id = 0;
		//			for (auto r : pModel->rigidbodies)
		//			{
		//				//if (/*r->mode != tke3RigidBody::Mode::eStatic && */(id != 0 && (id < 6 || id > 19)))
		//				//{
		//				//	id++;
		//				//	continue;
		//				//}
		//				//if (id != 41 && id != 42 && id != 33)
		//				//{
		//				//	id++;
		//				//	continue;
		//				//}

		//				PxRigidActor *body = (PxRigidActor*)r->phyActor;
		//				if (r->boneID == -1)
		//				{
		//					auto trans = body->getGlobalPose();
		//					auto coord = glm::vec3(trans.p.x, trans.p.y, trans.p.z);
		//					auto quat = glm::vec4(trans.q.x, trans.q.y, trans.q.z, trans.q.w);
		//					pObject->setCoord(coord);
		//					pObject->setQuat(quat);
		//					glm::mat3 axis;
		//					Math::quaternionToMatrix(quat, axis);
		//					pObject->rigidDatas[id].coord = coord;
		//					pObject->rigidDatas[id].rotation = axis;
		//				}
		//				else
		//				{
		//					auto solver = pObject->animationSolver;
		//					if (r->mode == Rigidbody::Mode::eStatic)
		//					{
		//						auto pBone = &pModel->bones[r->boneID];
		//						auto coord = objAxis * (glm::vec3(solver->boneMatrix[r->boneID][3]) + glm::mat3(solver->boneMatrix[r->boneID]) * r->getCoord()) * objScale + objCoord;
		//						auto axis = objAxis * glm::mat3(solver->boneMatrix[r->boneID]) * r->getAxis();
		//						PxTransform trans(coord.x, coord.y, coord.z, PxQuat(PxMat33(
		//							PxVec3(axis[0][0], axis[0][1], axis[0][2]),
		//							PxVec3(axis[1][0], axis[1][1], axis[1][2]),
		//							PxVec3(axis[2][0], axis[2][1], axis[2][2]))));
		//						((PxRigidDynamic*)body)->setKinematicTarget(trans);
		//						pObject->rigidDatas[id].coord = coord;
		//						pObject->rigidDatas[id].rotation = axis;
		//					}
		//					else
		//					{
		//						auto objAxisT = glm::transpose(objAxis);
		//						auto rigidAxis = r->getAxis();
		//						auto rigidAxisT = glm::transpose(rigidAxis);
		//						auto trans = body->getGlobalPose();
		//						auto coord = glm::vec3(trans.p.x, trans.p.y, trans.p.z);
		//						glm::mat3 axis;
		//						Math::quaternionToMatrix(glm::vec4(trans.q.x, trans.q.y, trans.q.z, trans.q.w), axis);
		//						pObject->rigidDatas[id].coord = coord;
		//						pObject->rigidDatas[id].rotation = axis;
		//						auto boneAxis = objAxisT * axis * rigidAxisT;
		//						glm::vec3 boneCoord;
		//						if (r->mode != Rigidbody::Mode::eDynamicLockLocation)
		//							boneCoord = (objAxisT * (coord - objCoord) - boneAxis * (r->getCoord() * objScale)) / objScale;
		//						else
		//							boneCoord = glm::vec3(solver->boneMatrix[r->boneID][3]);
		//						solver->boneMatrix[r->boneID] = Math::makeMatrix(boneAxis, boneCoord);
		//					}
		//				}

		//				id++;
		//			}
		//		}
		//	}

		//	if (engine->controllingObject)
		//	{
		//		auto pObject = engine->controllingObject;

		//		auto p = pxController->getPosition();
		//		glm::vec3 v = glm::vec3(p.x, p.y, p.z);
		//		v -= pObject->pModel->controllerPosition * pObject->getScale();
		//		pObject->setCoord(v);
		//	}
	}

	void viewPhysicsDebug()
	{
		return;
		//auto &rb = scene->getRenderBuffer();
		////for (int i = 0; i < rb.getNbPoints(); i++)
		////{
		////	p.color = _intToRGB(rb.getPoints()[i].color);
		////	p.coord = _pxVec3ToVec3(rb.getPoints()[i].pos);
		////	tke3_debugBuffer.points.push_back(p);
		////}
		//auto lineCount = rb.getNbLines();
		//for (int i = 0; i < lineCount; i++)
		//{
		//	auto &line = rb.getLines()[i];
		//	tke_dynamicVertexBuffer[i * 12 + 0] = line.pos0.x;
		//	tke_dynamicVertexBuffer[i * 12 + 1] = line.pos0.y;
		//	tke_dynamicVertexBuffer[i * 12 + 2] = line.pos0.z;
		//	tke_dynamicVertexBuffer[i * 12 + 3] = line.color0 % 256;
		//	tke_dynamicVertexBuffer[i * 12 + 4] = (line.color0 / 256) % 256;
		//	tke_dynamicVertexBuffer[i * 12 + 5] = (line.color0 / 65536) % 256;
		//	tke_dynamicVertexBuffer[i * 12 + 6] = line.pos1.x;
		//	tke_dynamicVertexBuffer[i * 12 + 7] = line.pos1.y;
		//	tke_dynamicVertexBuffer[i * 12 + 8] = line.pos1.z;
		//	tke_dynamicVertexBuffer[i * 12 + 9] = line.color1 % 256;
		//	tke_dynamicVertexBuffer[i * 12 + 10] = (line.color1 / 256) % 256;
		//	tke_dynamicVertexBuffer[i * 12 + 11] = (line.color1 / 65536) % 256;
		//}
		//glNamedBufferSubData(tke_dynamicVertexVBO, 0, sizeof(GLfloat) * 12 * lineCount, tke_dynamicVertexBuffer);
		//glBindVertexArray(tke3_dynamicVertexVAO);
		//glDrawArrays(GL_LINES, 0, lineCount * 2);

		//auto triangleCount = rb.getNbTriangles();
		//for (int i = 0; i < triangleCount; i++)
		//{
		//	auto &triangle = rb.getTriangles()[i];
		//	tke_dynamicVertexBuffer[i * 18 + 0] = triangle.pos0.x;
		//	tke_dynamicVertexBuffer[i * 18 + 1] = triangle.pos0.y;
		//	tke_dynamicVertexBuffer[i * 18 + 2] = triangle.pos0.z;
		//	tke_dynamicVertexBuffer[i * 18 + 3] = triangle.color0 % 256;
		//	tke_dynamicVertexBuffer[i * 18 + 4] = (triangle.color0 / 256) % 256;
		//	tke_dynamicVertexBuffer[i * 18 + 5] = (triangle.color0 / 65536) % 256;
		//	tke_dynamicVertexBuffer[i * 18 + 6] = triangle.pos1.x;
		//	tke_dynamicVertexBuffer[i * 18 + 7] = triangle.pos1.y;
		//	tke_dynamicVertexBuffer[i * 18 + 8] = triangle.pos1.z;
		//	tke_dynamicVertexBuffer[i * 18 + 9] = triangle.color1 % 256;
		//	tke_dynamicVertexBuffer[i * 18 + 10] = (triangle.color1 / 256) % 256;
		//	tke_dynamicVertexBuffer[i * 18 + 11] = (triangle.color1 / 65536) % 256;
		//	tke_dynamicVertexBuffer[i * 18 + 12] = triangle.pos2.x;
		//	tke_dynamicVertexBuffer[i * 18 + 13] = triangle.pos2.y;
		//	tke_dynamicVertexBuffer[i * 18 + 14] = triangle.pos2.z;
		//	tke_dynamicVertexBuffer[i * 18 + 15] = triangle.color2 % 256;
		//	tke_dynamicVertexBuffer[i * 18 + 16] = (triangle.color2 / 256) % 256;
		//	tke_dynamicVertexBuffer[i * 18 + 17] = (triangle.color2 / 65536) % 256;
		//}
		//glNamedBufferSubData(tke_dynamicVertexVBO, 0, sizeof(GLfloat) * 18 * triangleCount, tke_dynamicVertexBuffer);
		//glBindVertexArray(tke3_dynamicVertexVAO);
		//glDrawArrays(GL_TRIANGLES, 0, triangleCount * 3);
	}
}
