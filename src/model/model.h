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

		bool operator==(const ModelVertex &right);
	};

	struct ModelVertexSkeleton
	{
		glm::vec4 bone_weight;
		glm::vec4 bone_ID;

		bool operator==(const ModelVertexSkeleton &right);
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
		};

		std::vector<glm::vec3> unique_vertex;
		std::unique_ptr<Triangle[]> triangles;
	};

	struct Model
	{
		struct UV
		{
			std::string name;
			std::vector<glm::vec2> unique;
			std::vector<int> indices;
			std::vector<std::pair<int, int>> series;

			void add(const glm::vec2 &v);
		};

		std::string filename;
		std::string filepath;

		int vertex_base = 0;
		int indice_base = 0;

		std::vector<ModelVertex> vertexes;
		std::vector<ModelVertexSkeleton> vertexes_skeleton;
		std::vector<int> indices;

		std::vector<std::unique_ptr<Geometry>> geometries;
		std::vector<std::unique_ptr<UV>> uvs;
		UV *geometry_uv = nullptr;
		UV *bake_uv = nullptr;
		std::unique_ptr<GeometryAux> geometry_aux;

		int bake_grid_pixel_size = 4;
		int bake_image_cx = 256;
		int bake_image_cy = 256;

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

		glm::vec3 max_coord = glm::vec3(0.f);
		glm::vec3 min_coord = glm::vec3(0.f);

		glm::vec3 bounding_position = glm::vec3(0.f);
		float bounding_size = 1.f;

		glm::vec3 controller_position = glm::vec3(0.f);
		float controller_height = 1.f;
		float controller_radius = 0.5f;

		glm::vec3 eye_position = glm::vec3(0.f);

		void add_vertex_position_normal(const glm::vec3 &position, const glm::vec3 &normal);

		const char *get_uv_use_name(UV *uv) const;
		void create_geometry_aux();
		void create_uv();
		void remove_uv(UV *uv);
		void assign_uv_to_geometry(UV *uv);
		void assign_uv_to_bake(UV *uv);

		void set_state_animation(ModelStateAnimationKind kind, std::shared_ptr<AnimationBinding> b);

		Bone *new_bone();
		void remove_bone(Bone *b);

		BoneIK *new_bone_ik();
		void remove_bone_ik(BoneIK *b);

		Rigidbody *new_rigidbody();
		void remove_rigidbody(Rigidbody *r);

		Joint *new_joint();
		void remove_joint(Joint *j);
	};

	extern std::unique_ptr<VertexBuffer> vertex_static_buffer;
	extern std::unique_ptr<VertexBuffer> vertex_skeleton_Buffer;
	extern std::unique_ptr<IndexBuffer> index_buffer;

	void add_triangle_vertex(Model *m, glm::mat3 rotation, glm::vec3 center);
	void add_cube_vertex(Model *m, glm::mat3 rotation, glm::vec3 center, float length);
	void add_sphere_vertex(Model *m, glm::mat3 rotation, glm::vec3 center, float radius, int horiSubdiv, 
		int vertSubdiv);
	void add_cylinder_vertex(Model *m, glm::mat3 rotation, glm::vec3 center, float halfHeight, float radius, 
		int subdiv);
	void add_cone_vertex(Model *m, glm::mat3 rotation, glm::vec3 center, float height, float radius, int subdiv);
	void add_torus_vertex(Model *m, glm::mat3 rotation, glm::vec3 center, float radius, float sectionRadius, 
		int axisSubdiv, int heightSubdiv);

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
