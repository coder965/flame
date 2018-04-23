#pragma once

#ifdef _FLAME_MODEL_EXPORTS
#define FLAME_MODEL_EXPORTS __declspec(dllexport)
#else
#define FLAME_MODEL_EXPORTS __declspec(dllimport)
#endif

#define FLAME_MODEL_NAME_LENGTH 128

#include <flame/math.h>

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
		char name[FLAME_MODEL_NAME_LENGTH];
		glm::mat4 local_matrix;
		glm::mat4 global_matrix;

		ModelNode *parent;
		ModelNode *next_sibling;
		int children_count;
		ModelNode *first_child;
		ModelNode *last_child;

		ModelNodeType type;
		void *p;

		void calc_global_matrix()
		{
			global_matrix = local_matrix;
			if (parent)
				global_matrix = parent->global_matrix * global_matrix;

			auto c = first_child;
			while (c)
			{
				c->calc_global_matrix();
				c = c->next_sibling;
			}
		}
	};

	struct ModelVertexBuffer
	{
		int semantic_count;
		VertexSemantic *semantics;

		int size;
		unsigned char *pVertex;
	};

	struct ModelMesh
	{
		ModelNode *pNode;
		char name[FLAME_MODEL_NAME_LENGTH];

		int indice_base;
		int indice_count;

		int material_index;
	};

	struct ModelBone
	{
		ModelNode *pNode;
		char name[FLAME_MODEL_NAME_LENGTH];

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

	struct Model
	{
		int vertex_count;
		int indice_count;

		int vertex_buffer_count;
		ModelVertexBuffer *vertex_buffers;
		unsigned char *pIndices;

		ModelNode *root_node;

		int mesh_count;
		ModelMesh **meshes;

		int bone_count;
		ModelBone **bones;
		ModelNode *root_bone;

		bool bone_count_exceeded;
		bool bone_count_per_vertex_exceeded;
	};

	FLAME_MODEL_EXPORTS Model *load_model(const char *filename, 
		int desired_vertex_buffer_count = 0, VertexBufferDescription *desired_vertex_buffers = nullptr,
		int desired_map_count = 0, MapSemantic *desired_maps = nullptr,
		IndiceType indice_type = IndiceUint,
		int max_bone_count = 255,
		int max_bone_count_per_vertex = 4); //(currenly we don't know how to process more than 4, so this option does nothing)
	FLAME_MODEL_EXPORTS void save_model(Model *m, const char *filename);
	FLAME_MODEL_EXPORTS void release_model(Model *m);
}

