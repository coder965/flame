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

	struct BufferDescription
	{
		std::vector<VertexSemantic> vtx_sems;
		bool active_if_has_bone;
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

	struct ModelPrivate;

	struct Model
	{
		bool bone_count_exceeded;
		bool bone_count_per_vertex_exceeded;

		ModelPrivate *_priv;

		FLAME_MODEL_EXPORTS std::vector<BufferDescription> get_vertex_semantics();
		FLAME_MODEL_EXPORTS std::vector<std::vector<MapSemantic>> get_map_semantics();

		FLAME_MODEL_EXPORTS int get_vertex_count();
		FLAME_MODEL_EXPORTS int get_indice_count();

		FLAME_MODEL_EXPORTS void *get_vertexes(int index);
		FLAME_MODEL_EXPORTS int get_vertex_size(int index);
		FLAME_MODEL_EXPORTS void *get_indices();

		FLAME_MODEL_EXPORTS int get_mesh_count();
		FLAME_MODEL_EXPORTS int get_mesh_indice_base(int index);
		FLAME_MODEL_EXPORTS int get_mesh_indice_count(int index);
		FLAME_MODEL_EXPORTS int get_mesh_material_index(int index);

		FLAME_MODEL_EXPORTS int get_bone_count();
		FLAME_MODEL_EXPORTS int get_bone_root();
		FLAME_MODEL_EXPORTS void get_bone_matrix(int index, float *dst);
		FLAME_MODEL_EXPORTS int get_bone_parent(int index);
		FLAME_MODEL_EXPORTS int get_bone_children_count(int index);
		FLAME_MODEL_EXPORTS int get_bone_child(int index, int i);

		//FLAME_MODEL_EXPORTS int get_material_count();
		//FLAME_MODEL_EXPORTS std::string get_material_name();
		//FLAME_MODEL_EXPORTS int get_material_map_name(int index, MapSemantic sem);
	};

	FLAME_MODEL_EXPORTS void hint_model_buffer_description_freedom(); // default
	FLAME_MODEL_EXPORTS void hint_model_buffer_description(const std::vector<BufferDescription> &sems);
	FLAME_MODEL_EXPORTS void hint_model_map_semantics_freedom(); // default
	FLAME_MODEL_EXPORTS void hint_model_map_semantics(const std::vector<std::vector<MapSemantic>> &sems);
	FLAME_MODEL_EXPORTS void hint_model_indice_type(IndiceType type); // default is uint
	FLAME_MODEL_EXPORTS void hint_model_max_bone_count(int count); // default is 255
	FLAME_MODEL_EXPORTS void hint_model_max_bone_count_per_vertex(int count); // default is 4, (currenly we don't know how to process more than 4, so this API does nothing)
	FLAME_MODEL_EXPORTS Model *load_model(const std::string &filename);
	FLAME_MODEL_EXPORTS void save_model(Model *m, const std::string &filename);
	FLAME_MODEL_EXPORTS void release_model(Model *m);
}

