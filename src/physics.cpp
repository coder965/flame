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
