//MIT License
//
//Copyright (c) 2018 wjs
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

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
			global_matrix = glm::transpose(local_matrix);
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

		glm::mat4 offset_matrix;
		int id;
	};

	struct ModelPositionKey
	{
		float time;
		glm::vec3 value;
	};

	struct ModelRotationKey
	{
		float time;
		glm::vec4 value;
	};

	struct ModelMotion
	{
		char name[FLAME_MODEL_NAME_LENGTH];

		int position_key_count;
		ModelPositionKey *position_keys;

		int rotation_key_count;
		ModelRotationKey *rotation_keys;
	};

	struct ModelAnimation
	{
		char name[FLAME_MODEL_NAME_LENGTH];

		float total_ticks;
		int ticks_per_second;

		int motion_count;
		ModelMotion *motions;

		int find_motion(const char *name)
		{
			for (auto i = 0; i < motion_count; i++)
			{
				if (strcmp(motions[i].name, name) == 0)
					return i;
			}
			return -1;
		}
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

		int animation_count;
		ModelAnimation **animations;

		bool bone_count_exceeded;
		bool bone_count_per_vertex_exceeded;
	};

	struct ModelDescription
	{
		int desired_vertex_buffer_count;
		VertexBufferDescription *desired_vertex_buffers;
		int desired_map_count;
		MapSemantic *desired_maps;
		IndiceType indice_type;
		int max_bone_count;
		int max_bone_count_per_vertex;  //(currenly we don't know how to process more than 4, so this option does nothing)

		inline void set_to_default()
		{
			desired_vertex_buffer_count = 0;
			desired_vertex_buffers = nullptr;
			desired_map_count = 0;
			desired_maps = nullptr;
			indice_type = IndiceUint;
			max_bone_count = 255;
			max_bone_count_per_vertex = 4;
		}
	};

	FLAME_MODEL_EXPORTS Model *load_model(ModelDescription *desc, const char *filename);
	FLAME_MODEL_EXPORTS Model *create_cube_model(ModelDescription *desc, float hf_ext);
	FLAME_MODEL_EXPORTS void save_model(Model *m, const char *filename);
	FLAME_MODEL_EXPORTS void destroy_model(Model *m);
}

