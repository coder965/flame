#pragma once

#include <mutex>

#include "define.h"
#include "render.h"
#include "camera.h"
#include "transformer.h"
#include "controler.h"
#include "model.h"
#include "physics.h"

namespace tke
{
	enum LightType
	{
		LightTypeParallax,
		LightTypePoint,
		LightTypeSpot
	};

	std::string getLightTypeName(LightType _type);

	struct Light : Transformer
	{
		LightType type;
		glm::vec3 color = glm::vec3(0.5f);
		float range = 0.5f;

		bool shadow = false;

		int sceneIndex = -1;
		int sceneShadowIndex = -1;

		Light(LightType _type);
	};

	enum ObjectPhysicsType
	{
		ObjectPhysicsTypeNull = 0,
		ObjectPhysicsTypeStatic = 1 << 0, // cannot use with dynamic bit
		ObjectPhysicsTypeDynamic = 1 << 1, // cannot use with static bit
		ObjectPhysicsTypeController = 1 << 2
	};

	struct RigidBodyData
	{
		Rigidbody *rigidbody;
		physx::PxRigidActor *actor;
		glm::mat3 rotation;
		glm::vec3 coord;
	};

	struct Object : Transformer, Controller
	{
		Model *model;

		ObjectPhysicsType physicsType; // cannot change

		AnimationComponent *animationComponent = nullptr;
		std::vector<RigidBodyData> rigidbodyDatas;
		physx::PxController *pxController = nullptr;
		float floatingTime = 0.f;

		int sceneIndex = -1;

		Object(Model *_model, ObjectPhysicsType _physicsType = ObjectPhysicsTypeNull);
		~Object();
	};

	enum TerrainType
	{
		TerrainTypeHeightMap,
		TerrainTypeProcedural
	};

	struct Terrain : Transformer
	{
		TerrainType type;

		float ext = 10.f;
		float height = 10.f;
		float tessFactor = 0.75f;

		Image *heightMap = nullptr;
		Image *colorMap = nullptr;
		float spec = 0.04f;
		float roughness = 1.f;

		Terrain(TerrainType _type);
	};

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
		std::mutex mtx;

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

		Terrain *terrain = nullptr;

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
		Light *removeLight(Light *pLight);
		void addObject(Object *pObject);
		Object *removeObject(Object *pObject);
		int getCollisionGroupID(int ID, unsigned int mask);
		void addTerrain(Terrain *pTerrain);
		void removeTerrain();
		void clear();
		Framebuffer *createFramebuffer(Image *dst);
		void show(CommandBuffer *cb, Framebuffer *fb, VkEvent signalEvent);
		void load(const std::string &filename);
		void save(const std::string &filename);
	};

	void initScene();
}