#ifndef __TKE_SCENE__
#define __TKE_SCENE__

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
	//struct MasterRenderer
	//

	//	Pipeline heightMapTerrainPipeline;
	//	Pipeline proceduralTerrainPipeline;

	//	DrawAction *mrtHeightMapTerrainAction;

	//};

	struct Animation;
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
		Image *skyImage = nullptr;

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

		bool needUpdateSky = true;
		bool needUpdateIndirectBuffer = true;
		bool lightCountChanged = true;

		UniformBuffer *matrixBuffer = nullptr;
		UniformBuffer *staticObjectMatrixBuffer = nullptr;
		UniformBuffer *animatedObjectMatrixBuffer = nullptr;
		IndirectIndexBuffer *staticObjectIndirectBuffer = nullptr;
		IndirectIndexBuffer *animatedObjectIndirectBuffer = nullptr;
		UniformBuffer *heightMapTerrainBuffer = nullptr;
		UniformBuffer *proceduralTerrainBuffer = nullptr;
		UniformBuffer *lightBuffer = nullptr;
		UniformBuffer *ambientBuffer = nullptr;

		UniformBuffer *lightMatrixBuffer = nullptr; // remove ??

		std::vector<CollisionGroup*> pCollisionGroups;

		int shadowCount = 0;
		int staticIndirectCount = 0;
		int animatedIndirectCount = 0;

		physx::PxScene *pxScene = nullptr;
		physx::PxControllerManager *pxControllerManager = nullptr;

		bool firstUpdate = true;

		Scene();
		~Scene();

		void loadSky(const char *skyMapFilename, int radianceMapCount, const char *radianceMapFilenames[], const char *irradianceMapFilename);
		void load(char *file);
		void save(char *file);

		void addLight(Light *pLight);
		Light *deleteLight(Light *pLight);

		void addObject(Object *pObject);
		Object *deleteObject(Object *pObject);

		int getCollisionGroupID(int ID, unsigned int mask);

		void addTerrain(Terrain *pTerrain);
		Terrain *deleteTerrain(Terrain *pTerrain);

		void clear();

		void setRenderer(Renderer *r);
		void update();
	};

	// make sure tke3_scene always vaild
	//extern Scene *scene;

	//struct LightSave : Transformer
	//{
	//	Light::Type type;
	//	glm::vec3 color;
	//	glm::vec3 decayFactor;
	//	bool shadow;

	//	LightSave(Light &light);
	//};

	//struct ObjectSave : Transformer
	//{
	//	Model *pModel;

	//	bool phyx;
	//	Object::MoveType moveType;
	//	Object::UpMethod upMethod;

	//	ObjectSave(Object &object);
	//};

	//struct TerrainSave : Transformer
	//{
	//	float size, height;

	//	Texture *heightMap;
	//	Texture *colorMap;
	//	float spec;
	//	float roughness;

	//	TerrainSave(Terrain &terrain);
	//};

	struct SceneSave
	{
		//Atmosphere atmosphere;
		//HDR hdr;
		//Ambient ambient;
		float fogThickness;

		//std::vector<LightSave> lightSaves;
		//std::vector<ObjectSave> objectSaves;
		//std::vector<TerrainSave> terrainSaves;

		int controlingID = -1;

		void push(Scene *pScene);

		void pull(Scene *pScene);
	};

	void loadScene(char *s);
}

#endif