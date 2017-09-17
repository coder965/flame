#include "scene.h"
#include "../math/math.h"
#include "../core.h"
#include "../render/renderpass.h"
#include "../render/synchronization.h"

namespace tke
{
	static const float gravity = 9.81f;

	static Image *envrImageDownsample[3] = {};

	static RenderPass *sceneRenderPass = nullptr;

	static int mrt_bone_position = -1;

	static int terr_heightMap_position = -1;
	static int terr_blendMap_position = -1;
	static int terr_colorMap_position = -1;
	static int terr_normalMap_position = -1;

	static int defe_envr_position = -1;
	static int defe_shad_position = -1;

	struct MatrixBufferShaderStruct
	{
		glm::mat4 proj;
		glm::mat4 projInv;
		glm::mat4 view;
		glm::mat4 viewInv;
		glm::mat4 projView;
		glm::mat4 projViewRotate;
		glm::vec4 frustumPlanes[6];
		glm::vec2 viewportDim;
	};

	static void _setSunLight_attribute(Scene *s)
	{
		s->sunLight->setEuler(glm::vec3(s->sunDir.x, 0.f, s->sunDir.y));
	}

	Scene::Scene()
		:resource(&globalResource)
	{
		physx::PxSceneDesc pxSceneDesc(pxPhysics->getTolerancesScale());
		pxSceneDesc.gravity = physx::PxVec3(0.0f, -gravity, 0.0f);
		pxSceneDesc.cpuDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
		pxSceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
		pxScene = pxPhysics->createScene(pxSceneDesc);
		pxControllerManager = PxCreateControllerManager(*pxScene);

		envrImage = std::make_unique<Image>(EnvrSizeCx, EnvrSizeCy, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 4);
		mainImage = std::make_unique<Image>(resCx, resCy, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		depthImage = std::make_unique<Image>(resCx, resCy, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		albedoAlphaImage = std::make_unique<Image>(resCx, resCy, VK_FORMAT_R16G16B16A16_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		normalHeightImage = std::make_unique<Image>(resCx, resCy, VK_FORMAT_R16G16B16A16_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		specRoughnessImage = std::make_unique<Image>(resCx, resCy, VK_FORMAT_R16G16B16A16_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		esmImage = std::make_unique<Image>(ShadowMapCx, ShadowMapCy, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1, MaxShadowCount * 6);
		debugImages.emplace_back("Esm Image", esmImage.get());
		esmDepthImage = std::make_unique<Image>(ShadowMapCx, ShadowMapCy, VK_FORMAT_D16_UNORM, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

		resource.setImage(envrImage.get(), "Envr.Image");
		resource.setImage(mainImage.get(), "Main.Image");
		resource.setImage(depthImage.get(), "Depth.Image");
		resource.setImage(albedoAlphaImage.get(), "AlbedoAlpha.Image");
		resource.setImage(normalHeightImage.get(), "NormalHeight.Image");
		resource.setImage(specRoughnessImage.get(), "SpecRoughness.Image");

		matrixBuffer = std::make_unique<UniformBuffer>(sizeof MatrixBufferShaderStruct);
		staticObjectMatrixBuffer = std::make_unique<UniformBuffer>(sizeof(glm::mat4) * MaxStaticObjectCount);
		animatedObjectMatrixBuffer = std::make_unique<UniformBuffer>(sizeof(glm::mat4) * MaxAnimatedObjectCount);
		terrainBuffer = std::make_unique<UniformBuffer>(sizeof TerrainShaderStruct);
		waterBuffer = std::make_unique<UniformBuffer>(sizeof(WaterShaderStruct) * MaxWaterCount);
		lightBuffer = std::make_unique<UniformBuffer>(sizeof(LightBufferShaderStruct));
		ambientBuffer = std::make_unique<UniformBuffer>(sizeof AmbientBufferShaderStruct);
		shadowBuffer = std::make_unique<UniformBuffer>(sizeof(glm::mat4) * MaxShadowCount);
		staticObjectIndirectBuffer = std::make_unique<IndirectIndexBuffer>(sizeof(VkDrawIndexedIndirectCommand) * MaxIndirectCount);
		animatedObjectIndirectBuffer = std::make_unique<IndirectIndexBuffer>(sizeof(VkDrawIndexedIndirectCommand) * MaxIndirectCount);

		resource.setBuffer(matrixBuffer.get(), "Matrix.UniformBuffer");
		resource.setBuffer(staticObjectMatrixBuffer.get(), "StaticObjectMatrix.UniformBuffer");
		resource.setBuffer(animatedObjectMatrixBuffer.get(), "AnimatedObjectMatrix.UniformBuffer");
		resource.setBuffer(terrainBuffer.get(), "Terrain.UniformBuffer");
		resource.setBuffer(waterBuffer.get(), "Water.UniformBuffer");
		resource.setBuffer(lightBuffer.get(), "Light.UniformBuffer");
		resource.setBuffer(ambientBuffer.get(), "Ambient.UniformBuffer");
		resource.setBuffer(shadowBuffer.get(), "Shadow.UniformBuffer");
		resource.setBuffer(staticObjectIndirectBuffer.get(), "Scene.Static.IndirectBuffer");
		resource.setBuffer(animatedObjectIndirectBuffer.get(), "Scene.Animated.IndirectBuffer");

		ds_mrt = std::make_unique<DescriptorSet>(mrtPipeline);
		mrtPipeline->linkDescriptors(ds_mrt.get(), &resource);
		ds_mrtAnim = std::make_unique<DescriptorSet>(mrtAnimPipeline);
		mrtAnimPipeline->linkDescriptors(ds_mrtAnim.get(), &resource);
		ds_mrtAnim_bone = std::make_unique<DescriptorSet>(mrtAnimPipeline, 2);
		ds_terrain = std::make_unique<DescriptorSet>(terrainPipeline);
		terrainPipeline->linkDescriptors(ds_terrain.get(), &resource);
		ds_water = std::make_unique<DescriptorSet>(waterPipeline);
		waterPipeline->linkDescriptors(ds_water.get(), &resource);
		ds_esm = std::make_unique<DescriptorSet>(esmPipeline);
		esmPipeline->linkDescriptors(ds_esm.get(), &resource);
		ds_esmAnim = std::make_unique<DescriptorSet>(esmAnimPipeline);
		esmAnimPipeline->linkDescriptors(ds_esmAnim.get(), &resource);
		ds_defe = std::make_unique<DescriptorSet>(deferredPipeline);
		deferredPipeline->linkDescriptors(ds_defe.get(), &resource);
		ds_comp = std::make_unique<DescriptorSet>(composePipeline);
		composePipeline->linkDescriptors(ds_comp.get(), &resource);

		cb_shadow = std::make_unique<CommandBuffer>();
		cb_mrt = std::make_unique<CommandBuffer>();
		cb_deferred = std::make_unique<CommandBuffer>();

		for (int i = 0; i < MaxShadowCount * 6; i++)
		{
			VkImageView views[] = {
				esmImage->getView(0, 1, i),
				esmDepthImage->getView()
			};
			fb_esm[i] = std::move(std::unique_ptr<Framebuffer>(getFramebuffer(ShadowMapCx, ShadowMapCy, renderPass_depth_clear_image32f_clear, TK_ARRAYSIZE(views), views)));
		}

		shadowRenderFinished = createEvent();
		mrtRenderFinished = createEvent();

		sunLight = new Light(LightType::parallax);
		sunLight->shadow = true;
		_setSunLight_attribute(this);
		addLight(sunLight);
	}

	Scene::~Scene()
	{
		pxControllerManager->release();
		pxScene->release();

		destroyEvent(shadowRenderFinished);
		destroyEvent(mrtRenderFinished);
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
		if (((int)o->physics_type & (int)ObjectPhysicsType::static_r) || ((int)o->physics_type & (int)ObjectPhysicsType::dynamic))
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
					auto actor = (((int)o->physics_type & (int)ObjectPhysicsType::dynamic) && (r->type == RigidbodyType::dynamic || r->type == RigidbodyType::dynamic_but_location)) ?
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

		if ((int)o->physics_type & (int)ObjectPhysicsType::controller)
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
		sunDir = v;
		needUpdateSky = true;
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

	Framebuffer *Scene::createFramebuffer(Image *dst)
	{
		VkImageView views[] = {
			mainImage->getView(),
			depthImage->getView(),
			albedoAlphaImage->getView(),
			normalHeightImage->getView(),
			specRoughnessImage->getView(),
			dst->getView(),
		};
		return getFramebuffer(resCx, resCy, sceneRenderPass, ARRAYSIZE(views), views);
	}

	void Scene::show(Framebuffer *fb, VkEvent signalEvent)
	{
		// update animation and bones
		for (auto &o : objects)
		{
			if (o->animationComponent)
				o->animationComponent->update();
		}

		// update physics (controller should move first, then simulate, and then get the result coord)
		auto dist = (timeDisp) / 1000.f;
		if (dist > 0.f)
		{
			for (auto &o : objects) // set controller coord
			{
				if ((int)o->physics_type & (int)ObjectPhysicsType::controller)
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
				if ((int)o->physics_type & (int)ObjectPhysicsType::dynamic)
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

				if ((int)o->physics_type & (int)ObjectPhysicsType::controller)
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
		{ // always update the matrix buffer
			MatrixBufferShaderStruct stru;
			stru.proj = matPerspective;
			stru.projInv = matPerspective;
			stru.view = camera.getMatInv();
			stru.viewInv = camera.getMat();
			stru.projView = stru.proj * stru.view;
			stru.projViewRotate = stru.proj * glm::mat4(glm::mat3(stru.view));
			memcpy(stru.frustumPlanes, camera.frustumPlanes, sizeof(glm::vec4) * 6);
			stru.viewportDim = glm::vec2(resCx, resCy);
			matrixBuffer->update(&stru, stagingBuffer);
		}
		if (needUpdateSky)
		{
			needUpdateAmbientBuffer = true;

			switch (skyType)
			{
			case SkyType::atmosphere_scattering:
			{ // update Atmospheric Scattering
				_setSunLight_attribute(this);

				{
					auto cb = begineOnceCommandBuffer();
					auto fb = getFramebuffer(envrImage.get(), renderPass_image16);

					cb->beginRenderPass(renderPass_image16, fb);
					cb->bindPipeline(scatteringPipeline);
					auto dir = sunLight->getAxis()[2];
					cb->pushConstant(tke::StageType::frag, 0, sizeof(dir), &dir);
					cb->draw(3);
					cb->endRenderPass();

					endOnceCommandBuffer(cb);
					releaseFramebuffer(fb);
				}

				{ // update IBL
					if (defe_envr_position != -1)
					{
						static int down_source_position = -1;
						if (down_source_position == -1) down_source_position = downsamplePipeline->descriptorPosition("source");
						if (down_source_position != -1)
						{
							for (int i = 0; i < 3; i++)
							{
								auto cb = begineOnceCommandBuffer();
								auto fb = getFramebuffer(envrImageDownsample[i], renderPass_image16);

								cb->beginRenderPass(renderPass_image16, fb);
								cb->bindPipeline(downsamplePipeline);
								cb->setViewportAndScissor(EnvrSizeCx >> (i + 1), EnvrSizeCy >> (i + 1));
								auto size = glm::vec2(EnvrSizeCx >> (i + 1), EnvrSizeCy >> (i + 1));
								cb->pushConstant(StageType::frag, 0, sizeof glm::vec2, &size);
								downsamplePipeline->descriptorSet->setImage(down_source_position, 0, i == 0 ? envrImage.get() : envrImageDownsample[i - 1], plainSampler);
								cb->bindDescriptorSet();
								cb->draw(3);
								cb->endRenderPass();

								endOnceCommandBuffer(cb);
								releaseFramebuffer(fb);
							}
						}

						static int con_source_position = -1;
						if (con_source_position == -1) con_source_position = convolvePipeline->descriptorPosition("source");
						if (con_source_position != -1)
						{
							for (int i = 1; i < envrImage->levels.size(); i++)
							{
								auto cb = begineOnceCommandBuffer();
								auto fb = getFramebuffer(envrImage.get(), renderPass_image16, i);

								cb->beginRenderPass(renderPass_image16, fb);
								cb->bindPipeline(convolvePipeline);
								auto data = 1.f + 1024.f - 1024.f * (i / 3.f);
								cb->pushConstant(StageType::frag, 0, sizeof(float), &data);
								cb->setViewportAndScissor(EnvrSizeCx >> i, EnvrSizeCy >> i);
								convolvePipeline->descriptorSet->setImage(con_source_position, 0, envrImageDownsample[i - 1], plainSampler);
								cb->bindDescriptorSet();
								cb->draw(3);
								cb->endRenderPass();

								endOnceCommandBuffer(cb);
								releaseFramebuffer(fb);
							}
						}
					}
				}
			}
			break;
			case SkyType::panorama:
				// TODO : FIX SKY FROM FILE
				//if (skyImage)
				//{
				//	//writes.push_back(vk->writeDescriptorSet(engine->panoramaPipeline.m_descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, skyImage->getInfo(engine->colorSampler), 0));
				//	//writes.push_back(vk->writeDescriptorSet(engine->deferredPipeline.m_descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 7, radianceImage->getInfo(engine->colorSampler), 0));

				//	AmbientBufferShaderStruct stru;
				//	stru.v = glm::vec4(1.f, 1.f, 1.f, skyImage->level - 1);
				//	stru.fogcolor = glm::vec4(0.f, 0.f, 1.f, 1.f); // TODO : FIX FOG COLOR ACCORDING TO SKY
				//	ambientBuffer->update(&stru, *stagingBuffer);
				//}
				break;
			}

			needUpdateSky = false;
		}
		if (needUpdateAmbientBuffer)
		{
			AmbientBufferShaderStruct stru;
			stru.color = ambientColor;
			stru.envr_max_mipmap = envrImage->levels.size() - 1;
			stru.fogcolor = glm::vec4(fogColor, 1.f); // TODO : FIX FOG COLOR ACCORDING TO SKY
			ambientBuffer->update(&stru, stagingBuffer);

			needUpdateAmbientBuffer = false;
		}
		if (objects.size() > 0)
		{
			int updateCount = 0;
			std::vector<VkBufferCopy> staticUpdateRanges;
			std::vector<VkBufferCopy> animatedUpdateRanges;
			auto map = (unsigned char*)stagingBuffer->map(0, sizeof(glm::mat4) * objects.size());
			int staticObjectIndex = 0;
			int animatedObjectIndex = 0;

			for (auto &o : objects)
			{
				if (!o->model->animated)
				{
					if (o->changed)
					{
						auto srcOffset = sizeof(glm::mat4) * updateCount;
						memcpy(map + srcOffset, &o->getMat(), sizeof(glm::mat4));
						VkBufferCopy range = {};
						range.srcOffset = srcOffset;
						range.dstOffset = sizeof(glm::mat4) * staticObjectIndex;
						range.size = sizeof(glm::mat4);
						staticUpdateRanges.push_back(range);

						updateCount++;
					}
					o->sceneIndex = staticObjectIndex;
					staticObjectIndex++;
				}
				else
				{
					if (o->changed)
					{
						auto srcOffset = sizeof(glm::mat4) * updateCount;
						memcpy(map + srcOffset, &o->getMat(), sizeof(glm::mat4));
						VkBufferCopy range = {};
						range.srcOffset = srcOffset;
						range.dstOffset = sizeof(glm::mat4) * animatedObjectIndex;
						range.size = sizeof(glm::mat4);
						animatedUpdateRanges.push_back(range);

						updateCount++;
					}
					o->sceneIndex = animatedObjectIndex;
					animatedObjectIndex++;
				}

			}
			stagingBuffer->unmap();
			if (staticUpdateRanges.size() > 0) copyBuffer(stagingBuffer->v, staticObjectMatrixBuffer->v, staticUpdateRanges.size(), staticUpdateRanges.data());
			if (animatedUpdateRanges.size() > 0) copyBuffer(stagingBuffer->v, animatedObjectMatrixBuffer->v, animatedUpdateRanges.size(), animatedUpdateRanges.data());
		}
		if (terrain)
		{
			if (terrain->changed)
			{
				TerrainShaderStruct stru;
				stru.coord = terrain->getCoord();
				stru.blockCx = terrain->block_cx;
				stru.blockSize = terrain->block_size;
				stru.height = terrain->height;
				stru.tessellationFactor = terrain->tessellation_factor;
				stru.textureUvFactor = terrain->texture_uv_factor;
				stru.mapDimension = terrain->heightMap->levels[0].cx;

				terrainBuffer->update(&stru, stagingBuffer);

				if (terr_heightMap_position != -1 && terrain->heightMap)
					ds_terrain->setImage(terr_heightMap_position, 0, terrain->heightMap, colorBorderSampler);
				if (terr_blendMap_position != -1 && terrain->blendMap)
					ds_terrain->setImage(terr_blendMap_position, 0, terrain->blendMap, colorBorderSampler);
				if (terr_colorMap_position != -1)
				{
					for (int i = 0; i < 4; i++)
					{
						if (terrain->colorMaps[i])
							ds_terrain->setImage(terr_colorMap_position, i, terrain->colorMaps[i], colorWrapSampler);
					}
				}
				if (terr_normalMap_position != -1)
				{
					for (int i = 0; i < 4; i++)
					{
						if (terrain->normalMaps[i])
							ds_terrain->setImage(terr_normalMap_position, i, terrain->normalMaps[i], colorWrapSampler);
					}
				}
			}
		}
		if (waters.size() > 0)
		{
			int updateCount = 0;
			std::vector<VkBufferCopy> ranges;
			auto map = (unsigned char*)stagingBuffer->map(0, sizeof(WaterShaderStruct) * waters.size());

			for (auto &w : waters)
			{
				if (w->changed)
				{
					auto offset = sizeof(WaterShaderStruct) * updateCount;
					WaterShaderStruct stru;
					stru.coord = w->getCoord();
					stru.blockCx = w->blockCx;
					stru.blockSize = w->blockSize;
					stru.height = w->height;
					stru.tessellationFactor = w->tessellationFactor;
					stru.textureUvFactor = w->textureUvFactor;
					stru.mapDimension = 1024;
					memcpy(map + offset, &stru, sizeof(WaterShaderStruct));
					VkBufferCopy range = {};
					range.srcOffset = offset;
					range.dstOffset = offset;
					range.size = sizeof(WaterShaderStruct);
					ranges.push_back(range);

					updateCount++;
				}
			}
			stagingBuffer->unmap();
			if (ranges.size() > 0) copyBuffer(stagingBuffer->v, waterBuffer->v, ranges.size(), ranges.data());
		}
		static std::vector<Object*> staticObjects;
		static std::vector<Object*> animatedObjects;
		if (needUpdateIndirectBuffer)
		{
			staticObjects.clear();
			animatedObjects.clear();

			if (objects.size() > 0)
			{
				std::vector<VkDrawIndexedIndirectCommand> staticCommands;
				std::vector<VkDrawIndexedIndirectCommand> animatedCommands;

				int staticIndex = 0;
				int animatedIndex = 0;

				for (auto &o : objects)
				{
					auto m = o->model;

					if (!m->animated)
					{
						for (auto &g : m->geometries)
						{
							VkDrawIndexedIndirectCommand command = {};
							command.instanceCount = 1;
							command.indexCount = g->indiceCount;
							command.vertexOffset = m->vertexBase;
							command.firstIndex = m->indiceBase + g->indiceBase;
							command.firstInstance = (staticIndex << 8) + g->material->sceneIndex;

							staticCommands.push_back(command);
						}

						staticObjects.push_back(o.get());
						staticIndex++;
					}
					else
					{
						for (auto &g : m->geometries)
						{
							VkDrawIndexedIndirectCommand command = {};
							command.instanceCount = 1;
							command.indexCount = g->indiceCount;
							command.vertexOffset = m->vertexBase;
							command.firstIndex = m->indiceBase + g->indiceBase;
							command.firstInstance = (animatedIndex << 8) + g->material->sceneIndex;

							animatedCommands.push_back(command);
						}

						if (mrt_bone_position != -1)
							ds_mrtAnim_bone->setBuffer(mrt_bone_position, animatedIndex, o->animationComponent->boneMatrixBuffer);

						animatedObjects.push_back(o.get());
						animatedIndex++;
					}
				}

				staticIndirectCount = staticCommands.size();
				animatedIndirectCount = animatedCommands.size();

				if (staticCommands.size() > 0) staticObjectIndirectBuffer->update(staticCommands.data(), stagingBuffer, sizeof(VkDrawIndexedIndirectCommand) * staticCommands.size());
				if (animatedCommands.size() > 0) animatedObjectIndirectBuffer->update(animatedCommands.data(), stagingBuffer, sizeof(VkDrawIndexedIndirectCommand) * animatedCommands.size());
			}
			needUpdateIndirectBuffer = false;
		}
		if (needUpdateLightCount)
		{ // light count in light attribute
			auto count = lights.size();
			lightBuffer->update(&count, stagingBuffer, 4);
			needUpdateLightCount = false;
		}
		std::vector<Light*> shadowLights;
		if (lights.size() > 0)
		{ // shadow
			auto shadowIndex = 0;
			std::vector<VkBufferCopy> ranges;
			auto map = (unsigned char*)stagingBuffer->map(0, sizeof(glm::mat4) * lights.size());

			for (auto &l : lights)
			{
				if (!l->shadow)
				{
					l->sceneShadowIndex = -1;
					continue;
				}

				l->sceneShadowIndex = shadowIndex;
				shadowLights.push_back(l.get());

				if (l->type == LightType::parallax)
				{
					if (l->changed || camera.changed)
					{
						glm::vec3 p[8];
						auto cameraCoord = camera.coord;
						for (int i = 0; i < 8; i++) p[i] = camera.frustumPoints[i] - cameraCoord;
						auto lighAxis = l->getAxis();
						auto axisT = glm::transpose(lighAxis);
						auto vMax = axisT * p[0], vMin = vMax;
						for (int i = 1; i < 8; i++)
						{
							auto tp = axisT * p[i];
							vMax = glm::max(tp, vMax);
							vMin = glm::min(tp, vMin);
						}
						auto halfWidth = (vMax.z - vMin.z) * 0.5f;
						auto halfHeight = (vMax.y - vMin.y) * 0.5f;
						auto halfDepth = glm::max(vMax.x - vMin.x, near_plane) * 0.5f;
						auto center = lighAxis * ((vMax + vMin) * 0.5f) + cameraCoord;
						//auto shadowMatrix = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, TKE_NEAR, halfDepth + halfDepth) * 
						glm::lookAt(center + halfDepth * lighAxis[2], center, lighAxis[1]);
						auto shadowMatrix = glm::mat4(1.f, 0.f, 0.f, 0.f,
							0.f, 1.f, 0.f, 0.f,
							0.f, 0.f, 0.5f, 0.f,
							0.f, 0.f, 0.5f, 1.f) *
							glm::ortho(-1.f, 1.f, -1.f, 1.f, near_plane, far_plane) * glm::lookAt(camera.target + glm::vec3(0, 0, 100), camera.target, glm::vec3(0, 1, 0));

						auto srcOffset = sizeof(glm::mat4) * ranges.size();
						memcpy(map + srcOffset, &shadowMatrix, sizeof(glm::mat4));
						VkBufferCopy range = {};
						range.srcOffset = srcOffset;
						range.dstOffset = sizeof(glm::mat4) * shadowIndex;
						range.size = sizeof(glm::mat4);
						ranges.push_back(range);

						if (defe_shad_position != -1)
							ds_defe->setImage(defe_shad_position, shadowIndex, esmImage.get(), colorSampler, 0, 1, shadowIndex, 1);
					}
					shadowIndex += 6;
				}
				else if (l->type == LightType::point)
				{
					if (l->changed)
					{
						glm::mat4 shadowMatrix[6];

						auto coord = l->getCoord();
						auto proj = glm::perspective(90.f, 1.f, near_plane, far_plane);
						shadowMatrix[0] = proj * glm::lookAt(coord, coord + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0));
						shadowMatrix[1] = proj * glm::lookAt(coord, coord + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0));
						shadowMatrix[2] = proj * glm::lookAt(coord, coord + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
						shadowMatrix[3] = proj * glm::lookAt(coord, coord + glm::vec3(0, -1, 0), glm::vec3(0, 0, 1));
						shadowMatrix[4] = proj * glm::lookAt(coord, coord + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0));
						shadowMatrix[5] = proj * glm::lookAt(coord, coord + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0));
					}
					shadowIndex += 6;
				}
			}
			stagingBuffer->unmap();
			if (ranges.size() > 0) copyBuffer(stagingBuffer->v, shadowBuffer->v, ranges.size(), ranges.data());
		}
		if (lights.size() > 0)
		{ // light attribute
			int lightIndex = 0;
			std::vector<VkBufferCopy> ranges;
			auto map = (unsigned char*)stagingBuffer->map(0, sizeof(LightShaderStruct) * lights.size());
			for (auto &l : lights)
			{
				if (l->changed)
				{
					LightShaderStruct stru;
					if (l->type == LightType::parallax)
						stru.coord = glm::vec4(l->getAxis()[2], 0.f);
					else
						stru.coord = glm::vec4(l->getCoord(), l->type);
					stru.color = glm::vec4(l->color, l->sceneShadowIndex);
					stru.spotData = glm::vec4(-l->getAxis()[2], l->range);
					auto srcOffset = sizeof(LightShaderStruct) * ranges.size();
					memcpy(map + srcOffset, &stru, sizeof(LightShaderStruct));
					VkBufferCopy range = {};
					range.srcOffset = srcOffset;
					range.dstOffset = 16 + sizeof(LightShaderStruct) * lightIndex;
					range.size = sizeof(LightShaderStruct);
					ranges.push_back(range);
				}
				lightIndex++;
			}
			stagingBuffer->unmap();
			if (ranges.size() > 0) copyBuffer(stagingBuffer->v, lightBuffer->v, ranges.size(), ranges.data());
		}

		camera.changed = false;
		for (auto &l : lights)
			l->changed = false;
		for (auto &o : objects)
			o->changed = false;
		if (terrain)
			terrain->changed = false;

		// shadow
		cb_shadow->reset();
		cb_shadow->begin();

		for (int i = 0; i < shadowLights.size(); i++)
		{
			auto l = shadowLights[i];

			VkClearValue clearValues[] = {
				{ 1.f, 0 },
				{ 1.f, 1.f, 1.f, 1.f }
			};
			cb_shadow->beginRenderPass(renderPass_depth_clear_image32f_clear, fb_esm[i].get(), clearValues);
			// static
			if (staticObjects.size() > 0)
			{
				cb_shadow->bindVertexBuffer(staticVertexBuffer);
				cb_shadow->bindIndexBuffer(staticIndexBuffer);
				cb_shadow->bindPipeline(esmPipeline);
				VkDescriptorSet sets[] = {
					ds_esm->v,
					ds_maps->v
				};
				cb_shadow->bindDescriptorSet(sets, 0, TK_ARRAYSIZE(sets));
				for (int oId = 0; oId < staticObjects.size(); oId++)
				{
					auto o = staticObjects[oId];
					auto m = o->model;
					for (int gId = 0; gId < m->geometries.size(); gId++)
						cb_shadow->drawModel(m, gId, 1, (i << 28) + (oId << 8) + gId);
				}
			}
			// animated
			if (animatedObjects.size() > 0)
			{
				cb_shadow->bindVertexBuffer(animatedVertexBuffer);
				cb_shadow->bindIndexBuffer(animatedIndexBuffer);
				cb_shadow->bindPipeline(esmAnimPipeline);
				VkDescriptorSet sets[] = {
					ds_esmAnim->v,
					ds_maps->v,
					ds_mrtAnim_bone->v
				};
				cb_shadow->bindDescriptorSet(sets, 0, TK_ARRAYSIZE(sets));
				for (int oId = 0; oId < animatedObjects.size(); oId++)
				{
					auto o = animatedObjects[oId];
					auto m = o->model;
					for (int gId = 0; gId < m->geometries.size(); gId++)
						cb_shadow->drawModel(m, gId, 1, (i << 28) + (oId << 8) + gId);
				}
			}
			cb_shadow->endRenderPass();
		}

		cb_shadow->end();

		cb_deferred->reset();
		cb_deferred->begin();

		cb_deferred->beginRenderPass(sceneRenderPass, fb);

		// mrt
		// static
		if (staticIndirectCount > 0)
		{
			cb_deferred->bindVertexBuffer(staticVertexBuffer);
			cb_deferred->bindIndexBuffer(staticIndexBuffer);
			cb_deferred->bindPipeline(mrtPipeline);
			VkDescriptorSet sets[] = {
				ds_mrt->v,
				ds_maps->v
			};
			cb_deferred->bindDescriptorSet(sets, 0, TK_ARRAYSIZE(sets));
			cb_deferred->drawIndirectIndex(staticObjectIndirectBuffer.get(), staticIndirectCount);
		}
		// animated
		if (animatedIndirectCount)
		{
			cb_deferred->bindVertexBuffer(animatedVertexBuffer);
			cb_deferred->bindIndexBuffer(animatedIndexBuffer);
			cb_deferred->bindPipeline(mrtAnimPipeline);
			VkDescriptorSet sets[] = {
				ds_mrtAnim->v,
				ds_maps->v,
				ds_mrtAnim_bone->v
			};
			cb_deferred->bindDescriptorSet(sets, 0, TK_ARRAYSIZE(sets));
			cb_deferred->drawIndirectIndex(animatedObjectIndirectBuffer.get(), animatedIndirectCount);
		}
		// terrain
		if (terrain)
		{
			cb_deferred->bindPipeline(terrainPipeline);
			cb_deferred->bindDescriptorSet(&ds_terrain->v);
			cb_deferred->draw(4, 0, terrain->block_cx * terrain->block_cx);
		}
		// water
		if (waters.size() > 0)
		{
			int index = 0;
			for (auto &w : waters)
			{
				cb_deferred->bindPipeline(waterPipeline);
				cb_deferred->bindDescriptorSet(&ds_water->v);
				cb_deferred->draw(4, 0, w->blockCx * w->blockCx);
			}
		}

		//cb->imageBarrier(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 
		//	VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, 
		//	VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
		//	esmImage.get(), 0, 1, 0, TKE_MAX_SHADOW_COUNT * 8);

		// deferred
		cb_deferred->nextSubpass();
		cb_deferred->bindPipeline(deferredPipeline);
		cb_deferred->bindDescriptorSet(&ds_defe->v);
		cb_deferred->draw(3);

		// compose
		cb_deferred->nextSubpass();
		cb_deferred->bindPipeline(composePipeline);
		cb_deferred->bindDescriptorSet(&ds_comp->v);
		cb_deferred->draw(3);

		cb_deferred->endRenderPass();

		cb_deferred->setEvent(signalEvent);
		cb_deferred->end();
	}

	void Scene::loadSky(const char *skyMapFilename, int radianceMapCount, const char *radianceMapFilenames[], const char *irradianceMapFilename)
	{

		needUpdateSky = true;
	}

	void Scene::load(const std::string &_filename)
	{
		filename = std::experimental::filesystem::path(_filename).string();

		tke::AttributeTree at("scene", filename);
		at.obtainFromAttributes(this, b);
		for (auto &c : at.children)
		{
			if (c->name == "object")
			{
				auto o = new Object;
				c->obtainFromAttributes(o, o->b);
				o->model = getModel(o->model_filename);
				if (o->model && o->model->animated)
					o->animationComponent = std::make_unique<AnimationComponent>(o->model);
				o->needUpdateAxis = true;
				o->needUpdateQuat = true;
				o->needUpdateMat = true;
				o->changed = true;
				addObject(o);
			}
			else if (c->name == "light")
			{
				;
			}
			else if (c->name == "terrain")
			{
				auto t = new Terrain;
				c->obtainFromAttributes(t, t->b);
				t->heightMap = getTexture(t->height_map_filename);
				t->blendMap = getTexture(t->blend_map_filename);
				t->colorMaps[0] = getTexture(t->color_map0_filename);
				t->colorMaps[1] = getTexture(t->color_map1_filename);
				t->colorMaps[2] = getTexture(t->color_map2_filename);
				t->colorMaps[3] = getTexture(t->color_map3_filename);
				t->normalMaps[0] = getTexture(t->normal_map0_filename);
				t->normalMaps[1] = getTexture(t->normal_map1_filename);
				t->normalMaps[2] = getTexture(t->normal_map2_filename);
				t->normalMaps[3] = getTexture(t->normal_map3_filename);
				t->needUpdateAxis = true;
				t->needUpdateQuat = true;
				t->needUpdateMat = true;
				t->changed = true;
				addTerrain(t);
			}
		}
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

	void initScene()
	{
		for (int i = 0; i < 3; i++)
			envrImageDownsample[i] = new Image(EnvrSizeCx >> (i + 1), EnvrSizeCy >> (i + 1), VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

		VkAttachmentDescription atts[] = {
			colorAttachmentDesc(VK_FORMAT_R16G16B16A16_SFLOAT, VK_ATTACHMENT_LOAD_OP_DONT_CARE), // main
			depthAttachmentDesc(VK_FORMAT_D32_SFLOAT, VK_ATTACHMENT_LOAD_OP_CLEAR),				 // depth
			colorAttachmentDesc(VK_FORMAT_R16G16B16A16_UNORM, VK_ATTACHMENT_LOAD_OP_CLEAR),		 // albedo alpha
			colorAttachmentDesc(VK_FORMAT_R16G16B16A16_UNORM, VK_ATTACHMENT_LOAD_OP_CLEAR),		 // normal height
			colorAttachmentDesc(VK_FORMAT_R16G16B16A16_UNORM, VK_ATTACHMENT_LOAD_OP_CLEAR),		 // spec roughness
			colorAttachmentDesc(VK_FORMAT_R8G8B8A8_UNORM, VK_ATTACHMENT_LOAD_OP_DONT_CARE)		 // dst
		};
		VkAttachmentReference main_col_ref = { 0, VK_IMAGE_LAYOUT_GENERAL };
		VkAttachmentReference mrt_col_ref[] = {
			{ 2, VK_IMAGE_LAYOUT_GENERAL },
			{ 3, VK_IMAGE_LAYOUT_GENERAL },
			{ 4, VK_IMAGE_LAYOUT_GENERAL }
		};
		VkAttachmentReference dep_ref = { 1, VK_IMAGE_LAYOUT_GENERAL };
		VkAttachmentReference dst_col_ref = { 5, VK_IMAGE_LAYOUT_GENERAL };
		VkSubpassDescription subpasses[] = {
			subpassDesc(ARRAYSIZE(mrt_col_ref), mrt_col_ref, &dep_ref), // mrt
			subpassDesc(1, &main_col_ref),                              // deferred
			subpassDesc(1, &dst_col_ref)                                // compose
		};

		VkSubpassDependency dependencies[] = {
			subpassDependency(0, 1),
			subpassDependency(1, 2)
		};

		sceneRenderPass = new RenderPass(ARRAYSIZE(atts), atts, ARRAYSIZE(subpasses), subpasses, ARRAYSIZE(dependencies), dependencies);

		scatteringPipeline = new Pipeline(enginePath + "pipeline/sky/scattering.xml", renderPass_image16, 0);
		downsamplePipeline = new Pipeline(enginePath + "pipeline/sky/downsample.xml", renderPass_image16, 0, true);
		convolvePipeline = new Pipeline(enginePath + "pipeline/sky/convolve.xml", renderPass_image16, 0);
		mrtPipeline = new Pipeline(enginePath + "pipeline/deferred/mrt.xml", sceneRenderPass, 0);
		mrtAnimPipeline = new Pipeline(enginePath + "pipeline/deferred/mrt_anim.xml", sceneRenderPass, 0);
		mrt_bone_position = mrtAnimPipeline->descriptorPosition("BONE");
		terrainPipeline = new Pipeline(enginePath + "pipeline/deferred/terrain.xml", sceneRenderPass, 0);
		terr_heightMap_position = terrainPipeline->descriptorPosition("heightMap");
		terr_blendMap_position = terrainPipeline->descriptorPosition("blendMap");
		terr_colorMap_position = terrainPipeline->descriptorPosition("colorMaps");
		terr_normalMap_position = terrainPipeline->descriptorPosition("normalMaps");
		waterPipeline = new Pipeline(enginePath + "pipeline/deferred/water.xml", sceneRenderPass, 0);
		esmPipeline = new Pipeline(enginePath + "pipeline/esm/esm.xml", renderPass_depth_clear_image8_clear, 0);
		esmAnimPipeline = new Pipeline(enginePath + "pipeline/esm/esm_anim.xml", renderPass_depth_clear_image8_clear, 0);
		deferredPipeline = new Pipeline(enginePath + "pipeline/deferred/deferred.xml", sceneRenderPass, 1);
		defe_envr_position = deferredPipeline->descriptorPosition("envrSampler");
		defe_shad_position = deferredPipeline->descriptorPosition("shadowSampler");
		composePipeline = new Pipeline(enginePath + "pipeline/compose/compose.xml", sceneRenderPass, 2);
	}
}
