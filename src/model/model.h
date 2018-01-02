#pragma once

#include <memory>

#include "../refl.h"
#include "../math/math.h"
#include "../graphics/graphics.h"

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

	enum { MaxBoneCount = 256 };

	struct ModelVertex
	{
		glm::vec3 position;
		glm::vec2 uv;
		glm::vec3 normal;
		glm::vec3 tangent;
	};

	struct ModelVertexSkeleton
	{
		glm::vec4 bone_weight;
		glm::vec4 bone_ID;
	};

	struct Material;
	struct VertexBuffer;
	struct IndexBuffer;
	struct Animation;
	struct AnimationBinding;
	struct Rigidbody;
	struct Joint;

	struct Geometry
	{
		std::shared_ptr<Material> material;

		int indiceBase = 0; // offset of model base
		int indiceCount = 0;

		bool visible = true;
	};

	struct Bone
	{
		std::string name;
		char type = -1;

		int parent = -1;

		glm::vec3 rootCoord = glm::vec3(0.f);
		glm::vec3 relateCoord = glm::vec3(0.f);

		std::vector<int> children;
	};

	struct BoneIK
	{
		int targetID = -1;
		int effectorID = -1;
		unsigned short iterations = 0;
		float weight = 0.f;
		std::vector<int> chain;
	};

	REFLECTABLE struct Model
	{
		REFL_BANK;

		std::string filename;
		std::string filepath;

		int vertexBase = 0;
		int indiceBase = 0;

		int vertex_count = 0;
		std::unique_ptr<ModelVertex[]> vertex;
		std::unique_ptr<ModelVertexSkeleton[]> vertex_skeleton;
		int indice_count = 0;
		std::unique_ptr<int[]> indices;

		std::vector<std::unique_ptr<Geometry>> geometries;

		std::vector<std::unique_ptr<Bone>> bones;
		std::vector<std::unique_ptr<BoneIK>> iks;

		std::shared_ptr<AnimationBinding> stateAnimations[ModelStateAnimationCount];

		REFLv std::string stand_animation_filename;
		REFLv std::string forward_animation_filename;
		REFLv std::string leftward_animation_filename;
		REFLv std::string rightward_animation_filename;
		REFLv std::string backward_animation_filename;
		REFLv std::string jump_animation_filename;

		std::vector<std::unique_ptr<Rigidbody>> rigidbodies;
		std::vector<std::unique_ptr<Joint>> joints;

		glm::vec3 maxCoord = glm::vec3(0.f);
		glm::vec3 minCoord = glm::vec3(0.f);

		glm::vec3 bounding_position = glm::vec3(0.f);
		float bounding_size = 1.f;

		REFLv glm::vec3 controller_position = glm::vec3(0.f);
		REFLv float controller_height = 1.f;
		REFLv float controller_radius = 0.5f;

		REFLv glm::vec3 eye_position = glm::vec3(0.f);

		void setStateAnimation(ModelStateAnimationKind kind, std::shared_ptr<AnimationBinding> b);

		Bone *new_bone();
		void remove_bone(Bone *b);

		BoneIK *new_bone_ik();
		void remove_bone_ik(BoneIK *b);

		Rigidbody *new_rigidbody();
		void remove_rigidbody(Rigidbody *r);

		Joint *new_joint();
		void remove_joint(Joint *j);
	};

	IMPL() VkPipelineVertexInputStateCreateInfo vertexStatInputState;
	IMPL() VkPipelineVertexInputStateCreateInfo vertexAnimInputState;

	IMPL() std::unique_ptr<VertexBuffer> vertexStatBuffer;
	IMPL() std::unique_ptr<VertexBuffer> vertexAnimBuffer;
	IMPL() std::unique_ptr<IndexBuffer> indexBuffer;

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
