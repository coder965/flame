#pragma once

#include "../utils.h"
#include "../math/math.h"
#include "camera.h"
#include "light.h"
#include "object.h"
#include "terrain.h"
#include "water.h"
#include "../physics/physics.h"
#include "../render/buffer.h"
#include "../render/image.h"
#include "../render/framebuffer.h"
#include "../render/pipeline.h"
#include "../render/command_buffer.h"
#include "../resource/resource.h"

namespace tke
{
	enum { MaxStaticObjectCount = 1024 };
	enum { MaxAnimatedObjectCount = 8 };
	enum { MaxLightCount = 256 };
	enum { MaxWaterCount = 8 };
	enum { MaxShadowCount = 8 };

	enum { EnvrSizeCx = 128 * 4 };
	enum { EnvrSizeCy = 128 * 4 };
	enum { ShadowMapCx = 2048 };
	enum { ShadowMapCy = 2048 };

	enum { MaxIndirectCount = 1024 };

	enum class SkyType
	{
		null,
		atmosphere_scattering = 1 << 0,
		panorama = 1 << 1
	};

	struct LightShaderStruct
	{
		glm::vec4 coord;    // xyz - coord(point/spot)/dir(parallax), w - the light type
		glm::vec4 color;    // rgb - color, a - shadow index(-1 is no shadow)
		glm::vec4 spotData; // xyz - spot direction, a - spot range
	};

	struct LightBufferShaderStruct
	{
		unsigned int count;
		unsigned int dummy0;
		unsigned int dummy1;
		unsigned int dummy2;

		LightShaderStruct lights[MaxLightCount];
	};

	struct TerrainShaderStruct
	{
		glm::vec3 coord;
		int blockCx;
		float blockSize;
		float height;
		float tessellationFactor;
		float textureUvFactor;
		float mapDimension;
		unsigned int dummy0;
		unsigned int dummy1;
		unsigned int dummy2;
	};

	struct WaterShaderStruct
	{
		glm::vec3 coord;
		int blockCx;
		float blockSize;
		float height;
		float tessellationFactor;
		float textureUvFactor;
		float mapDimension;
		unsigned int dummy0;
		unsigned int dummy1;
		unsigned int dummy2;
	};

	struct AmbientBufferShaderStruct
	{
		glm::vec3 color;
		glm::uint envr_max_mipmap;
		glm::vec4 fogcolor;
	};

	IMPL(nullptr) Pipeline *scatteringPipeline;
	IMPL(nullptr) Pipeline *downsamplePipeline;
	IMPL(nullptr) Pipeline *convolvePipeline;

	IMPL(nullptr) Pipeline *mrtPipeline;
	IMPL(nullptr) Pipeline *mrtAnimPipeline;
	IMPL(nullptr) Pipeline *terrainPipeline;
	IMPL(nullptr) Pipeline *waterPipeline;
	IMPL(nullptr) Pipeline *proceduralTerrainPipeline;
	IMPL(nullptr) Pipeline *deferredPipeline;
	IMPL(nullptr) Pipeline *esmPipeline;
	IMPL(nullptr) Pipeline *esmAnimPipeline;
	IMPL(nullptr) Pipeline *composePipeline;

	REFLECTABLE struct Scene
	{
		REFL_BANK;

		std::mutex mtx;

		REFLv std::string name;
		std::string filename;
		std::string filepath;

		SkyType skyType = SkyType::atmosphere_scattering;
		glm::vec2 sunDir = glm::vec2(0.f, -90.f);
		float atmosphereSunE = 20.f;
		float atmosphereInnerRadius = 10.f; // The inner (planetary) radius
		float atmosphereOuterRadius = 10.25f; // The outer (atmosphere) radius
		float atmosphereCameraHeight = 10.0002f;
		float atmosphereKm = 0.0025f;
		float atmosphereKr = 0.001f;
		std::string skyFilename;

		float hdrExposure = 0.01f;
		float hdrWhite = 1.f;

		glm::vec3 ambientColor = glm::vec3(0.5f);
		glm::vec3 fogColor = glm::vec3(0.5f);

		float ssaoRadius = 10.f;
		float ssaoBias = 0.01f;
		float ssaoIntensity = 100000.f;

		float fogThickness = 0.01f;

		Camera camera;

		std::vector<std::unique_ptr<Light>> lights;
		Light *sunLight = nullptr;

		std::vector<std::unique_ptr<Object>> objects;

		std::unique_ptr<Terrain> terrain;

		std::vector<std::unique_ptr<Water>> waters;

		bool needUpdateSky = true;
		bool needUpdateIndirectBuffer = true;
		bool needUpdateAmbientBuffer = true;
		bool needUpdateLightCount = true;

		std::vector<CollisionGroup*> pCollisionGroups;

		int staticIndirectCount = 0;
		int animatedIndirectCount = 0;

		physx::PxScene *pxScene = nullptr;
		physx::PxControllerManager *pxControllerManager = nullptr;

		Resource resource;

		std::unique_ptr<Image> envrImage;

		std::unique_ptr<Image> mainImage;
		std::unique_ptr<Image> depthImage;
		std::unique_ptr<Image> albedoAlphaImage;
		std::unique_ptr<Image> normalHeightImage;
		std::unique_ptr<Image> specRoughnessImage;
		std::unique_ptr<Image> esmImage;
		std::unique_ptr<Image> esmDepthImage;

		std::unique_ptr<UniformBuffer> matrixBuffer;
		std::unique_ptr<UniformBuffer> staticObjectMatrixBuffer;
		std::unique_ptr<UniformBuffer> animatedObjectMatrixBuffer;
		std::unique_ptr<IndirectIndexBuffer> staticObjectIndirectBuffer;
		std::unique_ptr<IndirectIndexBuffer> animatedObjectIndirectBuffer;
		std::unique_ptr<UniformBuffer> terrainBuffer;
		std::unique_ptr<UniformBuffer> waterBuffer;
		std::unique_ptr<UniformBuffer> lightBuffer;
		std::unique_ptr<UniformBuffer> shadowBuffer;
		std::unique_ptr<UniformBuffer> ambientBuffer;

		std::unique_ptr<DescriptorSet> ds_mrt;
		std::unique_ptr<DescriptorSet> ds_mrtAnim;
		std::unique_ptr<DescriptorSet> ds_mrtAnim_bone;
		std::unique_ptr<DescriptorSet> ds_terrain;
		std::unique_ptr<DescriptorSet> ds_water;
		std::unique_ptr<DescriptorSet> ds_esm;
		std::unique_ptr<DescriptorSet> ds_esmAnim;
		std::unique_ptr<DescriptorSet> ds_defe;
		std::unique_ptr<DescriptorSet> ds_comp;

		std::unique_ptr<CommandBuffer> cb_shadow;
		std::unique_ptr<CommandBuffer> cb_mrt;
		std::unique_ptr<CommandBuffer> cb_deferred;

		std::shared_ptr<Framebuffer> fb_esm[MaxShadowCount * 6];

		VkEvent shadowRenderFinished;
		VkEvent mrtRenderFinished;

		Scene();
		~Scene();
		void addLight(Light *l);
		Light *removeLight(Light *l);
		void addObject(Object *o);
		Object *removeObject(Object *o);
		int getCollisionGroupID(int ID, unsigned int mask);
		void addTerrain(Terrain *t);
		void removeTerrain();
		void addWater(Water *w);
		Water *removeWater(Water *w);
		void clear();
		void setSunDir(const glm::vec2 &);
		void setAmbientColor(const glm::vec3 &);
		void setFogColor(const glm::vec3 &);
		std::shared_ptr<Framebuffer> createFramebuffer(Image *dst);
		void show(Framebuffer *fb, VkEvent signalEvent);
		void loadSky(const char *skyMapFilename, int radianceMapCount, const char *radianceMapFilenames[], const char *irradianceMapFilename);
		void load(const std::string &filename);
		void save(const std::string &filename);
	};

	void initScene();
}
