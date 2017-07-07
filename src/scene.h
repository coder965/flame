#pragma once

#include <vector>

#include "core.h"
#include "render.h"
#include "camera.h"
#include "light.h"
#include "object.h"
#include "terrain.h"
#include "model.h"
#include "physics.h"

namespace tke
{
	struct CollisionGroup;

	enum SkyType
	{
		SkyTypeNull,
		SkyTypeAtmosphereScattering,
		SkyTypePanorama
	};

	struct LightShaderStruct
	{
		glm::vec4 coord; // w - the light type
		glm::vec4 color;
		glm::vec4 spotData; // spot direction and spot range
	};

	struct LightBufferShaderStruct
	{
		unsigned int count;
		unsigned int dummy0;
		unsigned int dummy1;
		unsigned int dummy2;

		LightShaderStruct lights[TKE_MAX_LIGHT_COUNT];
	};

	struct HeightMapTerrainShaderStruct
	{
		unsigned int patchSize;
		float ext;
		float height;
		float tessFactor;
		float mapDim;
	};

	struct AmbientBufferShaderStruct
	{
		glm::vec4 v;
		glm::vec4 fogcolor;
	};

	extern Image *envrImage;
	extern Image *envrImageDownsample[3];

	extern Image *mainImage;
	extern Image *albedoAlphaImage;
	extern Image *normalHeightImage;
	extern Image *specRoughnessImage;

	extern RenderPass *sceneRenderPass;

	extern Pipeline *scatteringPipeline;
	extern Pipeline *downsamplePipeline;
	extern Pipeline *convolvePipeline;

	extern Pipeline *panoramaPipeline;
	extern Pipeline *mrtPipeline;
	extern Pipeline *mrtAnimPipeline;
	extern Pipeline *heightMapTerrainPipeline;
	extern Pipeline *proceduralTerrainPipeline;
	extern Pipeline *deferredPipeline;
	extern Pipeline *composePipeline;

	extern UniformBuffer *matrixBuffer;
	extern UniformBuffer *staticObjectMatrixBuffer;
	extern UniformBuffer *animatedObjectMatrixBuffer;
	extern IndirectIndexBuffer *staticObjectIndirectBuffer;
	extern IndirectIndexBuffer *animatedObjectIndirectBuffer;
	extern UniformBuffer *heightMapTerrainBuffer;
	extern UniformBuffer *proceduralTerrainBuffer;
	extern UniformBuffer *lightBuffer;
	extern UniformBuffer *ambientBuffer;

	struct Scene
	{
		CRITICAL_SECTION cs;

		std::string name;
		std::string filename;
		std::string filepath;

		SkyType skyType = SkyTypeAtmosphereScattering;
		glm::vec2 atmosphereSunDir = glm::vec2(0.f, -90.f);
		float atmosphereSunE = 20.f;
		float atmosphereInnerRadius = 10.f; // The inner (planetary) radius
		float atmosphereOuterRadius = 10.25f; // The outer (atmosphere) radius
		float atmosphereCameraHeight = 10.0002f;
		float atmosphereKm = 0.0025f;
		float atmosphereKr = 0.001f;
		std::string skyFilename;

		float exposure = 0.01f;
		float white = 1.f;

		glm::vec3 ambientColor = glm::vec3(0.5f);

		float ssaoRadius = 10.f;
		float ssaoBias = 0.01f;
		float ssaoIntensity = 100000.f;

		float fogThickness = 0.01f;

		Camera camera;

		std::vector<Light*> lights;
		Light *pSunLight = nullptr;

		std::vector<Object*> objects;

		std::vector<Terrain*> terrains;

		bool showProceduralTerrain = false;

		bool needUpdateSky = true;
		bool needUpdateIndirectBuffer = true;
		bool lightCountChanged = true;

		std::vector<CollisionGroup*> pCollisionGroups;

		int shadowCount = 0;
		int staticIndirectCount = 0;
		int animatedIndirectCount = 0;

		physx::PxScene *pxScene = nullptr;
		physx::PxControllerManager *pxControllerManager = nullptr;

		Scene();
		~Scene();

		void loadSky(const char *skyMapFilename, int radianceMapCount, const char *radianceMapFilenames[], const char *irradianceMapFilename);
		void addLight(Light *pLight);
		Light *deleteLight(Light *pLight);
		void addObject(Object *pObject);
		Object *deleteObject(Object *pObject);
		int getCollisionGroupID(int ID, unsigned int mask);
		void addTerrain(Terrain *pTerrain);
		Terrain *deleteTerrain(Terrain *pTerrain);
		void clear();
		Framebuffer *createFramebuffer(Image *dst);
		void show(CommandBuffer *cb, Framebuffer *fb, VkEvent signalEvent);
		void load(const std::string &filename);
		void save(const std::string &filename);
	};

	void initScene();
}
