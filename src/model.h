#pragma once

#include "utils.h"
#include "math.h"
#include "material.h"
#include "animation.h"
#include "physics.h"

namespace tke
{
	enum ModelStateAnimationKind
	{
		ModelStateAnimationStand,
		ModelStateAnimationForward,
		ModelStateAnimationBackward,
		ModelStateAnimationLeftward,
		ModelStateAnimationRightward,
		ModelStateAnimationJump,

		ModelStateAnimationCount
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
		std::vector<BoneIK> iks;

		std::vector<AnimationBinding*> animationBindings; 
		AnimationBinding *stateAnimations[ModelStateAnimationCount] = {};

		REFLv std::string stand_animation_filename;
		REFLv std::string forward_animation_filename;
		REFLv std::string leftward_animation_filename;
		REFLv std::string rightward_animation_filename;
		REFLv std::string backward_animation_filename;
		REFLv std::string jump_animation_filename;

		std::vector<Rigidbody*> rigidbodies;

		std::vector<Joint*> joints;

		glm::vec3 maxCoord = glm::vec3(0.f);
		glm::vec3 minCoord = glm::vec3(0.f);

		glm::vec3 bounding_position = glm::vec3(0.f);
		float bounding_size = 1.f;

		REFLv glm::vec3 controller_position = glm::vec3(0.f);
		REFLv float controller_height = 1.f;
		REFLv float controller_radius = 0.5f;

		REFLv glm::vec3 eye_position = glm::vec3(0.f);

		void loadData(bool needRigidbody);
		void saveData(bool needRigidbody);

		AnimationBinding *bindAnimation(Animation *a);
		void setStateAnimation(ModelStateAnimationKind kind, AnimationBinding *b);

		void addRigidbody(Rigidbody *pRigidbody);
		Rigidbody *deleteRigidbody(Rigidbody *pRigidbody);

		void addJoint(Joint *pJoint);
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
}
