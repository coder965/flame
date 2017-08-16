#pragma once

#include "math.h"
#include "scene.h"
#include "buffer.h"
#include "image.h"
#include "commnd_buffer.h"
#include "descriptor.h"
#include "pipeline.h"

namespace tke
{
	extern StagingBuffer *stagingBuffer;
	
	extern bool needUpdateVertexBuffer;
	extern bool needUpdateMaterialBuffer;
	extern bool needUpdateTexture;

	extern std::vector<std::pair<std::string, Image*>> debugImages;

	extern std::vector<std::unique_ptr<Image>> textures;

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

	extern std::vector<std::unique_ptr<Image>> modelTextures;
	Image *addModelTexture(const std::string &filename, bool sRGB = false);

	extern std::vector<Material*> modelMaterials;
	extern Material *defaultMaterial;
	Material *addModelMaterial(unsigned char albedoR, unsigned char albedoG, unsigned char albedoB, unsigned char alpha, 
		unsigned char spec, unsigned char roughness, Image *albedoAlphaMap, Image *normalHeightMap, Image *specRoughnessMap);

	extern std::vector<std::unique_ptr<Animation>> animations;
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

	extern std::vector<std::unique_ptr<Model>> models;
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

	extern std::vector<std::unique_ptr<Scene>> scenes;
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

	extern VertexBuffer *staticVertexBuffer;
	extern IndexBuffer *staticIndexBuffer;

	extern VertexBuffer *animatedVertexBuffer;
	extern IndexBuffer *animatedIndexBuffer;

	extern UniformBuffer *constantBuffer;
	extern UniformBuffer *materialBuffer;

	extern Image *plainDepthImage;
	extern Image *pickUpImage;

	extern RenderPass *renderPass_image8;
	extern RenderPass *renderPass_image8_clear;
	extern RenderPass *renderPass_image16;
	extern RenderPass *renderPass_image16_clear;
	extern RenderPass *renderPass_depth_clear;
	extern RenderPass *renderPass_depth_clear_image8;
	extern RenderPass *renderPass_depth_clear_image8_clear;
	extern RenderPass *renderPass_depth_clear_image32f_clear;

	extern Framebuffer *pickUpFb;

	extern Pipeline *plainPipeline_2d;
	extern Pipeline *plainPipeline_3d;
	extern Pipeline *plainPipeline_3d_anim;
	extern Pipeline *plainPipeline_3d_normal;
	extern Pipeline *plainPipeline_3d_tex;
	extern Pipeline *plainPipeline_3d_anim_tex;
	extern Pipeline *plainPipeline_3d_wire;
	extern Pipeline *plainPipeline_3d_anim_wire;
	extern Pipeline *plainPipeline_3d_line;
	extern int plain3d_bone_pos;

	extern DescriptorSet *ds_maps;

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
	int init(const std::string &path, int rcx, int rcy);
	void run();
}
