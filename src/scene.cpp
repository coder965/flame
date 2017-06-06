#include "core.h"
#include "scene.h"
#include "gui.h"

namespace tke
{
	//MasterRenderer::MasterRenderer(int _cx, int _cy, Window *pWindow, VertexBuffer *vertexBuffer, IndexBuffer *indexBuffer, IndexedIndirectBuffer *indirectBuffer)
	//{
	//	static ResourceBank _resources;

	//	originalImage.create(resCx, resCy, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
	//	albedoSpecImage.create(resCx, resCy, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
	//	normalRoughnessImage.create(resCx, resCy, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
	//	miscImage.create(resCx, resCy, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

	//	renderer = new Renderer();
	//	renderer->cx = _cx;
	//	renderer->cy = _cy;
	//	renderer->pResource = &_resources;

	//	_resources.setImage(&originalImage, "Original.Texture");
	//	_resources.setImage(&albedoSpecImage, "AlbedoSpec.Texture");
	//	_resources.setImage(&normalRoughnessImage, "NormalRoughness.Texture");
	//	_resources.setImage(&miscImage, "Misc.Texture");

	//	_resources.setPipeline(&panoramaPipeline, "Panorama.Pipeline");
	//	_resources.setPipeline(&heightMapTerrainPipeline, "HeightMapTerrain.Pipeline");
	//	_resources.setPipeline(&proceduralTerrainPipeline, "ProceduralTerrain.Pipeline");
	//	_resources.setPipeline(&mrtPipeline, "Mrt.Pipeline");
	//	_resources.setPipeline(&deferredPipeline, "Deferred.Pipeline");
	//	_resources.setPipeline(&combinePipeline, "Combine.Pipeline");

	//	auto skyPass = renderer->addPass();
	//	skyPass->addAttachment(&originalImage);
	//	skyAction = skyPass->addAction(&panoramaPipeline);

	//	auto mrtPass = renderer->addPass();
	//	mrtPass->addAttachment(&albedoSpecImage, VkClearValue{ 0.f, 0.f, 0.f, 0.f });
	//	mrtPass->addAttachment(&normalRoughnessImage, VkClearValue{ 0.f, 0.f, 0.f, 0.f });
	//	mrtPass->addAttachment(_resources.getImage("Depth.Image"), VkClearValue{ 1.f, 0.f });
	//	auto mrtObjectAction = mrtPass->addAction(&mrtPipeline);
	//	mrtObjectDrawcall = mrtObjectAction->addDrawcall(indirectBuffer);
	//	mrtHeightMapTerrainAction = mrtPass->addAction(&heightMapTerrainPipeline);
	//	// TODO : FIX PROCEDURAL TERRAIN
	//	//auto mrtProceduralTerrainAction = mrtPass->addAction(&proceduralTerrainPipeline);
	//	//mrtProceduralTerrainAction->addDrawcall(4, 0, 100 * 100, 0);

	//	auto deferredPass = renderer->addPass();
	//	deferredPass->addAttachment(&originalImage);
	//	deferredPass->addDependency(skyPass);
	//	deferredPass->addDependency(mrtPass);
	//	auto deferredAction = deferredPass->addAction(&deferredPipeline);
	//	deferredAction->addDrawcall(3, 0, 1, 0);

	//	miscPass = renderer->addPass();
	//	miscPass->addAttachment(&miscImage, VkClearValue{ 0.f, 0.f, 0.f, 0.f });
	//	miscPass->addAttachment(_resources.getImage("Depth.Image"), VkClearValue{ 1.f, 0.f });
	//	miscPass->addDependency(mrtPass);

	//	auto combinePass = renderer->addPass();
	//	combinePass->addAttachment(pWindow->image);
	//	combinePass->addDependency(deferredPass);
	//	combinePass->addDependency(miscPass);
	//	auto combineAction = combinePass->addAction(&combinePipeline);
	//	combineAction->addDrawcall(3, 0, 1, 0);

	//	renderer->initVertexBuffer = vertexBuffer;
	//	renderer->initIndexBuffer = indexBuffer;

	//	renderer->setup();

	//	panoramaPipeline.create(enginePath + "pipeline/sky/panorama.xml", &vertexInputState, renderer->vkRenderPass, skyPass->index);
	//	heightMapTerrainPipeline.create(enginePath + "pipeline/terrain/height_map/terrain.xml", &zeroVertexInputState, renderer->vkRenderPass, mrtPass->index);
	//	proceduralTerrainPipeline.create(enginePath + "pipeline/terrain/procedural/terrain.xml", &zeroVertexInputState, renderer->vkRenderPass, mrtPass->index);
	//	mrtPipeline.create(enginePath + "pipeline/deferred/mrt.xml", &vertexInputState, renderer->vkRenderPass, mrtPass->index);
	//	deferredPipeline.create(enginePath + "pipeline/deferred/deferred.xml", &zeroVertexInputState, renderer->vkRenderPass, deferredPass->index);
	//	combinePipeline.create(enginePath + "pipeline/combine/combine.xml", &zeroVertexInputState, renderer->vkRenderPass, combinePass->index);

	//	renderer->getDescriptorSets();
	//}

	void Atmosphere::set()
	{
		float fScaleDepth = 0.25;// The scale depth (i.e. the altitude at which the atmosphere's average density is found)

		auto mrX = glm::mat3(glm::rotate(sunDir.x, glm::vec3(0.f, 1.f, 0.f)));
		auto v3LightPosition = glm::normalize(glm::mat3(glm::rotate(sunDir.y, mrX * glm::vec3(0.f, 0.f, 1.f))) * mrX * glm::vec3(1.f, 0.f, 0.f));// The direction vector to the light source

		if (scene->pSunLight)
			scene->pSunLight->setEuler(glm::vec3(sunDir, 0.f));

		float fScale = 1.f / (outerRadius - innerRadius);	// 1 / (fOuterRadius - fInnerRadius)
		float fScaleOverScaleDepth = fScale / fScaleDepth;	// fScale / fScaleDepth
	}

	Scene::Scene()
	{
		InitializeCriticalSection(&cs);

		constantBuffer = new UniformBuffer(sizeof ConstantBufferStruct);
		matrixBuffer = new UniformBuffer(sizeof MatrixUniformBufferStruct);
		objectMatrixBuffer = new UniformBuffer(sizeof(glm::mat4) * TKE_MAX_OBJECT_COUNT);
		lightMatrixBuffer = new UniformBuffer(sizeof(glm::mat4) * TKE_MAX_LIGHT_COUNT);
		materialBuffer = new UniformBuffer(sizeof(MaterialUniformBufferStruct) * TKE_MAX_MATERIAL_COUNT);
		heightMapTerrainBuffer = new UniformBuffer(sizeof(HeightMapTerrainBufferStruct) * 8);
		proceduralTerrainBuffer = new UniformBuffer(sizeof(glm::vec2));
		lightBuffer = new UniformBuffer(sizeof(LightBufferStruct));
		ambientBuffer = new UniformBuffer(sizeof AmbientStruct);

		vertexBuffer = new VertexBuffer();
		indexBuffer = new IndexBuffer();
		objectIndirectBuffer = new IndirectIndexBuffer(sizeof(VkDrawIndexedIndirectCommand) * TKE_MAX_INDIRECT_COUNT);

		globalResource.setBuffer(constantBuffer, "Constant.UniformBuffer");
		globalResource.setBuffer(matrixBuffer, "Matrix.UniformBuffer");
		globalResource.setBuffer(objectMatrixBuffer, "ObjectMatrix.UniformBuffer");
		globalResource.setBuffer(lightMatrixBuffer, "LightMatrix.UniformBuffer");
		globalResource.setBuffer(materialBuffer, "Material.UniformBuffer");
		globalResource.setBuffer(heightMapTerrainBuffer, "HeightMapTerrain.UniformBuffer");
		globalResource.setBuffer(proceduralTerrainBuffer, "ProceduralTerrain.UniformBuffer");
		globalResource.setBuffer(lightBuffer, "Light.UniformBuffer");
		globalResource.setBuffer(ambientBuffer, "Ambient.UniformBuffer");

		globalResource.setBuffer(vertexBuffer, "Scene.VertexBuffer");
		globalResource.setBuffer(indexBuffer, "Scene.IndexBuffer");
		globalResource.setBuffer(objectIndirectBuffer, "Scene.IndirectBuffer");
	}

	Scene::~Scene()
	{
		delete constantBuffer;
		delete matrixBuffer;
		delete objectMatrixBuffer;
		delete lightMatrixBuffer;
		delete materialBuffer;
		delete heightMapTerrainBuffer;
		delete proceduralTerrainBuffer;
		delete lightBuffer;
		delete ambientBuffer;

		delete vertexBuffer;
		delete indexBuffer;
		delete objectIndirectBuffer;
	}

	void Scene::setUp()
	{
		ConstantBufferStruct stru;
		stru.depth_near = TKE_NEAR;
		stru.depth_far = TKE_FAR;
		stru.cx = resCx;
		stru.cy = resCy;
		stru.aspect = aspect;
		stru.fovy = TKE_FOVY;
		stru.tanHfFovy = std::tan(glm::radians(TKE_FOVY * 0.5f));
		stru.envrCx = TKE_ENVR_SIZE_CX;
		stru.envrCy = TKE_ENVR_SIZE_CY;
		constantBuffer->update(&stru, *stagingBuffer);

		atmosphere.set();
	}

	void Scene::loadSky(const char *skyMapFilename, int radianceMapCount, const char *radianceMapFilenames[], const char *irradianceMapFilename)
	{
		//WIN32_FIND_DATA fd;
		//HANDLE hFind;

		//hFind = FindFirstFile(sprintf("%s\\pano.*", dir), &fd);
		//if (hFind != INVALID_HANDLE_VALUE)
		//{
		//	auto pImage = createImage(sprintf("%s\\%s", dir, fd.cFileName), true);
		//	if (pImage)
		//	{
		//		delete skyImage;
		//		skyImage = pImage;
		//	}
		//	FindClose(hFind);
		//}

		//std::vector<std::string> mipmapNames;
		//for (int i = 0; i < 100; i++)
		//{
		//	hFind = FindFirstFile(sprintf("%s\\rad%d.*", dir, i), &fd);
		//	if (hFind == INVALID_HANDLE_VALUE)
		//		break;
		//	FindClose(hFind);
		//	mipmapNames.push_back(sprintf("%s\\%s", dir, fd.cFileName));
		//}
		//auto pImage = createImage(mipmapNames, true);
		//if (pImage)
		//{
		//	strcpy(pImage->m_fileName, dir);
		//	delete radianceImage;
		//	radianceImage = pImage;
		//}

		//hFind = FindFirstFile(sprintf("%s\\irr.*", dir), &fd);
		//if (hFind != INVALID_HANDLE_VALUE)
		//{
		//	auto pImage = createImage(sprintf("%s\\%s", dir, fd.cFileName), true);
		//	if (pImage)
		//	{
		//		delete irradianceImage;
		//		irradianceImage = pImage;
		//	}
		//	FindClose(hFind);
		//}

		//strcpy(skyName, dir);

		needUpdataSky = true;
	}

	void Scene::load(char *file)
	{
	}

	void Scene::save(char *file)
	{
	}

	void Scene::addModel(Model *pModel)
	{
		for (auto pSrcImage : pModel->pImages)
		{
			bool found = false;
			for (auto pStoreImage : storeImages)
			{
				if (pStoreImage == pSrcImage)
				{
					found = true;
					break;
				}
			}
			if (!found)
			{
				pSrcImage->sceneIndex = storeImages.size();
				storeImages.push_back(pSrcImage);
			}
		}
		for (auto &rg : pModel->renderGroups)
		{
			auto pSrcMaterial = &rg.material;

			MaterialUniformBufferStruct material;
			material.albedoSpecCompress = pSrcMaterial->albedoR + (pSrcMaterial->albedoG << 8) + (pSrcMaterial->albedoB << 16) + (pSrcMaterial->spec << 24);
			material.roughnessAlphaCompress = pSrcMaterial->roughness + (pSrcMaterial->alpha << 8);

			auto albedoAlphaMapIndex = getStoreImageIndex(pSrcMaterial->albedoAlphaMap) + 1;
			auto normalHeightMapIndex = getStoreImageIndex(pSrcMaterial->normalHeightMap) + 1;
			auto specRoughnessMapIndex = getStoreImageIndex(pSrcMaterial->specRoughnessMap) + 1;

			int sameIndex = -1;
			int materialIndex = 0;
			for (auto &storeMaterial : storeMaterials)
			{
				auto storeAlbedoAlphaMapIndex = storeMaterial.mapIndex & 0xff;
				auto storeNormalHeightMapIndex = (storeMaterial.mapIndex >> 10) & 0xff;
				auto storeSpecRoughnessMapIndex = (storeMaterial.mapIndex >> 20) & 0xff;

				bool theSameAlbedoAlpha = false;
				bool theSameNormalHeight = false;
				bool theSameSpecRoughness = false;

				if (albedoAlphaMapIndex != 0 && albedoAlphaMapIndex == storeAlbedoAlphaMapIndex)
					theSameAlbedoAlpha = true;
				else if (albedoAlphaMapIndex == 0 && storeAlbedoAlphaMapIndex == 0 &&
					material.albedoSpecCompress == storeMaterial.albedoSpecCompress)
					theSameAlbedoAlpha = true;
				if (normalHeightMapIndex == storeNormalHeightMapIndex)
					theSameNormalHeight = true;
				if (specRoughnessMapIndex != 0 && specRoughnessMapIndex == storeSpecRoughnessMapIndex)
					theSameSpecRoughness = true;
				else if (specRoughnessMapIndex == 0 && storeSpecRoughnessMapIndex == 0 &&
					material.roughnessAlphaCompress == storeMaterial.roughnessAlphaCompress)
					theSameSpecRoughness = true;

				if (theSameAlbedoAlpha && theSameNormalHeight && theSameSpecRoughness)
				{
					sameIndex = materialIndex;
					break;
				}

				materialIndex++;
			}
			if (sameIndex == -1)
			{
				pSrcMaterial->sceneIndex = storeMaterials.size();
				material.mapIndex = albedoAlphaMapIndex + (normalHeightMapIndex << 10) + (specRoughnessMapIndex << 20);
				storeMaterials.push_back(material);
			}
			else
			{
				pSrcMaterial->sceneIndex = sameIndex;
			}
		}
		pModels.push_back(pModel);
		needUpdateVertexBuffer = true;
		needUpdateMaterialBuffer = true;
		needUpdateSampler = true;
	}

	Model *Scene::getModel(char *name)
	{
		for (auto m : pModels)
		{
			if (m->name.compare(name) == 0)
				return m;
		}
		return nullptr;
	}

	void Scene::clearModel()
	{
		for (auto m : pModels)
			delete m;
		pModels.clear();
	}

	Rigidbody *Scene::getRigidbody(int id)
	{
		for (auto pModel : pModels)
		{
			for (auto pRigidbody : pModel->rigidbodies)
			{
				if (pRigidbody->id == id)
					return pRigidbody;
			}
		}
		return nullptr;
	}

	Shape *Scene::getShape(int id)
	{
		for (auto pModel : pModels)
		{
			for (auto pRigidbody : pModel->rigidbodies)
			{
				for (auto pShape : pRigidbody->shapes)
				{
					if (pShape->id == id)
						return pShape;
				}
			}
		}
		return nullptr;
	}

	Joint *Scene::getJoint(int id)
	{
		for (auto pModel : pModels)
		{
			for (auto pJoint : pModel->joints)
			{
				if (pJoint->id == id)
					return pJoint;
			}
		}
		return nullptr;
	}

	Light *Scene::getLight(int id)
	{
		for (auto pLight : pLights)
		{
			if (pLight->m_id == id)
				return pLight;
		}
		return nullptr;
	}

	Object *Scene::getObject(int id)
	{
		for (auto pObject : pObjects)
		{
			if (pObject->m_id == id)
				return pObject;
		}
		return nullptr;
	}

	Terrain *Scene::getTerrain(int id)
	{
		for (auto pTerrain : pTerrains)
		{
			if (pTerrain->m_id == id)
				return pTerrain;
		}
		return nullptr;
	}


	void Scene::addLight(Light *pLight, int id)
	{
		EnterCriticalSection(&cs);
		pLight->m_id = id;
		pLight->getRefrence();
		pLights.push_back(pLight);
		if (pLight->type == Light::Type::eParallax)
			pParallaxLights.push_back(pLight);
		else
			pPointLights.push_back(pLight);
		tke::needRedraw = true;
		lightCountChanged = true;
		LeaveCriticalSection(&cs);
	}

	void Scene::addLight(Light *pLight)
	{
		static auto magicNumber = 0;
		addLight(pLight, magicNumber++);
	}

	Light *Scene::deleteLight(Light *pLight)
	{
		EnterCriticalSection(&cs);
		if (pLight->type == Light::Type::eParallax)
		{
			for (auto it = pParallaxLights.begin(); it != pParallaxLights.end(); it++)
			{
				if (*it == pLight)
				{
					pParallaxLights.erase(it);
					break;
				}
			}
		}
		else
		{
			for (auto it = pPointLights.begin(); it != pPointLights.end(); it++)
			{
				if (*it == pLight)
				{
					pPointLights.erase(it);
					break;
				}
			}
		}
		for (auto it = pLights.begin(); it != pLights.end(); it++)
		{
			if (*it == pLight)
			{
				pLight->dying = true;
				pLight->release();
				for (auto itt = it + 1; itt != pLights.end(); itt++)
				{
					(*itt)->sceneIndex--;
					if ((*itt)->shadow && pLight->shadow)
					{
						if (pLight->type == Light::Type::eParallax)
							(*itt)->sceneShadowIndex--;
						else if (pLight->type == Light::Type::ePoint)
							(*itt)->sceneShadowIndex -= 6;
					}
					(*itt)->m_changed = true;
				}
				if (it > pLights.begin())
					pLight = *(it - 1);
				else
					pLight = nullptr;
				pLights.erase(it);
				break;
			}
		}
		tke::needRedraw = true;
		lightCountChanged = true;
		LeaveCriticalSection(&cs);
		return pLight;
	}

	void Scene::addObject(Object *pObject, int id)
	{
		EnterCriticalSection(&cs);
		pObject->getRefrence();
		pObject->m_id = id;
		pObjects.push_back(pObject);
		auto pModel = pObject->pModel;
		if (pModel->animated)
		{
			pObject->animationSolver = new AnimationSolver(pObject->pModel);
			pAnimatedObjects.push_back(pObject);
		}
		else
		{
			pStaticObjects.push_back(pObject);
		}
		if (pModel->rigidbodies.size() > 0)
			pObject->rigidDatas = new RigidData[pModel->rigidbodies.size()];

		tke::needRedraw = true;
		needUpdateIndirectBuffer = true;
		LeaveCriticalSection(&cs);
	}

	void Scene::addObject(Object *pObject)
	{
		static auto magicNumber = 0;
		addObject(pObject, magicNumber++);
	}

	Object *Scene::deleteObject(Object *pObject)
	{
		if (controllingObject == pObject)
			controllingObject = nullptr;

		EnterCriticalSection(&cs);
		if (pObject->pModel->animated)
		{
			for (auto it = pAnimatedObjects.begin(); it != pAnimatedObjects.end(); it++)
			{
				if (*it == pObject)
				{
					pAnimatedObjects.erase(it);
					break;
				}
			}
		}
		else
		{
			for (auto it = pStaticObjects.begin(); it != pStaticObjects.end(); it++)
			{
				if (*it == pObject)
				{
					pStaticObjects.erase(it);
					break;
				}
			}
		}
		for (auto it = pObjects.begin(); it != pObjects.end(); it++)
		{
			if (*it == pObject)
			{
				pObject->dying = true;
				pObject->release();
				for (auto itt = it + 1; itt != pObjects.end(); itt++)
				{
					(*itt)->sceneIndex--;
					(*itt)->m_changed = true;
				}

				if (it > pObjects.begin())
					pObject = *(it - 1);
				else
					pObject = nullptr;
				pObjects.erase(it);
				break;
			}
		}
		tke::needRedraw = true;
		needUpdateIndirectBuffer = true;
		LeaveCriticalSection(&cs);
		return pObject;
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

	void Scene::addTerrain(Terrain *pTerrain, int id)
	{
		EnterCriticalSection(&cs);

		pTerrain->m_id = id;
		pTerrain->getRefrence();
		pTerrains.push_back(pTerrain);

		tke::needRedraw = true;

		LeaveCriticalSection(&cs);
	}

	void Scene::addTerrain(Terrain *pTerrain)
	{
		static auto magicNumber = 0;
		addTerrain(pTerrain, magicNumber++);
	}

	Terrain *Scene::deleteTerrain(Terrain *pTerrain)
	{
		EnterCriticalSection(&cs);

		for (auto it = pTerrains.begin(); it != pTerrains.end(); it++)
		{
			if (*it == pTerrain)
			{
				pTerrain->dying = true;
				pTerrain->release();
				if (it > pTerrains.begin())
					pTerrain = *(it - 1);
				else
					pTerrain = nullptr;
				pTerrains.erase(it);
				break;
			}
		}

		LeaveCriticalSection(&cs);

		return pTerrain;
	}

	void Scene::clearActors()
	{
		EnterCriticalSection(&cs);

		pSunLight = nullptr;

		for (auto pLight : pLights)
			delete pLight;
		pLights.clear();
		pParallaxLights.clear();
		pPointLights.clear();

		for (auto pObject : pObjects)
			delete pObject;
		pObjects.clear();
		pAnimatedObjects.clear();
		pStaticObjects.clear();

		for (auto pTerrain : pTerrains)
			delete pTerrain;
		pTerrains.clear();

		LeaveCriticalSection(&cs);
	}

	static Pipeline *panoramaPipeline = nullptr;
	static Pipeline *deferredIblPipeline = nullptr;
	static Pipeline *mrtPipeline = nullptr;

	void Scene::setResources(Renderer *r)
	{
		panoramaPipeline = r->resource.getPipeline("Panorama.Pipeline");
		deferredIblPipeline = r->resource.getPipeline("Deferred_PBR_IBL.Pipeline");
		mrtPipeline = r->resource.getPipeline("Mrt.Pipeline");
	}

	void Scene::update()
	{
		camera.move();
		if (camera.m_changed)
		{
			camera.lookAtTarget();
			camera.updateFrustum(aspect, *pMatProj);
			// update procedural terrain
			{
				glm::vec2 seed;

				auto pos = camera.getCoord();
				seed.x = pos.x - 500.f;
				seed.y = pos.z - 500.f;
				seed /= 1000.f;
				proceduralTerrainBuffer->update(&seed, *stagingBuffer);
			}
		}
		if (needUpdateProjMatrix || camera.m_changed)
		{
			MatrixUniformBufferStruct stru;
			stru.proj = *pMatProj;
			stru.projInv = *pMatProjInv;
			stru.view = camera.getMatInv();
			stru.viewInv = camera.getMat();
			stru.projView = stru.proj * stru.view;
			stru.projViewRotate = stru.proj * glm::mat4(glm::mat3(stru.view));
			memcpy(stru.frustumPlanes, camera.m_frustumPlanes, sizeof(glm::vec4) * 6);
			stru.viewportDim = glm::vec2(resCx, resCy);
			matrixBuffer->update(&stru, *stagingBuffer);
		}
		if (needUpdataSky)
		{
			AmbientStruct stru;
			stru.v = glm::vec4(ambientColor, 0);
			stru.fogcolor = glm::vec4(0.f, 0.f, 0.f, 1.f); // TODO : FIX FOG COLOR ACCORDING TO SKY
			ambientBuffer->update(&stru, *stagingBuffer);

			if (skyType == SkyType::eNull)
			{
			}
			else if (skyType == SkyType::ePanorama)
			{
				// TODO : FIX SKY FROM FILE
				if (skyImage)
				{
					//writes.push_back(vk->writeDescriptorSet(engine->panoramaPipeline.m_descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, skyImage->getInfo(engine->colorSampler), 0));
					//writes.push_back(vk->writeDescriptorSet(engine->deferredPipeline.m_descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 7, radianceImage->getInfo(engine->colorSampler), 0));

					AmbientStruct stru;
					stru.v = glm::vec4(1.f, 1.f, 1.f, skyImage->m_mipmapLevels - 1);
					stru.fogcolor = glm::vec4(0.f, 0.f, 1.f, 1.f); // TODO : FIX FOG COLOR ACCORDING TO SKY
					ambientBuffer->update(&stru, *stagingBuffer);
				}
			}
			else if (skyType == SkyType::eAtmosphereScattering)
			{
				static VkRenderPass postRenderPass;

				static Image *envrImage = nullptr;
				static Framebuffer *envrFramebuffer[4];

				static Image *envrImageDownsample[3] = {};
				static Framebuffer *envrDownsampleFramebuffer[3];

				static VkDescriptorSet downsampleDescriptorSetLevel[3];
				static VkDescriptorSet convolveDescriptorSetLevel[3];

				static Pipeline scatteringPipeline;
				static Pipeline downsamplePipeline;
				static Pipeline convolvePipeline;

				static bool first = true;
				if (first)
				{
					first = false;

					{ // render pass
						VkAttachmentReference ref ={ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
						postRenderPass = createRenderPass(1, &colorAttachmentDesc(VK_FORMAT_R16G16B16A16_SFLOAT, VK_ATTACHMENT_LOAD_OP_DONT_CARE), 1, &subpassDesc(1, &ref), 0, nullptr);
					}

					scatteringPipeline.create(enginePath + "pipeline/sky/scattering.xml", &zeroVertexInputState, postRenderPass, 0);
					globalResource.setPipeline(&scatteringPipeline, "Scattering.Pipeline");

					downsamplePipeline.m_dynamics.push_back(VK_DYNAMIC_STATE_VIEWPORT);
					downsamplePipeline.m_dynamics.push_back(VK_DYNAMIC_STATE_SCISSOR);
					downsamplePipeline.create(enginePath + "pipeline/sky/downsample.xml", &zeroVertexInputState, postRenderPass, 0);
					globalResource.setPipeline(&downsamplePipeline, "Downsample.Pipeline");

					convolvePipeline.m_dynamics.push_back(VK_DYNAMIC_STATE_VIEWPORT);
					convolvePipeline.m_dynamics.push_back(VK_DYNAMIC_STATE_SCISSOR);
					convolvePipeline.create(enginePath + "pipeline/sky/convolve.xml", &zeroVertexInputState, postRenderPass, 0);
					globalResource.setPipeline(&convolvePipeline, "Convolve.Pipeline");

					envrImage = new Image(TKE_ENVR_SIZE_CX, TKE_ENVR_SIZE_CY, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 4);

					for (int i = 0; i < 4; i++)
					{
						std::vector<VkImageView> views = { envrImage->getView(0, i) };
						envrFramebuffer[i] = createFramebuffer(TKE_ENVR_SIZE_CX >> i, TKE_ENVR_SIZE_CY >> i, postRenderPass, views);
					}

					for (int i = 0; i < 3; i++)
					{
						envrImageDownsample[i]= new Image(TKE_ENVR_SIZE_CX >> (i + 1), TKE_ENVR_SIZE_CY >> (i + 1), VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

						std::vector<VkImageView> views = { envrImageDownsample[i]->getView() };
						envrDownsampleFramebuffer[i] = createFramebuffer(TKE_ENVR_SIZE_CX >> (i + 1), TKE_ENVR_SIZE_CY >> (i + 1), postRenderPass, views);
					}

					convolveDescriptorSetLevel[0] = convolvePipeline.m_descriptorSet;
					convolveDescriptorSetLevel[1] = descriptorPool.allocate(&convolvePipeline.m_descriptorSetLayout->v);
					convolveDescriptorSetLevel[2] = descriptorPool.allocate(&convolvePipeline.m_descriptorSetLayout->v);

					downsampleDescriptorSetLevel[0] = downsamplePipeline.m_descriptorSet;
					downsampleDescriptorSetLevel[1] = descriptorPool.allocate(&downsamplePipeline.m_descriptorSetLayout->v);
					downsampleDescriptorSetLevel[2] = descriptorPool.allocate(&downsamplePipeline.m_descriptorSetLayout->v);

					static int source_position = -1;
					if (source_position == -1) source_position = downsamplePipeline.descriptorPosition("source");

					descriptorPool.addWrite(downsampleDescriptorSetLevel[0], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, source_position, envrImage->getInfo(plainSampler));
					descriptorPool.addWrite(downsampleDescriptorSetLevel[1], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, source_position, envrImageDownsample[0]->getInfo(plainSampler));
					descriptorPool.addWrite(downsampleDescriptorSetLevel[2], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, source_position, envrImageDownsample[1]->getInfo(plainSampler));

					descriptorPool.addWrite(convolveDescriptorSetLevel[0], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, source_position, envrImageDownsample[0]->getInfo(plainSampler));
					descriptorPool.addWrite(convolveDescriptorSetLevel[1], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, source_position, envrImageDownsample[1]->getInfo(plainSampler));
					descriptorPool.addWrite(convolveDescriptorSetLevel[2], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, source_position, envrImageDownsample[2]->getInfo(plainSampler));

					descriptorPool.update();
				}

				if (panoramaPipeline)
				{ // update Atmospheric Scattering

					static int pano_tex_position = -1;
					if (pano_tex_position == -1) pano_tex_position = panoramaPipeline->descriptorPosition("tex");

					if (pano_tex_position != -1)
					{
						auto cmd = commandPool.begineOnce();

						vkCmdBeginRenderPass(cmd, &renderPassBeginInfo(postRenderPass, envrFramebuffer[0]->v,
							TKE_ENVR_SIZE_CX, TKE_ENVR_SIZE_CY, 0, nullptr), VK_SUBPASS_CONTENTS_INLINE);

						vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, scatteringPipeline.m_pipeline);
						vkCmdDraw(cmd, 3, 1, 0, 0);

						vkCmdEndRenderPass(cmd);

						commandPool.endOnce(cmd);

						descriptorPool.addWrite(panoramaPipeline->m_descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, pano_tex_position, envrImage->getInfo(colorSampler));

						if (deferredIblPipeline)
						{ // update IBL

							static int defe_envr_position = -1;
							if (defe_envr_position == -1) defe_envr_position = deferredIblPipeline->descriptorPosition("envrSampler");

							if (defe_envr_position != -1)
							{
								for (int i = 0; i < 3; i++)
								{
									auto cmd = commandPool.begineOnce();

									vkCmdBeginRenderPass(cmd, &renderPassBeginInfo(postRenderPass, envrDownsampleFramebuffer[i]->v,
										TKE_ENVR_SIZE_CX >> (i + 1), TKE_ENVR_SIZE_CY >> (i + 1), 0, nullptr), VK_SUBPASS_CONTENTS_INLINE);

									vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, downsamplePipeline.m_pipeline);

									VkViewport viewport;
									viewport.x = 0.f;
									viewport.y = 0.f;
									viewport.width = TKE_ENVR_SIZE_CX >> (i + 1);
									viewport.height = TKE_ENVR_SIZE_CY >> (i + 1);
									viewport.minDepth = 0.0f;
									viewport.maxDepth = 1.0f;
									vkCmdSetViewport(cmd, 0, 1, &viewport);

									VkRect2D scissor;
									scissor.offset.x = 0;
									scissor.offset.y = 0;
									scissor.extent.width = TKE_ENVR_SIZE_CX >> (i + 1);
									scissor.extent.height = TKE_ENVR_SIZE_CY >> (i + 1);
									vkCmdSetScissor(cmd, 0, 1, &scissor);

									auto size = glm::vec2(TKE_ENVR_SIZE_CX >> (i + 1), TKE_ENVR_SIZE_CY >> (i + 1));
									vkCmdPushConstants(cmd, downsamplePipeline.m_pipelineLayout->v, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof glm::vec2, &size);

									vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, downsamplePipeline.m_pipelineLayout->v, 0, 1, &downsampleDescriptorSetLevel[i], 0, nullptr);
									vkCmdDraw(cmd, 3, 1, 0, 0);

									vkCmdEndRenderPass(cmd);

									commandPool.endOnce(cmd);
								}

								for (int i = 1; i < envrImage->m_mipmapLevels; i++)
								{
									auto cmd = commandPool.begineOnce();

									vkCmdBeginRenderPass(cmd, &renderPassBeginInfo(postRenderPass, envrFramebuffer[i]->v,
										TKE_ENVR_SIZE_CX >> i, TKE_ENVR_SIZE_CY >> i, 0, nullptr), VK_SUBPASS_CONTENTS_INLINE);

									vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, convolvePipeline.m_pipeline);

									auto data = 1.f + 1024.f - 1024.f * (i / 3.f);
									vkCmdPushConstants(cmd, convolvePipeline.m_pipelineLayout->v, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(float), &data);

									VkViewport viewport;
									viewport.x = 0.f;
									viewport.y = 0.f;
									viewport.width = TKE_ENVR_SIZE_CX >> i;
									viewport.height = TKE_ENVR_SIZE_CY >> i;
									viewport.minDepth = 0.0f;
									viewport.maxDepth = 1.0f;
									vkCmdSetViewport(cmd, 0, 1, &viewport);

									VkRect2D scissor;
									scissor.offset.x = 0;
									scissor.offset.y = 0;
									scissor.extent.width = TKE_ENVR_SIZE_CX >> i;
									scissor.extent.height = TKE_ENVR_SIZE_CY >> i;
									vkCmdSetScissor(cmd, 0, 1, &scissor);

									vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, convolvePipeline.m_pipelineLayout->v, 0, 1, &convolveDescriptorSetLevel[i - 1], 0, nullptr);
									vkCmdDraw(cmd, 3, 1, 0, 0);

									vkCmdEndRenderPass(cmd);

									commandPool.endOnce(cmd);
								}

								descriptorPool.addWrite(deferredIblPipeline->m_descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, defe_envr_position, envrImage->getInfo(colorSampler, 0, 0, envrImage->m_mipmapLevels));

								AmbientStruct stru;
								stru.v = glm::vec4(1.f, 1.f, 1.f, 3);
								stru.fogcolor = glm::vec4(0.f, 0.f, 1.f, 1.f); // TODO : FIX FOG COLOR ACCORDING TO SKY
								ambientBuffer->update(&stru, *stagingBuffer);
							}
						}
					}

					descriptorPool.update();
				}
			}

			needUpdataSky = false;
		}
		if (pTerrains.size() > 0)
		{
			std::vector<VkBufferCopy> ranges;

			auto map = (unsigned char*)stagingBuffer->map(0, sizeof(HeightMapTerrainBufferStruct) * pTerrains.size());

			auto terrainIndex = 0;
			for (auto pTerrain : pTerrains)
			{
				if (pTerrain->m_changed)
				{
					HeightMapTerrainBufferStruct stru;
					stru.patchSize = pTerrain->patchSize;
					stru.ext = pTerrain->ext;
					stru.height = pTerrain->height;
					stru.tessFactor = pTerrain->tessFactor;
					stru.mapDim = pTerrain->heightMap->m_width;

					auto srcOffset = sizeof(HeightMapTerrainBufferStruct) * ranges.size();
					memcpy(map + srcOffset, &stru, sizeof(HeightMapTerrainBufferStruct));
					VkBufferCopy range = {};
					range.srcOffset = srcOffset;
					range.dstOffset = sizeof(HeightMapTerrainBufferStruct) * terrainIndex;
					range.size = sizeof(HeightMapTerrainBufferStruct);
					ranges.push_back(range);

					static int position = -1;
					// TODO : FIX TERRAIN
					//if (position == -1) position = masterRenderer->heightMapTerrainPipeline.descriptorPosition("samplerHeight");
					//vk::descriptorPool.addWrite(masterRenderer->heightMapTerrainPipeline.m_descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, position, pTerrain->heightMap->getInfo(vk::colorBorderSampler), terrainIndex);
				}

				terrainIndex++;
			}

			stagingBuffer->unmap();
			if (ranges.size() > 0) commandPool.cmdCopyBuffer(stagingBuffer->m_buffer, heightMapTerrainBuffer->m_buffer, ranges.size(), ranges.data());

			descriptorPool.update();
		}
		if (needUpdateVertexBuffer)
		{
			std::vector<Vertex> vertexs;
			std::vector<int> indices;

			for (auto pModel : pModels)
			{
				pModel->vertexBase = vertexs.size();
				pModel->indiceBase = indices.size();

				for (int i = 0; i < pModel->positions.size(); i++)
				{
					Vertex vertex;
					if (i < pModel->positions.size()) vertex.position = pModel->positions[i];
					else vertex.position = glm::vec3(0.f);
					if (i < pModel->uvs.size()) vertex.uv = pModel->uvs[i];
					else vertex.uv = glm::vec2(0.f);
					if (i < pModel->normals.size()) vertex.normal = pModel->normals[i];
					else vertex.normal = glm::vec3(0.f);
					if (i < pModel->tangents.size()) vertex.tangent = pModel->tangents[i];
					else vertex.tangent = glm::vec3(0.f);

					vertexs.push_back(vertex);
				}
				for (int i = 0; i < pModel->indices.size(); i++)
				{
					indices.push_back(pModel->indices[i]);
				}
			}

			vertexBuffer->recreate(sizeof(Vertex) * vertexs.size(), vertexs.data());
			indexBuffer->recreate(sizeof(int) * indices.size(), indices.data());

			tke::needRedraw = true;
			needUpdateVertexBuffer = false;
		}
		if (needUpdateMaterialBuffer)
		{
			if (storeMaterials.size() > 0)
				materialBuffer->update(storeMaterials.data(), *stagingBuffer, sizeof(MaterialUniformBufferStruct) * storeMaterials.size());
			needUpdateMaterialBuffer = false;
		}
		if (needUpdateSampler)
		{
			static int map_position = -1;
			if (map_position == -1) map_position = mrtPipeline->descriptorPosition("mapSamplers");
			for (int index = 0; index < storeImages.size(); index++)
				descriptorPool.addWrite(mrtPipeline->m_descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, map_position, storeImages[index]->getInfo(colorSampler), index);

			descriptorPool.update();
			needUpdateSampler = false;
		}
		if (pStaticObjects.size() > 0)
		{
			int objectIndex = 0;
			std::vector<VkBufferCopy> ranges;
			auto map = (unsigned char*)stagingBuffer->map(0, sizeof(glm::mat4) * pStaticObjects.size());
			for (auto pObject : pStaticObjects)
			{
				if (pObject->m_changed)
				{
					auto srcOffset = sizeof(glm::mat4) * ranges.size();
					memcpy(map + srcOffset, &pObject->getMat(), sizeof(glm::mat4));
					VkBufferCopy range = {};
					range.srcOffset = srcOffset;
					range.dstOffset = sizeof(glm::mat4) * objectIndex;
					range.size = sizeof(glm::mat4);
					ranges.push_back(range);
				}
				pObject->sceneIndex = objectIndex;
				objectIndex++;
			}
			stagingBuffer->unmap();
			if (ranges.size() > 0) commandPool.cmdCopyBuffer(stagingBuffer->m_buffer, objectMatrixBuffer->m_buffer, ranges.size(), ranges.data());
		}
		if (pLights.size() > 0)
		{ // light in editor
			int lightIndex = 0;
			std::vector<VkBufferCopy> ranges;
			auto map = (unsigned char*)stagingBuffer->map(0, sizeof(glm::mat4) * pLights.size());
			for (auto pLight : pLights)
			{
				if (pLight->m_changed)
				{
					auto srcOffset = sizeof(glm::mat4) * ranges.size();
					memcpy(map + srcOffset, &pLight->getMat(), sizeof(glm::mat4));
					VkBufferCopy range = {};
					range.srcOffset = srcOffset;
					range.dstOffset = sizeof(glm::mat4) * lightIndex;
					range.size = sizeof(glm::mat4);
					ranges.push_back(range);
				}
				pLight->sceneIndex = lightIndex;
				lightIndex++;
			}
			stagingBuffer->unmap();
			if (ranges.size() > 0) commandPool.cmdCopyBuffer(stagingBuffer->m_buffer, lightMatrixBuffer->m_buffer, ranges.size(), ranges.data());
		}
		if (needUpdateIndirectBuffer)
		{
			if (pStaticObjects.size() > 0)
			{
				std::vector<VkDrawIndexedIndirectCommand> commands;

				int objID = 0;
				for (auto pObject : pStaticObjects)
				{
					auto pModel = pObject->pModel;

					for (auto &rg : pModel->renderGroups)
					{
						VkDrawIndexedIndirectCommand command = {};
						command.instanceCount = 1;
						command.indexCount = rg.indiceCount;
						command.vertexOffset = pModel->vertexBase;
						command.firstIndex = pModel->indiceBase + rg.indiceBase;
						command.firstInstance = (objID << 16) + rg.material.sceneIndex;

						commands.push_back(command);
					}

					objID++;
				}

				drawCallCount = commands.size();

				objectIndirectBuffer->update(commands.data(), *stagingBuffer, sizeof(VkDrawIndexedIndirectCommand) * drawCallCount);
			}
			needUpdateIndirectBuffer = false;
		}
		if (pLights.size() > 0)
		{ // light attribute


			auto coord = camera.getMatInv() * glm::vec4(pLights[0]->getCoord(), 1.f);
			auto c1    = camera.getMatInv() * glm::vec4(glm::vec3(0), 1.f);

			auto c2 = c1 - coord;


			int lightIndex = 0;
			std::vector<VkBufferCopy> ranges;
			auto map = (unsigned char*)stagingBuffer->map(0, sizeof(LightStruct) * pLights.size());
			for (auto pLight : pLights)
			{
				if (pLight->m_changed)
				{
					auto srcOffset = sizeof(LightStruct) * ranges.size();
					LightStruct stru;
					if (pLight->type == Light::Type::eParallax)
						stru.coord = glm::vec4(-pLight->getAxis()[0], 0.f);
					else if (pLight->type == Light::Type::ePoint)
						stru.coord = glm::vec4(pLight->getCoord(), 1.f);
					stru.color = glm::vec4(pLight->color, 1.f);
					stru.decayFactor = glm::vec4(pLight->decayFactor, 0.f);
					memcpy(map + srcOffset, &stru, sizeof(LightStruct));
					VkBufferCopy range = {};
					range.srcOffset = srcOffset;
					range.dstOffset = 16 + sizeof(LightStruct) * lightIndex;
					range.size = sizeof(LightStruct);
					ranges.push_back(range);
				}
				lightIndex++;
			}
			stagingBuffer->unmap();
			if (ranges.size() > 0) commandPool.cmdCopyBuffer(stagingBuffer->m_buffer, lightBuffer->m_buffer, ranges.size(), ranges.data());
		}
		if (pLights.size() > 0)
		{ // shadow
			shadowCount = 0;

			for (auto pLight : pLights)
			{
				if (pLight->shadow)
				{
					pLight->sceneShadowIndex = shadowCount;
					if (pLight->type == Light::Type::eParallax)
					{
						if (pLight->m_changed || camera.m_changed)
						{
							glm::vec3 p[8];
							auto cameraCoord = camera.m_coord;
							for (int i = 0; i < 8; i++) p[i] = camera.m_frustumPoints[i] - cameraCoord;
							auto lighAxis = pLight->getAxis();
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
							auto halfDepth = glm::max(vMax.x - vMin.x, TKE_NEAR) * 0.5f;

							auto center = lighAxis * ((vMax + vMin) * 0.5f) + cameraCoord;

							auto shadowMatrix = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, TKE_NEAR, halfDepth + halfDepth) * glm::lookAt(center - halfDepth * lighAxis[0], center, lighAxis[1]);
						}
						shadowCount++;
					}
					else if (pLight->type == Light::Type::ePoint)
					{
						if (pLight->m_changed)
						{
							glm::mat4 shadowMatrix[6];

							auto coord = pLight->getCoord();
							auto proj = glm::perspective(90.f, 1.f, TKE_NEAR, TKE_FAR);
							shadowMatrix[0] = proj * glm::lookAt(coord, coord + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0));
							shadowMatrix[1] = proj * glm::lookAt(coord, coord + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0));
							shadowMatrix[2] = proj * glm::lookAt(coord, coord + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
							shadowMatrix[3] = proj * glm::lookAt(coord, coord + glm::vec3(0, -1, 0), glm::vec3(0, 0, 1));
							shadowMatrix[4] = proj * glm::lookAt(coord, coord + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0));
							shadowMatrix[5] = proj * glm::lookAt(coord, coord + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0));
						}
						shadowCount += 6;
					}
				}
			}
		}
		if (lightCountChanged)
		{ // light count in light attribute
			auto count = pLights.size();
			lightBuffer->update(&count, *stagingBuffer, 4);
			lightCountChanged = false;
		}

		camera.m_changed = false;

		for (auto pLight : pLights)
			pLight->m_changed = false;
		for (auto pObject : pObjects)
			pObject->m_changed = false;
		for (auto pTerrain : pTerrains)
			pTerrain->m_changed = false;
	}

	int Scene::getStoreImageIndex(Image *pImage)
	{
		int textureIndex = 0;
		for (auto storeImage : storeImages)
		{
			if (storeImage == pImage)
				return textureIndex;
			textureIndex++;
		}
		return -1;
	}

	void Scene::showWater()
	{
		//for (auto package : pgLists[pgWater])
		//{
		//	tke3Object *pObject = pObjects[package.id];
		//	glm::mat4 mat_mv = camera.getMat() * pObject->getMat();
		//	glm::mat3 mat_normal = camera.getMatNormal();
		//	tke3_program_3water::set_matrixMVP(*tke3_matProj * mat_mv);
		//	tke3_program_3water::set_matrixModelView(mat_mv);
		//	tke3_program_3water::set_matrixNormal(mat_normal);
		//	tke3_program_3water::set_matrixNormalTranspose(glm::transpose(mat_normal));
		//	glBindVertexArray(tke_vao_rect.vao);
		//	for (auto pmt : package.pMtList)
		//		glDrawArraysInstanced(GL_PATCHES, 0, 4, 64 * 64);
		//}
	}

	void Scene::updateAnimation()
	{
		for (auto pObject : pAnimatedObjects)
		{
			pObject->animationSolver->sample();
			for (int i = 0; i < pObject->pModel->bones.size(); i++)
				pObject->animationSolver->boneMatrix[i] = glm::mat4();
			pObject->pModel->refreshBone(pObject->animationSolver->boneData, pObject->animationSolver->boneMatrix);
			pObject->animationSolver->calcIK();
		}
	}

	void Scene::updateAnimationUBO()
	{
		for (auto pObject : pAnimatedObjects)
		{
			pObject->animationSolver->fixMatrix();
			pObject->animationSolver->updateUBO();
		}
	}

	Scene *scene;

	LightSave::LightSave(Light &light)
		: Transformer(light),
		type(light.type),
		color(light.color),
		decayFactor(light.decayFactor),
		shadow(light.shadow) {}

	ObjectSave::ObjectSave(Object &object)
		: Transformer(object),
		pModel(object.pModel),
		phyx(object.phyx),
		moveType(object.moveType),
		upMethod(object.upMethod) {}

	//TerrainSave::TerrainSave(Terrain &terrain)
	//	: Transformer(terrain),
	//	size(terrain.size),
	//	height(terrain.height),
	//	heightMap(terrain.heightMap),
	//	colorMap(terrain.colorMap),
	//	spec(terrain.spec),
	//	roughness(terrain.roughness) {}

	void SceneSave::push(Scene *pScene)
	{
		EnterCriticalSection(&pScene->cs);

		pScene->atmosphere = atmosphere;
		//pScene->hdr = hdr;
		//pScene->ambient = ambient;
		pScene->fogThickness = fogThickness;

		pScene->clearActors();

		for (auto &lightSave : lightSaves)
		{
			auto pLight = new Light;
			pLight->type = lightSave.type;
			memcpy(pLight, &lightSave, sizeof(Transformer));

			pLight->color = lightSave.color;
			pLight->decayFactor = lightSave.decayFactor;
			pLight->shadow = lightSave.shadow;

			scene->addLight(pLight, lightSave.m_id);
		}

		int objID = 0;
		for (auto &objectSave : objectSaves)
		{
			auto pObject = new Object;
			memcpy(pObject, &objectSave, sizeof(Transformer));
			pObject->pModel = objectSave.pModel;

			pObject->phyx = objectSave.phyx;
			pObject->moveType = objectSave.moveType;
			pObject->upMethod = objectSave.upMethod;

			pScene->addObject(pObject, objectSave.m_id);

			if (objID == controlingID)
				controllingObject = pObject;
		}

		//for (auto &terrainSave : terrainSaves)
		//{
		//	auto pTerrain = new Terrain;
		//	memcpy(pTerrain, &terrainSave, sizeof(Transformer));

		//	pTerrain->size = terrainSave.size;
		//	pTerrain->height = terrainSave.height;
		//	pTerrain->heightMap = terrainSave.heightMap;
		//	pTerrain->colorMap = terrainSave.colorMap;
		//	pTerrain->spec = terrainSave.spec;
		//	pTerrain->roughness = terrainSave.roughness;

		//	pScene->addTerrain(pTerrain, terrainSave.m_id);
		//}

		LeaveCriticalSection(&pScene->cs);
	}

	void SceneSave::pull(Scene *pScene)
	{
		EnterCriticalSection(&pScene->cs);

		atmosphere = pScene->atmosphere;
		//hdr = pScene->hdr;
		//ambient = pScene->ambient;
		fogThickness = pScene->fogThickness;

		lightSaves.clear();
		for (auto pLight : pScene->pLights)
			lightSaves.push_back(LightSave(*pLight));

		objectSaves.clear();
		controlingID = -1;
		int objID = 0;
		for (auto pObject : pScene->pObjects)
		{
			if (controllingObject == pObject)
				controlingID = objID;
			objectSaves.push_back(ObjectSave(*pObject));
			objID++;
		}

		//terrainSaves.clear();
		//for (auto pTerrain : pScene->pTerrains)
		//	terrainSaves.push_back(TerrainSave(*pTerrain));

		LeaveCriticalSection(&pScene->cs);
	}

	void loadScene(char *s)
	{
		scene->clearActors();
		scene->clearModel();

		scene->load(s);
		scene->setUp();
	}
}
