#pragma once

#include <mutex>
#include <memory>

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
		enum class State
		{
			stand = 0,
			forward = 1 << 0,
			backward = 1 << 1,
			left = 1 << 2,
			right = 1 << 3,
			up = 1 << 4,
			down = 1 << 5,
			turn_left = 1 << 6,
			turn_right = 1 << 7,
			turn_up = 1 << 8,
			turn_down = 1 << 9
		};

		REFL_BANK;

		int lastTime = 0;
		REFLv float ang_offset = 0.f;
		REFLv float speed = 1.f;
		REFLv float turn_speed = 75.f;
		State state = State::stand;

		bool setState(State _s, bool enable);
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

		bool shadow;

		int sceneIndex = -1;
		int sceneShadowIndex = -1;

		Light(LightType _type, bool _shadow = false);
		void setColor(const glm::vec3 &);
	};

	struct Material
	{
		unsigned char albedoR = 255, albedoG = 255, albedoB = 255;
		unsigned char alpha = 255;
		unsigned char spec = 0;
		unsigned char roughness = 255;

		Image *albedoAlphaMap = nullptr;
		Image *normalHeightMap = nullptr;
		Image *specRoughnessMap = nullptr;

		int sceneIndex = -1;
	};

	struct Geometry
	{
		Material *material = nullptr;

		int indiceBase = 0; // offset of model base
		int indiceCount = 0;

		bool visible = true;
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
		std::vector<std::unique_ptr<BoneMotion>> motions;
	};

	struct BoneMotionTrack
	{
		int boneID;
		std::vector<BoneMotion*> motions;
	};

	struct AnimationBinding
	{
		Animation *animation;
		int frameTotal;
		std::vector<std::unique_ptr<BoneMotionTrack>> tracks;
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
		std::vector<std::unique_ptr<Shape>> shapes;

		Rigidbody();
		Rigidbody(RigidbodyType _type);
		void addShape(Shape *s);
		Shape *removeShape(Shape *s);
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

	REFLECTABLE struct Model
	{
		REFL_BANK;

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

		std::vector<std::unique_ptr<Geometry>> geometries;

		std::vector<Bone> bones;
		std::vector<IK> iks;

		std::vector<AnimationBinding*> animationBindings;
		AnimationBinding* standAnimation = nullptr;
		AnimationBinding* forwardAnimation = nullptr;
		AnimationBinding* backwardAnimation = nullptr;
		AnimationBinding* leftAnimation = nullptr;
		AnimationBinding* rightAnimation = nullptr;
		AnimationBinding* jumpAnimation = nullptr;

		REFLv std::string stand_animation_filename;
		REFLv std::string forward_animation_filename;
		REFLv std::string left_animation_filename;
		REFLv std::string right_animation_filename;
		REFLv std::string backward_animation_filename;
		REFLv std::string jump_animation_filename;

		std::vector<Rigidbody*> rigidbodies;

		std::vector<Joint*> joints;

		glm::vec3 maxCoord;
		glm::vec3 minCoord;

		glm::vec3 bounding_position;
		float bounding_size = 1.f;

		REFLv glm::vec3 controller_position;
		REFLv float controller_height = 1.f;
		REFLv float controller_radius = 0.5f;

		REFLv glm::vec3 eye_position;

		void loadData(bool needRigidbody);
		void saveData(bool needRigidbody);

		AnimationBinding *bindAnimation(Animation *a);
		void setStandAnimation(AnimationBinding *b);
		void setForwardAnimation(AnimationBinding *b);
		void setBackwardAnimation(AnimationBinding *b);
		void setLeftAnimation(AnimationBinding *b);
		void setRightAnimation(AnimationBinding *b);
		void setJumpAnimation(AnimationBinding *b);

		void addRigidbody(Rigidbody *pRigidbody);
		Rigidbody *deleteRigidbody(Rigidbody *pRigidbody);

		void addJoint(Joint *pJoint);
	};

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

		REFLv std::string name;

		REFLe ObjectPhysicsType physics_type = ObjectPhysicsType::null; // cannot change

		AnimationComponent *animationComponent = nullptr;
		std::vector<RigidBodyData> rigidbodyDatas;
		physx::PxController *pxController = nullptr;
		float floatingTime = 0.f;

		int sceneIndex = -1;

		Object();
		Object(Model *_model, ObjectPhysicsType _physicsType = ObjectPhysicsType::null);
		~Object();
		void setState(Controller::State _s, bool enable);
	};

	REFLECTABLE struct Terrain : Transformer
	{
		REFL_BANK;

		REFLv bool use_physx = false;

		REFLv std::string blend_map_filename;
		REFLv std::string height_map_filename;
		REFLv std::string color_map0_filename;
		REFLv std::string color_map1_filename;
		REFLv std::string color_map2_filename;
		REFLv std::string color_map3_filename;

		int blockCx = 64;
		float blockSize = 16.f;
		float height = 100.f;
		float tessellationFactor = 0.75f;
		float textureUvFactor = 8.f;

		Image *blendMap = nullptr;
		Image *heightMap = nullptr;
		Image *colorMaps[4] = {};
		float spec = 0.04f;
		float roughness = 1.f;

		physx::PxRigidActor *actor = nullptr;

		Terrain();
		Terrain(bool _use_physx, Image *_heightMap, Image *_blendMap, Image *_colorMap0, Image *_colorMap1, Image *_colorMap2, Image *_colorMap3);
	};

	REFLECTABLE struct Water : Transformer
	{
		REFL_BANK;

		int blockCx = 64;
		float blockSize = 16.f;
		float height = 10.f;
		float tessellationFactor = 0.75f;
		float textureUvFactor = 8.f;
	};

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

		LightShaderStruct lights[TKE_MAX_LIGHT_COUNT];
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

	extern Image *envrImageDownsample[3];

	extern RenderPass *sceneRenderPass;

	extern Pipeline *scatteringPipeline;
	extern Pipeline *downsamplePipeline;
	extern Pipeline *convolvePipeline;

	extern Pipeline *mrtPipeline;
	extern Pipeline *mrtAnimPipeline;
	extern Pipeline *terrainPipeline;
	extern Pipeline *waterPipeline;
	extern Pipeline *proceduralTerrainPipeline;
	extern Pipeline *deferredPipeline;
	extern Pipeline *esmPipeline;
	extern Pipeline *esmAnimPipeline;
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

		ResourceBank resource;

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

		std::unique_ptr<Framebuffer> fb_esm[TKE_MAX_SHADOW_COUNT * 6];

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
		void setAmbientColor(const glm::vec3 &);
		void setFogColor(const glm::vec3 &);
		Framebuffer *createFramebuffer(Image *dst);
		void show(Framebuffer *fb, VkEvent signalEvent);
		void loadSky(const char *skyMapFilename, int radianceMapCount, const char *radianceMapFilenames[], const char *irradianceMapFilename);
		void load(const std::string &filename);
		void save(const std::string &filename);
	};

	void initScene();
}
