#include <sstream>
#include <regex>
#include <iostream>
#include <algorithm>
#include <map>
#include <deque>
#include <tuple>
#include "../file_utils.h"
#include "../string_utils.h"
#include "../resource/resource.h"
#include "../graphics/buffer.h"
#include "../graphics/image.h"
#include "../graphics/sampler.h"
#include "../graphics/pipeline.h"
#include "../graphics/material.h"
#include "model.h"
#include "animation.h"
#include "../physics/physics.h"

namespace tke
{
	std::map<unsigned int, std::weak_ptr<Model>> _models;
	void _create_vertex_and_index_buffer()
	{
		auto vertex_stat_count = 0;
		auto vertex_anim_count = 0;
		auto indice_count = 0;

		for (auto &m : _models)
		{
			auto s = m.second.lock();
			if (s)
			{
				vertex_stat_count += s->vertexes.size();
				if (s->vertexes_skeleton.size() > 0)
					vertex_anim_count += s->vertexes.size();
				indice_count += s->indices.size();
			}
		}

		auto vss = vertex_stat_count > 0 ? sizeof(ModelVertex) * vertex_stat_count : 1;
		auto vas = vertex_anim_count > 0 ? sizeof(ModelVertexSkeleton) * vertex_anim_count : 1;
		auto is = indice_count > 0 ? sizeof(int) * indice_count : 1;

		vertex_static_buffer = std::make_unique<VertexBuffer>(vss);
		vertex_skeleton_Buffer = std::make_unique<VertexBuffer>(vas);
		index_buffer = std::make_unique<IndexBuffer>(is);

		auto total_size = vss + vas + is;
		StagingBuffer stagingBuffer(total_size);

		auto vso = 0;
		auto vao = vso + vss;
		auto io = vao + vas;
		unsigned char *map = (unsigned char*)stagingBuffer.map(0, total_size);
		auto vs_map = map + vso;
		auto va_map = map + vao;
		auto i_map = map + io;
		auto vertex_offset = 0;
		auto indice_offset = 0;
		for (auto &m : _models)
		{
			auto s = m.second.lock();
			if (s)
			{
				if (s->vertexes_skeleton.size() > 0)
				{
					s->vertex_base = vertex_offset;
					s->indice_base = indice_offset;
					memcpy(vs_map + vertex_offset * sizeof(ModelVertex), s->vertexes.data(), sizeof(ModelVertex) * s->vertexes.size());
					memcpy(va_map + vertex_offset * sizeof(ModelVertexSkeleton), s->vertexes_skeleton.data(), sizeof(ModelVertexSkeleton) * s->vertexes.size());
					memcpy(i_map + indice_offset * sizeof(int), s->indices.data(), sizeof(int) * s->indices.size());
					vertex_offset += s->vertexes.size();
					indice_offset += s->indices.size();
				}
			}
		}
		for (auto &m : _models)
		{
			auto s = m.second.lock();
			if (s)
			{
				if (!s->vertexes_skeleton.size() > 0)
				{
					s->vertex_base = vertex_offset;
					s->indice_base = indice_offset;
					memcpy(vs_map + vertex_offset * sizeof(ModelVertex), s->vertexes.data(), sizeof(ModelVertex) * s->vertexes.size());
					memcpy(i_map + indice_offset * sizeof(int), s->indices.data(), sizeof(int) * s->indices.size());
					vertex_offset += s->vertexes.size();
					indice_offset += s->indices.size();
				}
			}
		}
		stagingBuffer.unmap();

		if (vertex_stat_count > 0)
		{
			VkBufferCopy range = {};
			range.srcOffset = vso;
			range.dstOffset = 0;
			range.size = sizeof(ModelVertex) * vertex_stat_count;
			stagingBuffer.copyTo(vertex_static_buffer.get(), 1, &range);
		}
		if (vertex_anim_count > 0)
		{
			VkBufferCopy range = {};
			range.srcOffset = vao;
			range.dstOffset = 0;
			range.size = sizeof(ModelVertexSkeleton) * vertex_anim_count;
			stagingBuffer.copyTo(vertex_skeleton_Buffer.get(), 1, &range);
		}
		if (indice_count > 0)
		{
			VkBufferCopy range = {};
			range.srcOffset = io;
			range.dstOffset = 0;
			range.size = sizeof(int) * indice_count;
			stagingBuffer.copyTo(index_buffer.get(), 1, &range);
		}
	}

	void Model::UV::add(const glm::vec2 &v)
	{
		int index = -1;
		for (int i = 0; i < unique.size(); i++)
		{
			if (is_same(unique[i], v))
			{
				index = i;
				break;
			}
		}
		if (index == -1)
		{
			index = unique.size();
			unique.push_back(v);
		}
		indices.push_back(index);
	}

	void Model::add_vertex_position_normal(const glm::vec3 &position, const glm::vec3 &normal)
	{
		ModelVertex vertex;
		vertex.position = position;
		vertex.normal = normal;
		vertex.tangent = glm::vec3(0.f);
		vertex.uv = glm::vec2(0.f);
		vertexes.push_back(vertex);
	}

	void Model::create_geometry_aux()
	{
		if (geometry_aux)
			return;

		auto triangle_count = indices.size() / 3;

		geometry_aux = std::make_unique<GeometryAux>();
		geometry_aux->triangles = std::make_unique<GeometryAux::Triangle[]>(triangle_count);
		for (int i = 0; i < triangle_count; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				auto ii = indices[i * 3 + j];

				auto idx = -1;
				for (int k = 0; k < geometry_aux->unique_vertex.size(); k++)
				{
					if (is_same(vertexes[ii].position, geometry_aux->unique_vertex[k]))
					{
						idx = k;
						break;
					}
				}
				if (idx == -1)
				{
					idx = geometry_aux->unique_vertex.size();
					geometry_aux->unique_vertex.push_back(vertexes[ii].position);
				}
				geometry_aux->triangles[i].indices[j] = idx;
				geometry_aux->triangles[i].adjacency[j].first = -1;
				geometry_aux->triangles[i].adjacency[j].second = -1;
			}
		}
		for (int i = 0; i < triangle_count; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				if (geometry_aux->triangles[i].adjacency[j].first == -1)
				{
					bool ok = false;

					auto i0 = geometry_aux->triangles[i].indices[j];
					auto i1 = geometry_aux->triangles[i].indices[(j + 1) % 3];
					for (int k = 0; k < triangle_count; k++)
					{
						if (i == k)
							continue;

						for (int l = 0; l < 3; l++)
						{
							if (geometry_aux->triangles[k].adjacency[l].first == -1)
							{
								auto i2 = geometry_aux->triangles[k].indices[l];
								auto i3 = geometry_aux->triangles[k].indices[(l + 1) % 3];
								if (i0 == i3 && i1 == i2)
								{
									geometry_aux->triangles[k].adjacency[l] = { i, (j + 2) % 3 };
									geometry_aux->triangles[i].adjacency[j] = { k, (l + 2) % 3 };
									ok = true;
									break;
								}
							}
						}

						if (ok)
							break;
					}
				}
			}
		}
	}

	void Model::create_uv()
	{
		create_geometry_aux();

		auto aux = geometry_aux.get();

		float min_x = 0.f, max_x = 0.f, min_z = 0.f, max_z = 0.f;

		auto triangle_count = indices.size() / 3;

		std::vector<int> remain_triangles;
		remain_triangles.resize(triangle_count - 1);
		for (int i = 0; i < triangle_count - 1; i++)
			remain_triangles[i] = i + 1;

		static const auto up_dir = glm::vec3(0.f, 1.f, 0.f);

		std::deque<std::tuple<int, int, glm::vec4>> jobs;

		auto uv = new UV;
		uv->name = "new uv";

		uv->series.resize(1);
		uv->series.back().first = 0;

		static  const auto fUnfoldTri = [&](std::tuple<int, int, glm::vec4> &_d) {
			auto tri_idx = std::get<0>(_d); 
			auto swizzle_base = std::get<1>(_d); 
			auto base = std::get<2>(_d);
			int indices[3];
			glm::vec3 positions[3];
			for (int i = 0; i < 3; i++)
			{
				indices[i] = aux->triangles[tri_idx].indices[i];
				positions[i] = aux->unique_vertex[indices[i]];
			}
			glm::ivec3 swizzle;
			swizzle.z = swizzle_base;
			swizzle.x = (swizzle_base + 1) % 3;
			swizzle.y = (swizzle_base + 2) % 3;
			auto v0 = glm::normalize(positions[swizzle[0]] - positions[swizzle[1]]);
			auto v1 = glm::normalize(positions[swizzle[2]] - positions[swizzle[1]]);
			auto src_mat_inv = glm::inverse(make_matrix(v0, glm::normalize(glm::cross(v1, v0)), positions[swizzle[1]]));
			glm::vec2 uv_temp[3];
			auto dst_mat = make_matrix(glm::vec3(base.x, 0.f, base.y), up_dir, glm::vec3(base.z, 0.f, base.w));
			for (int i = 0; i < 3; i++)
			{
				auto p = src_mat_inv * glm::vec4(positions[i], 1.f);
				p = dst_mat * p;
				uv_temp[i].x = p.x;
				uv_temp[i].y = p.z;
				if (p.x < min_x)
					min_x = p.x;
				if (p.x > max_x)
					max_x = p.x;
				if (p.z < min_z)
					min_z = p.z;
				if (p.z > max_z)
					max_z = p.z;
				uv->add(uv_temp[i]);
			}

			for (int i = 0; i < 3; i++)
			{
				auto adj_idx = aux->triangles[tri_idx].adjacency[i];
				if (adj_idx.first != -1)
				{
					auto it = std::find(remain_triangles.begin(), remain_triangles.end(), adj_idx.first);
					if (it != remain_triangles.end())
					{
						remain_triangles.erase(it);
						jobs.emplace_back(adj_idx.first, adj_idx.second, 
							glm::vec4(glm::normalize(uv_temp[(i + 1) % 3] - uv_temp[i]), uv_temp[i]));
					}
				}
			}
		};

		jobs.emplace_back(0, 2, glm::vec4(1.f, 0.f, 0.f, 0.f));
		do
		{
			fUnfoldTri(jobs.front());
			jobs.pop_front();
			if (jobs.size() == 0)
			{
				if (remain_triangles.size() > 0)
				{
					uv->series.back().second = uv->indices.size();
					uv->series.resize(uv->series.size() + 1);
					uv->series.back().first = uv->indices.size();
					remain_triangles.erase(remain_triangles.begin());
					jobs.emplace_back(remain_triangles[0], 2, glm::vec4(1.f, 0.f, 0.f, 0.f));
				}
				else
					break;
			}
		}while (true);

		uv->series.back().second = uv->indices.size();

		auto cx = max_x - min_x;
		auto cz = max_z - min_z;
		for (int i = 0; i < uv->unique.size(); i++)
		{
			auto &v = uv->unique[i];
			v.x -= min_x;
			v.y -= min_z;
			v.x /= cx;
			v.y /= cz;
		}

		uvs.emplace_back(uv);
	}

	void Model::remove_uv(UV *uv)
	{
		for (auto it = uvs.begin(); it != uvs.end(); it++)
		{
			if ((*it).get() == uv)
			{
				if (geometry_uv == uv)
					assign_uv_to_geometry(nullptr);
				if (bake_uv == uv)
					assign_uv_to_bake(nullptr);
				uvs.erase(it);
				return;
			}
		}
	}

	void Model::assign_uv_to_geometry(UV *uv)
	{
		if (geometry_uv == uv)
			return;

		geometry_uv = uv;

		std::vector<ModelVertex> new_vertexes;
		std::vector<ModelVertexSkeleton> new_vertexes_skeleton;
		std::vector<int> new_indices;

		for (int i = 0; i < indices.size(); i++)
		{
			int index = -1;
			for (int j = 0; j < new_vertexes.size(); j++)
			{
				;
			}
		}

		_create_vertex_and_index_buffer();
	}

	void Model::assign_uv_to_bake(UV *uv)
	{
		if (bake_uv == uv)
			return;

		bake_uv = uv;
	}

	void Model::set_state_animation(ModelStateAnimationKind kind, std::shared_ptr<AnimationBinding> b)
	{
		stateAnimations[kind] = b;
		switch (kind)
		{
			case ModelStateAnimationStand:
				stand_animation_filename = b ? b->animation->filename : "";
				break;
			case ModelStateAnimationForward:
				forward_animation_filename = b ? b->animation->filename : "";
				break;
			case ModelStateAnimationBackward:
				backward_animation_filename = b ? b->animation->filename : "";
				break;
			case ModelStateAnimationLeftward:
				leftward_animation_filename = b ? b->animation->filename : "";
				break;
			case ModelStateAnimationRightward:
				rightward_animation_filename = b ? b->animation->filename : "";
				break;
			case ModelStateAnimationJump:
				jump_animation_filename = b ? b->animation->filename : "";
				break;
		}
	}

	Bone *Model::new_bone()
	{
		auto b = new Bone;
		bones.emplace_back(b);
		return b;
	}

	void Model::remove_bone(Bone *b)
	{
		for (auto it = bones.begin(); it != bones.end(); it++)
		{
			if (it->get() == b)
			{
				bones.erase(it);
				return;
			}
		}
	}

	BoneIK *Model::new_bone_ik()
	{
		auto b = new BoneIK;
		iks.emplace_back(b);
		return b;
	}

	void Model::remove_bone_ik(BoneIK *b)
	{
		for (auto it = iks.begin(); it != iks.end(); it++)
		{
			if (it->get() == b)
			{
				iks.erase(it);
				return;
			}
		}
	}

	Rigidbody *Model::new_rigidbody()
	{
		auto r = new Rigidbody;
		rigidbodies.emplace_back(r);
		return r;
	}

	void Model::remove_rigidbody(Rigidbody *r)
	{
		for (auto it = rigidbodies.begin(); it != rigidbodies.end(); it++)
		{
			if (it->get() == r)
			{
				rigidbodies.erase(it);
				return;
			}
		}
	}

	Joint *Model::new_joint()
	{
		auto j = new Joint;
		joints.emplace_back(j);
		return j;
	}

	void Model::remove_joint(Joint *j)
	{
		for (auto it = joints.begin(); it != joints.end(); it++)
		{
			if (it->get() == j)
			{
				joints.erase(it);
				return;
			}
		}
	}

	std::unique_ptr<VertexBuffer> vertex_static_buffer;
	std::unique_ptr<VertexBuffer> vertex_skeleton_Buffer;
	std::unique_ptr<IndexBuffer> index_buffer;

	void add_triangle_vertex(Model *m, glm::mat3 rotation, glm::vec3 center)
	{
		int baseVertex = m->vertexes.size();

		m->add_vertex_position_normal(center + rotation * glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f));
		m->add_vertex_position_normal(center + rotation * glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f));
		m->add_vertex_position_normal(center + rotation * glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 1.f));
		for (int i = 0; i < 3; i++)
			m->indices.push_back(baseVertex + i);
	}

	void add_cube_vertex(Model *m, glm::mat3 rotation, glm::vec3 center, float length)
	{
		int baseVertex = m->vertexes.size();

		glm::vec3 a = center + rotation * (glm::vec3(0.5f, -0.5f, 0.5f) * length);
		glm::vec3 b = center + rotation * (glm::vec3(0.5f, -0.5f, -0.5f) * length);
		glm::vec3 c = center + rotation * (glm::vec3(0.5f, 0.5f, -0.5f) * length);
		glm::vec3 d = center + rotation * (glm::vec3(0.5f, 0.5f, 0.5f) * length);
		glm::vec3 e = center + rotation * (glm::vec3(-0.5f, -0.5f, 0.5f) * length);
		glm::vec3 f = center + rotation * (glm::vec3(-0.5f, -0.5f, -0.5f) * length);
		glm::vec3 g = center + rotation * (glm::vec3(-0.5f, 0.5f, -0.5f) * length);
		glm::vec3 h = center + rotation * (glm::vec3(-0.5f, 0.5f, 0.5f) * length);

		m->add_vertex_position_normal(a, glm::vec3(1.f, 0.f, 0.f));
		m->add_vertex_position_normal(b, glm::vec3(1.f, 0.f, 0.f));
		m->add_vertex_position_normal(c, glm::vec3(1.f, 0.f, 0.f));
		m->add_vertex_position_normal(d, glm::vec3(1.f, 0.f, 0.f));
		m->add_vertex_position_normal(e, glm::vec3(-1.f, 0.f, 0.f));
		m->add_vertex_position_normal(f, glm::vec3(-1.f, 0.f, 0.f));
		m->add_vertex_position_normal(g, glm::vec3(-1.f, 0.f, 0.f));
		m->add_vertex_position_normal(h, glm::vec3(-1.f, 0.f, 0.f));
		m->add_vertex_position_normal(c, glm::vec3(0.f, 1.f, 0.f));
		m->add_vertex_position_normal(d, glm::vec3(0.f, 1.f, 0.f));
		m->add_vertex_position_normal(g, glm::vec3(0.f, 1.f, 0.f));
		m->add_vertex_position_normal(h, glm::vec3(0.f, 1.f, 0.f));
		m->add_vertex_position_normal(a, glm::vec3(0.f, -1.f, 0.f));
		m->add_vertex_position_normal(b, glm::vec3(0.f, -1.f, 0.f));
		m->add_vertex_position_normal(e, glm::vec3(0.f, -1.f, 0.f));
		m->add_vertex_position_normal(f, glm::vec3(0.f, -1.f, 0.f));
		m->add_vertex_position_normal(a, glm::vec3(0.f, 0.f, 1.f));
		m->add_vertex_position_normal(d, glm::vec3(0.f, 0.f, 1.f));
		m->add_vertex_position_normal(e, glm::vec3(0.f, 0.f, 1.f));
		m->add_vertex_position_normal(h, glm::vec3(0.f, 0.f, 1.f));
		m->add_vertex_position_normal(b, glm::vec3(0.f, 0.f, -1.f));
		m->add_vertex_position_normal(c, glm::vec3(0.f, 0.f, -1.f));
		m->add_vertex_position_normal(f, glm::vec3(0.f, 0.f, -1.f));
		m->add_vertex_position_normal(g, glm::vec3(0.f, 0.f, -1.f));

		std::vector<int> list = {
			3, 0, 1, 3, 1, 2,
			6, 5, 4, 6, 4, 7,
			11, 9, 8, 11, 8, 10,
			12, 14, 15, 12, 15, 13,
			19, 18, 16, 19, 16, 17,
			21, 20, 22, 21, 22, 23
		};

		for (auto &i : list)
			i += baseVertex;

		m->indices.insert(m->indices.end(), list.begin(), list.end());
	}

	void add_sphere_vertex(Model *m, glm::mat3 rotation, glm::vec3 center, float radius, int horiSubdiv, int vertSubdiv)
	{
		std::vector<std::vector<int>> indexs;
		indexs.resize(horiSubdiv + 1);

		for (int level = 1; level < horiSubdiv; level++)
		{
			for (int i = 0; i < vertSubdiv; i++)
			{
				auto radian = glm::radians(level * 180.f / horiSubdiv - 90.f);
				auto ringRadius = cos(radian) * radius;
				auto height = sin(radian) * radius;
				auto ang = glm::radians(i * 360.f / vertSubdiv);
				indexs[level].push_back(m->vertexes.size());
				glm::vec3 v = rotation * glm::vec3(cos(ang) * ringRadius, height, sin(ang) * ringRadius);
				m->add_vertex_position_normal(center + v, glm::normalize(v));
			}
		}

		{
			indexs[0].push_back(m->vertexes.size());
			glm::vec3 v = rotation * glm::vec3(0.f, -radius, 0.f);
			m->add_vertex_position_normal(center + v, glm::normalize(v));
		}

		{
			indexs[horiSubdiv].push_back(m->vertexes.size());
			glm::vec3 v = rotation * glm::vec3(0.f, radius, 0.f);
			m->add_vertex_position_normal(center + v, glm::normalize(v));
		}

		for (int level = 0; level < horiSubdiv; level++)
		{
			if (level == 0)
			{
				for (int i = 0; i < vertSubdiv; i++)
				{
					auto ii = i + 1; if (ii == vertSubdiv) ii = 0;

					m->indices.push_back(indexs[0][0]);
					m->indices.push_back(indexs[1][i]);
					m->indices.push_back(indexs[1][ii]);
				}
			}
			else if (level == horiSubdiv - 1)
			{
				for (int i = 0; i < vertSubdiv; i++)
				{
					auto ii = i + 1; if (ii == vertSubdiv) ii = 0;

					m->indices.push_back(indexs[horiSubdiv - 1][i]);
					m->indices.push_back(indexs[horiSubdiv][0]);
					m->indices.push_back(indexs[horiSubdiv - 1][ii]);
				}
			}
			else
			{
				for (int i = 0; i < vertSubdiv; i++)
				{
					auto ii = i + 1; if (ii == vertSubdiv) ii = 0;

					m->indices.push_back(indexs[level][i]);
					m->indices.push_back(indexs[level + 1][i]);
					m->indices.push_back(indexs[level][ii]);

					m->indices.push_back(indexs[level][ii]);
					m->indices.push_back(indexs[level + 1][i]);
					m->indices.push_back(indexs[level + 1][ii]);
				}
			}
		}
	}

	void add_cylinder_vertex(Model *m, glm::mat3 rotation, glm::vec3 center, float halfHeight, float radius, int subdiv)
	{
		std::vector<std::vector<int>> indexs;
		indexs.resize(4);

		// top cap
		for (int i = 0; i < subdiv; i++)
		{
			auto ang = glm::radians(i * 360.f / subdiv);
			indexs[0].push_back(m->vertexes.size());
			m->add_vertex_position_normal(rotation * glm::vec3(cos(ang) * radius, halfHeight, sin(ang) * radius) + 
				center, rotation * glm::vec3(0.f, 1.f, 0.f));
		}

		// bottom cap
		for (int i = 0; i < subdiv; i++)
		{
			auto ang = glm::radians(i * 360.f / subdiv);
			indexs[1].push_back(m->vertexes.size());
			m->add_vertex_position_normal(rotation * glm::vec3(cos(ang) * radius, -halfHeight, sin(ang) * radius) + 
				center, rotation * glm::vec3(0.f, -1.f, 0.f));
		}

		// top
		for (int i = 0; i < subdiv; i++)
		{
			auto ang = glm::radians(i * 360.f / subdiv);
			indexs[2].push_back(m->vertexes.size());
			m->add_vertex_position_normal(rotation * glm::vec3(cos(ang) * radius, halfHeight, sin(ang) * radius) + 
				center, rotation * glm::vec3(cos(ang), 0.f, sin(ang)));
		}

		// bottom
		for (int i = 0; i < subdiv; i++)
		{
			auto ang = glm::radians(i * 360.f / subdiv);
			indexs[3].push_back(m->vertexes.size());
			m->add_vertex_position_normal(rotation * glm::vec3(cos(ang) * radius, -halfHeight, sin(ang) * radius) + 
				center, rotation * glm::vec3(cos(ang), 0.f, sin(ang)));
		}

		// top cap
		for (int i = 1; i < subdiv - 1; i++)
		{
			m->indices.push_back(indexs[0][i]);
			m->indices.push_back(indexs[0][0]);
			m->indices.push_back(indexs[0][i + 1]);
		}

		// bottom cap
		for (int i = 1; i < subdiv - 1; i++)
		{
			m->indices.push_back(indexs[1][i + 1]);
			m->indices.push_back(indexs[1][0]);
			m->indices.push_back(indexs[1][i]);
		}

		// middle
		for (int i = 0; i < subdiv; i++)
		{
			auto ii = i + 1;
			if (ii == subdiv)
				ii = 0;

			m->indices.push_back(indexs[2][i]);
			m->indices.push_back(indexs[2][ii]);
			m->indices.push_back(indexs[3][i]);

			m->indices.push_back(indexs[2][ii]);
			m->indices.push_back(indexs[3][ii]);
			m->indices.push_back(indexs[3][i]);
		}
	}

	void add_cone_vertex(Model *m, glm::mat3 rotation, glm::vec3 center, float height, float radius, int subdiv)
	{
		std::vector<std::vector<int>> indexs;
		indexs.resize(3);

		// top
		{
			indexs[0].push_back(m->vertexes.size());
			m->add_vertex_position_normal(rotation * glm::vec3(0.f, height, 0.f) + center, rotation * glm::vec3(0.f, 1.f, 0.f));
		}

		// cap
		for (int i = 0; i < subdiv; i++)
		{
			auto ang = glm::radians(i * 360.f / subdiv);
			indexs[1].push_back(m->vertexes.size());
			m->add_vertex_position_normal(rotation * glm::vec3(cos(ang) * radius, 0.f, sin(ang) * radius) + center, rotation * 
				glm::vec3(0.f, -1.f, 0.f));
		}

		for (int i = 0; i < subdiv; i++)
		{
			auto ang = glm::radians(i * 360.f / subdiv);
			indexs[2].push_back(m->vertexes.size());
			m->add_vertex_position_normal(rotation * glm::vec3(cos(ang) * radius, 0.f, sin(ang) * radius) + center, rotation * 
				glm::vec3(cos(ang), 0.f, sin(ang)));
		}

		for (int i = 0; i < subdiv; i++)
		{
			auto ii = i + 1;
			if (ii == subdiv)
				ii = 0;

			m->indices.push_back(indexs[2][i]);
			m->indices.push_back(indexs[0][0]);
			m->indices.push_back(indexs[2][ii]);
		}

		for (int i = 1; i < subdiv - 1; i++)
		{
			m->indices.push_back(indexs[1][i + 1]);
			m->indices.push_back(indexs[1][0]);
			m->indices.push_back(indexs[1][i]);
		}
	}

	void add_torus_vertex(Model *m, glm::mat3 rotation, glm::vec3 center, float radius, float sectionRadius, int axisSubdiv, 
		int heightSubdiv)
	{
		std::vector<std::vector<int>> indexs;
		indexs.resize(axisSubdiv);

		for (int i = 0; i < axisSubdiv; i++)
		{
			float ang = i * 360.f / axisSubdiv;
			glm::mat3 R = glm::mat3(glm::rotate(glm::radians(-ang), glm::vec3(0.f, 1.f, 0.f)));
			for (int j = 0; j < heightSubdiv; j++)
			{
				auto secang = glm::radians(j * 360.f / heightSubdiv);
				indexs[i].push_back(m->vertexes.size());
				m->add_vertex_position_normal(rotation * (center + R * (glm::vec3(cos(secang) * sectionRadius + radius, 
					sin(secang) * sectionRadius, 0.f))), rotation * R * glm::vec3(cos(secang), sin(secang), 0.f));
			}
		}

		for (int i = 0; i < axisSubdiv; i++)
		{
			auto ii = i + 1; if (ii == axisSubdiv) ii = 0;

			for (int j = 0; j < heightSubdiv; j++)
			{
				auto jj = j + 1; if (jj == heightSubdiv) jj = 0;

				m->indices.push_back(indexs[i][j]);
				m->indices.push_back(indexs[i][jj]);
				m->indices.push_back(indexs[ii][j]);

				m->indices.push_back(indexs[i][jj]);
				m->indices.push_back(indexs[ii][jj]);
				m->indices.push_back(indexs[ii][j]);
			}
		}
	}

	std::shared_ptr<Model> triangleModel;
	std::shared_ptr<Model> cubeModel;
	std::shared_ptr<Model> sphereModel;
	std::shared_ptr<Model> cylinderModel;
	std::shared_ptr<Model> coneModel;
	std::shared_ptr<Model> arrowModel;
	std::shared_ptr<Model> torusModel;
	std::shared_ptr<Model> hamerModel;

	static void _process_model(Model *m, bool generateTangent)
	{
		if (m->vertexes.size() > 0)
		{
			m->max_coord = m->vertexes[0].position;
			m->min_coord = m->vertexes[0].position;
			for (int i = 1; i < m->vertexes.size(); i++)
			{
				m->max_coord = glm::max(m->max_coord, m->vertexes[i].position);
				m->min_coord = glm::min(m->min_coord, m->vertexes[i].position);
			}
		}

		if (generateTangent)
		{
			for (int i = 0; i < m->vertexes.size(); i += 3)
			{
				int id[3] = {
					m->indices[i],
					m->indices[i + 1],
					m->indices[i + 2]
				};

				auto u0 = m->vertexes[id[1]].uv.s - m->vertexes[id[0]].uv.s;
				auto v0 = m->vertexes[id[1]].uv.t - m->vertexes[id[0]].uv.t;

				auto u1 = m->vertexes[id[2]].uv.s - m->vertexes[id[0]].uv.s;
				auto v1 = m->vertexes[id[2]].uv.t - m->vertexes[id[0]].uv.t;

				auto e0 = m->vertexes[id[1]].position - m->vertexes[id[0]].position;
				auto e1 = m->vertexes[id[2]].position - m->vertexes[id[0]].position;

				auto d = u0 * v1 - u1 * v0;
				if (d == 0.f) 
					continue;

				auto tangent = glm::vec3(v1 * e0.x - v0 * e1.x, v1 * e0.y - v0 * e1.y, v1 * e0.z - v0 * e1.z);
				if (glm::length(tangent) > 0.f)
				{
					tangent = glm::normalize(tangent);
					m->vertexes[id[0]].tangent = tangent;
					m->vertexes[id[1]].tangent = tangent;
					m->vertexes[id[2]].tangent = tangent;
				}
				else
				{
					m->vertexes[id[0]].tangent = glm::vec3(0.f);
					m->vertexes[id[1]].tangent = glm::vec3(0.f);
					m->vertexes[id[2]].tangent = glm::vec3(0.f);
				}
			}
		}

		for (int i = 0; i < m->bones.size(); i++)
		{
			m->bones[i]->relateCoord = m->bones[i]->rootCoord;
			int parentID = m->bones[i]->parent;
			if (parentID != -1)
			{
				m->bones[i]->relateCoord -= m->bones[parentID]->rootCoord;
				m->bones[parentID]->children.push_back(i);
			}
		}

		if (m->vertexes_skeleton.size())
		{
			auto a = getAnimation(m->stand_animation_filename);
			if (a) 
				m->stateAnimations[ModelStateAnimationStand] = get_animation_binding(m, a);
			a = getAnimation(m->forward_animation_filename);
			if (a) 
				m->stateAnimations[ModelStateAnimationForward] = get_animation_binding(m, a);
			a = getAnimation(m->backward_animation_filename);
			if (a) 
				m->stateAnimations[ModelStateAnimationBackward] = get_animation_binding(m, a);
			a = getAnimation(m->leftward_animation_filename);
			if (a) 
				m->stateAnimations[ModelStateAnimationLeftward] = get_animation_binding(m, a);
			a = getAnimation(m->rightward_animation_filename);
			if (a) 
				m->stateAnimations[ModelStateAnimationRightward] = get_animation_binding(m, a);
			a = getAnimation(m->jump_animation_filename);
			if (a) 
				m->stateAnimations[ModelStateAnimationJump] = get_animation_binding(m, a);
		}
	}

	namespace OBJ
	{
		void load(Model *m, const std::string &filename)
		{
			std::ifstream file(filename);

			Geometry *currentGeometry = nullptr;

			std::vector<glm::vec3> temp_positions;
			std::vector<glm::vec2> temp_uvs;
			std::vector<glm::vec3> temp_normals;
			std::vector<glm::ivec3> temp_indices;

			while (!file.eof())
			{
				std::string line;
				std::getline(file, line);

				std::stringstream ss(line);
				std::string token;
				ss >> token;
				if (token == "v")
				{
					glm::vec3 v;
					ss >> v.x;
					ss >> v.y;
					ss >> v.z;
					temp_positions.push_back(v);
				}
				else if (token == "vn")
				{
					glm::vec3 n;
					ss >> n.x;
					ss >> n.y;
					ss >> n.z;
					temp_normals.push_back(n);
				}
				else if (token == "vt")
				{
					glm::vec2 t;
					ss >> t.x;
					ss >> t.y;
					temp_uvs.push_back(t);
				}
				else if (token == "f")
				{
					for (int i = 0; i < 3; i++)
					{
						std::string token;
						ss >> token;

						std::regex pattern(R"(([0-9]+)?/([0-9]+)?/([0-9]+)?)");
						std::smatch match;
						std::regex_search(token, match, pattern);

						glm::ivec3 ids;
						for (int j = 0; j < 3; j++)
							ids[j] = match[j + 1].matched ? std::stoi(match[j + 1].str()) - 1 : -1;

						int index = -1;
						for (int j = 0; j < temp_indices.size(); j++)
						{
							if (ids == temp_indices[j])
							{
								index = j;
								break;
							}
						}
						if (index == -1)
						{
							index = m->vertexes.size();
							m->vertexes.push_back({
								temp_positions[ids[0]],
								ids[1] != -1 ? temp_uvs[ids[1]] : glm::vec2(0.f),
								ids[2] != -1 ? temp_normals[ids[2]] : glm::vec3(0.f),
								glm::vec3(0.f)
							});
							temp_indices.push_back(ids);

						}
						m->indices.push_back(index);
						currentGeometry->indiceCount++;
					}
				}
				else if (token == "usemtl")
				{
					std::string name;
					ss >> name;
					auto g = new Geometry;
					currentGeometry = g;
					currentGeometry->material = getMaterial(name);
					currentGeometry->indiceBase = m->indices.size();
					m->geometries.emplace_back(g);
				}
				else if (token == "mtllib")
				{
					std::string libName;
					ss >> libName;

					if (libName != "")
					{
						std::ifstream file(m->filepath + "/" + libName);
						if (file.good())
						{

							std::string mtlName;
							float spec = 0.f, roughness = 1.f;
							std::shared_ptr<Image> albedoAlphaMap;
							std::shared_ptr<Image> normalHeightMap;

							while (!file.eof())
							{
								std::string line;
								std::getline(file, line);

								std::stringstream ss(line);
								std::string token;
								ss >> token;

								if (token == "newmtl")
									ss >> mtlName;
								else if (token == "tk_spec")
									ss >> spec;
								else if (token == "tk_roughness")
									ss >> roughness;
								else if (token == "map_Kd")
								{
									std::string filename;
									ss >> filename;
									albedoAlphaMap = getMaterialImage(m->filepath + "/" + filename);
								}
								else if (token == "map_bump")
								{
									std::string filename;
									ss >> filename;
									normalHeightMap = getMaterialImage(m->filepath + "/" + filename);
								}
							}

							auto m = getMaterial(glm::vec4(1.f), glm::vec2(spec, roughness), albedoAlphaMap, nullptr, normalHeightMap);
							m->name = mtlName;
						}
					}
				}
			}

			_process_model(m, true);
		}
	}

	namespace PMD
	{
#pragma pack(1)
		struct Header
		{
			char pmdStr[3];
			float version;
			char name[20];
			char comment[256];
		};

		struct VertexData
		{
			glm::vec3 position;
			glm::vec3 normal;
			glm::vec2 uv;
			unsigned short boneID0;
			unsigned short boneID1;
			BYTE weight;
			BYTE enableEdges;
		};

		struct MaterialData
		{
			glm::vec4 diffuse;
			float specPower;
			glm::vec3 specColor;
			glm::vec3 materialShadow;
			BYTE toonIndex;
			BYTE edgeFlag;
			int indiceCount;
			char mapName[20];
		};

		struct BoneData
		{
			char name[20];
			short parents;
			short boneID;
			char type;
			short ik;
			glm::vec3 coord;
		};

		struct IkData
		{
			short target;
			short effector;
			char chainLength;
			unsigned short iterations;
			float weight;
		};

		struct MorphHeadData
		{
			char name[20];
			unsigned int size;
			char type;
		};

		struct MorphData
		{
			unsigned int index;
			glm::vec3 offset;
		};

		struct RigidData
		{
			char name[20];
			short bone;
			char collisionGroupNumber;
			unsigned short collisionGroupMask;
			char type;
			glm::vec3 size;
			glm::vec3 location;
			glm::vec3 rotation;
			float mass;
			float velocityAttenuation;
			float rotationAttenuation;
			float bounce;
			float friction;
			char mode;
		};

		struct JointData
		{
			char name[20];
			int rigid0;
			int rigid1;
			glm::vec3 coord;
			glm::vec3 rotation;
			glm::vec3 maxCoord;
			glm::vec3 minCoord;
			glm::vec3 maxRotation;
			glm::vec3 minRotation;
			glm::vec3 springConstant;
			glm::vec3 springRotationConstant;
		};
#pragma pack()

		void load(Model *m, const std::string &filename)
		{
			std::ifstream file(filename, std::ios::binary);

			static_assert(sizeof(Header) == 283, "");
			static_assert(sizeof(VertexData) == 38, "");
			static_assert(sizeof(MaterialData) == 70, "");
			static_assert(sizeof(BoneData) == 39, "");
			static_assert(sizeof(MorphHeadData) == 25, "");
			static_assert(sizeof(MorphData) == 16, "");
			static_assert(sizeof(RigidData) == 83, "");
			static_assert(sizeof(JointData) == 124, "");

			Header header;
			file.read((char*)&header, sizeof(Header));

			int vertex_count;
			file & vertex_count;
			m->vertexes.resize(vertex_count);
			m->vertexes_skeleton.resize(vertex_count);
			for (int i = 0; i < vertex_count; i++)
			{
				VertexData data;
				file.read((char*)&data, sizeof(VertexData));
				m->vertexes[i].position = data.position;
				m->vertexes[i].position.z *= -1.f;
				m->vertexes[i].normal = data.normal;
				m->vertexes[i].normal.z *= -1.f;
				m->vertexes[i].uv = data.uv;
				m->vertexes[i].uv.y = 1.f - m->vertexes[i].uv.y;
				float fWeight = data.weight / 100.f;
				m->vertexes_skeleton[i].bone_weight.x = fWeight;
				m->vertexes_skeleton[i].bone_weight.y = 1.f - fWeight;
				m->vertexes_skeleton[i].bone_ID.x = data.boneID0;
				m->vertexes_skeleton[i].bone_ID.y = data.boneID1;
			}

			int indice_count;
			file & indice_count;
			m->indices.resize(indice_count);
			for (int i = 0; i < indice_count; i += 3)
			{
				unsigned short indice;
				file & indice;
				m->indices[i + 0] = indice;
				file & indice;
				m->indices[i + 2] = indice;
				file & indice;
				m->indices[i + 1] = indice;
			}

			int materialCount;
			file & materialCount;
			int currentIndiceVertex = 0;
			for (int i = 0; i < materialCount; i++)
			{
				MaterialData data;
				file.read((char*)&data, sizeof(MaterialData));

				auto g = new Geometry;
				g->material = getMaterial(data.diffuse, glm::vec2(0.f, 1.f), 
					getMaterialImage(m->filepath + "/" + data.mapName), nullptr, nullptr);
				g->indiceBase = currentIndiceVertex;
				g->indiceCount = data.indiceCount;

				currentIndiceVertex += data.indiceCount;

				m->geometries.emplace_back(g);
			}

			unsigned short boneCount;
			file & boneCount;
			for (int i = 0; i < boneCount; i++)
			{
				BoneData data;
				file.read((char*)&data, sizeof(BoneData));

				auto b = m->new_bone();

				b->name = japanese_to_chinese(data.name);
				b->parent = data.parents;
				b->type = data.type;
				b->rootCoord = data.coord;
				b->rootCoord.z *= -1.f;
			}

			unsigned short ikCount;
			file & ikCount;
			for (int i = 0; i < ikCount; i++)
			{
				IkData data;
				file.read((char*)&data, sizeof(IkData));

				auto b = m->new_bone_ik();

				b->targetID = data.target;
				b->effectorID = data.effector;
				b->iterations = data.iterations;
				b->weight = data.weight;
				b->chain.resize(data.chainLength);
				for (int j = 0; j < data.chainLength; j++)
				{
					short boneID;
					file & boneID;
					b->chain[j] = boneID;
				}
			}

			unsigned short morphsCount;
			file & morphsCount;
			for (int i = 0; i < morphsCount; i++)
			{
				MorphHeadData data;
				file.read((char*)&data, sizeof(MorphHeadData));

				for (int j = 0; j < data.size; j++)
				{
					MorphData data;
					file.read((char*)&data, sizeof(MorphData));
				}
			}

			char dispMorphsListLength;
			file & dispMorphsListLength;
			for (int i = 0; i < dispMorphsListLength; i++)
			{
				unsigned short id;
				file & id;
			}
			char dispBoneListLength;
			file & dispBoneListLength;
			for (int i = 0; i < dispBoneListLength; i++)
			{
				char name[50];
				file.read(name, 50);
			}

			unsigned int dispBoneCount;
			file & dispBoneCount;
			for (int i = 0; i < dispBoneCount; i++)
			{
				unsigned short boneIndex;
				char index;
				file & boneIndex;
				file & index;
			}

			char endFlag;
			file & endFlag;
			if (endFlag)
			{
				char englishName[20];
				char englishComment[256];
				file.read(englishName, 20);
				file.read(englishComment, 256);
				for (int i = 0; i < boneCount; i++)
				{
					char name[20];
					file.read(name, 20);
				}
				for (int i = 1; i < morphsCount; i++)
				{
					char name[20];
					file.read(name, 20);
				}
				for (int i = 0; i < dispBoneListLength; i++)
				{
					char name[50];
					file.read(name, 50);
				}
			}

			for (int i = 0; i < 10; i++)
			{
				char toonTextureName[100];
				file.read(toonTextureName, 100);
			}

			unsigned int rigidCount;
			file & rigidCount;
			for (int i = 0; i < rigidCount; i++)
			{
				RigidData data;
				file.read((char*)&data, sizeof(RigidData));

				auto r = m->new_rigidbody();
				r->name = japanese_to_chinese(data.name);
				r->boneID = data.bone;
				r->originCollisionGroupID = data.collisionGroupNumber;
				r->originCollisionFreeFlag = data.collisionGroupMask;
				data.location.z *= -1.f;
				r->coord = data.location;
				data.rotation = glm::degrees(data.rotation);
				r->quat = mat3_to_quaternion(euler_to_mat3(-data.rotation.y, -data.rotation.x, data.rotation.z));
				r->type = (RigidbodyType)data.mode;
				auto s = r->new_shape();
				switch (data.type)
				{
					case 0: 
						s->type = ShapeType::sphere; 
						break;
					case 1: 
						s->type = ShapeType::box; 
						break;
					case 2:
						s->type = ShapeType::capsule; 
						break;
				}
				switch (s->type)
				{
					case ShapeType::sphere:
						data.size.y = data.size.z = data.size.x;
						break;
					case ShapeType::capsule:
						data.size.y *= 0.5f;
						data.size.z = data.size.x;
						break;
				}
				s->scale = data.size;
				auto v = s->getVolume();
				if (v != 0.f) 
					r->density = data.mass / v;
			}

			unsigned int jointCount;
			file & jointCount;
			for (int i = 0; i < jointCount; i++)
			{
				JointData data;
				file.read((char*)&data, sizeof(JointData));

				auto j = m->new_joint();
				j->name = japanese_to_chinese(data.name);
				j->rigid0ID = data.rigid0;
				j->rigid1ID = data.rigid1;
				j->maxCoord = data.maxCoord;
				j->minCoord = data.minCoord;
				j->maxRotation = data.maxRotation;
				j->minRotation = data.minRotation;
				j->springConstant = data.springConstant;
				j->sprintRotationConstant = data.springRotationConstant;

				data.coord.z *= -1.f;
				j->coord = data.coord;
				j->quat = mat3_to_quaternion(euler_to_mat3(-data.rotation.y, -data.rotation.x, data.rotation.z));
			}

			_process_model(m, true);
		}
	}

	namespace COLLADA
	{
		std::string getId(const std::string &str)
		{
			assert(str.size() > 0);
			if (str[0] == '#')
				return std::string(str.c_str() + 1);
		}

		struct Source
		{
			std::string id;
			float *float_array;

			~Source()
			{
				delete[]float_array;
			}
			glm::vec2 &v2(int index)
			{
				return *(glm::vec2*)&float_array[index * 2];
			}
			glm::vec3 &v3(int index)
			{
				return *(glm::vec3*)&float_array[index * 3];
			}
		};

		struct VertexInfo
		{
			int position_source_index;
		};

		void load(Model *m, const std::string &filename)
		{
			XMLDoc at("COLLADA", filename);
			XMLNode *n;
			n = at.firstNode("library_geometries"); assert(n);
			n = n->firstNode("geometry"); assert(n);
			n = n->firstNode("mesh"); assert(n);
			std::vector<std::unique_ptr<Source>> sources;
			VertexInfo vertex_info;

			for (auto &c : n->children)
			{
				if (c->name == "source")
				{
					XMLNode *n;
					XMLAttribute *a;
					auto s = new Source;
					a = c->firstAttribute("id"); assert(a);
					s->id = a->value;
					n = c->firstNode("float_array"); assert(n);
					a = n->firstAttribute("count"); assert(a);
					auto count = std::stoi(a->value);
					s->float_array = new float[count];
					auto str = n->value;
					std::regex pattern(R"(([0-9e\.\+\-]+))");
					std::smatch match;
					int id = 0;
					while (std::regex_search(str, match, pattern) && id < count)
					{
						s->float_array[id] = std::stof(match[1].str());
						id++;
						str = match.suffix();
					}
					sources.emplace_back(s);
				}
				else if (c->name == "vertices")
				{
					for (auto &cc : c->children)
					{
						if (cc->name == "input")
						{
							auto a = cc->firstAttribute("semantic"); assert(a);
							if (a->value == "POSITION")
							{
								a = cc->firstAttribute("source"); assert(a);
								auto id = getId(a->value);
								for (int i = 0; i < sources.size(); i++)
								{
									if (sources[i]->id == id)
									{
										vertex_info.position_source_index = i;
										break;
									}
								}
							}
						}
					}
				}
				else if (c->name == "polylist")
				{
					int position_source_index = -1;
					int position_offset = -1;
					int uv_source_index = -1;
					int uv_offset = -1;
					int normal_source_index = -1;
					int normal_offset = -1;
					int element_count_per_vertex = 0;
					std::vector<int> vcount;
					for (auto &cc : c->children)
					{
						if (cc->name == "input")
						{
							XMLAttribute *a;
							a = cc->firstAttribute("source"); assert(a);
							auto id = getId(a->value);
							int source_index = -1;
							for (int i = 0; i < sources.size(); i++)
							{
								if (sources[i]->id == id)
								{
									source_index = i;
									break;
								}
							}
							a = cc->firstAttribute("offset"); assert(a);
							auto offset = std::stoi(a->value);
							a = cc->firstAttribute("semantic"); assert(a);
							if (a->value == "VERTEX")
							{
								position_source_index = vertex_info.position_source_index;
								position_offset = offset;
							}
							else if (a->value == "NORMAL")
							{
								normal_source_index = source_index;
								normal_offset = offset;
							}
							element_count_per_vertex++;
						}
						else if (cc->name == "vcount")
						{
							auto str = cc->value;
							std::regex pattern(R"([0-9]+)");
							std::smatch match;
							while (std::regex_search(str, match, pattern))
							{
								auto count = std::stoi(match[0].str());
								assert(count == 3);
								vcount.push_back(count);
								str = match.suffix();
							}
						}
						else if (cc->name == "p")
						{
							auto str = cc->value;
							std::smatch match;
							assert(element_count_per_vertex > 0 && element_count_per_vertex <= 3);
							switch (element_count_per_vertex)
							{
								case 1:
								{
									std::regex pattern(R"([0-9]+)");
									auto indice_count = vcount.size() * 3;
									while (std::regex_search(str, match, pattern) && indice_count > 0)
									{
										auto index = std::stoi(match[0].str());
										m->vertexes.push_back({
											sources[position_source_index]->v3(index),
											glm::vec2(0.f),
											glm::vec3(0.f),
											glm::vec3(0.f)
											});
										m->indices.push_back(index);
										indice_count--;
										str = match.suffix();
									}
									break;
								}
								case 2:
								{
									std::vector<glm::ivec2> ids;
									std::regex pattern(R"(([0-9]+)\s+([0-9]+))");
									auto indice_count = vcount.size() * 3;
									while (std::regex_search(str, match, pattern) && indice_count > 0)
									{
										glm::ivec2 id;
										id[0] = std::stoi(match[1].str());
										id[1] = std::stoi(match[2].str());
										auto index = -1;
										for (int i = 0; i < ids.size(); i++)
										{
											if (id == ids[i])
											{
												index = i;
												break;
											}
										}
										if (index == -1)
										{
											index = m->vertexes.size();
											m->vertexes.push_back({
												sources[position_source_index]->v3(id[position_offset]),
												uv_source_index == -1 ? glm::vec2(0.f) : sources[uv_source_index]->v2(id[uv_offset]),
												normal_source_index == -1 ? glm::vec3(0.f) : sources[normal_source_index]->v3(id[normal_offset]),
												glm::vec3(0.f)
											});
											ids.push_back(id);

										}
										m->indices.push_back(index);
										indice_count--;
										str = match.suffix();
									}
									break;
								}
								case 3:
								{
									std::vector<glm::ivec3> ids;
									std::regex pattern(R"(([0-9]+)\s+([0-9]+)\s+([0-9]+))");
									auto indice_count = vcount.size() * 3;
									while (std::regex_search(str, match, pattern) && indice_count > 0)
									{
										glm::ivec3 id;
										id[0] = std::stoi(match[1].str());
										id[1] = std::stoi(match[2].str());
										id[2] = std::stoi(match[3].str());
										auto index = -1;
										for (int i = 0; i < ids.size(); i++)
										{
											if (id == ids[i])
											{
												index = i;
												break;
											}
										}
										if (index == -1)
										{
											index = m->vertexes.size();
											m->vertexes.push_back({
												sources[position_source_index]->v3(id[position_offset]),
												uv_source_index == -1 ? glm::vec2(0.f) : sources[uv_source_index]->v2(id[uv_offset]),
												normal_source_index == -1 ? glm::vec3(0.f) : sources[normal_source_index]->v3(id[normal_offset]),
												glm::vec3(0.f)
											});
											ids.push_back(id);

										}
										m->indices.push_back(index);
										indice_count--;
										str = match.suffix();
									}
									break;
								}
							}
						}
					}
				}
			}

			auto g = new Geometry;
			g->material = default_material;
			g->indiceCount = m->indices.size();
			m->geometries.emplace_back(g);

			_process_model(m, true);
		}
	}

	namespace TKM
	{
		void load(Model *m, const std::string &filename)
		{
			std::ifstream file(filename, std::ios::binary);

			bool animated;
			file & animated;

			int vertex_count;
			int indice_count;
			file & vertex_count;
			file & indice_count;
			if (vertex_count > 0)
			{
				m->vertexes.resize(vertex_count);
				file.read((char*)m->vertexes.data(), sizeof(ModelVertex) * vertex_count);
				if (animated)
				{
					m->vertexes_skeleton.resize(indice_count);
					file.read((char*)m->vertexes_skeleton.data(), sizeof(ModelVertexSkeleton) * indice_count);
				}
			}
			if (indice_count)
			{
				m->indices.resize(indice_count);
				file.read((char*)m->indices.data(), sizeof(int) * indice_count);
			}

			int geometryCount;
			file & geometryCount;
			for (int i = 0; i < geometryCount; i++)
			{
				glm::vec4 albedo_alpha;
				glm::vec2 spec_roughness;
				file & albedo_alpha;
				file & spec_roughness;
				std::string albedoAlphaMapName;
				std::string normalHeightMapName;
				std::string specRoughnessMapName;
				file > albedoAlphaMapName;
				file > normalHeightMapName;
				file > specRoughnessMapName;

				auto g = new Geometry;
				g->material = getMaterial(albedo_alpha, spec_roughness,
					getMaterialImage(m->filepath + "/" + albedoAlphaMapName),
					getMaterialImage(m->filepath + "/" + specRoughnessMapName),
					getMaterialImage(m->filepath + "/" + normalHeightMapName));
				file & g->indiceBase;
				file & g->indiceCount;

				m->geometries.emplace_back(g);
			}

			int boneCount;
			file & boneCount;
			for (int i = 0; i < boneCount; i++)
			{
				auto b = m->new_bone();

				char name[20];
				file.read(name, 20);
				b->name = name;

				file & b->type;
				file & b->parent;
				file & b->rootCoord;
			}

			int ikCount;
			file & ikCount;
			m->iks.resize(boneCount);
			for (int i = 0; i < ikCount; i++)
			{
				auto b = m->new_bone_ik();

				file & b->targetID;
				file & b->effectorID;
				file & b->iterations;
				file & b->weight;

				int chainLength;
				file & chainLength;
				b->chain.resize(chainLength);
				file.read((char*)b->chain.data(), sizeof(int) * chainLength);
			}

			if (animated)
			{
				file > m->stand_animation_filename;
				file > m->forward_animation_filename;
				file > m->backward_animation_filename;
				file > m->leftward_animation_filename;
				file > m->rightward_animation_filename;
				file > m->jump_animation_filename;
			}

			int rigidbodyCount;
			file & rigidbodyCount;
			for (int i = 0; i < rigidbodyCount; i++)
			{
				auto r = m->new_rigidbody();
				int type;
				file & type;
				r->type = (RigidbodyType)type;
				file > r->name;
				file & r->originCollisionGroupID;
				file & r->originCollisionFreeFlag;
				file & r->boneID;
				file & r->coord;
				file & r->quat;
				file & r->density;
				file & r->velocityAttenuation;
				file & r->rotationAttenuation;
				file & r->bounce;
				file & r->friction;

				int shapeCount;
				file & shapeCount;
				for (int j = 0; j < shapeCount; j++)
				{
					auto s = r->new_shape();
					file & s->coord;
					file & s->quat;
					file & s->scale;
					int type;
					file & type;
					s->type = (ShapeType)type;
				}
			}

			int jointCount = 0;
			file & jointCount;
			for (int i = 0; i < jointCount; i++)
			{
				auto j = m->new_joint();
				file & j->coord;
				file & j->quat;
				file & j->rigid0ID;
				file & j->rigid1ID;
				file & j->maxCoord;
				file & j->minCoord;
				file & j->maxRotation;
				file & j->minRotation;
				file & j->springConstant;
				file & j->sprintRotationConstant;
			}

			file & m->bounding_position;
			file & m->bounding_size;

			file & m->controller_height;
			file & m->controller_radius;

			file & m->eye_position;

			_process_model(m, false);
		}

		void save(Model *m, const std::string &filename)
		{
			std::ofstream file(filename, std::ios::binary);

			bool animated = m->vertexes_skeleton.size() > 0;
			file & animated;

			int vertex_count = m->vertexes.size();
			int indice_count = m->indices.size();
			file & vertex_count;
			file & indice_count;
			if (vertex_count > 0)
			{
				file.write((char*)m->vertexes.data(), sizeof(ModelVertex) * vertex_count);
				if (animated)
					file.write((char*)m->vertexes_skeleton.data(), sizeof(ModelVertexSkeleton) * vertex_count);
			}
			if (indice_count > 0)
				file.write((char*)m->indices.data(), sizeof(int) * indice_count);

			int geometryCount = m->geometries.size();
			file & geometryCount;
			for (auto &g : m->geometries)
			{
				file & g->material->albedo_alpha;
				file & g->material->spec_roughness;
				file < (g->material->albedo_alpha_map ? g->material->albedo_alpha_map->filename : "");
				file < (g->material->normal_height_map ? g->material->normal_height_map->filename : "");
				file < (g->material->spec_roughness_map ? g->material->spec_roughness_map->filename : "");

				file & g->indiceBase;
				file & g->indiceCount;
			}

			int boneCount = m->bones.size();
			file & boneCount;
			for (auto &b : m->bones)
			{
				file & b->name;
				file & b->type;
				file & b->parent;
				file & b->rootCoord;
			}

			int ikCount = m->iks.size();
			file & ikCount;
			for (auto &b : m->iks)
			{
				file & b->targetID;
				file & b->effectorID;
				file & b->iterations;
				file & b->weight;

				int chainSize = b->chain.size();
				file & chainSize;
				file.write((char*)b->chain.data(), sizeof(int) * b->chain.size());
			}

			if (animated)
			{
				file < m->stand_animation_filename;
				file < m->forward_animation_filename;
				file < m->backward_animation_filename;
				file < m->leftward_animation_filename;
				file < m->rightward_animation_filename;
				file < m->jump_animation_filename;
			}

			int rigidbodyCount = m->rigidbodies.size();
			file & rigidbodyCount;
			for (auto &r : m->rigidbodies)
			{
				int mode = (int)r->type;
				file & mode;
				file < r->name;
				file & r->originCollisionGroupID;
				file & r->originCollisionFreeFlag;
				file & r->boneID;
				file & r->coord;
				file & r->quat;
				file & r->density;
				file & r->velocityAttenuation;
				file & r->rotationAttenuation;
				file & r->bounce;
				file & r->friction;

				int shapeCount = r->shapes.size();
				file & shapeCount;
				for (auto &s : r->shapes)
				{
					file & s->coord;
					file & s->quat;
					file & s->scale;
					int type = (int)s->type;
					file & type;
				}
			}

			int jointCount = m->joints.size();
			file & jointCount;
			for (auto &j : m->joints)
			{
				file & j->coord;
				file & j->quat;
				file & j->rigid0ID;
				file & j->rigid1ID;
				file & j->maxCoord;
				file & j->minCoord;
				file & j->maxRotation;
				file & j->minRotation;
				file & j->springConstant;
				file & j->sprintRotationConstant;
			}

			file & m->bounding_position;
			file & m->bounding_size;

			file & m->controller_height;
			file & m->controller_radius;

			file & m->eye_position;
		}
	}

	std::shared_ptr<Model> getModel(const std::string &filename)
	{
		auto hash = HASH(filename.c_str());
		auto it = _models.find(hash);
		if (it != _models.end())
		{
			auto s = it->second.lock();
			if (s) return s;
		}

		std::experimental::filesystem::path path(filename);
		if (!std::experimental::filesystem::exists(path))
			return nullptr;

		auto ext = path.extension().string();
		void(*load_func)(Model *, const std::string &) = nullptr;
		if (ext == ".obj")
			load_func = &OBJ::load;
		else if (ext == ".pmd")
			load_func = &PMD::load;
		else if (ext == ".dae")
			load_func = &COLLADA::load;
		else if (ext == ".tkm")
			load_func = &TKM::load;
		else
			return nullptr;

		auto m = std::make_shared<Model>();
		m->filename = filename;
		m->filepath = path.parent_path().string();
		if (m->filepath == "")
			m->filepath = ".";
		load_func(m.get(), filename);

		_models[hash] = m;
		_create_vertex_and_index_buffer();
		return m;
	}

	void saveModel(Model *m, const std::string &filename)
	{
		std::experimental::filesystem::path path(filename);

		auto ext = path.extension().string();
		void(*save_func)(Model *, const std::string &) = nullptr;
		if (ext == ".tkm")
			save_func = &TKM::save;
		else
			return;

		save_func(m, filename);
	}

	void initModel()
	{
		{
			auto m = std::make_shared<Model>();
			m->filename = "Triangle";

			add_triangle_vertex(m.get(), glm::mat3(1.f), glm::vec3(0.f));

			auto g = new Geometry;
			g->name = "0";
			g->material = default_material;
			g->indiceCount = m->indices.size();
			m->geometries.emplace_back(g);

			_process_model(m.get(), true);

			_models[HASH(m->filename.c_str())] = m;

			triangleModel = m;
		}

		{
			auto m = std::make_shared<Model>();
			m->filename = "Cube";

			add_cube_vertex(m.get(), glm::mat3(1.f), glm::vec3(0.f), 1.f);

			auto g = new Geometry;
			g->name = "0";
			g->material = default_material;
			g->indiceCount = m->indices.size();
			m->geometries.emplace_back(g);

			auto r = m->new_rigidbody();
			r->type = RigidbodyType::dynamic;
			auto s = r->new_shape();
			s->type = ShapeType::box;
			s->scale = glm::vec3(0.5f);

			_process_model(m.get(), true);

			_models[HASH(m->filename.c_str())] = m;

			cubeModel = m;
		}

		{
			auto m = std::make_shared<Model>();
			m->filename = "Sphere";

			add_sphere_vertex(m.get(), glm::mat3(1.f), glm::vec3(0.f), 0.5f, 32, 32);

			auto g0 = new Geometry;
			g0->name = "0";
			g0->material = default_material;
			g0->indiceCount = m->indices.size() / 2;
			auto g1 = new Geometry;
			g1->name = "1";
			g1->material = default_material;
			g1->indiceBase = g0->indiceCount;
			g1->indiceCount = g0->indiceCount;
			m->geometries.emplace_back(g0);
			m->geometries.emplace_back(g1);

			auto r = m->new_rigidbody();
			r->type = RigidbodyType::dynamic;
			auto s = r->new_shape();
			s->type = ShapeType::sphere;
			s->scale = glm::vec3(0.5f);

			_process_model(m.get(), true);

			_models[HASH(m->filename.c_str())] = m;

			sphereModel = m;
		}

		{
			auto m = std::make_shared<Model>();
			m->filename = "Cylinder";

			add_cylinder_vertex(m.get(), glm::mat3(1.f), glm::vec3(0.f), 0.5f, 0.5f, 32);

			auto g = new Geometry;
			g->name = "0";
			g->material = default_material;
			g->indiceCount = m->indices.size();
			m->geometries.emplace_back(g);

			auto r = m->new_rigidbody();
			r->type = RigidbodyType::dynamic;
			auto s = r->new_shape();
			s->type = ShapeType::capsule;
			s->scale = glm::vec3(0.5f);

			_process_model(m.get(), true);

			_models[HASH(m->filename.c_str())] = m;

			cylinderModel = m;
		}

		{
			auto m = std::make_shared<Model>();
			m->filename = "Cone";

			add_cone_vertex(m.get(), glm::mat3(1.f), glm::vec3(0.f), 0.5f, 0.5f, 32);

			auto g = new Geometry;
			g->name = "0";
			g->material = default_material;
			g->indiceCount = m->indices.size();
			m->geometries.emplace_back(g);

			_process_model(m.get(), true);

			_models[HASH(m->filename.c_str())] = m;

			coneModel = m;
		}

		{
			auto m = std::make_shared<Model>();
			m->filename = "Arrow";

			glm::mat3 matR = glm::mat3(glm::rotate(glm::radians(-90.f), glm::vec3(0.f, 0.f, 1.f)));

			add_cylinder_vertex(m.get(), matR, glm::vec3(0.4f, 0.f, 0.f), 0.4f, 0.01f, 32);
			add_cone_vertex(m.get(), matR, glm::vec3(0.8f, 0.f, 0.f), 0.2f, 0.05f, 32);

			auto g = new Geometry;
			g->name = "0";
			g->material = default_material;
			g->indiceCount = m->indices.size();
			m->geometries.emplace_back(g);

			_process_model(m.get(), true);

			_models[HASH(m->filename.c_str())] = m;

			arrowModel = m;
		}

		{
			auto m = std::make_shared<Model>();
			m->filename = "Torus";

			glm::mat3 matR = glm::mat3(glm::rotate(glm::radians(-90.f), glm::vec3(0.f, 0.f, 1.f)));

			add_torus_vertex(m.get(), matR, glm::vec3(), 1.f, 0.01f, 32, 32);

			auto g = new Geometry;
			g->name = "0";
			g->material = default_material;
			g->indiceCount = m->indices.size();
			m->geometries.emplace_back(g);

			_process_model(m.get(), true);

			_models[HASH(m->filename.c_str())] = m;

			torusModel = m;
		}

		{
			auto m = std::make_shared<Model>();
			m->filename = "Hammer";

			glm::mat3 matR = glm::mat3(glm::rotate(glm::radians(-90.f), glm::vec3(0.f, 0.f, 1.f)));

			add_cylinder_vertex(m.get(), matR, glm::vec3(0.45f, 0.f, 0.f), 0.45f, 0.01f, 32);
			int ic0 = m->indices.size();
			add_cube_vertex(m.get(), matR, glm::vec3(0.9f, 0.f, 0.f), 0.1f);
			int ic1 = m->indices.size();

			auto g0 = new Geometry;
			g0->name = "0";
			g0->material = default_material;
			g0->indiceCount = ic0;
			auto g1 = new Geometry;
			g1->name = "1";
			g1->material = default_material;
			g1->indiceBase = ic0;
			g1->indiceCount = ic1 - ic0;
			m->geometries.emplace_back(g0);
			m->geometries.emplace_back(g1);

			_process_model(m.get(), true);

			_models[HASH(m->filename.c_str())] = m;

			hamerModel = m;
		}

		_create_vertex_and_index_buffer();
	}
}
