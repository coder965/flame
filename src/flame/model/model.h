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
		VertexSemanticsPosition,
		VertexSemanticsUV0,
		VertexSemanticsUV1,
		VertexSemanticsUV2,
		VertexSemanticsUV3,
		VertexSemanticsUV4,
		VertexSemanticsUV5,
		VertexSemanticsUV6,
		VertexSemanticsUV7,
		VertexSemanticsNormal,
		VertexSemanticsTangent,
		VertexSemanticsBitangent,
		VertexSemanticsColor,

		VertexSemanticsCount
	};

	enum MapType
	{
		MapTypeAlbedo,
		MapTypeAlpha,
		MapTypeNormal,
		MapTypeHeight
	};

	struct ModelPrivate;

	struct Model
	{
		ModelPrivate *_priv;

		FLAME_MODEL_EXPORTS int get_material_count();
		FLAME_MODEL_EXPORTS std::string get_material_name();
		FLAME_MODEL_EXPORTS int get_material_indice_base(int index);
		FLAME_MODEL_EXPORTS int get_material_indice_count(int index);

	};

	FLAME_MODEL_EXPORTS void hint_model_semantics(const std::vector<std::vector<VertexSemantics>> &sems);
}

