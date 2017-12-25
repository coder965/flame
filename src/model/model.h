#pragma once

#include "../refl.h"
#include "../render/buffer.h"
#include "../render/descriptor.h"
#include "../physics/physics.h"
#include "material.h"
#include "animation.h"

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

	struct Vertex
	{
		glm::vec3 position;
		glm::vec2 uv;
		glm::vec3 normal;
		glm::vec3 tangent;
	};

	struct VertexSkeleton
	{
		glm::vec4 bone_weight;
		glm::vec4 bone_ID;
	};

	REFLECTABLE struct Model
	{
		REFL_BANK;

		std::string filename;
		std::string filepath;

		int vertexBase = 0;
		int indiceBase = 0;

		int vertex_count = 0;
		std::unique_ptr<Vertex[]> vertex;
		std::unique_ptr<VertexSkeleton[]> vertex_skeleton;
		int indice_count = 0;
		std::unique_ptr<int[]> indices;

		std::vector<std::unique_ptr<Geometry>> geometries;

		std::vector<Bone> bones;
		std::vector<BoneIK> iks;

		std::vector<std::weak_ptr<AnimationBinding>> animation_bindings;
		std::shared_ptr<AnimationBinding> stateAnimations[ModelStateAnimationCount];

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

		std::shared_ptr<AnimationBinding> bindAnimation(std::shared_ptr<Animation> a);
		void setStateAnimation(ModelStateAnimationKind kind, std::shared_ptr<AnimationBinding> b);

		void addRigidbody(Rigidbody *pRigidbody);
		Rigidbody *deleteRigidbody(Rigidbody *pRigidbody);

		void addJoint(Joint *pJoint);
	};

	IMPL() VkPipelineVertexInputStateCreateInfo vertexStatInputState;
	IMPL() VkPipelineVertexInputStateCreateInfo vertexAnimInputState;

	IMPL() std::unique_ptr<VertexBuffer> vertexStatBuffer;
	IMPL() std::unique_ptr<VertexBuffer> vertexAnimBuffer;
	IMPL() std::unique_ptr<IndexBuffer> indexBuffer;

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

	void addTriangleVertex(std::vector<glm::vec3> &positions, std::vector<glm::vec3> &normals, std::vector<int> &indices, glm::mat3 rotation, glm::vec3 center);
	void addCubeVertex(std::vector<glm::vec3> &positions, std::vector<glm::vec3> &normals, std::vector<int> &indices, glm::mat3 rotation, glm::vec3 center, float length);
	void addSphereVertex(std::vector<glm::vec3> &positions, std::vector<glm::vec3> &normals, std::vector<int> &indices, glm::mat3 rotation, glm::vec3 center, float radius, int horiSubdiv, int vertSubdiv);
	void addCylinderVertex(std::vector<glm::vec3> &positions, std::vector<glm::vec3> &normals, std::vector<int> &indices, glm::mat3 rotation, glm::vec3 center, float halfHeight, float radius, int subdiv);
	void addConeVertex(std::vector<glm::vec3> &positions, std::vector<glm::vec3> &normals, std::vector<int> &indices, glm::mat3 rotation, glm::vec3 center, float height, float radius, int subdiv);
	void addTorusVertex(std::vector<glm::vec3> &positions, std::vector<glm::vec3> &normals, std::vector<int> &indices, glm::mat3 rotation, glm::vec3 center, float radius, float sectionRadius, int axisSubdiv, int heightSubdiv);

	IMPL() std::shared_ptr<Model> triangleModel;
	IMPL() std::shared_ptr<Model> cubeModel;
	IMPL() std::shared_ptr<Model> sphereModel;
	IMPL() std::shared_ptr<Model> cylinderModel;
	IMPL() std::shared_ptr<Model> coneModel;
	IMPL() std::shared_ptr<Model> arrowModel;
	IMPL() std::shared_ptr<Model> torusModel;
	IMPL() std::shared_ptr<Model> hamerModel;

	std::shared_ptr<Model> getModel(const std::string &filename);
	void saveModel(Model *m, const std::string &filename);
	void initModel();
}
