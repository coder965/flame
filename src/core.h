#pragma once

#include "math.h"
#include "entity/scene.h"
#include "render/buffer.h"
#include "render/image.h"
#include "render/command_buffer.h"
#include "render/descriptor.h"
#include "render/pipeline.h"

/*

UBO:

Constant - 0, 0
Material - 0, 1

Matrix - 0, 2
StaticObjectMatrix - 0, 3
AnimatedObjectMatrix - 0, 3
Terrain - 0, 4
Water - 0, 5
Light - 0, 6
Ambient - 0, 7
Shadow - 0, 8

Bone - 2, 0

Texture:

Envr - 0, 9
Main - 0, 10
Depth - 0, 11
AlbedoAlpha - 0, 12
NormalHeight - 0, 13
SpecRoughness - 0, 14
Esm - 0, 15
Ao - 0, 16

--Terrain Special--
HeightMap - 0, 17
BlendMap - 0, 18
ColorMaps - 0, 19
NormalMaps - 0, 20
--Terrain Special--

Maps - 1, 0

*/

enum { MaxMaterialCount = 256 };
enum { MaxTextureCount = 256 };

namespace tke
{
	IMPL(0.1f) float near_plane;
	IMPL(1000.f) float far_plane;
	IMPL(60.f) float fovy;

	IMPL() int lastTime;
	IMPL() int nowTime;
	IMPL() int timeDisp;

	IMPL() std::string enginePath;
	IMPL() int resCx;
	IMPL() int resCy;
	IMPL() float screenAspect;

	IMPL() glm::mat4 matOrtho;
	IMPL() glm::mat4 matOrthoInv;
	IMPL() glm::mat4 matPerspective;
	IMPL() glm::mat4 matPerspectiveInv;

	IMPL(nullptr) StagingBuffer *stagingBuffer;
	
	IMPL() bool needUpdateVertexBuffer;
	IMPL() bool needUpdateMaterialBuffer;
	IMPL() bool needUpdateTexture;

	IMPL() std::vector<std::pair<std::string,Image*>> debugImages;

	IMPL() std::vector<std::unique_ptr<Image>> textures;

	inline void addTexture(Image *i)
	{
		textures.push_back(std::move(std::unique_ptr<Image>(i)));
	}

	inline Image *getTexture(const std::string &_filename)
	{
		auto filename = std::experimental::filesystem::path(_filename).string();
		for (auto &i : textures)
		{
			if (i->full_filename == filename)
				return i.get();
		}
		return nullptr;
	}

	IMPL() std::vector<std::unique_ptr<Image>> modelTextures;
	Image *addModelTexture(const std::string &filename, bool sRGB = false);

	IMPL() std::vector<Material*> modelMaterials;
	IMPL(nullptr) Material *defaultMaterial;
	Material *addModelMaterial(unsigned char albedoR, unsigned char albedoG, unsigned char albedoB, unsigned char alpha, 
		unsigned char spec, unsigned char roughness, Image *albedoAlphaMap, Image *normalHeightMap, Image *specRoughnessMap);

	IMPL() std::vector<std::unique_ptr<Animation>> animations;
	inline Animation *getAnimation(const std::string &_filename)
	{
		auto filename = std::experimental::filesystem::path(_filename).string();
		for (auto &a : animations)
		{
			if (a->filename == filename)
				return a.get();
		}
		return nullptr;
	}

	IMPL() std::vector<std::unique_ptr<Model>> models;
	inline Model *getModel(const std::string &_filename)
	{
		auto filename = std::experimental::filesystem::path(_filename).string();
		for (auto &m : models)
		{
			if (m->filename == filename)
				return m.get();
		}
		return nullptr;
	}

	IMPL() std::vector<std::unique_ptr<Scene>> scenes;
	inline void addScene(Scene *s)
	{
		scenes.push_back(std::move(std::unique_ptr<Scene>(s)));
	}
	inline Scene *getScene(const std::string &_filename)
	{
		auto filename = std::experimental::filesystem::path(_filename).string();
		for (auto &s : scenes)
		{
			if (s->filename == filename)
				return s.get();
		}
		return nullptr;
	}

	IMPL(nullptr) VertexBuffer *staticVertexBuffer;
	IMPL(nullptr) IndexBuffer *staticIndexBuffer;

	IMPL(nullptr) VertexBuffer *animatedVertexBuffer;
	IMPL(nullptr) IndexBuffer *animatedIndexBuffer;

	IMPL(nullptr) UniformBuffer *constantBuffer;
	IMPL(nullptr) UniformBuffer *materialBuffer;

	IMPL(nullptr) Image *plainDepthImage;
	IMPL(nullptr) Image *pickUpImage;

	IMPL(nullptr) RenderPass *renderPass_image8;
	IMPL(nullptr) RenderPass *renderPass_image8_clear;
	IMPL(nullptr) RenderPass *renderPass_image16;
	IMPL(nullptr) RenderPass *renderPass_image16_clear;
	IMPL(nullptr) RenderPass *renderPass_depth_clear;
	IMPL(nullptr) RenderPass *renderPass_depth_clear_image8;
	IMPL(nullptr) RenderPass *renderPass_depth_clear_image8_clear;
	IMPL(nullptr) RenderPass *renderPass_depth_clear_image32f_clear;

	IMPL() std::shared_ptr<Framebuffer> pickUpFb;

	IMPL(nullptr) Pipeline *plainPipeline_2d;
	IMPL(nullptr) Pipeline *plainPipeline_3d;
	IMPL(nullptr) Pipeline *plainPipeline_3d_anim;
	IMPL(nullptr) Pipeline *plainPipeline_3d_normal;
	IMPL(nullptr) Pipeline *plainPipeline_3d_tex;
	IMPL(nullptr) Pipeline *plainPipeline_3d_anim_tex;
	IMPL(nullptr) Pipeline *plainPipeline_3d_wire;
	IMPL(nullptr) Pipeline *plainPipeline_3d_anim_wire;
	IMPL(nullptr) Pipeline *plainPipeline_3d_line;
	IMPL(-1) int plain3d_bone_pos;

	IMPL(nullptr) DescriptorSet *ds_maps;

	typedef void(*PF_TICK)(int);
	typedef void(*PF_EXEC)();

	struct Event
	{
		PF_TICK tickFunc = nullptr;
		int duration = 1;
		PF_EXEC execFunc = nullptr;

		int currentTime = 0;
	};

	struct EventList
	{
		std::vector<Event> events;
		bool repeat = false;
		int currentEventIndex = 0;
	};

	void addEventList(EventList *);
	void removeEventList(EventList *);
	void processEvents();
	void processCmdLine(const std::string &str, bool record = true);
	unsigned int pickUp(int x, int y, void(*drawCallback)(CommandBuffer*));
	int init(bool vulkan_debug, const std::string &path, int rcx, int rcy, bool only_2d = false);
	void run();
}
