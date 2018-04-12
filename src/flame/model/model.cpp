#include "model.h"

namespace flame
{
	struct ModelPrivate
	{

	};

	std::vector<std::vector<VertexSemantics>> Model::get_vertex_semantics()
	{

	}

	std::vector<std::vector<MapSemantics>> Model::get_map_semantics()
	{

	}

	int Model::get_vertex_count()
	{

	}

	int Model::get_indice_count()
	{

	}

	void *Model::get_vertexes()
	{

	}

	void *Model::get_indices()
	{

	}

	int Model::get_material_count()
	{

	}

	std::string Model::get_material_name()
	{

	}

	int Model::get_material_indice_base(int i_material)
	{

	}

	int Model::get_material_indice_count(int i_material)
	{

	}

	int Model::get_material_map_name(int i_material, int i_map)
	{

	}

	void hint_model_vertex_semantics_freedom()
	{

	}
	
	void hint_model_vertex_semantics(const std::vector<std::vector<VertexSemantics>> &sems)
	{

	}

	void hint_model_map_semantics_freedom()
	{

	}

	void hint_model_map_semantics(const std::vector<std::vector<MapSemantics>> &sems)
	{

	}

	void hint_model_indice_type(IndiceType type)
	{

	}

	Model *load_model(const std::string &filename)
	{
		auto m = new Model;
		m->_priv = new ModelPrivate;

		return m;
	}

	void save_model(Model *m, const std::string &filename)
	{

	}

	void release_model(Model *m)
	{
		delete m->_priv;
		delete m;
	}
}

