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

	struct AnimationTemplate
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

	struct Animation
	{
		~Animation();
		AnimationTemplate *pTemplate;
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

		std::vector<Animation*> animations;
		Animation* animationStand = nullptr;
		Animation* animationForward = nullptr;
		Animation* animationLeft = nullptr;
		Animation* animationRight = nullptr;
		Animation* animationBackward = nullptr;
		Animation* animationJump = nullptr;

		std::vector<Rigidbody*> rigidbodies;

		std::vector<Joint*> joints;

		glm::vec3 controllerPosition;
		float controllerHeight = 1.f;
		float controllerRadius = 1.f;

		glm::vec3 boundingPosition;
		float boundingSize = 1.f;

		glm::vec3 eyePosition;
		glm::vec3 mainWeaponPosition;

		void createTangent();
		void loadDat(const std::string &filename);
		void saveDat(const std::string &filename);

		Image *getImage(const char *name);

		void arrangeBone();
		Animation *bindAnimation(AnimationTemplate *pAnimationTemplate);

		void refreshBone(BoneData *boneData, glm::mat4 *outMat);
		void refreshBone(int boneID, BoneData *boneData, glm::mat4 *outMat);

		void addRigidbody(Rigidbody *pRigidbody);
		Rigidbody *deleteRigidbody(Rigidbody *pRigidbody);

		void addJoint(Joint *pJoint);
	};
}

#endif