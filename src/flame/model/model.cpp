#include "model.h"

#include "assert.h"
#include <flame/global.h>
#include <flame/filesystem.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace flame
{
	struct ModelMap
	{
		std::string filename;
	};

	struct ModelMaterial
	{
		ModelMap maps[MapSemanticsCount];

		int indice_base;
		int indice_count;
	};

	struct ModelVertexBuffer
	{
		int vertex_size;
		unsigned char *pVertex;
	};

	struct ModelPrivate
	{
		std::vector<BufferDescription> vertex_semantics;
		std::vector<std::vector<MapSemantic>> map_semantics;

		int vertex_count;
		int indice_count;

		std::vector<ModelVertexBuffer> vbs;
		unsigned char *pIndices;

		std::vector<ModelMaterial> materials;
	};

	static std::vector<BufferDescription> hint_vertex_semantics;
	static std::vector<std::vector<MapSemantic>> hint_map_semantics;
	static int hint_indice_type = 0; // 0 for uint, 1 for ushort

	std::vector<BufferDescription> Model::get_vertex_semantics()
	{
		return _priv->vertex_semantics;
	}

	std::vector<std::vector<MapSemantic>> Model::get_map_semantics()
	{
		return _priv->map_semantics;
	}

	int Model::get_vertex_count()
	{
		return _priv->vertex_count;
	}

	int Model::get_indice_count()
	{
		return _priv->indice_count;
	}

	void *Model::get_vertexes(int index)
	{
		assert(index < _priv->vbs.size());

		return _priv->vbs[index].pVertex;
	}

	int Model::get_vertex_size(int index)
	{
		assert(index < _priv->vbs.size());

		return _priv->vbs[index].vertex_size;
	}

	void *Model::get_indices()
	{
		return _priv->pIndices;
	}

	int Model::get_material_count()
	{
		return _priv->materials.size();
	}

	std::string Model::get_material_name()
	{
		return "";
	}

	int Model::get_material_indice_base(int index)
	{
		assert(index < _priv->materials.size());

		return _priv->materials[index].indice_base;
	}

	int Model::get_material_indice_count(int index)
	{
		assert(index < _priv->materials.size());

		return _priv->materials[index].indice_count;
	}

	int Model::get_material_map_name(int index, MapSemantic sem)
	{
		return 0;
	}

	void hint_model_buffer_description_freedom()
	{

	}
	
	void hint_model_buffer_description(const std::vector<BufferDescription> &sems)
	{

	}

	void hint_model_map_semantics_freedom()
	{

	}

	void hint_model_map_semantics(const std::vector<std::vector<MapSemantic>> &sems)
	{

	}

	void hint_model_indice_type(IndiceType type)
	{

	}

	Model *load_model(const std::string &filename)
	{
		std::filesystem::path path(filename);
		if (!std::filesystem::exists(path))
			return nullptr;

		auto m = new Model;

		m->_priv = new ModelPrivate;
		m->_priv->vertex_semantics = hint_vertex_semantics;
		m->_priv->map_semantics = hint_map_semantics;

		Assimp::Importer importer;
		auto scene = importer.ReadFile(filename.c_str(), aiProcess_Triangulate);

		auto vertex_count = 0;
		auto indice_count = 0;

		struct VertexSemanticsAttribute
		{
			bool need;
			bool has;
			int buffer_index;
			int offset;
		};

		VertexSemanticsAttribute vtx_sem_attr[VertexSemanticsCount];

		for (auto i = 0; i < VertexSemanticsCount; i++)
		{
			vtx_sem_attr[i].need = false;
			vtx_sem_attr[i].has = false;
			vtx_sem_attr[i].buffer_index = -1;
			vtx_sem_attr[i].offset = -1;
		}

		m->_priv->materials.resize(scene->mNumMeshes);

		bool has_bone = false;

		for (auto i = 0; i < scene->mNumMeshes; i++)
		{
			auto mesh = scene->mMeshes[i];

			auto mid = mesh->mMaterialIndex;
			auto mtl = scene->mMaterials[mid];
			if (mtl->GetTextureCount(aiTextureType_DIFFUSE) > 0)
			{
				aiString ret_name;
				mtl->GetTexture(aiTextureType_DIFFUSE, 0, &ret_name);
			}

			if (mesh->mVertices)
				vtx_sem_attr[VertexPosition].has = true;
			auto num_uv_channel = mesh->GetNumUVChannels();
			for (auto j = 0; j < num_uv_channel; j++)
				vtx_sem_attr[VertexUV0 + j].has = true;
			if (mesh->mNormals)
				vtx_sem_attr[VertexNormal].has = true;
			if (mesh->mTangents)
				vtx_sem_attr[VertexTangent].has = true;
			if (mesh->mBitangents)
				vtx_sem_attr[VertexBitangent].has = true;
			auto num_color_channel = mesh->GetNumColorChannels();
			assert(num_color_channel <= 1);
			if (num_color_channel == 1)
				vtx_sem_attr[VertexColor].has = true;
			if (mesh->mBones)
			{
				has_bone = true;
				vtx_sem_attr[VertexBoneID].has = true;
				vtx_sem_attr[VertexBoneWeight].has = true;
			}

			m->_priv->materials[i].indice_base = indice_count;
			m->_priv->materials[i].indice_count = mesh->mNumFaces * 3;

			vertex_count += mesh->mNumVertices;
			indice_count += m->_priv->materials[i].indice_count;
		}

		if (m->_priv->vertex_semantics.empty())
		{
			// by default, we accept all vertex semantics and put them all in one buffer
			m->_priv->vertex_semantics.resize(1);
			m->_priv->vertex_semantics[0].active_if_has_bone = false;
			for (auto i = 0; i < VertexSemanticsCount; i++)
			{
				if (vtx_sem_attr[i].has)
					m->_priv->vertex_semantics[0].vtx_sems.push_back(VertexSemantic(i));
			}
		}

		for (auto i = 0; i < m->_priv->vertex_semantics.size(); i++)
		{
			auto &v = m->_priv->vertex_semantics[i];
			if (v.active_if_has_bone && !has_bone)
				continue;

			ModelVertexBuffer d;
			d.vertex_size = 0;
			for (auto s : v.vtx_sems)
			{
				if (vtx_sem_attr[s].need)
					assert(0); // vertex semantics cannot repeat
				vtx_sem_attr[s].need = true;
				vtx_sem_attr[s].buffer_index = m->_priv->vbs.size();
				vtx_sem_attr[s].offset = d.vertex_size;

				switch (s)
				{
					case VertexPosition:
						d.vertex_size += 3;
						break;
					case VertexUV0: case VertexUV1: case VertexUV2: case VertexUV3:
					case VertexUV4: case VertexUV5: case VertexUV6: case VertexUV7:
						d.vertex_size += 2;
						break;
					case VertexNormal:
						d.vertex_size += 3;
						break;
					case VertexTangent:
						d.vertex_size += 3;
						break;
					case VertexBitangent:
						d.vertex_size += 3;
						break;
					case VertexColor:
						d.vertex_size += 4;
						break;
				}
			}
			m->_priv->vbs.push_back(d);
		}

		m->_priv->vertex_count = vertex_count;
		m->_priv->indice_count = indice_count;

		for (auto &d : m->_priv->vbs)
		{
			d.vertex_size *= sizeof(float);
			d.pVertex = (unsigned char*)malloc(vertex_count * d.vertex_size);
		}
		m->_priv->pIndices = (unsigned char*)malloc(indice_count * (hint_indice_type == 0 ? sizeof(uint) : sizeof(ushort)));

		auto v_base = 0;
		for (auto i = 0; i < scene->mNumMeshes; i++)
		{
			auto mesh = scene->mMeshes[i];

			auto v_count = mesh->mNumVertices;

			for (auto isem = 0; isem < VertexSemanticsCount; isem++)
			{
				auto &s = vtx_sem_attr[isem];
				if (s.need)
				{
					auto &d = m->_priv->vbs[s.buffer_index];
					switch (VertexSemantic(isem))
					{
						case VertexPosition:
							if (s.has)
							{
								for (auto j = 0; j < v_count; j++)
								{
									*(float*)(d.pVertex + s.offset * sizeof(float) + (j + v_base) * d.vertex_size) = mesh->mVertices[j].x;
									*(float*)(d.pVertex + s.offset * sizeof(float) + (j + v_base) * d.vertex_size + sizeof(float)) = mesh->mVertices[j].y;
									*(float*)(d.pVertex + s.offset * sizeof(float) + (j + v_base) * d.vertex_size + sizeof(float) * 2) = mesh->mVertices[j].z;
								}
							}
							break;
						case VertexUV0: case VertexUV1: case VertexUV2: case VertexUV3:
						case VertexUV4: case VertexUV5: case VertexUV6: case VertexUV7:
							if (s.has)
							{
								auto tc = mesh->mTextureCoords[isem - VertexUV0];
								for (auto j = 0; j < v_count; j++)
								{
									*(float*)(d.pVertex + s.offset * sizeof(float) + (j + v_base) * d.vertex_size) = tc[j].x;
									*(float*)(d.pVertex + s.offset * sizeof(float) + (j + v_base) * d.vertex_size + sizeof(float)) = tc[j].y;
								}
							}
							break;
						case VertexNormal:
							if (s.has)
							{
								for (auto j = 0; j < v_count; j++)
								{
									*(float*)(d.pVertex + s.offset * sizeof(float) + (j + v_base) * d.vertex_size) = mesh->mNormals[j].x;
									*(float*)(d.pVertex + s.offset * sizeof(float) + (j + v_base) * d.vertex_size + sizeof(float)) = mesh->mNormals[j].y;
									*(float*)(d.pVertex + s.offset * sizeof(float) + (j + v_base) * d.vertex_size + sizeof(float) * 2) = mesh->mNormals[j].z;
								}
							}
							break;
						case VertexTangent:
							if (s.has)
							{
								for (auto j = 0; j < v_count; j++)
								{
									*(float*)(d.pVertex + s.offset * sizeof(float) + (j + v_base) * d.vertex_size) = mesh->mTangents[j].x;
									*(float*)(d.pVertex + s.offset * sizeof(float) + (j + v_base) * d.vertex_size + sizeof(float)) = mesh->mTangents[j].y;
									*(float*)(d.pVertex + s.offset * sizeof(float) + (j + v_base) * d.vertex_size + sizeof(float) * 2) = mesh->mTangents[j].z;
								}
							}
							break;
						case VertexBitangent:
							if (s.has)
							{
								for (auto j = 0; j < v_count; j++)
								{
									*(float*)(d.pVertex + s.offset * sizeof(float) + (j + v_base) * d.vertex_size) = mesh->mBitangents[j].x;
									*(float*)(d.pVertex + s.offset * sizeof(float) + (j + v_base) * d.vertex_size + sizeof(float)) = mesh->mBitangents[j].y;
									*(float*)(d.pVertex + s.offset * sizeof(float) + (j + v_base) * d.vertex_size + sizeof(float) * 2) = mesh->mBitangents[j].z;
								}
							}
							break;
						case VertexColor:
							if (s.has)
							{
								auto col = mesh->mColors[0];
								for (auto j = 0; j < v_count; j++)
								{
									*(float*)(d.pVertex + s.offset * sizeof(float) + (j + v_base) * d.vertex_size) = col[j].r;
									*(float*)(d.pVertex + s.offset * sizeof(float) + (j + v_base) * d.vertex_size + sizeof(float)) = col[j].g;
									*(float*)(d.pVertex + s.offset * sizeof(float) + (j + v_base) * d.vertex_size + sizeof(float) * 2) = col[j].b;
									*(float*)(d.pVertex + s.offset * sizeof(float) + (j + v_base) * d.vertex_size + sizeof(float) * 3) = col[j].a;
								}
							}
							break;
						case VertexBoneID:
							break;
						case VertexBoneWeight:
							break;
					}
				}
			}

			v_base += v_count;

			auto i_base = m->_priv->materials[i].indice_base;
			auto face_count = mesh->mNumFaces;
			for (auto j = 0; j < face_count; j++)
			{
				for (auto k = 0; k < 3; k++)
				{
					if (hint_indice_type == 0)
						*(uint*)(m->_priv->pIndices + (j * 3 + k + i_base) * sizeof(uint)) = i_base + mesh->mFaces[j].mIndices[k];
					else
						*(ushort*)(m->_priv->pIndices + (j * 3 + k + i_base) * sizeof(ushort)) = i_base + mesh->mFaces[j].mIndices[k];
				}
			}
		}

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

