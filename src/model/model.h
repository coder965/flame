#pragma once

#include <memory>
#include <vector>

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
		std::string name;

		int indiceBase = 0; // offset of model base
		int indiceCount = 0;

		std::shared_ptr<Material> material;

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

	struct GeometryAux
	{
		struct Triangle
		{
			int indices[3];
			std::pair<int, int> adjacency[3]; // tri idx and vtx idx
			glm::vec2 bake_uv[3];
		};

		std::vector<glm::vec3> unique_vertex;
		std::unique_ptr<Triangle[]> triangles;
	};

	struct Model
	{
		std::string filename;
		std::string filepath;

		int vertexBase = 0;
		int indiceBase = 0;

		int vertex_count = 0;
		std::unique_ptr<ModelVertex[]> vertex;
		std::unique_ptr<ModelVertexSkeleton[]> vertex_skeleton;
		int indice_count = 0;
		std::unique_ptr<int[]> indices;

		std::unique_ptr<GeometryAux> geometry_aux;

		std::vector<std::unique_ptr<Geometry>> geometries;

		std::vector<std::unique_ptr<Bone>> bones;
		std::vector<std::unique_ptr<BoneIK>> iks;

		std::shared_ptr<AnimationBinding> stateAnimations[ModelStateAnimationCount];

		std::string stand_animation_filename;
		std::string forward_animation_filename;
		std::string leftward_animation_filename;
		std::string rightward_animation_filename;
		std::string backward_animation_filename;
		std::string jump_animation_filename;

		std::vector<std::unique_ptr<Rigidbody>> rigidbodies;
		std::vector<std::unique_ptr<Joint>> joints;

		glm::vec3 maxCoord = glm::vec3(0.f);
		glm::vec3 minCoord = glm::vec3(0.f);

		glm::vec3 bounding_position = glm::vec3(0.f);
		float bounding_size = 1.f;

		glm::vec3 controller_position = glm::vec3(0.f);
		float controller_height = 1.f;
		float controller_radius = 0.5f;

		glm::vec3 eye_position = glm::vec3(0.f);

		void create_geometry_aux();
		void create_uv();

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

	extern std::unique_ptr<VertexBuffer> vertexStatBuffer;
	extern std::unique_ptr<VertexBuffer> vertexAnimBuffer;
	extern std::unique_ptr<IndexBuffer> indexBuffer;

	void addTriangleVertex(std::vector<glm::vec3> &positions, std::vector<glm::vec3> &normals, std::vector<int> &indices, glm::mat3 rotation, glm::vec3 center);
	void addCubeVertex(std::vector<glm::vec3> &positions, std::vector<glm::vec3> &normals, std::vector<int> &indices, glm::mat3 rotation, glm::vec3 center, float length);
	void addSphereVertex(std::vector<glm::vec3> &positions, std::vector<glm::vec3> &normals, std::vector<int> &indices, glm::mat3 rotation, glm::vec3 center, float radius, int horiSubdiv, int vertSubdiv);
	void addCylinderVertex(std::vector<glm::vec3> &positions, std::vector<glm::vec3> &normals, std::vector<int> &indices, glm::mat3 rotation, glm::vec3 center, float halfHeight, float radius, int subdiv);
	void addConeVertex(std::vector<glm::vec3> &positions, std::vector<glm::vec3> &normals, std::vector<int> &indices, glm::mat3 rotation, glm::vec3 center, float height, float radius, int subdiv);
	void addTorusVertex(std::vector<glm::vec3> &positions, std::vector<glm::vec3> &normals, std::vector<int> &indices, glm::mat3 rotation, glm::vec3 center, float radius, float sectionRadius, int axisSubdiv, int heightSubdiv);

	extern std::shared_ptr<Model> triangleModel;
	extern std::shared_ptr<Model> cubeModel;
	extern std::shared_ptr<Model> sphereModel;
	extern std::shared_ptr<Model> cylinderModel;
	extern std::shared_ptr<Model> coneModel;
	extern std::shared_ptr<Model> arrowModel;
	extern std::shared_ptr<Model> torusModel;
	extern std::shared_ptr<Model> hamerModel;

	std::shared_ptr<Model> getModel(const std::string &filename);
	void saveModel(Model *m, const std::string &filename);

	void initModel();
}
