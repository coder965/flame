#pragma once

#ifdef _FLAME_MODEL_EXPORTS
#define FLAME_MODEL_EXPORTS __declspec(dllexport)
#else
#define FLAME_MODEL_EXPORTS __declspec(dllimport)
#endif

#include <vector>

namespace flame
{
	enum VertexSemantics
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

	inline int get_num_channel_of_vertex_semantics(VertexSemantics s)
	{
		switch (s)
		{
			case VertexPosition:
				return 3;
			case VertexUV0: case VertexUV1: case VertexUV2: case VertexUV3:
			case VertexUV4: case VertexUV5: case VertexUV6: case VertexUV7:
				return 2;
			case VertexNormal:
				return 3;
			case VertexTangent:
				return 3;
			case VertexBitangent:
				return 3;
			case VertexColor:
				return 4;
		}
	}

	struct BufferDescription
	{
		std::vector<VertexSemantics> vtx_sems;
		bool active_if_has_bone;
	};

	enum MapSemantics
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
		ModelPrivate *_priv;

		FLAME_MODEL_EXPORTS std::vector<BufferDescription> get_vertex_semantics();
		FLAME_MODEL_EXPORTS std::vector<std::vector<MapSemantics>> get_map_semantics();

		FLAME_MODEL_EXPORTS int get_vertex_count();
		FLAME_MODEL_EXPORTS int get_indice_count();

		FLAME_MODEL_EXPORTS void *get_vertexes();
		FLAME_MODEL_EXPORTS void *get_indices();

		FLAME_MODEL_EXPORTS int get_material_count();
		FLAME_MODEL_EXPORTS std::string get_material_name();
		FLAME_MODEL_EXPORTS int get_material_indice_base(int i_material);
		FLAME_MODEL_EXPORTS int get_material_indice_count(int i_material);
		FLAME_MODEL_EXPORTS int get_material_map_name(int i_material, int i_map);
	};

	FLAME_MODEL_EXPORTS void hint_model_buffer_description_freedom(); // default
	FLAME_MODEL_EXPORTS void hint_model_buffer_description(const std::vector<BufferDescription> &sems);
	FLAME_MODEL_EXPORTS void hint_model_map_semantics_freedom(); // default
	FLAME_MODEL_EXPORTS void hint_model_map_semantics(const std::vector<std::vector<MapSemantics>> &sems);
	FLAME_MODEL_EXPORTS void hint_model_indice_type(IndiceType type); // default is uint
	FLAME_MODEL_EXPORTS Model *load_model(const std::string &filename);
	FLAME_MODEL_EXPORTS void save_model(Model *m, const std::string &filename);
	FLAME_MODEL_EXPORTS void release_model(Model *m);
}

