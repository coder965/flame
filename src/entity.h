#pragma once

#include <mutex>

#include "utils.h"
#include "math.h"
#include "define.h"
#include "render.h"
#include "physics.h"

namespace tke
{
	struct Object;

	REFLECTABLE struct Transformer
	{
		REFL_BANK;

		enum Axis
		{
			AxisNull = -1,
			AxisX,
			AxisY,
			AxisZ
		};

		REFLv glm::vec3 coord;
		REFLv glm::vec3 euler; // (yaw(y), pitch(z), roll(x))
		glm::vec4 quat = glm::vec4(0.f, 0.f, 0.f, 1.f);
		REFLv glm::vec3 scale = glm::vec3(1.f);
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

	REFLECTABLE struct Controller
	{
		REFL_BANK;

		int lastTime = 0;
		REFLv float ang_offset = 0.f;
		REFLv float speed = 1.f;
		REFLv float turn_speed = 75.f;
		bool forward = false, backward = false, left = false, right = false, up = false, down = false, turnLeft = false, turnRight = false, turnUp = false, turnDown = false;

		void reset();
		bool move(float inEulerX, glm::vec3 &outCoord, glm::vec3 &outEuler);
	};

	enum class CameraMode
	{
		free = 1 << 0,
		targeting = 1 << 1
	};

	struct Camera : Transformer, Controller
	{
		CameraMode mode = CameraMode::free;
		glm::vec3 target;
		float length = 1.f;

		glm::vec3 frustumPoints[8];
		glm::vec4 frustumPlanes[6];

		Object *object = nullptr;

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

	enum class LightType
	{
		parallax = 1 << 0,
		point = 1 << 1,
		spot = 1 << 2
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

	struct Material
	{
		std::string name;

		unsigned char albedoR = 255, albedoG = 255, albedoB = 255;
		unsigned char alpha = 255;
		unsigned char spec = 0;
		unsigned char roughness = 255;

		Image *albedoAlphaMap = nullptr;
		Image *normalHeightMap = nullptr;
		Image *specRoughnessMap = nullptr;

		int indiceBase = 0; // offset of model base
		int indiceCount = 0;

		bool visible = true;

		int sceneIndex = -1;

		void *ptr = nullptr;
	};

	struct Bone
	{
		std::string name;
		char type = -1;

		int parents = -1;

		glm::vec3 rootCoord;
		glm::vec3 relateCoord;

		std::vector<int> children;
	};

	struct BoneData
	{
		glm::mat3 rotation;
		glm::vec3 coord;
	};

	struct IK
	{
		int targetID = -1;
		int effectorID = -1;
		unsigned short iterations = 0;
		float weight = 0.f;
		std::vector<int> chain;
	};

	struct BoneMotion
	{
		std::string name;
		int frame = 0;
		glm::vec3 coord;
		glm::vec4 quaternion;
		char bezier[64] = {};
	};

	struct Animation
	{
		std::string name;
		std::string comment;
		std::string filename;
		std::string filepath;
		std::vector<BoneMotion> motions;
	};

	extern std::vector<Animation*> animations;

	struct BoneMotionTrack
	{
		int boneID;
		std::vector<BoneMotion*> pMotions;
	};

	struct AnimationBinding
	{
		~AnimationBinding();
		Animation *pTemplate;
		int frameTotal;
		std::vector<BoneMotionTrack*> pTracks;
	};

	struct CollisionGroup
	{
		int originalID;
		unsigned int originalmask;
	};

	enum class ShapeType
	{
		box = 1 << 0,
		sphere = 1 << 1,
		capsule = 1 << 2, // scale: 0 - radius, scale 1 - length
		plane = 1 << 3,
		convex_mesh = 1 << 4,
		triangle_mesh = 1 << 5,
		height_field = 1 << 6
	};

	std::string shapeTypeName(ShapeType t);

	struct Shape : Transformer
	{
		int id;

		ShapeType type = ShapeType::box;

		Shape();
		Shape(ShapeType _type);
		float getVolume() const;
	};

	enum class RigidbodyType
	{
		static_r = 1 << 0,
		dynamic = 1 << 1,
		dynamic_but_location = 1 << 2 // special for pmd/pmx 
	};

	struct Rigidbody : Transformer
	{
		int id;

		RigidbodyType type = RigidbodyType::static_r;

		std::string name;
		int boneID = -1;
		int originCollisionGroupID;
		int originCollisionFreeFlag;
		int collisionGroupID = -1;
		float density = 10.f;
		float velocityAttenuation;
		float rotationAttenuation;
		float bounce;
		float friction;
		std::vector<Shape*> shapes;

		Rigidbody();
		Rigidbody(RigidbodyType _type);
		~Rigidbody();
		void addShape(Shape *pShape);
		Shape *deleteShape(Shape *pRigidBody);
	};

	struct Joint : Transformer
	{
		int id;

		std::string name;
		int rigid0ID;
		int rigid1ID;
		glm::vec3 maxCoord;
		glm::vec3 minCoord;
		glm::vec3 maxRotation;
		glm::vec3 minRotation;
		glm::vec3 springConstant;
		glm::vec3 sprintRotationConstant;
	};

	struct Model
	{
		std::string name;
		std::string comment;
		std::string filename;
		std::string filepath;
		bool animated = false;

		int vertexBase = 0;
		int indiceBase = 0;

		std::vector<glm::vec3> positions;
		std::vector<glm::vec2> uvs;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec3> tangents;
		std::vector<glm::vec4> boneWeights;
		std::vector<glm::vec4> boneIDs;
		std::vector<int> indices;

		std::vector<Material*> materials;

		std::vector<Image*> pImages;

		std::vector<Bone> bones;
		std::vector<IK> iks;

		std::vector<AnimationBinding*> animations;
		AnimationBinding* animationStand = nullptr;
		AnimationBinding* animationForward = nullptr;
		AnimationBinding* animationLeft = nullptr;
		AnimationBinding* animationRight = nullptr;
		AnimationBinding* animationBackward = nullptr;
		AnimationBinding* animationJump = nullptr;

		std::vector<Rigidbody*> rigidbodies;

		std::vector<Joint*> joints;

		glm::vec3 maxCoord;
		glm::vec3 minCoord;

		glm::vec3 boundingPosition;
		float boundingSize = 1.f;

		glm::vec3 controllerPosition;
		float controllerHeight = 1.f;
		float controllerRadius = 0.5f;

		glm::vec3 eyePosition;

		void loadData(bool needRigidbody);
		void saveData(bool needRigidbody);

		Image *getImage(const char *name);

		AnimationBinding *bindAnimation(Animation *pAnimationTemplate);

		void addRigidbody(Rigidbody *pRigidbody);
		Rigidbody *deleteRigidbody(Rigidbody *pRigidbody);

		void addJoint(Joint *pJoint);
	};

	extern std::vector<Model*> models;
	inline Model *getModel(const std::string &filename)
	{
		for (auto m : models)
		{
			if (m->filename == filename)
				return m;
		}
		return nullptr;
	}
	void clearModel();

	struct AnimationComponent
	{
		Model *model;
		AnimationBinding *currentAnimation = nullptr;
		float currentFrame = 0.f;
		float currentTime = 0.f;
		BoneData *boneData = nullptr;
		glm::mat4 *boneMatrix = nullptr;
		UniformBuffer *boneMatrixBuffer = nullptr;
		bool processedIK = true;

		AnimationComponent(Model *_model);
		~AnimationComponent();
		void refreshBone();
		void refreshBone(int i);
		void setAnimation(AnimationBinding *animation);
		void update();
	};

	void addTriangleVertex(Model *m, glm::mat3 rotation, glm::vec3 center);
	void addCubeVertex(Model *m, glm::mat3 rotation, glm::vec3 center, float length);
	void addSphereVertex(Model *m, glm::mat3 rotation, glm::vec3 center, float radius, int horiSubdiv, int vertSubdiv);
	void addCylinderVertex(Model *m, glm::mat3 rotation, glm::vec3 center, float halfHeight, float radius, int subdiv);
	void addConeVertex(Model *m, glm::mat3 rotation, glm::vec3 center, float height, float radius, int subdiv);
	void addTorusVertex(Model *m, glm::mat3 rotation, glm::vec3 center, float radius, float sectionRadius, int axisSubdiv, int heightSubdiv);

	extern Model *triangleModel;
	extern Model *cubeModel;
	extern Model *sphereModel;
	extern Model *cylinderModel;
	extern Model *coneModel;
	extern Model *arrowModel;
	extern Model *torusModel;
	extern Model *hamerModel;

	void initGeneralModels();

	namespace OBJ
	{
		void load(Model *m, std::ifstream &file);
	}

	namespace PMD
	{
		void load(Model *m, std::ifstream &file);
	}

	namespace VMD
	{

		void load(Animation *a, std::ifstream &file);
	}

	namespace TKM
	{
		void load(Model *m, std::ifstream &file);
		void save(Model *, const std::string &filename, bool copyTexture);
	}

	namespace TKA
	{
		void load(Animation *a, std::ifstream &file);
		void save(Animation *a, const std::string &filename);
	}

	Model *createModel(const std::string &filename);

	Animation *createAnimation(const std::string &filename);

	REFLECTABLE enum class ObjectPhysicsType
	{
		REFLe null,
		REFLe static_r = 1 << 0, // cannot use with dynamic bit
		REFLe dynamic = 1 << 1, // cannot use with static bit
		REFLe controller = 1 << 2
	};

	struct RigidBodyData
	{
		Rigidbody *rigidbody;
		physx::PxRigidActor *actor = nullptr;
		glm::mat3 rotation;
		glm::vec3 coord;
	};

	REFLECTABLE struct Object : Transformer, Controller, Observed
	{
		REFL_BANK;

		REFLv std::string model_filename;
		Model *model = nullptr;

		REFLe ObjectPhysicsType physics_type = ObjectPhysicsType::null; // cannot change

		AnimationComponent *animationComponent = nullptr;
		std::vector<RigidBodyData> rigidbodyDatas;
		physx::PxController *pxController = nullptr;
		float floatingTime = 0.f;

		int sceneIndex = -1;

		Object();
		Object(Model *_model, ObjectPhysicsType _physicsType = ObjectPhysicsType::null);
		~Object();
	};

	REFLECTABLE enum class TerrainType
	{
		REFLe height_map = 1 << 0,
		REFLe procedural = 1 << 1
	};

	REFLECTABLE struct Terrain : Transformer
	{
		REFL_BANK;

		REFLe TerrainType type = TerrainType::height_map;
		REFLv bool use_physx = false;

		REFLv std::string height_map_filename;
		REFLv std::string color_map_filename;

		float ext = 16.f;
		float height = 100.f;
		float tessFactor = 0.75f;

		Image *heightMap = nullptr;
		Image *colorMaps[4] = {};
		float spec = 0.04f;
		float roughness = 1.f;

		physx::PxRigidActor *actor = nullptr;

		Terrain();
		Terrain(TerrainType _type, bool _use_physx, Image *_heightMap, Image *_colorMap);
	};

	enum class SkyType
	{
		null,
		atmosphere_scattering = 1 << 0,
		panorama = 1 << 1
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

	extern Image *envrImageDownsample[3];

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

	REFLECTABLE struct Scene
	{
		REFL_BANK;

		std::mutex mtx;

		REFLv std::string name;
		std::string filename;
		std::string filepath;

		SkyType skyType = SkyType::atmosphere_scattering;
		glm::vec2 atmosphereSunDir = glm::vec2(0.f, -90.f);
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

		ResourceBank resource;

		Image *envrImage;

		Image *mainImage;
		Image *albedoAlphaImage;
		Image *normalHeightImage;
		Image *specRoughnessImage;

		UniformBuffer *matrixBuffer;
		UniformBuffer *staticObjectMatrixBuffer;
		UniformBuffer *animatedObjectMatrixBuffer;
		IndirectIndexBuffer *staticObjectIndirectBuffer;
		IndirectIndexBuffer *animatedObjectIndirectBuffer;
		UniformBuffer *heightMapTerrainBuffer;
		UniformBuffer *proceduralTerrainBuffer;
		UniformBuffer *lightBuffer;
		UniformBuffer *ambientBuffer;

		DescriptorSet *ds_pano;
		DescriptorSet *ds_mrt;
		DescriptorSet *ds_mrtAnim;
		DescriptorSet *ds_heightMapTerrain;
		DescriptorSet *ds_defe;
		DescriptorSet *ds_comp;

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
