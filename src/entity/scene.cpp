#include <map>

#include "scene.h"
#include "../math/math.h"
#include "../core.h"
#include "../render/renderpass.h"
#include "../render/synchronization.h"

namespace tke
{
	static const float gravity = 9.81f;

	Scene::Scene()
	{
		physx::PxSceneDesc pxSceneDesc(pxPhysics->getTolerancesScale());
		pxSceneDesc.gravity = physx::PxVec3(0.0f, -gravity, 0.0f);
		pxSceneDesc.cpuDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
		pxSceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
		pxScene = pxPhysics->createScene(pxSceneDesc);
		pxControllerManager = PxCreateControllerManager(*pxScene);

		sunLight = new Light(LightType::parallax);
		//sunLight->shadow = true;
		//_setSunLight_attribute(this);
		//addLight(sunLight);
	}

	Scene::~Scene()
	{
		pxControllerManager->release();
		pxScene->release();
	}

	void Scene::addLight(Light *l) // when a light is added to scene, the owner is the scene, light cannot be deleted elsewhere
	{
		mtx.lock();
		lights.push_back(std::move(std::unique_ptr<Light>(l)));
		needUpdateLightCount = true;
		mtx.unlock();
	}

	Light *Scene::removeLight(Light *l)
	{
		mtx.lock();
		for (auto it = lights.begin(); it != lights.end(); it++)
		{
			if (it->get() == l)
			{
				for (auto itt = it + 1; itt != lights.end(); itt++)
				{
					(*itt)->sceneIndex--;
					(*itt)->changed = true;
				}
				delete l;
				it = lights.erase(it);
				l = it == lights.end() ? nullptr : it->get();
				break;
			}
		}
		needUpdateLightCount = true;
		mtx.unlock();
		return l;
	}

	static int _objectMagicIndex = 0;

	void Scene::addObject(Object *o) // when a object is added to scene, the owner is the scene, object cannot be deleted elsewhere
									 // and, if object has physics componet, it can be only moved by physics
	{
		auto m = o->model;
		if (!m)
		{
			delete o;
			return;
		}

		if (o->name == "")
		{
			o->name = std::to_string(_objectMagicIndex);
			_objectMagicIndex++;
		}

		mtx.lock();

		// since object can move to somewhere first, we create physics component here
		if (o->physics_type != 0)
		{
			if (m->rigidbodies.size() > 0)
			{
				auto objScale = o->getScale();
				auto objCoord = o->getCoord();
				auto objAxis = o->getAxis();
				physx::PxTransform objTrans(objCoord.x, objCoord.y, objCoord.z, physx::PxQuat(physx::PxMat33(
					physx::PxVec3(objAxis[0][0], objAxis[0][1], objAxis[0][2]),
					physx::PxVec3(objAxis[1][0], objAxis[1][1], objAxis[1][2]),
					physx::PxVec3(objAxis[2][0], objAxis[2][1], objAxis[2][2]))));

				for (auto r : m->rigidbodies)
				{
					ObjectRigidBodyData rigidbodyData;
					rigidbodyData.rigidbody = r;

					auto rigidCoord = r->getCoord();
					if (r->boneID != -1) rigidCoord += m->bones[r->boneID].rootCoord;
					rigidCoord *= objScale;
					auto rigidAxis = r->getAxis();

					rigidbodyData.rotation = objAxis * rigidAxis;
					rigidbodyData.coord = objCoord + objAxis * rigidCoord;
					physx::PxTransform rigTrans(rigidCoord.x, rigidCoord.y, rigidCoord.z, physx::PxQuat(physx::PxMat33(
						physx::PxVec3(rigidAxis[0][0], rigidAxis[0][1], rigidAxis[0][2]),
						physx::PxVec3(rigidAxis[1][0], rigidAxis[1][1], rigidAxis[1][2]),
						physx::PxVec3(rigidAxis[2][0], rigidAxis[2][1], rigidAxis[2][2]))));
					rigTrans = objTrans * rigTrans;
					auto actor = ((o->physics_type & (int)ObjectPhysicsType::dynamic) && (r->type == RigidbodyType::dynamic || r->type == RigidbodyType::dynamic_but_location)) ?
						createDynamicRigidActor(rigTrans, false, r->density) : createStaticRigidActor(rigTrans);

					for (auto &s : r->shapes)
					{
						glm::vec3 coord = s->getCoord() * objScale;
						glm::mat3 axis = s->getAxis();
						glm::vec3 scale = s->getScale() * objScale;
						physx::PxTransform trans(coord.x, coord.y, coord.z, physx::PxQuat(physx::PxMat33(
							physx::PxVec3(axis[0][0], axis[0][1], axis[0][2]),
							physx::PxVec3(axis[1][0], axis[1][1], axis[1][2]),
							physx::PxVec3(axis[2][0], axis[2][1], axis[2][2]))));
						switch (s->type)
						{
						case ShapeType::box:
							actor->createShape(physx::PxBoxGeometry(scale[0], scale[1], scale[2]), *pxDefaultMaterial, trans);
							break;
						case ShapeType::sphere:
							actor->createShape(physx::PxSphereGeometry(scale[0]), *pxDefaultMaterial, trans);
							break;
						case ShapeType::capsule:
							actor->createShape(physx::PxCapsuleGeometry(scale[0], scale[1]), *pxDefaultMaterial, trans * physx::PxTransform(physx::PxQuat(physx::PxHalfPi, physx::PxVec3(0, 0, 1))));
							break;
						}
					}

					rigidbodyData.actor = actor;

					o->rigidbodyDatas.push_back(rigidbodyData);

					pxScene->addActor(*actor);
				}
			}

			// create joints

			//		int jID = 0;
			//		for (auto j : pModel->joints)
			//		{
			//			if (j->rigid0ID == j->rigid1ID)
			//			{
			//				jID++;
			//				continue;
			//			}

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
		}

		if (o->physics_type & (int)ObjectPhysicsType::controller)
		{
			auto c = m->controller_position * o->getScale() + o->getCoord();
			physx::PxCapsuleControllerDesc capsuleDesc;
			capsuleDesc.radius = (m->controller_radius * o->getScale().x) / 0.8f;
			capsuleDesc.height = (m->controller_height * o->getScale().y) / 0.8f;
			capsuleDesc.climbingMode = physx::PxCapsuleClimbingMode::eCONSTRAINED;
			capsuleDesc.material = pxDefaultMaterial;
			capsuleDesc.position.x = c.x;
			capsuleDesc.position.y = c.y;
			capsuleDesc.position.z = c.z;
			capsuleDesc.stepOffset = capsuleDesc.radius;

			o->pxController = pxControllerManager->createController(capsuleDesc);
		}

		objects.push_back(std::move(std::unique_ptr<Object>(o)));

		needUpdateIndirectBuffer = true;
		mtx.unlock();
	}

	Object *Scene::removeObject(Object *o)
	{
		mtx.lock();
		for (auto it = objects.begin(); it != objects.end(); it++)
		{
			if (it->get() == o)
			{
				for (auto itt = it + 1; itt != objects.end(); itt++)
				{
					(*itt)->sceneIndex--;
					(*itt)->changed = true;
				}
				for (auto &r : o->rigidbodyDatas)
				{
					if (r.actor)
						r.actor->release();
				}
				if (o->pxController)
					o->pxController->release();
				delete o;
				it = objects.erase(it);
				o = it == objects.end() ? nullptr : it->get();
				break;
			}
		}
		needUpdateIndirectBuffer = true;
		mtx.unlock();
		return o;
	}

	int Scene::getCollisionGroupID(int ID, unsigned int mask)
	{
		if (mask == 0)
		{
			return -1;
		}
		auto count = pCollisionGroups.size();
		for (int i = 0; i < count; i++)
		{
			if (pCollisionGroups[i]->originalID == ID && pCollisionGroups[i]->originalmask == mask)
			{
				return i;
			}
		}
		auto c = new CollisionGroup;
		c->originalID = ID;
		c->originalmask = mask;
		pCollisionGroups.push_back(c);
		return count;
	}

	void Scene::addTerrain(Terrain *t) // when a terrain is added to scene, the owner is the scene, terrain cannot be deleted elsewhere
	{
		mtx.lock();

		if (t->use_physx && t->heightMap)
		{
			auto m = t->heightMap;

			auto numVerts = m->levels[0].cx * m->levels[0].cy;

			auto samples = new physx::PxHeightFieldSample[numVerts];
			memset(samples, 0, numVerts * sizeof(physx::PxHeightFieldSample));

			for (int y = 0; y < m->levels[0].cy; y++)
			{
				for (int x = 0; x < m->levels[0].cy; x++)
					samples[y + x * m->levels[0].cx].height = m->getR(x - 0.5, y - 0.5);
			}

			physx::PxHeightFieldDesc hfDesc;
			hfDesc.format = physx::PxHeightFieldFormat::eS16_TM;
			hfDesc.nbRows = m->levels[0].cx;
			hfDesc.nbColumns = m->levels[0].cy;
			hfDesc.samples.data = samples;
			hfDesc.samples.stride = sizeof(physx::PxHeightFieldSample);

			physx::PxHeightFieldGeometry hfGeom(pxPhysics->createHeightField(hfDesc), physx::PxMeshGeometryFlags(), t->height / 255.f, t->block_size * t->block_cx / m->levels[0].cx, t->block_size * t->block_cx / m->levels[0].cy);
			t->actor = pxPhysics->createRigidStatic(physx::PxTransform(physx::PxIdentity));
			t->actor->createShape(hfGeom, *pxDefaultMaterial);

			pxScene->addActor(*t->actor);

			delete[]samples;
		}

		terrain = std::unique_ptr<Terrain>(t);

		mtx.unlock();
	}

	void Scene::removeTerrain()
	{
		mtx.lock();

		if (terrain->actor)
			terrain->actor->release();

		terrain.reset();

		mtx.unlock();
	}

	void Scene::addWater(Water *w)
	{
		mtx.lock();
		waters.push_back(std::move(std::unique_ptr<Water>(w)));
		mtx.unlock();
	}

	Water *Scene::removeWater(Water *w)
	{
		mtx.lock();
		for (auto it = waters.begin(); it != waters.end(); it++)
		{
			if (it->get() == w)
			{
				for (auto itt = it + 1; itt != waters.end(); itt++)
				{
					//(*itt)->sceneIndex--;
					(*itt)->changed = true;
				}
				delete w;
				it = waters.erase(it);
				w = it == waters.end() ? nullptr : it->get();
				break;
			}
		}
		mtx.unlock();
		return w;
	}

	void Scene::reset()
	{
		needUpdateSky = false;
		needUpdateAmbientBuffer = false;
		needUpdateIndirectBuffer = false;
		needUpdateLightCount = false;
		camera.changed = false;
		for (auto &l : lights)
			l->changed = false;
		for (auto &o : objects)
			o->changed = false;
		if (terrain)
			terrain->changed = false;
		for (auto &w : waters)
			w->changed = false;
	}

	void Scene::clear()
	{
		mtx.lock();

		sunLight = nullptr;
		lights.clear();
		objects.clear();
		terrain.reset();

		mtx.unlock();
	}

	void Scene::setSunDir(const glm::vec2 &v)
	{
		sunLight->setEuler(glm::vec3(v.x, 0.f, v.y));
		needUpdateSky = true;
		needUpdateAmbientBuffer = true;
	}

	void Scene::setAmbientColor(const glm::vec3 &v)
	{
		ambientColor = v;
		needUpdateAmbientBuffer = true;
	}

	void Scene::setFogColor(const glm::vec3 &v)
	{
		fogColor = v;
		needUpdateAmbientBuffer = true;
	}

	void Scene::update()
	{
		// update animation and bones
		for (auto &o : objects)
		{
			if (o->animationComponent)
				o->animationComponent->update();
		}

		// update physics (controller should move first, then simulate, and then get the result coord)
		auto dist = 1.f / FPS;
		if (dist > 0.f)
		{
			for (auto &o : objects) // set controller coord
			{
				if (o->physics_type & (int)ObjectPhysicsType::controller)
				{
					glm::vec3 e, c;
					o->move(o->getEuler().x, c, e);
					o->addEuler(e);

					physx::PxVec3 disp(c.x, -gravity * o->floatingTime * o->floatingTime, c.z);
					o->floatingTime += dist;

					if (o->pxController->move(disp, 0.f, dist, nullptr) & physx::PxControllerCollisionFlag::eCOLLISION_DOWN)
						o->floatingTime = 0.f;
				}
			}
			pxScene->simulate(dist);
			//pxScene->simulate(1.f / 60.f);
			pxScene->fetchResults(true);
			for (auto &o : objects)
			{
				if (o->physics_type & (int)ObjectPhysicsType::dynamic)
				{
					auto pModel = o->model;

					auto objScale = o->getScale();
					auto objCoord = o->getCoord();
					auto objAxis = o->getAxis();
					physx::PxTransform objTrans(objCoord.x, objCoord.y, objCoord.z, physx::PxQuat(physx::PxMat33(
						physx::PxVec3(objAxis[0][0], objAxis[0][1], objAxis[0][2]),
						physx::PxVec3(objAxis[1][0], objAxis[1][1], objAxis[1][2]),
						physx::PxVec3(objAxis[2][0], objAxis[2][1], objAxis[2][2]))));

					for (auto &data : o->rigidbodyDatas)
					{
						if (data.rigidbody->boneID == -1)
						{
							auto trans = data.actor->getGlobalPose();
							auto coord = glm::vec3(trans.p.x, trans.p.y, trans.p.z);
							auto quat = glm::vec4(trans.q.x, trans.q.y, trans.q.z, trans.q.w);
							o->setCoord(coord);
							o->setQuat(quat);
							glm::mat3 axis;
							quaternionToMatrix(quat, axis);
							data.coord = coord;
							data.rotation = axis;
						}
						//else
						//{
						//	auto solver = pObject->animationSolver;
						//	if (r->mode == Rigidbody::Mode::eStatic)
						//	{
						//		auto pBone = &pModel->bones[r->boneID];
						//		auto coord = objAxis * (glm::vec3(solver->boneMatrix[r->boneID][3]) + glm::mat3(solver->boneMatrix[r->boneID]) * r->getCoord()) * objScale + objCoord;
						//		auto axis = objAxis * glm::mat3(solver->boneMatrix[r->boneID]) * r->getAxis();
						//		PxTransform trans(coord.x, coord.y, coord.z, PxQuat(PxMat33(
						//			PxVec3(axis[0][0], axis[0][1], axis[0][2]),
						//			PxVec3(axis[1][0], axis[1][1], axis[1][2]),
						//			PxVec3(axis[2][0], axis[2][1], axis[2][2]))));
						//		((PxRigidDynamic*)body)->setKinematicTarget(trans);
						//		pObject->rigidDatas[id].coord = coord;
						//		pObject->rigidDatas[id].rotation = axis;
						//	}
						//	else
						//	{
						//		auto objAxisT = glm::transpose(objAxis);
						//		auto rigidAxis = r->getAxis();
						//		auto rigidAxisT = glm::transpose(rigidAxis);
						//		auto trans = body->getGlobalPose();
						//		auto coord = glm::vec3(trans.p.x, trans.p.y, trans.p.z);
						//		glm::mat3 axis;
						//		Math::quaternionToMatrix(glm::vec4(trans.q.x, trans.q.y, trans.q.z, trans.q.w), axis);
						//		pObject->rigidDatas[id].coord = coord;
						//		pObject->rigidDatas[id].rotation = axis;
						//		auto boneAxis = objAxisT * axis * rigidAxisT;
						//		glm::vec3 boneCoord;
						//		if (r->mode != Rigidbody::Mode::eDynamicLockLocation)
						//			boneCoord = (objAxisT * (coord - objCoord) - boneAxis * (r->getCoord() * objScale)) / objScale;
						//		else
						//			boneCoord = glm::vec3(solver->boneMatrix[r->boneID][3]);
						//		solver->boneMatrix[r->boneID] = Math::makeMatrix(boneAxis, boneCoord);
						//	}
						//}
					}
				}

				if (o->physics_type & (int)ObjectPhysicsType::controller)
				{
					auto p = o->pxController->getPosition();
					auto c = glm::vec3(p.x, p.y, p.z) - o->model->controller_position * o->getScale();
					o->setCoord(c);
				}
			}
		}

		camera.move();
		if (camera.changed || camera.object)
			camera.lookAtTarget();
		if (camera.changed)
			camera.updateFrustum();
	}

	void Scene::loadSky(const char *skyMapFilename, int radianceMapCount, const char *radianceMapFilenames[], const char *irradianceMapFilename)
	{

		needUpdateSky = true;
		needUpdateAmbientBuffer = true;
	}

	void Scene::save(const std::string &filename)
	{
		tke::AttributeTree at("scene");
		at.addAttributes(this, b);
		for (auto &o : objects)
		{
			auto n = new AttributeTreeNode("object");
			o->getCoord();
			o->getEuler();
			o->getScale();
			n->addAttributes(o.get(), o->b);
			at.add(n);
		}
		if (terrain)
		{
			auto n = new AttributeTreeNode("terrain");
			terrain->getCoord();
			terrain->getEuler();
			terrain->getScale();
			n->addAttributes(terrain.get(), terrain->b);
			at.add(n);
		}
		at.saveXML(filename);
	}

	std::map<unsigned int, std::weak_ptr<Scene>> _scenes;
	std::shared_ptr<Scene> getScene(const std::string &filename)
	{
		auto hash = HASH(filename.c_str());
		auto it = _scenes.find(hash);
		if (it != _scenes.end())
		{
			auto s = it->second.lock();
			if (s) return s;
		}

		std::experimental::filesystem::path path(filename);
		if (!std::experimental::filesystem::exists(filename))
			return nullptr;

		auto s = std::make_shared<Scene>();
		s->filename = filename;

		tke::AttributeTree at("scene", filename);
		at.obtainFromAttributes(s.get(), s->b);
		for (auto &c : at.children)
		{
			if (c->name == "object")
			{
				auto o = new Object;
				c->obtainFromAttributes(o, o->b);
				o->model = getModel(o->model_filename);
				if (o->model && o->model->animated)
					o->animationComponent = std::make_unique<AnimationComponent>(o->model.get());
				o->needUpdateAxis = true;
				o->needUpdateQuat = true;
				o->needUpdateMat = true;
				o->changed = true;
				s->addObject(o);
			}
			else if (c->name == "light")
			{
				;
			}
			else if (c->name == "terrain")
			{
				auto t = new Terrain;
				c->obtainFromAttributes(t, t->b);
				//t->heightMap = getTexture(t->height_map_filename);
				//t->normalMap = getTexture(t->normal_map_filename);
				//t->blendMap = getTexture(t->blend_map_filename);
				//t->colorMaps[0] = getTexture(t->color_map0_filename);
				//t->colorMaps[1] = getTexture(t->color_map1_filename);
				//t->colorMaps[2] = getTexture(t->color_map2_filename);
				//t->colorMaps[3] = getTexture(t->color_map3_filename);
				//t->normalMaps[0] = getTexture(t->normal_map0_filename);
				//t->normalMaps[1] = getTexture(t->normal_map1_filename);
				//t->normalMaps[2] = getTexture(t->normal_map2_filename);
				//t->normalMaps[3] = getTexture(t->normal_map3_filename);
				t->needUpdateAxis = true;
				t->needUpdateQuat = true;
				t->needUpdateMat = true;
				t->changed = true;
				s->addTerrain(t);
			}
		}

		_scenes[hash] = s;
		return s;
	}
}
