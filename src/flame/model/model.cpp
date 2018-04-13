#include "model.h"

#include "assert.h"
#include <flame/global.h>
#include <flame/filesystem.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace flame
{
	struct Material
	{
		int indice_base;
		int indice_count;
	};

	struct ModelPrivate
	{

	};

	static std::vector<BufferDescription> hint_vertex_semantics;
	static std::vector<std::vector<MapSemantics>> hint_map_semantics;
	static int hint_indice_type = 0; // 0 for uint, 1 for ushort

	std::vector<BufferDescription> Model::get_vertex_semantics()
	{
		return hint_vertex_semantics;
	}

	std::vector<std::vector<MapSemantics>> Model::get_map_semantics()
	{
		return hint_map_semantics;
	}

	int Model::get_vertex_count()
	{
		return 0;
	}

	int Model::get_indice_count()
	{
		return 0;
	}

	void *Model::get_vertexes()
	{
		return nullptr;
	}

	void *Model::get_indices()
	{
		return nullptr;
	}

	int Model::get_material_count()
	{
		return 0;
	}

	std::string Model::get_material_name()
	{
		return "";
	}

	int Model::get_material_indice_base(int i_material)
	{
		return 0;
	}

	int Model::get_material_indice_count(int i_material)
	{
		return 0;
	}

	int Model::get_material_map_name(int i_material, int i_map)
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

	void hint_model_map_semantics(const std::vector<std::vector<MapSemantics>> &sems)
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

		std::vector<Material> materials;
		materials.resize(scene->mNumMeshes);

		bool has_bone = false;

		for (auto i = 0; i < scene->mNumMeshes; i++)
		{
			auto mesh = scene->mMeshes[i];

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

			materials[i].indice_base = indice_count;
			materials[i].indice_count = mesh->mNumFaces * 3;

			vertex_count += mesh->mNumVertices;
			indice_count += materials[i].indice_count;
		}

		struct BufferData
		{
			unsigned char *pVertex;
			int vertex_size;
		};

		std::vector<BufferData> buffer_datas;
		if (!hint_vertex_semantics.empty())
		{
			buffer_datas.resize(hint_vertex_semantics.size());
			for (auto i = 0; i < hint_vertex_semantics.size(); i++)
			{
				auto &v = hint_vertex_semantics[i];
				if (v.active_if_has_bone && !has_bone)
					continue;

				BufferData d;
				d.vertex_size = 0;
				for (auto s : v.vtx_sems)
				{
					if (vtx_sem_attr[s].need)
						assert(0); // vertex semantics cannot repeat
					vtx_sem_attr[s].need = true;
					vtx_sem_attr[s].buffer_index = buffer_datas.size();
					vtx_sem_attr[s].offset = d.vertex_size;

					d.vertex_size += get_num_channel_of_vertex_semantics(s);
				}
				buffer_datas.push_back(d);
			}
		}
		else
		{
			// by default, we accept all vertex semantics and put them all in one buffer
			buffer_datas.resize(1);
			buffer_datas[0].vertex_size = 0;
			for (auto i = 0; i < VertexSemanticsCount; i++)
			{
				if (vtx_sem_attr[i].has)
				{
					vtx_sem_attr[i].need = true;
					vtx_sem_attr[i].buffer_index = 0;
					vtx_sem_attr[i].offset = buffer_datas[0].vertex_size;

					buffer_datas[0].vertex_size += get_num_channel_of_vertex_semantics(VertexSemantics(i));
				}
			}
		}

		for (auto &d : buffer_datas)
			d.pVertex = (unsigned char*)malloc(vertex_count * d.vertex_size * sizeof(float));
		auto pIndices = malloc(indice_count * (hint_indice_type == 0 ? sizeof(uint) : sizeof(ushort)));

		for (auto i = 0; i < scene->mNumMeshes; i++)
		{
			auto mesh = scene->mMeshes[i];

			auto i_base = materials[i].indice_base;
			auto i_count = materials[i].indice_count;

			for (auto isem = 0; isem < VertexSemanticsCount; isem++)
			{
				auto &s = vtx_sem_attr[isem];
				auto &d = buffer_datas[s.buffer_index];
				if (s.need)
				{
					switch (VertexSemantics(isem))
					{
						case VertexPosition:
							if (s.has)
							{
								for (auto j = 0; j < i_count; j++)
								{
									*(float*)(d.pVertex + s.offset + (j + i_base) * d.vertex_size) = mesh->mVertices[j].x;
									*(float*)(d.pVertex + s.offset + (j + i_base) * d.vertex_size + sizeof(float)) = mesh->mVertices[j].y;
									*(float*)(d.pVertex + s.offset + (j + i_base) * d.vertex_size + sizeof(float) * 2) = mesh->mVertices[j].z;
								}
							}
							break;
						case VertexUV0: case VertexUV1: case VertexUV2: case VertexUV3:
						case VertexUV4: case VertexUV5: case VertexUV6: case VertexUV7:
							if (s.has)
							{
								auto tc = mesh->mTextureCoords[isem - VertexUV0];
								for (auto j = 0; j < i_count; j++)
								{
									*(float*)(d.pVertex + s.offset + (j + i_base) * d.vertex_size) = tc[j].x;
									*(float*)(d.pVertex + s.offset + (j + i_base) * d.vertex_size + sizeof(float)) = tc[j].y;
								}
							}
							break;
						case VertexNormal:
							if (s.has)
							{
								for (auto j = 0; j < i_count; j++)
								{
									*(float*)(d.pVertex + s.offset + (j + i_base) * d.vertex_size) = mesh->mNormals[j].x;
									*(float*)(d.pVertex + s.offset + (j + i_base) * d.vertex_size + sizeof(float)) = mesh->mNormals[j].y;
									*(float*)(d.pVertex + s.offset + (j + i_base) * d.vertex_size + sizeof(float) * 2) = mesh->mNormals[j].z;
								}
							}
							break;
						case VertexTangent:
							if (s.has)
							{
								for (auto j = 0; j < i_count; j++)
								{
									*(float*)(d.pVertex + s.offset + (j + i_base) * d.vertex_size) = mesh->mTangents[j].x;
									*(float*)(d.pVertex + s.offset + (j + i_base) * d.vertex_size + sizeof(float)) = mesh->mTangents[j].y;
									*(float*)(d.pVertex + s.offset + (j + i_base) * d.vertex_size + sizeof(float) * 2) = mesh->mTangents[j].z;
								}
							}
							break;
						case VertexBitangent:
							if (s.has)
							{
								for (auto j = 0; j < i_count; j++)
								{
									*(float*)(d.pVertex + s.offset + (j + i_base) * d.vertex_size) = mesh->mBitangents[j].x;
									*(float*)(d.pVertex + s.offset + (j + i_base) * d.vertex_size + sizeof(float)) = mesh->mBitangents[j].y;
									*(float*)(d.pVertex + s.offset + (j + i_base) * d.vertex_size + sizeof(float) * 2) = mesh->mBitangents[j].z;
								}
							}
							break;
						case VertexColor:
							if (s.has)
							{
								auto col = mesh->mColors[0];
								for (auto j = 0; j < i_count; j++)
								{
									*(float*)(d.pVertex + s.offset + (j + i_base) * d.vertex_size) = col[j].r;
									*(float*)(d.pVertex + s.offset + (j + i_base) * d.vertex_size + sizeof(float)) = col[j].g;
									*(float*)(d.pVertex + s.offset + (j + i_base) * d.vertex_size + sizeof(float) * 2) = col[j].b;
									*(float*)(d.pVertex + s.offset + (j + i_base) * d.vertex_size + sizeof(float) * 3) = col[j].a;
								}
							}
							break;
						case VertexBoneID:
							mesh->mNumBones
							break;
						case VertexBoneWeight:
							break;
					}
				}
			}
		}

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

