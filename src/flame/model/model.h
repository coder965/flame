#pragma once

#ifdef _FLAME_MODEL_EXPORTS
#define FLAME_MODEL_EXPORTS __declspec(dllexport)
#else
#define FLAME_MODEL_EXPORTS __declspec(dllimport)
#endif

#include <vector>

namespace flame
{
	enum VertexSemantic
	{
		VertexPosition,
		VertexUV0,
		VertexUV1,
		VertexUV2,
		VertexUV3,
		VertexUV4,
		VertexUV5,
		VertexUV6,
		VertexUV7,
		VertexNormal,
		VertexTangent,
		VertexBitangent,
		VertexColor,
		VertexBoneID,
		VertexBoneWeight,

		VertexSemanticsCount
	};

	struct VertexBufferDescription
	{
		int semantic_count;
		VertexSemantic *semantics;
		bool active_if_has_bone;
	};

	enum ModelNodeType
	{
		ModelNodeNode,
		ModelNodeMesh,
		ModelNodeBone
	};

	struct ModelNode
	{
		ModelNodeType type;
		std::string name;
		float matrix[16];

		ModelNode *parent;
		int children_count;
		ModelNode *childrens;
	};

	struct ModelVertexBuffer
	{
		int semantic_count;
		VertexSemantic *semantics;

		int size;
		unsigned char *pVertex;
	};

	struct ModelMesh : ModelNode
	{
		int indice_base;
		int indice_count;

		int material_index;
	};

	struct ModelBone : ModelNode
	{
		float offset_matrix[16];
	};

	enum MapSemantic
	{
		MapAlbedo,
		MapMask,
		MapAlpha,
		MapNormal,
		MapHeight,
		MapSpec,
		MapRoughness,
		MapEmission,
		MapOcclusion,

		MapSemanticsCount
	};

	enum IndiceType
	{
		IndiceUshort,
		IndiceUint,

		IndiceTypeCount
	};

	//struct ModelPrivate;

	struct Model
	{
		int vertex_count;
		int indice_count;

		int vertex_buffer_count;
		ModelVertexBuffer *vertex_buffers;
		unsigned char *pIndices;

		ModelNode *root_node;

		bool bone_count_exceeded;
		bool bone_count_per_vertex_exceeded;

		//ModelPrivate *_priv;

		//FLAME_MODEL_EXPORTS int get_mesh_count();
		//FLAME_MODEL_EXPORTS int get_mesh_indice_base(int index);
		//FLAME_MODEL_EXPORTS int get_mesh_indice_count(int index);
		//FLAME_MODEL_EXPORTS int get_mesh_material_index(int index);

		//FLAME_MODEL_EXPORTS int get_bone_count();
		//FLAME_MODEL_EXPORTS int get_bone_root();
		//FLAME_MODEL_EXPORTS void get_bone_matrix(int index, float *dst);
		//FLAME_MODEL_EXPORTS int get_bone_parent(int index);
		//FLAME_MODEL_EXPORTS int get_bone_children_count(int index);
		//FLAME_MODEL_EXPORTS int get_bone_child(int index, int i);

		//FLAME_MODEL_EXPORTS int get_material_count();
		//FLAME_MODEL_EXPORTS std::string get_material_name();
		//FLAME_MODEL_EXPORTS int get_material_map_name(int index, MapSemantic sem);
	};

	FLAME_MODEL_EXPORTS Model *load_model(const std::string &filename, 
		int desired_vertex_buffer_count = 0, VertexBufferDescription *desired_vertex_buffers = nullptr,
		int desired_map_count = 0, MapSemantic *desired_maps = nullptr,
		IndiceType indice_type = IndiceUint,
		int max_bone_count = 255,
		int max_bone_count_per_vertex = 4); //(currenly we don't know how to process more than 4, so this option does nothing)
	FLAME_MODEL_EXPORTS void save_model(Model *m, const std::string &filename);
	FLAME_MODEL_EXPORTS void release_model(Model *m);
}

