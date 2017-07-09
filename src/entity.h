#pragma once

#include <mutex>

#include "utils.h"
#include "math.h"
#include "define.h"
#include "render.h"
#include "camera.h"
#include "transformer.h"
#include "controler.h"
#include "model.h"
#include "physics.h"

namespace tke
{
	struct Transformer : ObservedObject
	{
		enum class Axis
		{
			eNull = -1,
			eX,
			eY,
			eZ
		};

		glm::vec3 coord;
		glm::vec3 euler; // (yaw(y), pitch(z), roll(x))
		glm::vec4 quat = glm::vec4(0.f, 0.f, 0.f, 1.f);
		glm::vec3 scale = glm::vec3(1.f);
		glm::vec3 worldScale = glm::vec3(1.f);

		glm::mat3 axis;
		glm::mat4 mat;
		glm::mat4 matInv;

		bool needUpdateAxis = false;
		bool needUpdateEuler = false;
		bool needUpdateQuat = false;
		bool needUpdateMat = false;

		bool changed = true;

		Transformer();
		Transformer(glm::mat3 &rotation, glm::vec3 coord);

		glm::vec3 getCoord() const;
		glm::vec3 getEuler();
		glm::vec4 getQuat();
		glm::vec3 getScale() const;
		glm::vec3 getWorldScale() const;

		glm::mat3 getAxis();
		glm::mat4 getMat();
		glm::mat4 getMatInv();

		void setCoord(const glm::vec3 &_coord);
		inline void setCoord(float x, float y, float z) { setCoord(glm::vec3(x, y, z)); }
		inline void setCoord(float v) { setCoord(glm::vec3(v)); }
		inline void setCoordX(float v) { auto p = getCoord();  setCoord(glm::vec3(v, p.y, p.z)); }
		inline void setCoordY(float v) { auto p = getCoord();  setCoord(glm::vec3(p.x, v, p.z)); }
		inline void setCoordZ(float v) { auto p = getCoord();  setCoord(glm::vec3(p.x, p.y, v)); }
		void addCoord(const glm::vec3 &_coord);
		inline void addCoord(float x, float y, float z) { addCoord(glm::vec3(x, y, z)); }
		inline void addCoord(float v) { addCoord(glm::vec3(v)); }
		inline void addCoordX(float v) { addCoord(glm::vec3(v, 0.f, 0.f)); }
		inline void addCoordY(float v) { addCoord(glm::vec3(0.f, v, 0.f)); }
		inline void addCoordZ(float v) { addCoord(glm::vec3(0.f, 0.f, v)); }
		void setEuler(const glm::vec3 &_euler);
		inline void setEuler(float x, float y, float z) { setEuler(glm::vec3(x, y, z)); }
		inline void setEuler(float v) { setEuler(glm::vec3(v)); }
		inline void setEulerX(float v) { auto p = getEuler();  setCoord(glm::vec3(v, p.y, p.z)); }
		inline void setEulerY(float v) { auto p = getEuler();  setCoord(glm::vec3(p.x, v, p.z)); }
		inline void setEulerZ(float v) { auto p = getEuler();  setCoord(glm::vec3(p.x, p.y, v)); }
		void addEuler(const glm::vec3 &_euler);
		inline void addEuler(float x, float y, float z) { addEuler(glm::vec3(x, y, z)); }
		inline void addEuler(float v) { addEuler(glm::vec3(v)); }
		inline void addEulerX(float v) { addEuler(glm::vec3(v, 0.f, 0.f)); }
		inline void addEulerY(float v) { addEuler(glm::vec3(0.f, v, 0.f)); }
		inline void addEulerZ(float v) { addEuler(glm::vec3(0.f, 0.f, v)); }
		void setQuat(const glm::vec4 &_quat);
		inline void setQuat(float x, float y, float z, float w) { setQuat(glm::vec4(x, y, z, w)); }
		void leftRotate(const glm::mat3 &left);
		void rightRotate(const glm::mat3 &right);
		void axisRotate(Axis which, float angle);
		void setScale(const glm::vec3 &_scale);
		inline void setScale(float x, float y, float z) { setScale(glm::vec3(x, y, z)); }
		inline void setScale(float v) { setScale(glm::vec3(v)); }
		inline void setScaleX(float v) { auto p = getScale();  setScale(glm::vec3(v, p.y, p.z)); }
		inline void setScaleY(float v) { auto p = getScale();  setScale(glm::vec3(p.x, v, p.z)); }
		inline void setScaleZ(float v) { auto p = getScale();  setScale(glm::vec3(p.x, p.y, v)); }
		void addScale(const glm::vec3 &_scale);
		inline void addScale(float x, float y, float z) { addScale(glm::vec3(x, y, z)); }
		inline void addScale(float v) { addScale(glm::vec3(v)); }
		inline void addScaleX(float v) { addScale(glm::vec3(v, 0.f, 0.f)); }
		inline void addScaleY(float v) { addScale(glm::vec3(0.f, v, 0.f)); }
		inline void addScaleZ(float v) { addScale(glm::vec3(0.f, 0.f, v)); }
		void setWorldScale(const glm::vec3 &_scale);
		inline void setWorldScale(float x, float y, float z) { setWorldScale(glm::vec3(x, y, z)); }
		inline void setWorldScale(float v) { setWorldScale(glm::vec3(v)); }

		void relate(Transformer *t);
		void scaleRelate(Transformer *t);

		void updateMat();

		void updateAxis();
		void updateEuler();
		void updateQuat();
	};

	struct Controller
	{
		int lastTime = 0;
		float baseForwardAng = 0.f;
		float speed = 1.f;
		float turnSpeed = 75.f;
		bool front = false, back = false, left = false, right = false, up = false, down = false, turnLeft = false, turnRight = false, turnUp = false, turnDown = false;

		void reset();
		bool move(float inEulerX, glm::vec3 &outCoord, glm::vec3 &outEuler);
		bool keyDown(int key);
		bool keyUp(int key);
	};

	enum CameraMode
	{
		CameraModeFree,
		CameraModeTargeting
	};

	struct Camera : Transformer, Controller
	{
		CameraMode mode = CameraModeFree;
		glm::vec3 target;
		float length = 1.f;

		glm::vec3 frustumPoints[8];
		glm::vec4 frustumPlanes[6];

		Camera();
		void setMode(CameraMode _mode);
		void setLength(float _length);
		void setTarget(const glm::vec3 &_target);
		void lookAtTarget();
		void updateFrustum();
		void reset();
		void rotateByCursor(float x, float y);
		void moveByCursor(float x, float y);
		void scroll(float value);
		void move();
	};

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