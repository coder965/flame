#ifndef __TKE_MODEL__
#define __TKE_MODEL__

#include <vector>
#include <string>
#include <fstream>

#include "transformer.h"
#include "render.h"

namespace tke
{
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

		int indiceBase = 0;
		int indiceCount = 0;

		bool visible = true;

		int sceneIndex = -1;

		void *ptr = nullptr;
	};

	struct Bone
	{
		std::string name;
		char type = -1;

		int parent = -1;

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

	enum ShapeType
	{
		ShapeTypeBox,
		ShapeTypeSphere,
		ShapeTypeCapsule, // scale: 0 - radius, scale 1 - length
		ShapeTypePlane,
		ShapeTypeConvexMesh,
		ShapeTypeTriangleMesh,
		ShapeTypeHeightField,

		ShapeTypeLast
	};

	struct Shape : Transformer
	{
		int id;

		ShapeType type = ShapeTypeBox;

		Shape();
		Shape(ShapeType _type);
		static char *getTypeName(Type _type);
		char *getTypeName();
		float getVolume() const;
	};

	enum RigidbodyType
	{
		RigidbodyTypeStatic,
		RigidbodyTypeDynamic,
		RigidbodyTypeDynamicButLocation // special for pmd/pmx 
	};

	struct Rigidbody : Transformer
	{
		int id;

		RigidbodyType type = RigidbodyTypeStatic;

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

		glm::vec3 eyePosition;

		void loadDat(const std::string &filename);
		void saveDat(const std::string &filename);

		Image *getImage(const char *name);

		AnimationBinding *bindAnimation(Animation *pAnimationTemplate);

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

		AnimationComponent(Model *_pModel);
		~AnimationComponent();
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
}

#endif