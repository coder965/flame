#pragma once

#include "../utils.h"
#include "../render/descriptor.h"
#include "material.h"
#include "animation.h"
#include "../physics/physics.h"

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

	enum { MaxMaterialCount = 256 };
	enum { MaxTextureCount = 256 };
	enum { MaxBoneCount = 256 };

	REFLECTABLE struct Model
	{
		REFL_BANK;

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

	IMPL(nullptr) Model *triangleModel;
	IMPL(nullptr) Model *cubeModel;
	IMPL(nullptr) Model *sphereModel;
	IMPL(nullptr) Model *cylinderModel;
	IMPL(nullptr) Model *coneModel;
	IMPL(nullptr) Model *arrowModel;
	IMPL(nullptr) Model *torusModel;
	IMPL(nullptr) Model *hamerModel;

	extern std::vector<Model*> generalModels;

	IMPL() std::weak_ptr<Material> modelMaterials[MaxMaterialCount];
	IMPL(nullptr) std::shared_ptr<Material> defaultMaterial;
	std::shared_ptr<Material> getModelMaterial(unsigned char albedoR, unsigned char albedoG, unsigned char albedoB,
		unsigned char alpha, unsigned char spec, unsigned char roughness, 
		std::shared_ptr<Image> albedoAlphaMap, std::shared_ptr<Image> normalHeightMap, std::shared_ptr<Image> specRoughnessMap);
	std::shared_ptr<Material> getModelMaterial(const std::string name);
	IMPL(nullptr) UniformBuffer *materialBuffer;

	IMPL() std::weak_ptr<Image> modelTextures[MaxTextureCount];
	std::shared_ptr<Image> getModelTexture(const std::string &filename, bool sRGB = false);
	IMPL(nullptr) DescriptorSet *ds_textures;

	void initModel();
	Model *createModel(const std::string &filename);
}
