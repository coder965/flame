#include <sstream>
#include <regex>
#include <iostream>
#include <algorithm>
#include <map>
#include <deque>
#include <tuple>

#include <flame/string.h>
#include <flame/filesystem.h>
#include <flame/engine/resource/resource.h>
#include <flame/engine/graphics/buffer.h>
#include <flame/engine/graphics/texture.h>
#include <flame/engine/graphics/sampler.h>
#include <flame/engine/graphics/pipeline.h>
#include <flame/engine/graphics/material.h>
#include <flame/engine/entity/model.h>
#include <flame/engine/entity/animation.h>
#include <flame/engine/physics/physics.h>

namespace flame
{
	bool ModelVertex::operator==(const ModelVertex &right)
	{
		return is_same(position, right.position) &&
			is_same(normal, right.normal) &&
			is_same(tangent, right.tangent) &&
			is_same(uv, right.uv);
	}

	bool ModelVertexSkeleton::operator==(const ModelVertexSkeleton &right)
	{
		return is_same(bone_weight, right.bone_weight) &&
			is_same(bone_ID, right.bone_ID);
	}

	std::map<unsigned int, std::weak_ptr<Model>> _models;
	void _create_vertex_and_index_buffer()
	{
		auto vertex_stat_count = 0;
		auto vertex_anim_count = 0;
		auto indice_count = 0;

		for (auto it = _models.begin(); it != _models.end();)
		{
			auto s = it->second.lock();
			if (s)
			{
				vertex_stat_count += s->vertexes.size();
				vertex_anim_count += s->vertexes_skeleton.size();
				indice_count += s->indices.size();
				it++;
			}
			else
				it = _models.erase(it);
		}

		vertex_static_buffer = std::make_unique<Buffer>(BufferTypeVertex, sizeof(ModelVertex) * vertex_stat_count);
		vertex_skeleton_Buffer = std::make_unique<Buffer>(BufferTypeVertex, sizeof(ModelVertexSkeleton) * vertex_anim_count);
		index_buffer = std::make_unique<Buffer>(BufferTypeIndex, sizeof(int) * indice_count);

		Buffer stagingBuffer(BufferTypeStaging, vertex_static_buffer->size + vertex_skeleton_Buffer->size + index_buffer->size);

		stagingBuffer.map(0, stagingBuffer.size);
		unsigned char *vs_map = (unsigned char*)stagingBuffer.mapped;
		auto va_map = vs_map + vertex_static_buffer->size;
		auto i_map = va_map + vertex_skeleton_Buffer->size;
		auto vertex_offset = 0;
		auto indice_offset = 0;
		for (auto &m : _models)
		{
			auto s = m.second.lock();
			if (s && !s->vertexes_skeleton.empty())
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
		for (auto &m : _models)
		{
			auto s = m.second.lock();
			if (s && s->vertexes_skeleton.empty())
			{
				s->vertex_base = vertex_offset;
				s->indice_base = indice_offset;
				memcpy(vs_map + vertex_offset * sizeof(ModelVertex), s->vertexes.data(), sizeof(ModelVertex) * s->vertexes.size());
				memcpy(i_map + indice_offset * sizeof(int), s->indices.data(), sizeof(int) * s->indices.size());
				vertex_offset += s->vertexes.size();
				indice_offset += s->indices.size();
			}
		}
		stagingBuffer.unmap();

		if (vertex_stat_count > 0)
			stagingBuffer.copy_to(vertex_static_buffer.get(), vertex_static_buffer->size);
		if (vertex_anim_count > 0)
			stagingBuffer.copy_to(vertex_skeleton_Buffer.get(), vertex_skeleton_Buffer->size, va_map - vs_map);
		if (indice_count > 0)
			stagingBuffer.copy_to(index_buffer.get(), index_buffer->size, i_map - vs_map);
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

	const char *Model::get_uv_use_name(UV *uv) const
	{
		if (geometry_uv == uv && bake_uv == uv)
			return "geometry and bake";
		else if (geometry_uv == uv)
			return "geometry";
		else if (bake_uv == uv)
			return "bake";
		else
			return "null";
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

		auto fUnfoldTri = [&](std::tuple<int, int, glm::vec4> &_d) {
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

		if (vertexes_skeleton.size() == 0)
		{

			std::vector<ModelVertex> new_vertexes;
			std::vector<int> new_indices;

			for (int i = 0; i < indices.size(); i++)
			{
				ModelVertex a = vertexes[indices[i]];
				a.uv = uv ? uv->unique[uv->indices[i]] : glm::vec2(0.f);

				int index = -1;
				for (int j = 0; j < new_vertexes.size(); j++)
				{
					if (a == new_vertexes[j])
					{
						index = j;
						break;
					}
				}
				if (index == -1)
				{
					index = new_vertexes.size();
					new_vertexes.push_back(a);
				}
				new_indices.push_back(index);
			}

			vertexes = new_vertexes;
			indices = new_indices;
		}
		else
		{

			std::vector<ModelVertex> new_vertexes;
			std::vector<ModelVertexSkeleton> new_vertexes_skeleton;
			std::vector<int> new_indices;

			for (int i = 0; i < indices.size(); i++)
			{
				ModelVertex a0 = vertexes[indices[i]];
				a0.uv = uv ? uv->unique[uv->indices[i]] : glm::vec2(0.f);
				ModelVertexSkeleton a1 = vertexes_skeleton[indices[i]];

				int index = -1;
				for (int j = 0; j < new_vertexes.size(); j++)
				{
					if (a0 == new_vertexes[j] && a1 == new_vertexes_skeleton[j])
					{
						index = j;
						break;
					}
				}
				if (index == -1)
				{
					index = new_vertexes.size();
					new_vertexes.push_back(a0);
					new_vertexes_skeleton.push_back(a1);
				}
				new_indices.push_back(index);
			}

			vertexes = new_vertexes;
			vertexes_skeleton = new_vertexes_skeleton;
			indices = new_indices;
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

#if FLAME_ENABLE_PHYSICS != 0
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
#endif

	std::unique_ptr<Buffer> vertex_static_buffer;
	std::unique_ptr<Buffer> vertex_skeleton_Buffer;
	std::unique_ptr<Buffer> index_buffer;

	void add_triangle_vertex(Model *m, glm::mat3 rotation, glm::vec3 center)
	{
		int baseVertex = m->vertexes.size();

		m->add_vertex_position_normal(center + rotation * glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f));
		m->add_vertex_position_normal(center + rotation * glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f));
		m->add_vertex_position_normal(center + rotation * glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 1.f));
		for (int i = 0; i < 3; i++)
			m->indices.push_back(baseVertex + i);
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

			std::vector<std::shared_ptr<Material>> temp_materials;

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
					for (auto &m : temp_materials)
					{
						if (m->get_name() == name)
							currentGeometry->material = m;
					}
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
							std::string albedo_alpha_map_name;
							std::string normal_height_map_name;

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
									albedo_alpha_map_name = m->filepath + "/" + filename;
								}
								else if (token == "map_bump")
								{
									std::string filename;
									ss >> filename;
									normal_height_map_name = m->filepath + "/" + filename;
								}
							}

							auto m = getMaterial(glm::vec4(1.f), spec, roughness, albedo_alpha_map_name, "", normal_height_map_name);
							m->set_name(mtlName);
							temp_materials.push_back(m);
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

			auto vertex_count = read<int>(file);
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

			auto indice_count = read<int>(file);
			m->indices.resize(indice_count);
			for (int i = 0; i < indice_count; i += 3)
			{
				m->indices[i + 0] = (unsigned short)read<short>(file);
				m->indices[i + 2] = (unsigned short)read<short>(file);
				m->indices[i + 1] = (unsigned short)read<short>(file);
			}

			auto materialCount = read<int>(file);
			int currentIndiceVertex = 0;
			for (int i = 0; i < materialCount; i++)
			{
				MaterialData data;
				file.read((char*)&data, sizeof(MaterialData));

				auto g = new Geometry;
				g->material = getMaterial(data.diffuse, 0.f, 1.f, 
					m->filepath + "/" + data.mapName, "", "");
				g->indiceBase = currentIndiceVertex;
				g->indiceCount = data.indiceCount;

				currentIndiceVertex += data.indiceCount;

				m->geometries.emplace_back(g);
			}

			unsigned short boneCount = read<short>(file);
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

			unsigned short ikCount = read<short>(file);
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
					b->chain[j] = read<short>(file); // bone id
			}

			unsigned short morphsCount = read<short>(file);
			for (int i = 0; i < morphsCount; i++)
			{
				MorphHeadData data;
				file.read((char*)&data, sizeof(MorphHeadData));
				file.seekg(sizeof(MorphData) * data.size, std::ios::cur); // MorphData
			}

			auto dispMorphsListLength = read<char>(file);
			file.seekg(sizeof(short) * dispMorphsListLength, std::ios::cur); //id
			auto dispBoneListLength = read<char>(file);
			file.seekg(50 * dispBoneListLength, std::ios::cur); // char name[50]

			auto dispBoneCount = read<int>(file);
			file.seekg((sizeof(short) + sizeof(char)) * dispBoneCount, std::ios::cur); // bone index, index

			auto endFlag = read<char>(file);
			if (endFlag)
			{
				file.seekg(
					20 + // english name
					256 + // english comment
					20 * boneCount + // char name[20]
					20 * morphsCount + // char name[20]
					50 * dispBoneListLength // char name[50]
				, std::ios::cur);
			}

			file.seekg(10 * 100, std::ios::cur); // char toonTextureName[100] * 10

			auto rigidCount = read<int>(file);
			for (int i = 0; i < rigidCount; i++)
			{
				RigidData data;
				file.read((char*)&data, sizeof(RigidData));

				#if FLAME_ENABLE_PHYSICS
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
				#endif
			}

			auto jointCount = read<int>(file);
			for (int i = 0; i < jointCount; i++)
			{
				JointData data;
				file.read((char*)&data, sizeof(JointData));

				#if FLAME_ENABLE_PHYSICS
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
				#endif
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
			auto xml = load_xml("COLLADA", filename);
			XMLNode *n;
			n = xml->find_node("library_geometries"); assert(n);
			n = n->find_node("geometry"); assert(n);
			n = n->find_node("mesh"); assert(n);
			std::vector<std::unique_ptr<Source>> sources;
			VertexInfo vertex_info;

			for (auto &c : n->children)
			{
				if (c->name == "source")
				{
					XMLNode *n;
					XMLAttribute *a;
					auto s = new Source;
					a = c->find_attribute("id"); assert(a);
					s->id = a->value;
					n = c->find_node("float_array"); assert(n);
					a = n->find_attribute("count"); assert(a);
					auto count = std::stoi(a->value);
					s->float_array = new float[count];
					auto str = n->content;
					int offset = 0, read_count;
					int id = 0;
					while (sscanf(str.c_str() + offset, "%f%n", &s->float_array[id], &read_count) == 1 && id < count)
					{
						offset += read_count;
						id++;
					}
					sources.emplace_back(s);
				}
				else if (c->name == "vertices")
				{
					for (auto &cc : c->children)
					{
						if (cc->name == "input")
						{
							auto a = cc->find_attribute("semantic"); assert(a);
							if (a->value == "POSITION")
							{
								a = cc->find_attribute("source"); assert(a);
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
							a = cc->find_attribute("source"); assert(a);
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
							a = cc->find_attribute("offset"); assert(a);
							auto offset = std::stoi(a->value);
							a = cc->find_attribute("semantic"); assert(a);
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
							auto str = cc->content;
							int offset = 0, read_count;
							int count;
							while (sscanf(str.c_str() + offset, "%d%n", &count, &read_count) == 1)
							{
								assert(count == 3);
								vcount.push_back(count);
								offset += read_count;
							}
						}
						else if (cc->name == "p")
						{
							auto str = cc->content;
							int offset = 0, read_count;
							assert(element_count_per_vertex > 0 && element_count_per_vertex <= 3);
							switch (element_count_per_vertex)
							{
								case 1:
								{
									auto indice_count = vcount.size() * 3;
									int index;
									while (sscanf(str.c_str() + offset, "%d%n", &index, &read_count) == 1 && indice_count > 0)
									{
										m->vertexes.push_back({
											sources[position_source_index]->v3(index),
											glm::vec2(0.f),
											glm::vec3(0.f),
											glm::vec3(0.f)
											});
										m->indices.push_back(index);
										indice_count--;
										offset += read_count;
									}
									break;
								}
								case 2:
								{
									std::vector<glm::ivec2> ids;
									std::regex pattern(R"(([0-9]+)\s+([0-9]+))");
									auto indice_count = vcount.size() * 3;
									glm::ivec2 id;
									while (sscanf(str.c_str() + offset, "%d %d%n", &id.x, &id.y, &read_count) == 2 && indice_count > 0)
									{
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
										offset += read_count;
									}
									break;
								}
								case 3:
								{
									std::vector<glm::ivec3> ids;
									std::regex pattern(R"(([0-9]+)\s+([0-9]+)\s+([0-9]+))");
									auto indice_count = vcount.size() * 3;
									glm::ivec3 id;
									while (sscanf(str.c_str() + offset, "%d %d %d%n", &id.x, &id.y, &id.z, &read_count) == 3 && indice_count > 0)
									{
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
										offset += read_count;
									}
									break;
								}
							}
						}
					}
				}
			}

			release_xml(xml);

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

			bool animated = read<char>(file);

			auto vertex_count = read<int>(file);
			auto indice_count = read<int>(file);
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

			auto geometryCount = read<int>(file);
			for (int i = 0; i < geometryCount; i++)
			{
				auto albedo_alpha = read<glm::vec4>(file);
				auto spec = read<float>(file);
				auto roughness = read<float>(file);
				auto albedoAlphaMapName = read_string(file);
				auto specRoughnessMapName = read_string(file);
				auto normalHeightMapName = read_string(file);

				auto g = new Geometry;
				g->material = getMaterial(albedo_alpha, spec, roughness,
					m->filepath + "/" + albedoAlphaMapName,
					m->filepath + "/" + specRoughnessMapName,
					m->filepath + "/" + normalHeightMapName);
				g->indiceBase = read<int>(file);
				g->indiceCount = read<int>(file);

				m->geometries.emplace_back(g);
			}

			auto boneCount = read<int>(file);
			for (int i = 0; i < boneCount; i++)
			{
				auto b = m->new_bone();

				b->name = read_string(file);
				b->type = read<char>(file);
				b->parent = read<int>(file);
				b->rootCoord = read<glm::vec3>(file);
			}

			auto ikCount = read<int>(file);
			m->iks.resize(boneCount);
			for (int i = 0; i < ikCount; i++)
			{
				auto b = m->new_bone_ik();

				b->targetID = read<int>(file);
				b->effectorID = read<int>(file);
				b->iterations = read<short>(file);
				b->weight = read<float>(file);

				b->chain.resize(read<int>(file));
				file.read((char*)b->chain.data(), sizeof(int) * b->chain.size());
			}

			if (animated)
			{
				m->stand_animation_filename = read_string(file);
				m->forward_animation_filename = read_string(file);
				m->backward_animation_filename = read_string(file);
				m->leftward_animation_filename = read_string(file);
				m->rightward_animation_filename = read_string(file);
				m->jump_animation_filename = read_string(file);
			}

			#if FLAME_ENABLE_PHYSICS
			auto rigidbodyCount = read_int(file);
			for (int i = 0; i < rigidbodyCount; i++)
			{
				auto r = m->new_rigidbody();

				r->type = (RigidbodyType)read_int(file);
				r->name = read_string(file);
				r->originCollisionGroupID = read_int(file);
				r->originCollisionFreeFlag = read_int(file);
				r->boneID = read_int(file);
				r->coord = read_float3(file);
				r->quat = read_float4(file);
				r->density = read_float(file);
				r->velocityAttenuation = read_float(file);
				r->rotationAttenuation = read_float(file);
				r->bounce = read_float(file);
				r->friction = read_float(file);

				auto shapeCount = read_int(file);
				for (int j = 0; j < shapeCount; j++)
				{
					auto s = r->new_shape();
					s->coord = read_float3(file);
					s->quat = read_float4(file);
					s->scale = read_float3(file);
					s->type = (ShapeType)read_int(file);
				}
			}

			auto jointCount = read_int(file);
			for (int i = 0; i < jointCount; i++)
			{
				auto j = m->new_joint();
				j->coord = read_float3(file);
				j->quat = read_float4(file);
				j->rigid0ID = read_int(file);
				j->rigid1ID = read_int(file);
				j->maxCoord = read_float3(file);
				j->minCoord = read_float3(file);
				j->maxRotation = read_float3(file);
				j->minRotation = read_float3(file);
				j->springConstant = read_float3(file);
				j->sprintRotationConstant = read_float3(file);
			}
			#endif

			m->bounding_position = read<glm::vec3>(file);
			m->bounding_size = read<float>(file);

			m->controller_height = read<float>(file);
			m->controller_radius = read<float>(file);

			m->eye_position = read<glm::vec3>(file);

			_process_model(m, false);
		}

		void save(Model *m, const std::string &filename)
		{
			std::ofstream file(filename, std::ios::binary);

			write<char>(file, m->vertexes_skeleton.size() > 0);

			write<int>(file, m->vertexes.size());
			write<int>(file, m->indices.size());
			if (m->vertexes.size() > 0)
			{
				file.write((char*)m->vertexes.data(), sizeof(ModelVertex) * m->vertexes.size());
				if (m->vertexes_skeleton.size() > 0)
					file.write((char*)m->vertexes_skeleton.data(), sizeof(ModelVertexSkeleton) * m->vertexes.size());
			}
			if (m->indices.size() > 0)
				file.write((char*)m->indices.data(), sizeof(int) * m->indices.size());

			write<int>(file, m->geometries.size());
			for (auto &g : m->geometries)
			{
				write<glm::vec4>(file, g->material->get_albedo_alpha());
				write<float>(file, g->material->get_spec());
				write<float>(file, g->material->get_roughness());
				write_string(file, g->material->get_albedo_alpha_map_name());
				write_string(file, g->material->get_spec_roughness_map_name());
				write_string(file, g->material->get_normal_height_map_name());

				write<int>(file, g->indiceBase);
				write<int>(file, g->indiceCount);
			}

			write<int>(file, m->bones.size());
			for (auto &b : m->bones)
			{
				write_string(file, b->name);
				write<char>(file, b->type);
				write<int>(file, b->parent);
				write<glm::vec3>(file, b->rootCoord);
			}

			write<int>(file, m->iks.size());
			for (auto &b : m->iks)
			{
				write<int>(file, b->targetID);
				write<int>(file, b->effectorID);
				write<short>(file, b->iterations);
				write<float>(file, b->weight);

				write<int>(file, b->chain.size());
				file.write((char*)b->chain.data(), sizeof(int) * b->chain.size());
			}

			if (m->vertexes_skeleton.size() > 0)
			{
				write_string(file, m->stand_animation_filename);
				write_string(file, m->forward_animation_filename);
				write_string(file, m->backward_animation_filename);
				write_string(file, m->leftward_animation_filename);
				write_string(file, m->rightward_animation_filename);
				write_string(file, m->jump_animation_filename);
			}

			#if FLAME_ENABLE_PHYSICS
			write_int(file, m->rigidbodies.size());
			for (auto &r : m->rigidbodies)
			{
				write_int(file, (int)r->type);
				write_string(file, r->name);
				write_int(file, r->originCollisionGroupID);
				write_int(file, r->originCollisionFreeFlag);
				write_int(file, r->boneID);
				write_float3(file, r->coord);
				write_float4(file, r->quat);
				write_float(file, r->density);
				write_float(file, r->velocityAttenuation);
				write_float(file, r->rotationAttenuation);
				write_float(file, r->bounce);
				write_float(file, r->friction);

				write_int(file, r->shapes.size());
				for (auto &s : r->shapes)
				{
					write_float3(file, s->coord);
					write_float4(file, s->quat);
					write_float3(file, s->scale);
					write_int(file, (int)s->type);
				}
			}

			write_int(file, m->joints.size());
			for (auto &j : m->joints)
			{
				write_float3(file, j->coord);
				write_float4(file, j->quat);
				write_int(file, j->rigid0ID);
				write_int(file, j->rigid1ID);
				write_float3(file, j->maxCoord);
				write_float3(file, j->minCoord);
				write_float3(file, j->maxRotation);
				write_float3(file, j->minRotation);
				write_float3(file, j->springConstant);
				write_float3(file, j->sprintRotationConstant);
			}
			#endif

			write<glm::vec3>(file, m->bounding_position);
			write<float>(file, m->bounding_size);

			write<float>(file, m->controller_height);
			write<float>(file, m->controller_radius);

			write<glm::vec3>(file, m->eye_position);
		}
	}

	void add_model(std::shared_ptr<Model> m)
	{
		auto hash = HASH(m->filename.c_str());
		_models[hash] = m;
		_create_vertex_and_index_buffer();
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

	void init_model()
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

#if FLAME_ENABLE_PHYSICS
			auto r = m->new_rigidbody();
			r->type = RigidbodyType::dynamic;
			auto s = r->new_shape();
			s->type = ShapeType::box;
			s->scale = glm::vec3(0.5f);
#endif

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

#if FLAME_ENABLE_PHYSICS
			auto r = m->new_rigidbody();
			r->type = RigidbodyType::dynamic;
			auto s = r->new_shape();
			s->type = ShapeType::sphere;
			s->scale = glm::vec3(0.5f);
#endif

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

#if FLAME_ENABLE_PHYSICS
			auto r = m->new_rigidbody();
			r->type = RigidbodyType::dynamic;
			auto s = r->new_shape();
			s->type = ShapeType::capsule;
			s->scale = glm::vec3(0.5f);
#endif

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
