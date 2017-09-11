#include <map>
#include <regex>

#include "define.h"
#include "core.h"
#include "ui/ui.h"
#include "physics/physics.h"
#include "sound/sound.h"
#include "window.h"
#include "render/renderpass.h"

namespace tke
{
	StagingBuffer *stagingBuffer = nullptr;

	bool needUpdateVertexBuffer = true;
	bool needUpdateMaterialBuffer = true;
	bool needUpdateTexture = true;

	std::vector<std::pair<std::string, Image*>> debugImages;

	std::vector<std::unique_ptr<Image>> textures;

	std::vector<std::unique_ptr<Image>> modelTextures;

	Image *addModelTexture(const std::string &_filename, bool sRGB)
	{
		auto filename = std::experimental::filesystem::path(_filename).string();
		for (auto &i : modelTextures)
		{
			if (i->full_filename == filename)
				return i.get();
		}
		auto i = createImage(filename, sRGB);
		i->index = modelTextures.size();
		modelTextures.push_back(std::move(std::unique_ptr<Image>(i)));
		return i;
	}

	std::vector<Material*> modelMaterials;
	Material *defaultMaterial = nullptr;
	Material *addModelMaterial(unsigned char albedoR, unsigned char albedoG, unsigned char albedoB, unsigned char alpha,
		unsigned char spec, unsigned char roughness, Image *albedoAlphaMap, Image *normalHeightMap, Image *specRoughnessMap)
	{
		for (auto m : modelMaterials)
		{
			if (m->albedoAlphaMap != albedoAlphaMap ? false : (!albedoAlphaMap && m->albedoR == albedoR && m->albedoG == albedoG && m->albedoB == albedoB && m->alpha == alpha)
				&& m->specRoughnessMap != specRoughnessMap ? false : (!specRoughnessMap && m->spec == spec && m->roughness == roughness)
				&& m->normalHeightMap == normalHeightMap)
				return m;
		}
		auto m = new Material;
		m->albedoR = albedoR;
		m->albedoG = albedoG;
		m->albedoB = albedoB;
		m->alpha = alpha;
		m->spec = spec;
		m->roughness = roughness;
		m->albedoAlphaMap = albedoAlphaMap;
		m->normalHeightMap = normalHeightMap;
		m->specRoughnessMap = specRoughnessMap;
		m->sceneIndex = modelMaterials.size();
		modelMaterials.push_back(m);
		return m;
	}

	std::vector<std::unique_ptr<Animation>> animations;

	std::vector<std::unique_ptr<Model>> models;

	std::vector<std::unique_ptr<Scene>> scenes;

	VertexBuffer *staticVertexBuffer = nullptr;
	IndexBuffer *staticIndexBuffer = nullptr;

	VertexBuffer *animatedVertexBuffer = nullptr;
	IndexBuffer *animatedIndexBuffer = nullptr;

	UniformBuffer *constantBuffer = nullptr;
	UniformBuffer *materialBuffer = nullptr;

	Image *plainDepthImage = nullptr;
	Image *pickUpImage = nullptr;

	RenderPass *renderPass_image8;
	RenderPass *renderPass_image8_clear;
	RenderPass *renderPass_image16;
	RenderPass *renderPass_image16_clear;
	RenderPass *renderPass_depth_clear;
	RenderPass *renderPass_depth_clear_image8;
	RenderPass *renderPass_depth_clear_image8_clear;
	RenderPass *renderPass_depth_clear_image32f_clear;

	Framebuffer *pickUpFb = nullptr;

	Pipeline *plainPipeline_2d = nullptr;
	Pipeline *plainPipeline_3d = nullptr;
	Pipeline *plainPipeline_3d_anim = nullptr;
	Pipeline *plainPipeline_3d_normal = nullptr;
	Pipeline *plainPipeline_3d_tex = nullptr;
	Pipeline *plainPipeline_3d_anim_tex = nullptr;
	Pipeline *plainPipeline_3d_wire = nullptr;
	Pipeline *plainPipeline_3d_anim_wire = nullptr;
	Pipeline *plainPipeline_3d_line = nullptr;
	int plain3d_bone_pos = -1;

	DescriptorSet *ds_maps = nullptr;

	std::vector<EventList*> eventLists;

	void addEventList(EventList *p)
	{
		eventLists.push_back(p);
	}

	void removeEventList(EventList *p)
	{
		for (auto it = eventLists.begin(); it != eventLists.end(); it++)
		{
			if (*it == p)
			{
				eventLists.erase(it);
				return;
			}
		}
	}

	void processEvents()
	{
		for (auto it = eventLists.begin(); it != eventLists.end(); )
		{
			auto list = *it;

			if (list->currentEventIndex >= list->events.size())
			{
				if (list->repeat)
				{
					list->currentEventIndex = 0;
				}
				else
				{
					delete list;
					it = eventLists.erase(it);
					continue;
				}
				it++;
				continue;
			}

			Event &e = list->events[list->currentEventIndex];
			e.currentTime += timeDisp;
			if (e.tickFunc) e.tickFunc(e.currentTime);

			if (e.currentTime >= e.duration)
			{
				if (e.execFunc) e.execFunc();
				list->currentEventIndex++;
			}

			it++;
		}
	}

	void processCmdLine(const std::string &str, bool record)
	{
		static std::string last_cmd;

		std::string string(str);

		std::regex pat(R"([\w\.]+)");
		std::smatch sm;

		if (std::regex_search(string, sm, pat))
		{
			if (sm[0].str() == "r")
			{
				processCmdLine(last_cmd.c_str(), false);
			}
		}
	}

	unsigned int pickUp(int x, int y, void(*drawCallback)(CommandBuffer*))
	{
		if (x < 0 || y < 0 || x > pickUpImage->levels[0].cx || y > pickUpImage->levels[0].cy)
			return 0;

		auto cb = begineOnceCommandBuffer();
		cb->beginRenderPass(renderPass_depth_clear_image8_clear, pickUpFb);
		drawCallback(cb);
		cb->endRenderPass();
		endOnceCommandBuffer(cb);

		cb = begineOnceCommandBuffer();
		VkBufferImageCopy range = {};
		range.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.imageSubresource.layerCount = 1;
		range.imageOffset.x = x;
		range.imageOffset.y = y;
		range.imageExtent.width = 1;
		range.imageExtent.height = 1;
		range.imageExtent.depth = 1;
		vkCmdCopyImageToBuffer(cb->v, pickUpImage->v, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingBuffer->v, 1, &range);
		endOnceCommandBuffer(cb);

		auto pixel = (unsigned char*)stagingBuffer->map(0, 4);
		unsigned int index = pixel[0] + (pixel[1] << 8) + (pixel[2] << 16) + (pixel[3] << 24);
		stagingBuffer->unmap();

		return index;
	}

	struct ConstantBufferStruct
	{
		float depth_near;
		float depth_far;
		float cx;
		float cy;
		float aspect;
		float fovy;
		float tanHfFovy;
		float envrCx;
		float envrCy;
	};

	struct MaterialShaderStruct
	{
		unsigned int albedoAlphaCompress;
		unsigned int specRoughnessCompress;

		unsigned int mapIndex;

		unsigned int dummy;
	};

	int init(const std::string &path, int rcx, int rcy)
	{
		enginePath = path;

		resCx = rcx;
		resCy = rcy;

		matOrtho = glm::mat4(glm::vec4(1.f, 0.f, 0.f, 0.f), glm::vec4(0.f, -1.f, 0.f, 0.f), glm::vec4(0.f, 0.f, 1.f, 0.f), glm::vec4(0.f, 0.f, 0.f, 1.f)) * glm::ortho(-1.f, 1.f, -1.f, 1.f, TKE_NEAR, TKE_FAR * 2);
		matOrthoInv = glm::inverse(matOrtho);
		aspect = (float)resCx / resCy;
		matPerspective = glm::mat4(glm::vec4(1.f, 0.f, 0.f, 0.f), glm::vec4(0.f, -1.f, 0.f, 0.f), glm::vec4(0.f, 0.f, 1.f, 0.f), glm::vec4(0.f, 0.f, 0.f, 1.f)) * glm::perspective(TKE_FOVY, aspect, TKE_NEAR, TKE_FAR);
		matPerspectiveInv = glm::inverse(matPerspective);

		initVulkan(
#if defined(_DEBUG)
			true
#else
			false
#endif
		);

		defaultMaterial = new Material;
		defaultMaterial->sceneIndex = 0;
		modelMaterials.push_back(defaultMaterial);

		stagingBuffer = new StagingBuffer(67108864);

		plainDepthImage = new Image(resCx, resCy, VK_FORMAT_D16_UNORM, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		globalResource.setImage(plainDepthImage, "Depth.Image");

		pickUpImage = new Image(resCx, resCy, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);

		{
			auto att0 = colorAttachmentDesc(VK_FORMAT_R8G8B8A8_UNORM, VK_ATTACHMENT_LOAD_OP_DONT_CARE);
			auto att1 = colorAttachmentDesc(VK_FORMAT_R8G8B8A8_UNORM, VK_ATTACHMENT_LOAD_OP_CLEAR);
			auto att2 = colorAttachmentDesc(VK_FORMAT_R16G16B16A16_SFLOAT, VK_ATTACHMENT_LOAD_OP_DONT_CARE);
			auto att3 = colorAttachmentDesc(VK_FORMAT_R16G16B16A16_SFLOAT, VK_ATTACHMENT_LOAD_OP_CLEAR);
			auto att4 = swapchainAttachmentDesc(VK_ATTACHMENT_LOAD_OP_DONT_CARE);
			auto att5 = swapchainAttachmentDesc(VK_ATTACHMENT_LOAD_OP_CLEAR);
			auto att6 = depthAttachmentDesc(VK_FORMAT_D16_UNORM, VK_ATTACHMENT_LOAD_OP_CLEAR);
			auto att7 = colorAttachmentDesc(VK_FORMAT_R32_SFLOAT, VK_ATTACHMENT_LOAD_OP_CLEAR);
			VkAttachmentReference col_ref = { 0, VK_IMAGE_LAYOUT_GENERAL };
			VkAttachmentReference dep_ref0 = { 0, VK_IMAGE_LAYOUT_GENERAL };
			VkAttachmentReference dep_ref1 = { 1, VK_IMAGE_LAYOUT_GENERAL };
			VkSubpassDescription subpass0 = subpassDesc(1, &col_ref);
			VkSubpassDescription subpass1 = subpassDesc(0, nullptr, &dep_ref0);
			VkSubpassDescription subpass2 = subpassDesc(1, &col_ref, &dep_ref1);
			VkAttachmentDescription atts0[] = {
				att0,
				att6
			};
			VkAttachmentDescription atts1[] = {
				att1,
				att6
			};
			VkAttachmentDescription atts2[] = {
				att7,
				att6
			};
			renderPass_image8 = new RenderPass(1, &att0, 1, &subpass0);
			renderPass_image8_clear = new RenderPass(1, &att1, 1, &subpass0);
			renderPass_image16 = new RenderPass(1, &att2, 1, &subpass0);
			renderPass_image16_clear = new RenderPass(1, &att3, 1, &subpass0);
			renderPass_depth_clear = new RenderPass(1, &att6, 1, &subpass1);
			renderPass_depth_clear_image8 = new RenderPass(ARRAYSIZE(atts0), atts0, 1, &subpass2);
			renderPass_depth_clear_image8_clear = new RenderPass(ARRAYSIZE(atts1), atts1, 1, &subpass2);
			renderPass_depth_clear_image32f_clear = new RenderPass(ARRAYSIZE(atts2), atts2, 1, &subpass2);
		}

		{
			VkImageView views[] = {
				pickUpImage->getView(),
				plainDepthImage->getView()
			};
			pickUpFb = getFramebuffer(resCx, resCy, renderPass_depth_clear_image8_clear, ARRAYSIZE(views), views);
		}

		plainPipeline_2d = new Pipeline;
		plainPipeline_2d->loadXML(enginePath + "pipeline/plain2d/plain2d.xml");
		plainPipeline_2d->setup(renderPass_image8, 0, true);

		plainPipeline_3d = new Pipeline;
		plainPipeline_3d->loadXML(enginePath + "pipeline/plain3d/plain3d.xml");
		plainPipeline_3d->setup(renderPass_depth_clear_image8, 0, false);
		plainPipeline_3d_anim = new Pipeline;
		plainPipeline_3d_anim->loadXML(enginePath + "pipeline/plain3d/plain3d_anim.xml");
		plainPipeline_3d_anim->setup(renderPass_depth_clear_image8, 0, true);
		plainPipeline_3d_normal = new Pipeline;
		plainPipeline_3d_normal->loadXML(enginePath + "pipeline/plain3d/plain3d_normal.xml");
		plainPipeline_3d_normal->setup(renderPass_depth_clear_image8, 0, false);
		plainPipeline_3d_tex = new Pipeline;
		plainPipeline_3d_tex->loadXML(enginePath + "pipeline/plain3d/plain3d_tex.xml");
		plainPipeline_3d_tex->setup(renderPass_depth_clear_image8, 0, false);
		plainPipeline_3d_anim_tex = new Pipeline;
		plainPipeline_3d_anim_tex->loadXML(enginePath + "pipeline/plain3d/plain3d_anim_tex.xml");
		plainPipeline_3d_anim_tex->setup(renderPass_depth_clear_image8, 0, true);
		plainPipeline_3d_wire = new Pipeline;
		plainPipeline_3d_wire->loadXML(enginePath + "pipeline/plain3d/plain3d_wire.xml");
		plainPipeline_3d_wire->setup(renderPass_image8, 0, false);
		plainPipeline_3d_anim_wire = new Pipeline;
		plainPipeline_3d_anim_wire->loadXML(enginePath + "pipeline/plain3d/plain3d_anim_wire.xml");
		plainPipeline_3d_anim_wire->setup(renderPass_image8, 0, true);
		plainPipeline_3d_line = new Pipeline;
		plainPipeline_3d_line->loadXML(enginePath + "pipeline/plain3d/plain3d_line.xml");
		plainPipeline_3d_line->setup(renderPass_image8, 0, false);
		plain3d_bone_pos = plainPipeline_3d_anim_wire->descriptorPosition("BONE");

		staticVertexBuffer = new VertexBuffer();
		staticIndexBuffer = new IndexBuffer();

		animatedVertexBuffer = new VertexBuffer();
		animatedIndexBuffer = new IndexBuffer();

		constantBuffer = new UniformBuffer(sizeof ConstantBufferStruct);
		materialBuffer = new UniformBuffer(sizeof(MaterialShaderStruct) * TKE_MAX_MATERIAL_COUNT);

		globalResource.setBuffer(staticVertexBuffer, "Static.VertexBuffer");
		globalResource.setBuffer(staticIndexBuffer, "Static.IndexBuffer");

		globalResource.setBuffer(animatedVertexBuffer, "Animated.VertexBuffer");
		globalResource.setBuffer(animatedIndexBuffer, "Animated.IndexBuffer");

		globalResource.setBuffer(constantBuffer, "Constant.UniformBuffer");
		globalResource.setBuffer(materialBuffer, "Material.UniformBuffer");

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
			constantBuffer->update(&stru, stagingBuffer);
		}

		initScene();
		initGeneralModels();
		initPhysics();
		//initSound();
		initWindow();

		return NoErr;
	}

	void run()
	{
		lastTime = GetTickCount();

		ds_maps = new DescriptorSet(mrtPipeline, 1);

		for (;;)
		{
			nowTime = GetTickCount();
			timeDisp = nowTime - lastTime;
			processEvents();

			MSG msg;
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				if (needUpdateVertexBuffer)
				{
					std::vector<Vertex> staticVertexs;
					std::vector<int> staticIndices;

					std::vector<VertexAnimated> animatedVertexs;
					std::vector<int> animatedIndices;

					for (auto &m : models)
					{
						if (!m->animated)
						{
							m->vertexBase = staticVertexs.size();
							m->indiceBase = staticIndices.size();

							for (int i = 0; i < m->positions.size(); i++)
							{
								Vertex vertex;
								vertex.position = i < m->positions.size() ? m->positions[i] : glm::vec3(0.f);
								vertex.uv       = i < m->uvs.size()       ? m->uvs[i]       : glm::vec2(0.f);
								vertex.normal   = i < m->normals.size()   ? m->normals[i]   : glm::vec3(0.f);
								vertex.tangent  = i < m->tangents.size()  ? m->tangents[i]  : glm::vec3(0.f);

								staticVertexs.push_back(vertex);
							}
							for (int i = 0; i < m->indices.size(); i++)
							{
								staticIndices.push_back(m->indices[i]);
							}
						}
						else
						{
							m->vertexBase = animatedVertexs.size();
							m->indiceBase = animatedIndices.size();

							for (int i = 0; i < m->positions.size(); i++)
							{
								VertexAnimated vertex;
								vertex.position   = i < m->positions.size()   ? m->positions[i]   : glm::vec3(0.f);
								vertex.uv         = i < m->uvs.size()         ? m->uvs[i]         : glm::vec2(0.f);
								vertex.normal     = i < m->normals.size()     ? m->normals[i]     : glm::vec3(0.f);
								vertex.tangent    = i < m->tangents.size()    ? m->tangents[i]    : glm::vec3(0.f);
								vertex.boneWeight = i < m->boneWeights.size() ? m->boneWeights[i] : glm::vec4(0.f);
								vertex.boneID     = i < m->boneIDs.size()     ? m->boneIDs[i]     : glm::vec4(0.f);

								animatedVertexs.push_back(vertex);
							}
							for (int i = 0; i < m->indices.size(); i++)
							{
								animatedIndices.push_back(m->indices[i]);
							}
						}
					}

					if (staticVertexs.size() > 0) staticVertexBuffer->recreate(sizeof(Vertex) * staticVertexs.size(), staticVertexs.data());
					if (staticIndices.size() > 0) staticIndexBuffer->recreate(sizeof(int) * staticIndices.size(), staticIndices.data());

					if (animatedVertexs.size() > 0) animatedVertexBuffer->recreate(sizeof(VertexAnimated) * animatedVertexs.size(), animatedVertexs.data());
					if (animatedIndices.size() > 0) animatedIndexBuffer->recreate(sizeof(int) * animatedIndices.size(), animatedIndices.data());

					needUpdateVertexBuffer = false;
				}
				if (needUpdateTexture)
				{
					static int map_position = -1;
					if (map_position == -1 && mrtPipeline) map_position = mrtPipeline->descriptorPosition("maps");
					if (map_position != -1)
					{
						for (int index = 0; index < modelTextures.size(); index++)
							ds_maps->setImage(map_position, index, modelTextures[index].get(), colorSampler);
						needUpdateTexture = false;
					}
				}
				if (needUpdateMaterialBuffer)
				{
					if (modelMaterials.size() > 0)
					{
						std::unique_ptr<MaterialShaderStruct[]> mts(new MaterialShaderStruct[modelMaterials.size()]);

						for (int i = 0; i < modelMaterials.size(); i++)
						{
							auto m = modelMaterials[i];

							MaterialShaderStruct mt;
							mts[i].albedoAlphaCompress = m->albedoR + (m->albedoG << 8) + (m->albedoB << 16) + (m->alpha << 24);
							mts[i].specRoughnessCompress = m->spec + (m->roughness << 8);
							mts[i].mapIndex = (m->albedoAlphaMap ? m->albedoAlphaMap->index + 1 : 0) +
								((m->normalHeightMap ? m->normalHeightMap->index + 1 : 0) << 8) +
								((m->specRoughnessMap ? m->specRoughnessMap->index + 1 : 0) << 16);
						}

						materialBuffer->update(mts.get(), stagingBuffer, sizeof(MaterialShaderStruct) * modelMaterials.size());
					}
					needUpdateMaterialBuffer = false;
				}

				if (current_window->dead)
				{
					delete current_window;
					return;
				}
				else
				{
					current_window->mouseDispX = current_window->mouseX - current_window->mousePrevX;
					current_window->mouseDispY = current_window->mouseY - current_window->mousePrevY;

					current_window->renderEvent();
					current_window->frameCount++;

					current_window->mouseLeft.justDown = false;
					current_window->mouseLeft.justUp = false;
					current_window->mousePrevX = current_window->mouseX;
					current_window->mousePrevY = current_window->mouseY;
					current_window->mouseScroll = 0;
				}
			}
			lastTime = nowTime;
		}
	}
}
