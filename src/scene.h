#ifndef __TKE_SCENE__
#define __TKE_SCENE__

#include <vector>

#include "render.h"
#include "camera.h"
#include "light.h"
#include "object.h"
#include "terrain.h"
#include "model.h"

#define TKE_MAX_OBJECT_COUNT (1024)
#define TKE_MAX_LIGHT_COUNT (1024)
#define TKE_MAX_TEXTURE_COUNT (1024)
#define TKE_MAX_MATERIAL_COUNT (1024)
#define TKE_MAX_INDIRECT_COUNT (1024)

namespace tke
{
	struct Window;
	//struct MasterRenderer
	//{
	//	Image originalImage;
	//	Image albedoSpecImage;
	//	Image normalRoughnessImage;
	//	Image miscImage;

	//	Pipeline panoramaPipeline;
	//	Pipeline heightMapTerrainPipeline;
	//	Pipeline proceduralTerrainPipeline;
	//	Pipeline mrtPipeline;
	//	Pipeline deferredPipeline;
	//	Pipeline combinePipeline;

	//	Drawcall *mrtObjectDrawcall;

	//	DrawAction *skyAction;
	//	DrawAction *mrtHeightMapTerrainAction;

	//	RenderPass *miscPass;

	//	Renderer *renderer;

	//	MasterRenderer(int _cx, int _cy, Window *pWindow, VertexBuffer *vertexBuffer, IndexBuffer *indexBuffer, IndexedIndirectBuffer *indirectBuffer);
	//};

	struct AnimationTemplate;
	struct CollisionGroup;
	enum class SkyType
	{
		eNull,
		eAtmosphereScattering,
		ePanorama
	};

	struct Atmosphere
	{
		glm::vec2 sunDir = glm::vec2(0.f, -90.f);
		float eSun = 20.f;
		float innerRadius = 10.f; // The inner (planetary) radius
		float outerRadius = 10.25f; // The outer (atmosphere) radius
		float cameraHeight = 10.0002f;
		float km = 0.0025f;
		float kr = 0.001f;

		void set();
	};

	struct HDR
	{
		float exposure = 0.01f;
		float white = 1.f;

		void set();
	};

	struct Ambient
	{
		glm::vec3 color = glm::vec3(0.5f);

		float ssaoRadius = 10.f;
		float ssaoBias = 0.01f;
		float ssaoIntensity = 100000.f;

		void set();
	};

	struct MatrixUniformBufferStruct
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

	struct MaterialUniformBufferStruct
	{
		unsigned int albedoSpecCompress;
		unsigned int roughnessAlphaCompress;

		unsigned int mapIndex;

		unsigned int dummy;
	};

	struct LightStruct
	{
		glm::vec4 coord;
		glm::vec4 color;
		glm::vec4 decayFactor;
	};

	struct LightBufferStruct
	{
		unsigned int count;
		unsigned int dummy0;
		unsigned int dummy1;
		unsigned int dummy2;

		LightStruct lights[TKE_MAX_LIGHT_COUNT];
	};

	struct HeightMapTerrainBufferStruct
	{
		unsigned int patchSize;
		float ext;
		float height;
		float tessFactor;
		float mapDim;
	};

	struct AmbientStruct
	{
		glm::vec4 v;
		glm::vec4 fogcolor;
	};

	struct Scene
	{
		CRITICAL_SECTION cs;

		char name[50];
		char filePath[50];

		SkyType skyType = SkyType::eAtmosphereScattering;
		Atmosphere atmosphere;
		char skyName[50];
		Image *skyImage = nullptr;

		HDR hdr;

		Ambient ambient;

		float fogThickness = 0.01f;

		Camera camera;

		std::vector<Model*> pModels;

		std::vector<AnimationTemplate*> pAnimTemps;

		std::vector<Light*> pLights;
		std::vector<Light*> pParallaxLights;
		std::vector<Light*> pPointLights;

		Light *pSunLight = nullptr;

		std::vector<Object*> pObjects;
		std::vector<Object*> pStaticObjects;
		std::vector<Object*> pAnimatedObjects;

		std::vector<Terrain*> pTerrains;

		int vertexBase = 0;

		bool needUpdataSky = true;
		bool needUpdateVertexBuffer = true;
		bool needUpdateMaterialBuffer = true;
		bool needUpdateSampler = true;
		bool needUpdateIndirectBuffer = true;
		bool lightCountChanged = true;

		std::vector<Image*> storeImages;
		int getStoreImageIndex(Image *pImage);
		std::vector<MaterialUniformBufferStruct> storeMaterials;

		VertexBuffer vertexBuffer;
		IndexBuffer indexBuffer;

		UniformBuffer matrixBuffer;
		UniformBuffer objectMatrixBuffer;
		UniformBuffer lightMatrixBuffer;
		UniformBuffer materialBuffer;
		IndexedIndirectBuffer objectIndirectBuffer;
		UniformBuffer heightMapTerrainBuffer;
		UniformBuffer proceduralTerrainBuffer;
		UniformBuffer lightBuffer;
		UniformBuffer ambientBuffer;

		std::vector<CollisionGroup*> pCollisionGroups;

		unsigned int shadowCount = 0;
		unsigned int drawCallCount = 0;

		Scene();
		~Scene();

		void setUp();

		void loadSky(const char *skyMapFilename, int radianceMapCount, const char *radianceMapFilenames[], const char *irradianceMapFilename);
		void load(char *file);
		void save(char *file);

		void addModel(Model *pModel);
		Model *getModel(char *name);
		void clearModel();

		Rigidbody *getRigidbody(int id);
		Shape *getShape(int id);
		Joint *getJoint(int id);
		Light *getLight(int id);
		Object *getObject(int id);
		Terrain *getTerrain(int id);

		void addLight(Light *pLight, int id);
		void addLight(Light *pLight);
		Light *deleteLight(Light *pLight);

		void addObject(Object *pObject, int id);
		void addObject(Object *pObject);
		Object *deleteObject(Object *pObject);

		int getCollisionGroupID(int ID, unsigned int mask);

		void addTerrain(Terrain *pTerrain, int id);
		void addTerrain(Terrain *pTerrain);
		Terrain *deleteTerrain(Terrain *pTerrain);

		void clearActors();

		void setResources(Renderer *r);
		void update();

		void showWater();
		void updateAnimation();
		void updateAnimationUBO();
	};

	// make sure tke3_scene always vaild
	extern Scene *scene;

	struct LightSave : Transformer
	{
		Light::Type type;
		glm::vec3 color;
		glm::vec3 decayFactor;
		bool shadow;

		LightSave(Light &light);
	};

	struct ObjectSave : Transformer
	{
		Model *pModel;

		bool phyx;
		Object::MoveType moveType;
		Object::UpMethod upMethod;

		ObjectSave(Object &object);
	};

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
		Atmosphere atmosphere;
		HDR hdr;
		Ambient ambient;
		float fogThickness;

		std::vector<LightSave> lightSaves;
		std::vector<ObjectSave> objectSaves;
		//std::vector<TerrainSave> terrainSaves;

		int controlingID = -1;

		void push(Scene *pScene);

		void pull(Scene *pScene);
	};

	void loadScene(char *s);
}

#endif