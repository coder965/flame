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
	};

	static void process_node(Model *m, ModelNode *p, aiNode *n)
	{
		auto c = new ModelNode;
		strncpy(c->name, n->mName.data, FLAME_MODEL_NAME_LENGTH);
		memcpy(&c->local_matrix[0][0], &n->mTransformation.a1, sizeof(glm::mat4));
		c->global_matrix = glm::mat4(1.f);

		if (!p)
		{
			m->root_node = c;
			c->parent = nullptr;
		}
		else
		{
			c->parent = p;
			if (p->first_child)
				p->last_child->next_sibling = c;
			else
				p->first_child = p->last_child = c;
			p->last_child = c;
			p->children_count++;
		}

		c->next_sibling = nullptr;
		c->children_count = 0;
		c->first_child = c->last_child = nullptr;

		for (auto i = 0; i < m->mesh_count; i++)
		{
			if (strcmp(m->meshes[i]->name, c->name) == 0)
			{
				c->type = ModelNodeMesh;
				c->p = m->meshes[i];
				m->meshes[i]->pNode = c;

				goto do_recursion;
			}
		}

		for (auto i = 0; i < m->bone_count; i++)
		{
			if (strcmp(m->bones[i]->name, c->name) == 0)
			{
				c->type = ModelNodeBone;
				c->p = m->bones[i];
				m->bones[i]->pNode = c;

				if (!m->root_bone)
					m->root_bone = c;

				goto do_recursion;
			}
		}

		c->type = ModelNodeNode;
		c->p = nullptr;

		do_recursion:

		for (auto i = 0; i < n->mNumChildren; i++)
			process_node(m, c, n->mChildren[i]);
	};

	static void destroy_node(ModelNode *n)
	{
		auto c = n->first_child;
		while (c)
		{
			destroy_node(c);
			c = c->next_sibling;
		}
		delete n;
	}

	Model *load_model(const char *filename,
		int desired_vertex_buffer_count, VertexBufferDescription *desired_vertex_buffers,
		int desired_map_count, MapSemantic *desired_maps,
		IndiceType indice_type,
		int max_bone_count,
		int max_bone_count_per_vertex)
	{
		std::filesystem::path path(filename);
		if (!std::filesystem::exists(path))
			return nullptr;

		auto m = new Model;
		m->bone_count_exceeded = false;
		m->bone_count_per_vertex_exceeded = false;

		Assimp::Importer importer;
		auto scene = importer.ReadFile(filename, aiProcess_Triangulate);

		m->vertex_count = 0;
		m->indice_count = 0;

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

		m->mesh_count = scene->mNumMeshes;
		m->meshes = new ModelMesh*[m->mesh_count];

		m->bone_count = 0;
		m->bones = nullptr;

		for (auto i = 0; i < scene->mNumMeshes; i++)
		{
			auto mesh = scene->mMeshes[i];

			m->meshes[i] = new ModelMesh;
			m->meshes[i]->pNode = nullptr;
			strncpy(m->meshes[i]->name, mesh->mName.data, FLAME_MODEL_NAME_LENGTH);

			m->meshes[i]->material_index = 0;

			//auto mid = mesh->mMaterialIndex;
			//auto mtl = scene->mMaterials[mid];
			//if (mtl->GetTextureCount(aiTextureType_DIFFUSE) > 0)
			//{
			//	aiString ret_name;
			//	mtl->GetTexture(aiTextureType_DIFFUSE, 0, &ret_name);
			//}

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

			m->bone_count += mesh->mNumBones;

			auto ic = mesh->mNumFaces * 3;

			m->meshes[i]->indice_base = m->indice_count;
			m->meshes[i]->indice_count = ic;

			m->vertex_count += mesh->mNumVertices;
			m->indice_count += ic;
		}

		if (m->bone_count > 0)
		{
			vtx_sem_attr[VertexBoneID].has = true;
			vtx_sem_attr[VertexBoneWeight].has = true;
		}

		if (desired_vertex_buffer_count == 0)
		{
			// by default, we accept all vertex semantics and put them all in one buffer

			m->vertex_buffer_count = 1;
			m->vertex_buffers = new ModelVertexBuffer[1];

			m->vertex_buffers[0].semantic_count = 0;
			for (auto i = 0; i < VertexSemanticsCount; i++)
			{
				if (vtx_sem_attr[i].has)
					m->vertex_buffers[0].semantic_count++;
			}

			m->vertex_buffers[0].semantics = new VertexSemantic[m->vertex_buffers[0].semantic_count];

			for (auto i = 0, j = 0; i < VertexSemanticsCount; i++)
			{
				if (vtx_sem_attr[i].has)
				{
					m->vertex_buffers[0].semantics[j] = VertexSemantic(i);
					j++;
				}
			}
		}
		else
		{
			for (auto i = 0; i < desired_vertex_buffer_count; i++)
			{
				if (!desired_vertex_buffers[i].active_if_has_bone || m->bone_count > 0)
					m->vertex_buffer_count++;
			}

			m->vertex_buffers = new ModelVertexBuffer[m->vertex_buffer_count];

			for (auto i = 0, j = 0; i < desired_vertex_buffer_count; i++)
			{
				if (desired_vertex_buffers[i].active_if_has_bone && m->bone_count == 0)
					continue;

				m->vertex_buffers[j].semantic_count = desired_vertex_buffers[i].semantic_count;
				m->vertex_buffers[j].semantics = new VertexSemantic[m->vertex_buffers[j].semantic_count];

				for (auto k = 0; k < desired_vertex_buffers[i].semantic_count; k++)
					m->vertex_buffers[j].semantics[k] = desired_vertex_buffers[i].semantics[k];
			}
		}

		for (auto i = 0; i < m->vertex_buffer_count; i++)
		{
			auto vb = &m->vertex_buffers[i];

			vb->size = 0;
			for (auto j = 0; j < vb->semantic_count; j++)
			{
				auto s = vb->semantics[j];

				if (vtx_sem_attr[s].need)
					assert(0); // vertex semantics cannot repeat
				vtx_sem_attr[s].need = true;
				vtx_sem_attr[s].buffer_index = i;
				vtx_sem_attr[s].offset = vb->size;

				switch (s)
				{
					case VertexPosition:
						vb->size += 3;
						break;
					case VertexUV0: case VertexUV1: case VertexUV2: case VertexUV3:
					case VertexUV4: case VertexUV5: case VertexUV6: case VertexUV7:
						vb->size += 2;
						break;
					case VertexNormal:
						vb->size += 3;
						break;
					case VertexTangent:
						vb->size += 3;
						break;
					case VertexBitangent:
						vb->size += 3;
						break;
					case VertexColor:
						vb->size += 4;
						break;
					case VertexBoneID:
						vb->size += 4; // we treat bone id as float, since there are enough bits for bone count
						break;
					case VertexBoneWeight:
						vb->size += 4;
						break;
				}
			}
		}

		for (auto i = 0; i < m->vertex_buffer_count; i++)
			m->vertex_buffers[i].pVertex = (unsigned char*)malloc(m->vertex_count * m->vertex_buffers[i].size * sizeof(float));
		m->pIndices = (unsigned char*)malloc(m->indice_count * (indice_type == IndiceUint ? sizeof(uint) : sizeof(ushort)));

		std::vector<int> vertex_bone_count;
		if (m->bone_count > 0)
		{
			// do initialize the bone ids and bone weights, since we don't know if there are some vertexs not covered by bones
			auto initialize_ = [&](VertexSemantic sem) {
				auto s = &vtx_sem_attr[sem];
				if (s->need)
				{
					auto d = &m->vertex_buffers[s->buffer_index];
					auto p = (float*)d->pVertex + s->offset;
					for (auto i = 0; i < m->vertex_count; i++)
					{
						p[0] = 0.f;
						p[1] = 0.f;
						p[2] = 0.f;
						p[3] = 0.f;
						p += d->size;
					}
				}
			};
			initialize_(VertexBoneID);
			initialize_(VertexBoneWeight);

			vertex_bone_count.resize(m->vertex_count);

			m->bones = new ModelBone*[m->bone_count];
			for (auto i = 0; i < m->bone_count; i++)
			{
				m->bones[i] = new ModelBone;
				m->bones[i]->pNode = nullptr;
			}
		}

		auto v_base = 0;
		auto i_base = 0;
		auto b_id = 0;

		for (auto i = 0; i < scene->mNumMeshes; i++)
		{
			auto mesh = scene->mMeshes[i];

			for (auto isem = 0; isem < VertexSemanticsCount; isem++)
			{
				auto s = &vtx_sem_attr[isem];
				if (s->need)
				{
					auto d = &m->vertex_buffers[s->buffer_index];
					auto p = (float*)d->pVertex + v_base * d->size + s->offset;
					switch (VertexSemantic(isem))
					{
						case VertexPosition:
							if (s->has)
							{
								for (auto j = 0; j < mesh->mNumVertices; j++)
								{
									p[0] = mesh->mVertices[j].x;
									p[1] = mesh->mVertices[j].y;
									p[2] = mesh->mVertices[j].z;
									p += d->size;
								}
							}
							else
							{
								for (auto j = 0; j < mesh->mNumVertices; j++)
								{
									p[0] = 0.f;
									p[1] = 0.f;
									p[2] = 0.f;
									p += d->size;
								}
							}
							break;
						case VertexUV0: case VertexUV1: case VertexUV2: case VertexUV3:
						case VertexUV4: case VertexUV5: case VertexUV6: case VertexUV7:
							if (s->has)
							{
								auto tc = mesh->mTextureCoords[isem - VertexUV0];
								for (auto j = 0; j < mesh->mNumVertices; j++)
								{
									p[0] = tc[j].x;
									p[1] = tc[j].y;
									p += d->size;
								}
							}
							else
							{
								for (auto j = 0; j < mesh->mNumVertices; j++)
								{
									p[0] = 0.f;
									p[1] = 0.f;
									p += d->size;
								}
							}
							break;
						case VertexNormal:
							if (s->has)
							{
								for (auto j = 0; j < mesh->mNumVertices; j++)
								{
									p[0] = mesh->mNormals[j].x;
									p[1] = mesh->mNormals[j].y;
									p[2] = mesh->mNormals[j].z;
									p += d->size;
								}
							}
							else
							{
								for (auto j = 0; j < mesh->mNumVertices; j++)
								{
									p[0] = 0.f;
									p[1] = 1.f;
									p[2] = 0.f;
									p += d->size;
								}
							}
							break;
						case VertexTangent:
							if (s->has)
							{
								for (auto j = 0; j < mesh->mNumVertices; j++)
								{
									p[0] = mesh->mTangents[j].x;
									p[1] = mesh->mTangents[j].y;
									p[2] = mesh->mTangents[j].z;
									p += d->size;
								}
							}
							else
							{
								for (auto j = 0; j < mesh->mNumVertices; j++)
								{
									p[0] = 1.f;
									p[1] = 0.f;
									p[2] = 0.f;
									p += d->size;
								}
							}
							break;
						case VertexBitangent:
							if (s->has)
							{
								for (auto j = 0; j < mesh->mNumVertices; j++)
								{
									p[0] = mesh->mBitangents[j].x;
									p[1] = mesh->mBitangents[j].y;
									p[2] = mesh->mBitangents[j].z;
									p += d->size;
								}
							}
							else
							{
								for (auto j = 0; j < mesh->mNumVertices; j++)
								{
									p[0] = 0.f;
									p[1] = 0.f;
									p[2] = 1.f;
									p += d->size;
								}
							}
							break;
						case VertexColor:
							if (s->has)
							{
								auto col = mesh->mColors[0];
								for (auto j = 0; j < mesh->mNumVertices; j++)
								{
									p[0] = col[j].r;
									p[1] = col[j].g;
									p[2] = col[j].b;
									p[3] = col[j].a;
									p += d->size;
								}
							}
							else
							{
								for (auto j = 0; j < mesh->mNumVertices; j++)
								{
									p[0] = 1.f;
									p[1] = 0.f;
									p[2] = 1.f;
									p[3] = 1.f;
									p += d->size;
								}
							}
							break;
					}
				}
			}

			if (m->bone_count > 0)
			{
				float *pBoneID = nullptr;
				int strid1;
				float *pBoneWeight = nullptr;
				int strid2;
				{
					auto s = &vtx_sem_attr[VertexBoneID];
					if (s->need)
					{
						auto d = &m->vertex_buffers[s->buffer_index];
						strid1 = d->size;
						pBoneID = (float*)d->pVertex + v_base * strid1 + s->offset;
					}
				}
				{
					auto s = &vtx_sem_attr[VertexBoneWeight];
					if (s->need)
					{
						auto d = &m->vertex_buffers[s->buffer_index];
						strid2 = d->size;
						pBoneWeight = (float*)d->pVertex + v_base * strid2 + s->offset;
					}
				}
				for (auto j = 0; j < mesh->mNumBones; j++)
				{
					auto b = mesh->mBones[j];
					for (auto k = 0; k < b->mNumWeights; k++)
					{
						auto vid = b->mWeights[k].mVertexId;
						auto &bc = vertex_bone_count[v_base + vid];
						if (bc < 4)
						{
							if (pBoneID)
								pBoneID[vid * strid1 + bc] = b_id;
							if (pBoneWeight)
								pBoneWeight[vid * strid2 + bc] = b->mWeights[k].mWeight;
							bc++;
						}
						else
							m->bone_count_per_vertex_exceeded = true;
					}

					strncpy(m->bones[b_id]->name, b->mName.data, FLAME_MODEL_NAME_LENGTH);
					memcpy(m->bones[b_id]->offset_matrix, &b->mOffsetMatrix, sizeof(float) * 16);

					b_id++;
				}
			}

			v_base += mesh->mNumVertices;

			if (indice_type == IndiceUint)
			{
				auto p = (uint*)m->pIndices + i_base;
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
				auto p = (ushort*)m->pIndices + i_base;
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

		m->root_bone = nullptr;

		process_node(m, nullptr, scene->mRootNode);

		return m;
	}

	void save_model(Model *m, const char *filename)
	{

	}

	void release_model(Model *m)
	{
		for (auto i = 0; i < m->vertex_buffer_count; i++)
		{
			delete[]m->vertex_buffers[i].semantics;
			delete[]m->vertex_buffers[i].pVertex;
		}
		delete[]m->vertex_buffers;
		delete[]m->pIndices;

		for (auto i = 0; i < m->mesh_count; i++)
			delete m->meshes[i];
		delete[]m->meshes;

		for (auto i = 0; i < m->bone_count; i++)
			delete m->bones[i];
		delete[]m->bones;

		destroy_node(m->root_node);

		delete m;
	}
}

