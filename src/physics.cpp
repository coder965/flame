#include "physics.h"

namespace tke
{
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
