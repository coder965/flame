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

	struct ModelMesh
	{
		ModelMap maps[MapSemanticsCount];

		int indice_base;
		int indice_count;

		int material_index;
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

		std::vector<ModelMesh> meshs;
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

		return _priv->vbs[index].vertex_size * sizeof(float);
	}

	void *Model::get_indices()
	{
		return _priv->pIndices;
	}

	int Model::get_mesh_count()
	{
		return _priv->meshs.size();
	}

	int Model::get_mesh_indice_base(int index)
	{
		assert(index < _priv->meshs.size());

		return _priv->meshs[index].indice_base;
	}

	int Model::get_mesh_indice_count(int index)
	{
		assert(index < _priv->meshs.size());

		return _priv->meshs[index].indice_count;
	}

	int Model::get_mesh_material_index(int index)
	{
		assert(index < _priv->meshs.size());

		return _priv->meshs[index].material_index;
	}


	//int Model::get_material_count()
	//{
	//	return _priv->materials.size();
	//}

	//std::string Model::get_material_name()
	//{
	//	return "";
	//}

	//int Model::get_material_map_name(int index, MapSemantic sem)
	//{
	//	return 0;
	//}

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

		m->_priv->meshs.resize(scene->mNumMeshes);

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

			auto ic = mesh->mNumFaces * 3;

			m->_priv->meshs[i].indice_base = indice_count;
			m->_priv->meshs[i].indice_count = ic;

			vertex_count += mesh->mNumVertices;
			indice_count += ic;
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
					case VertexBoneID:
						d.vertex_size += 4; // we treat bone id as float, since there are enough bits for bone count
						break;
					case VertexBoneWeight:
						d.vertex_size += 4;
						break;
				}
			}
			m->_priv->vbs.push_back(d);
		}

		m->_priv->vertex_count = vertex_count;
		m->_priv->indice_count = indice_count;

		for (auto &d : m->_priv->vbs)
			d.pVertex = (unsigned char*)malloc(vertex_count * d.vertex_size * sizeof(float));
		m->_priv->pIndices = (unsigned char*)malloc(indice_count * (hint_indice_type == 0 ? sizeof(uint) : sizeof(ushort)));

		std::vector<int> vertex_bone_count;
		if (has_bone)
		{
			// do initialize the bone ids and bone weights, since we don't know if there are some vertexs not covered by bones
			auto initialize_ = [&](VertexSemantic sem) {
				auto &s = vtx_sem_attr[sem];
				if (s.need)
				{
					auto &d = m->_priv->vbs[s.buffer_index];
					auto p = (float*)d.pVertex + s.offset;
					for (auto i = 0; i < vertex_count; i++)
					{
						p[0] = 0.f;
						p[1] = 0.f;
						p[2] = 0.f;
						p[3] = 0.f;
						p += d.vertex_size;
					}
				}
			};
			initialize_(VertexBoneID);
			initialize_(VertexBoneWeight);

			vertex_bone_count.resize(vertex_count);
		}

		auto v_base = 0;
		auto i_base = 0;
		for (auto i = 0; i < scene->mNumMeshes; i++)
		{
			auto mesh = scene->mMeshes[i];

			for (auto isem = 0; isem < VertexSemanticsCount; isem++)
			{
				auto &s = vtx_sem_attr[isem];
				if (s.need)
				{
					auto &d = m->_priv->vbs[s.buffer_index];
					auto p = (float*)d.pVertex + v_base * d.vertex_size + s.offset;
					switch (VertexSemantic(isem))
					{
						case VertexPosition:
							if (s.has)
							{
								for (auto j = 0; j < mesh->mNumVertices; j++)
								{
									p[0] = mesh->mVertices[j].x;
									p[1] = mesh->mVertices[j].y;
									p[2] = mesh->mVertices[j].z;
									p += d.vertex_size;
								}
							}
							break;
						case VertexUV0: case VertexUV1: case VertexUV2: case VertexUV3:
						case VertexUV4: case VertexUV5: case VertexUV6: case VertexUV7:
							if (s.has)
							{
								auto tc = mesh->mTextureCoords[isem - VertexUV0];
								for (auto j = 0; j < mesh->mNumVertices; j++)
								{
									p[0] = tc[j].x;
									p[1] = tc[j].y;
									p += d.vertex_size;
								}
							}
							break;
						case VertexNormal:
							if (s.has)
							{
								for (auto j = 0; j < mesh->mNumVertices; j++)
								{
									p[0] = mesh->mNormals[j].x;
									p[1] = mesh->mNormals[j].y;
									p[2] = mesh->mNormals[j].z;
									p += d.vertex_size;
								}
							}
							break;
						case VertexTangent:
							if (s.has)
							{
								for (auto j = 0; j < mesh->mNumVertices; j++)
								{
									p[0] = mesh->mTangents[j].x;
									p[1] = mesh->mTangents[j].y;
									p[2] = mesh->mTangents[j].z;
									p += d.vertex_size;
								}
							}
							break;
						case VertexBitangent:
							if (s.has)
							{
								for (auto j = 0; j < mesh->mNumVertices; j++)
								{
									p[0] = mesh->mBitangents[j].x;
									p[1] = mesh->mBitangents[j].y;
									p[2] = mesh->mBitangents[j].z;
									p += d.vertex_size;
								}
							}
							break;
						case VertexColor:
							if (s.has)
							{
								auto col = mesh->mColors[0];
								for (auto j = 0; j < mesh->mNumVertices; j++)
								{
									p[0] = col[j].r;
									p[1] = col[j].g;
									p[2] = col[j].b;
									p[3] = col[j].a;
									p += d.vertex_size;
								}
							}
							break;
					}
				}
			}

			if (has_bone)
			{
				for (auto j = 0; j < mesh->mNumBones; j++)
				{
					auto b = mesh->mBones[j];
					for (auto k = 0; k < b->mNumWeights; k++)
					{
						auto vid = b->mWeights[k].mVertexId;
						if (vertex_bone_count[v_base + vid] < 4)
						{
							{
								auto &s = vtx_sem_attr[VertexBoneID];
								if (s.need)
								{
									auto &d = m->_priv->vbs[s.buffer_index];
									auto p = (float*)d.pVertex + v_base * d.vertex_size + s.offset;

								}
							}
							{
								auto &s = vtx_sem_attr[VertexBoneWeight];
								if (s.need)
								{
									auto &d = m->_priv->vbs[s.buffer_index];
									auto p = (float*)d.pVertex + v_base * d.vertex_size + s.offset;
								}
							}
							vertex_bone_count[v_base + vid]++;
						}
						else
							m->bone_count_per_vertex_exceeded = true;
					}
				}
			}

			v_base += mesh->mNumVertices;

			if (hint_indice_type == 0)
			{
				auto p = (uint*)m->_priv->pIndices + i_base;
				for (auto j = 0; j < mesh->mNumFaces; j++)
				{
					for (auto k = 0; k < 3; k++)
					{
						p[0] = i_base + mesh->mFaces[j].mIndices[k];
						p++;
					}
				}
			}
			else
			{
				auto p = (ushort*)m->_priv->pIndices + i_base;
				for (auto j = 0; j < mesh->mNumFaces; j++)
				{
					for (auto k = 0; k < 3; k++)
					{
						p[0] = i_base + mesh->mFaces[j].mIndices[k];
						p++;
					}
				}
			}

			i_base += mesh->mNumFaces * 3;
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

