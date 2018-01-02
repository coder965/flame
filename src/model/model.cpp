#include <sstream>
#include <regex>
#include <iostream>
#include <algorithm>
#include <map>

#include "../file_utils.h"
#include "../hash.h"
#include "../language.h"
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
	void Model::setStateAnimation(ModelStateAnimationKind kind, std::shared_ptr<AnimationBinding> b)
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
		static auto magicNumber = 0;
		auto r = new Rigidbody;
		r->id = magicNumber++;
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
		static auto magicNumber = 0;
		auto j = new Joint;
		j->id = magicNumber++;
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

	void addTriangleVertex(std::vector<glm::vec3> &positions, std::vector<glm::vec3> &normals, std::vector<int> &indices, glm::mat3 rotation, glm::vec3 center)
	{
		int baseVertex = positions.size();

		positions.insert(positions.end(), {
			center + rotation * glm::vec3(0.f, 0.f, 0.f), center + rotation * glm::vec3(1.f, 0.f, 0.f), center + rotation * glm::vec3(0.f, 1.f, 0.f)
			});
		normals.insert(normals.end(), 3, glm::vec3(0.f, 0.f, 1.f));
		for (int i = 0; i < 3; i++)
			indices.push_back(baseVertex + i);
	}

	void addCubeVertex(std::vector<glm::vec3> &positions, std::vector<glm::vec3> &normals, std::vector<int> &indices, glm::mat3 rotation, glm::vec3 center, float length)
	{
		int baseVertex = positions.size();

		glm::vec3 a = center + rotation * (glm::vec3(0.5f, -0.5f, 0.5f) * length);
		glm::vec3 b = center + rotation * (glm::vec3(0.5f, -0.5f, -0.5f) * length);
		glm::vec3 c = center + rotation * (glm::vec3(0.5f, 0.5f, -0.5f) * length);
		glm::vec3 d = center + rotation * (glm::vec3(0.5f, 0.5f, 0.5f) * length);
		glm::vec3 e = center + rotation * (glm::vec3(-0.5f, -0.5f, 0.5f) * length);
		glm::vec3 f = center + rotation * (glm::vec3(-0.5f, -0.5f, -0.5f) * length);
		glm::vec3 g = center + rotation * (glm::vec3(-0.5f, 0.5f, -0.5f) * length);
		glm::vec3 h = center + rotation * (glm::vec3(-0.5f, 0.5f, 0.5f) * length);

		positions.insert(positions.end(), {
			a, b, c, d,
			e, f, g, h,
			c, d, g, h,
			a, b, e, f,
			a, d, e, h,
			b, c, f, g
			});

		normals.insert(normals.end(), 4, rotation * glm::vec3(1.f, 0.f, 0.f));
		normals.insert(normals.end(), 4, rotation * glm::vec3(-1.f, 0.f, 0.f));
		normals.insert(normals.end(), 4, rotation * glm::vec3(0.f, 1.f, 0.f));
		normals.insert(normals.end(), 4, rotation * glm::vec3(0.f, -1.f, 0.f));
		normals.insert(normals.end(), 4, rotation * glm::vec3(0.f, 0.f, 1.f));
		normals.insert(normals.end(), 4, rotation * glm::vec3(0.f, 0.f, -1.f));

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

		indices.insert(indices.end(), list.begin(), list.end());
	}

	void addSphereVertex(std::vector<glm::vec3> &positions, std::vector<glm::vec3> &normals, std::vector<int> &indices, glm::mat3 rotation, glm::vec3 center, float radius, int horiSubdiv, int vertSubdiv)
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
				auto index = positions.size();
				indexs[level].push_back(index);
				glm::vec3 v = rotation * glm::vec3(cos(ang) * ringRadius, height, sin(ang) * ringRadius);
				positions.push_back(center + v);
				normals.push_back(glm::normalize(v));
			}
		}

		{
			auto index = positions.size();
			indexs[0].push_back(index);
			glm::vec3 v = rotation * glm::vec3(0.f, -radius, 0.f);
			positions.push_back(center + v);
			normals.push_back(glm::normalize(v));
		}

		{
			auto index = positions.size();
			indexs[horiSubdiv].push_back(index);
			glm::vec3 v = rotation * glm::vec3(0.f, radius, 0.f);
			positions.push_back(center + v);
			normals.push_back(glm::normalize(v));
		}

		for (int level = 0; level < horiSubdiv; level++)
		{
			if (level == 0)
			{
				for (int i = 0; i < vertSubdiv; i++)
				{
					auto ii = i + 1; if (ii == vertSubdiv) ii = 0;

					indices.push_back(indexs[0][0]);
					indices.push_back(indexs[1][i]);
					indices.push_back(indexs[1][ii]);
				}
			}
			else if (level == horiSubdiv - 1)
			{
				for (int i = 0; i < vertSubdiv; i++)
				{
					auto ii = i + 1; if (ii == vertSubdiv) ii = 0;

					indices.push_back(indexs[horiSubdiv - 1][i]);
					indices.push_back(indexs[horiSubdiv][0]);
					indices.push_back(indexs[horiSubdiv - 1][ii]);
				}
			}
			else
			{
				for (int i = 0; i < vertSubdiv; i++)
				{
					auto ii = i + 1; if (ii == vertSubdiv) ii = 0;

					indices.push_back(indexs[level][i]);
					indices.push_back(indexs[level + 1][i]);
					indices.push_back(indexs[level][ii]);

					indices.push_back(indexs[level][ii]);
					indices.push_back(indexs[level + 1][i]);
					indices.push_back(indexs[level + 1][ii]);
				}
			}
		}
	}

	void addCylinderVertex(std::vector<glm::vec3> &positions, std::vector<glm::vec3> &normals, std::vector<int> &indices, glm::mat3 rotation, glm::vec3 center, float halfHeight, float radius, int subdiv)
	{
		std::vector<std::vector<int>> indexs;
		indexs.resize(4);

		// top cap
		for (int i = 0; i < subdiv; i++)
		{
			auto ang = glm::radians(i * 360.f / subdiv);
			auto index = positions.size();
			indexs[0].push_back(index);
			positions.push_back(rotation * glm::vec3(cos(ang) * radius, halfHeight, sin(ang) * radius) + center);
			normals.push_back(rotation * glm::vec3(0.f, 1.f, 0.f));
		}

		// bottom cap
		for (int i = 0; i < subdiv; i++)
		{
			auto ang = glm::radians(i * 360.f / subdiv);
			auto index = positions.size();
			indexs[1].push_back(index);
			positions.push_back(rotation * glm::vec3(cos(ang) * radius, -halfHeight, sin(ang) * radius) + center);
			normals.push_back(rotation * glm::vec3(0.f, -1.f, 0.f));
		}

		// top
		for (int i = 0; i < subdiv; i++)
		{
			auto ang = glm::radians(i * 360.f / subdiv);
			auto index = positions.size();
			indexs[2].push_back(index);
			positions.push_back(rotation * glm::vec3(cos(ang) * radius, halfHeight, sin(ang) * radius) + center);
			normals.push_back(rotation * glm::vec3(cos(ang), 0.f, sin(ang)));
		}

		// bottom
		for (int i = 0; i < subdiv; i++)
		{
			auto ang = glm::radians(i * 360.f / subdiv);
			auto index = positions.size();
			indexs[3].push_back(index);
			positions.push_back(rotation * glm::vec3(cos(ang) * radius, -halfHeight, sin(ang) * radius) + center);
			normals.push_back(rotation * glm::vec3(cos(ang), 0.f, sin(ang)));
		}

		// top cap
		for (int i = 1; i < subdiv - 1; i++)
		{
			indices.push_back(indexs[0][i]);
			indices.push_back(indexs[0][0]);
			indices.push_back(indexs[0][i + 1]);
		}

		// bottom cap
		for (int i = 1; i < subdiv - 1; i++)
		{
			indices.push_back(indexs[1][i + 1]);
			indices.push_back(indexs[1][0]);
			indices.push_back(indexs[1][i]);
		}

		// middle
		for (int i = 0; i < subdiv; i++)
		{
			auto ii = i + 1;
			if (ii == subdiv)
				ii = 0;

			indices.push_back(indexs[2][i]);
			indices.push_back(indexs[2][ii]);
			indices.push_back(indexs[3][i]);

			indices.push_back(indexs[2][ii]);
			indices.push_back(indexs[3][ii]);
			indices.push_back(indexs[3][i]);
		}
	}

	void addConeVertex(std::vector<glm::vec3> &positions, std::vector<glm::vec3> &normals, std::vector<int> &indices, glm::mat3 rotation, glm::vec3 center, float height, float radius, int subdiv)
	{
		std::vector<std::vector<int>> indexs;
		indexs.resize(3);

		// top
		{
			auto index = positions.size();
			indexs[0].push_back(index);
			positions.push_back(rotation * glm::vec3(0.f, height, 0.f) + center);
			normals.push_back(rotation * glm::vec3(0.f, 1.f, 0.f));
		}

		// cap
		for (int i = 0; i < subdiv; i++)
		{
			auto ang = glm::radians(i * 360.f / subdiv);
			auto index = positions.size();
			indexs[1].push_back(index);
			positions.push_back(rotation * glm::vec3(cos(ang) * radius, 0.f, sin(ang) * radius) + center);
			normals.push_back(rotation * glm::vec3(0.f, -1.f, 0.f));
		}

		for (int i = 0; i < subdiv; i++)
		{
			auto ang = glm::radians(i * 360.f / subdiv);
			auto index = positions.size();
			indexs[2].push_back(index);
			positions.push_back(rotation * glm::vec3(cos(ang) * radius, 0.f, sin(ang) * radius) + center);
			normals.push_back(rotation * glm::vec3(cos(ang), 0.f, sin(ang)));
		}

		for (int i = 0; i < subdiv; i++)
		{
			auto ii = i + 1;
			if (ii == subdiv)
				ii = 0;

			indices.push_back(indexs[2][i]);
			indices.push_back(indexs[0][0]);
			indices.push_back(indexs[2][ii]);
		}

		for (int i = 1; i < subdiv - 1; i++)
		{
			indices.push_back(indexs[1][i + 1]);
			indices.push_back(indexs[1][0]);
			indices.push_back(indexs[1][i]);
		}
	}

	void addTorusVertex(std::vector<glm::vec3> &positions, std::vector<glm::vec3> &normals, std::vector<int> &indices, glm::mat3 rotation, glm::vec3 center, float radius, float sectionRadius, int axisSubdiv, int heightSubdiv)
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
				auto index = positions.size();
				indexs[i].push_back(index);
				positions.push_back(rotation * (center + R * (glm::vec3(cos(secang) * sectionRadius + radius, sin(secang) * sectionRadius, 0.f))));
				normals.push_back(rotation * R * glm::vec3(cos(secang), sin(secang), 0.f));
			}
		}

		for (int i = 0; i < axisSubdiv; i++)
		{
			auto ii = i + 1; if (ii == axisSubdiv) ii = 0;

			for (int j = 0; j < heightSubdiv; j++)
			{
				auto jj = j + 1; if (jj == heightSubdiv) jj = 0;

				indices.push_back(indexs[i][j]);
				indices.push_back(indexs[i][jj]);
				indices.push_back(indexs[ii][j]);

				indices.push_back(indexs[i][jj]);
				indices.push_back(indexs[ii][jj]);
				indices.push_back(indexs[ii][j]);
			}
		}
	}

	static void _process_model(Model *m, bool generateTangent)
	{
		m->maxCoord = m->vertex[0].position;
		m->minCoord = m->vertex[0].position;
		for (int i = 1; i < m->vertex_count; i++)
		{
			m->maxCoord = glm::max(m->maxCoord, m->vertex[i].position);
			m->minCoord = glm::min(m->minCoord, m->vertex[i].position);
		}

		if (generateTangent)
		{
			for (int i = 0; i < m->vertex_count; i += 3)
			{
				int id[3] = {
					m->indices[i],
					m->indices[i + 1],
					m->indices[i + 2]
				};

				auto u0 = m->vertex[id[1]].uv.s - m->vertex[id[0]].uv.s;
				auto v0 = m->vertex[id[1]].uv.t - m->vertex[id[0]].uv.t;

				auto u1 = m->vertex[id[2]].uv.s - m->vertex[id[0]].uv.s;
				auto v1 = m->vertex[id[2]].uv.t - m->vertex[id[0]].uv.t;

				auto e0 = m->vertex[id[1]].position - m->vertex[id[0]].position;
				auto e1 = m->vertex[id[2]].position - m->vertex[id[0]].position;

				auto d = u0 * v1 - u1 * v0;
				if (d == 0.f) continue;

				auto tangent = glm::vec3(v1 * e0.x - v0 * e1.x, v1 * e0.y - v0 * e1.y, v1 * e0.z - v0 * e1.z);
				if (glm::length(tangent) > 0.f)
				{
					tangent = glm::normalize(tangent);
					m->vertex[id[0]].tangent = tangent;
					m->vertex[id[1]].tangent = tangent;
					m->vertex[id[2]].tangent = tangent;
				}
				else
				{
					m->vertex[id[0]].tangent = glm::vec3(0.f);
					m->vertex[id[1]].tangent = glm::vec3(0.f);
					m->vertex[id[2]].tangent = glm::vec3(0.f);
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

		if (m->vertex_skeleton)
		{
			auto a = getAnimation(m->stand_animation_filename);
			if (a) m->stateAnimations[ModelStateAnimationStand] = get_animation_binding(m, a);
			a = getAnimation(m->forward_animation_filename);
			if (a) m->stateAnimations[ModelStateAnimationForward] = get_animation_binding(m, a);
			a = getAnimation(m->backward_animation_filename);
			if (a) m->stateAnimations[ModelStateAnimationBackward] = get_animation_binding(m, a);
			a = getAnimation(m->leftward_animation_filename);
			if (a) m->stateAnimations[ModelStateAnimationLeftward] = get_animation_binding(m, a);
			a = getAnimation(m->rightward_animation_filename);
			if (a) m->stateAnimations[ModelStateAnimationRightward] = get_animation_binding(m, a);
			a = getAnimation(m->jump_animation_filename);
			if (a) m->stateAnimations[ModelStateAnimationJump] = get_animation_binding(m, a);
		}
	}

	namespace OBJ
	{
		void load(Model *m, const std::string &filename)
		{
			std::ifstream file(filename);

			int currentIndex = 0;

			Geometry *currentGeometry = nullptr;

			std::vector<glm::vec3> rawPositions;
			std::vector<glm::vec2> rawTexcoords;
			std::vector<glm::vec3> rawNormals;
			std::vector<glm::ivec3> rawIndexs;

			std::vector<ModelVertex> vertexs;
			std::vector<int> indices;

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
					rawPositions.push_back(v);
				}
				else if (token == "vn")
				{
					glm::vec3 n;
					ss >> n.x;
					ss >> n.y;
					ss >> n.z;
					rawNormals.push_back(n);
				}
				else if (token == "vt")
				{
					glm::vec2 t;
					ss >> t.x;
					ss >> t.y;
					rawTexcoords.push_back(t);
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
						for (int i = 0; i < rawIndexs.size(); i++)
						{
							if (ids == rawIndexs[i])
							{
								index = i;
								break;
							}
						}
						if (index == -1)
						{
							index = m->vertex_count;
							vertexs.push_back({
								rawPositions[ids[0]],
								ids[1] != -1 ? rawTexcoords[ids[1]] : glm::vec2(0.f),
								ids[2] != -1 ? rawNormals[ids[2]] : glm::vec3(0.f),
								glm::vec3(0.f)
								});
							rawIndexs.push_back(ids);

						}
						indices.push_back(index);
						currentIndex++;
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
					currentGeometry->indiceBase = currentIndex;
					m->geometries.emplace_back(g);
				}
				else if (token == "mtllib")
				{
					std::string libName;
					ss >> libName;

					if (libName != "")
					{
						std::ifstream file(m->filepath + "/" + libName);

						std::string mtlName;
						float spec, roughness;
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
								albedoAlphaMap = getMaterialImage(m->filepath + "/" + filename, true);
							}
							else if (token == "map_bump")
							{
								std::string filename;
								ss >> filename;
								normalHeightMap = getMaterialImage(m->filepath + "/" + filename);
							}
						}

						auto m = getMaterial(glm::vec4(1.f), spec, roughness, albedoAlphaMap, normalHeightMap, nullptr);
						m->name = mtlName;
					}
				}
			}

			m->vertex_count = vertexs.size();
			m->vertex = std::make_unique<ModelVertex[]>(m->vertex_count);
			memcpy(m->vertex.get(), vertexs.data(), sizeof(ModelVertex) * m->vertex_count);
			m->indice_count = indices.size();
			m->indices = std::make_unique<int[]>(m->indice_count);
			memcpy(m->indices.get(), indices.data(), sizeof(int) * m->indice_count);

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

			file & m->vertex_count;
			m->vertex = std::make_unique<ModelVertex[]>(m->vertex_count);
			m->vertex_skeleton = std::make_unique<ModelVertexSkeleton[]>(m->vertex_count);
			for (int i = 0; i < m->vertex_count; i++)
			{
				VertexData data;
				file.read((char*)&data, sizeof(VertexData));
				m->vertex[i].position = data.position;
				m->vertex[i].position.z *= -1.f;
				m->vertex[i].normal = data.normal;
				m->vertex[i].normal.z *= -1.f;
				m->vertex[i].uv = data.uv;
				m->vertex[i].uv.y = 1.f - m->vertex[i].uv.y;
				float fWeight = data.weight / 100.f;
				m->vertex_skeleton[i].bone_weight.x = fWeight;
				m->vertex_skeleton[i].bone_weight.y = 1.f - fWeight;
				m->vertex_skeleton[i].bone_ID.x = data.boneID0;
				m->vertex_skeleton[i].bone_ID.y = data.boneID1;
			}

			file & m->indice_count;
			m->indices = std::make_unique<int[]>(m->indice_count);
			for (int i = 0; i < m->indice_count; i += 3)
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
				g->material = getMaterial(data.diffuse, 0.f, 1.f, 
					getMaterialImage(m->filepath + "/" + data.mapName, true), nullptr, nullptr);
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
				r->setCoord(data.location);
				data.rotation = glm::degrees(data.rotation);
				r->setQuat(mat3_to_quaternion(euler_yxz_to_mat3(
					glm::vec3(-data.rotation.y, -data.rotation.x, data.rotation.z))));
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
				s->setScale(data.size);
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
				j->setCoord(data.coord);
				j->setQuat(mat3_to_quaternion(euler_yxz_to_mat3(
					glm::vec3(-data.rotation.y, -data.rotation.x, data.rotation.z))));
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

			std::vector<ModelVertex> vertexs;
			std::vector<int> indices;
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
										vertexs.push_back({
											sources[position_source_index]->v3(index),
											glm::vec2(0.f),
											glm::vec3(0.f),
											glm::vec3(0.f)
											});
										indices.push_back(index);
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
											index = vertexs.size();
											vertexs.push_back({
												sources[position_source_index]->v3(id[position_offset]),
												uv_source_index == -1 ? glm::vec2(0.f) : sources[uv_source_index]->v2(id[uv_offset]),
												normal_source_index == -1 ? glm::vec3(0.f) : sources[normal_source_index]->v3(id[normal_offset]),
												glm::vec3(0.f)
												});
											ids.push_back(id);

										}
										indices.push_back(index);
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
											index = vertexs.size();
											vertexs.push_back({
												sources[position_source_index]->v3(id[position_offset]),
												uv_source_index == -1 ? glm::vec2(0.f) : sources[uv_source_index]->v2(id[uv_offset]),
												normal_source_index == -1 ? glm::vec3(0.f) : sources[normal_source_index]->v3(id[normal_offset]),
												glm::vec3(0.f)
												});
											ids.push_back(id);

										}
										indices.push_back(index);
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

			m->vertex_count = vertexs.size();
			m->vertex = std::make_unique<ModelVertex[]>(m->vertex_count);
			memcpy(m->vertex.get(), vertexs.data(), sizeof(ModelVertex) * m->vertex_count);
			m->indice_count = indices.size();
			m->indices = std::make_unique<int[]>(m->indice_count);
			memcpy(m->indices.get(), indices.data(), sizeof(int) * m->indice_count);

			auto g = new Geometry;
			g->material = defaultMaterial;
			g->indiceCount = m->indice_count;
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

			file & m->vertex_count;
			file & m->indice_count;
			if (m->vertex_count > 0)
			{
				m->vertex = std::make_unique<ModelVertex[]>(m->vertex_count);
				file.read((char*)m->vertex.get(), sizeof(ModelVertex) * m->vertex_count);
				if (animated)
				{
					m->vertex_skeleton = std::make_unique<ModelVertexSkeleton[]>(m->vertex_count);
					file.read((char*)m->vertex_skeleton.get(), sizeof(ModelVertexSkeleton) * m->vertex_count);
				}
			}
			if (m->indice_count > 0)
			{
				m->indices = std::make_unique<int[]>(m->indice_count);
				file.read((char*)m->indices.get(), sizeof(int) * m->indice_count);
			}

			int geometryCount;
			file & geometryCount;
			for (int i = 0; i < geometryCount; i++)
			{
				glm::vec4 albedo_alpha;
				float spec, roughness;
				file & albedo_alpha;
				file & spec;
				file & roughness;
				std::string albedoAlphaMapName;
				std::string normalHeightMapName;
				std::string specRoughnessMapName;
				file > albedoAlphaMapName;
				file > normalHeightMapName;
				file > specRoughnessMapName;

				auto g = new Geometry;
				g->material = getMaterial(albedo_alpha, spec, roughness,
					getMaterialImage(m->filepath + "/" + albedoAlphaMapName, true),
					getMaterialImage(m->filepath + "/" + normalHeightMapName, true),
					getMaterialImage(m->filepath + "/" + specRoughnessMapName, true));
				file & g->indiceBase;
				file & g->indiceCount;
				file & g->visible;

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
				glm::vec3 coord;
				file & coord;
				r->setCoord(coord);
				glm::vec3 euler;
				file & euler;
				r->setEuler(euler);
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
					glm::vec3 coord;
					file & coord;
					s->setCoord(coord);
					glm::vec3 euler;
					file & euler;
					s->setEuler(euler);
					glm::vec3 scale;
					file & scale;
					s->setScale(scale);
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
				glm::vec3 coord;
				file & coord;
				j->setCoord(coord);
				glm::vec3 euler;
				file & euler;
				j->setEuler(euler);
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

			bool animated = m->vertex_skeleton.get();
			file & animated;

			file & m->vertex_count;
			file & m->indice_count;
			if (m->vertex_count > 0)
			{
				file.write((char*)m->vertex.get(), sizeof(ModelVertex) * m->vertex_count);
				if (animated)
					file.write((char*)m->vertex_skeleton.get(), sizeof(ModelVertexSkeleton) * m->vertex_count);
			}
			if (m->indice_count > 0)
				file.write((char*)m->indices.get(), sizeof(int) * m->indice_count);

			int geometryCount = m->geometries.size();
			file & geometryCount;
			for (auto &g : m->geometries)
			{
				file & g->material->albedo_alpha;
				file & g->material->spec;
				file & g->material->roughness;
				file < (g->material->albedoAlphaMap ? g->material->albedoAlphaMap->filename : "");
				file < (g->material->normalHeightMap ? g->material->normalHeightMap->filename : "");
				file < (g->material->specRoughnessMap ? g->material->specRoughnessMap->filename : "");

				file & g->indiceBase;
				file & g->indiceCount;
				file & g->visible;
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
				file & r->getCoord();
				file & r->getEuler();
				file & r->density;
				file & r->velocityAttenuation;
				file & r->rotationAttenuation;
				file & r->bounce;
				file & r->friction;

				int shapeCount = r->shapes.size();
				file & shapeCount;
				for (auto &s : r->shapes)
				{
					file & s->getCoord();
					file & s->getEuler();
					file & s->getScale();
					int type = (int)s->type;
					file & type;
				}
			}

			int jointCount = m->joints.size();
			file & jointCount;
			for (auto &j : m->joints)
			{
				file & j->getCoord();
				file & j->getEuler();
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

	std::map<unsigned int, std::weak_ptr<Model>> _models;
	void _processVertexAndIndexBuffer()
	{
		auto vertex_stat_count = 0;
		auto vertex_anim_count = 0;
		auto indice_count = 0;

		for (auto &m : _models)
		{
			auto s = m.second.lock();
			if (s)
			{
				vertex_stat_count += s->vertex_count;
				if (s->vertex_skeleton)
					vertex_anim_count += s->vertex_count;
				indice_count += s->indice_count;
			}
		}

		auto vss = vertex_stat_count > 0 ? sizeof(ModelVertex) * vertex_stat_count : 1;
		auto vas = vertex_anim_count > 0 ? sizeof(ModelVertexSkeleton) * vertex_anim_count : 1;
		auto is = indice_count > 0 ? sizeof(int) * indice_count : 1;

		vertexStatBuffer = std::make_unique<VertexBuffer>(vss);
		vertexAnimBuffer = std::make_unique<VertexBuffer>(vas);
		indexBuffer = std::make_unique<IndexBuffer>(is);

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
				if (s->vertex_skeleton)
				{
					s->vertexBase = vertex_offset;
					s->indiceBase = indice_offset;
					memcpy(vs_map + vertex_offset * sizeof(ModelVertex), s->vertex.get(), sizeof(ModelVertex) * s->vertex_count);
					memcpy(va_map + vertex_offset * sizeof(ModelVertexSkeleton), s->vertex_skeleton.get(), sizeof(ModelVertexSkeleton) * s->vertex_count);
					memcpy(i_map + indice_offset * sizeof(int), s->indices.get(), sizeof(int) * s->indice_count);
					vertex_offset += s->vertex_count;
					indice_offset += s->indice_count;
				}
			}
		}
		for (auto &m : _models)
		{
			auto s = m.second.lock();
			if (s)
			{
				if (!s->vertex_skeleton)
				{
					s->vertexBase = vertex_offset;
					s->indiceBase = indice_offset;
					memcpy(vs_map + vertex_offset * sizeof(ModelVertex), s->vertex.get(), sizeof(ModelVertex) * s->vertex_count);
					memcpy(i_map + indice_offset * sizeof(int), s->indices.get(), sizeof(int) * s->indice_count);
					vertex_offset += s->vertex_count;
					indice_offset += s->indice_count;
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
			stagingBuffer.copyTo(vertexStatBuffer.get(), 1, &range);
		}
		if (vertex_anim_count > 0)
		{
			VkBufferCopy range = {};
			range.srcOffset = vao;
			range.dstOffset = 0;
			range.size = sizeof(ModelVertexSkeleton) * vertex_anim_count;
			stagingBuffer.copyTo(vertexAnimBuffer.get(), 1, &range);
		}
		if (indice_count > 0)
		{
			VkBufferCopy range = {};
			range.srcOffset = io;
			range.dstOffset = 0;
			range.size = sizeof(int) * indice_count;
			stagingBuffer.copyTo(indexBuffer.get(), 1, &range);
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
		_processVertexAndIndexBuffer();
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
			static VkVertexInputBindingDescription bindings = {0, sizeof(ModelVertex), VK_VERTEX_INPUT_RATE_VERTEX};

			static VkVertexInputAttributeDescription attributes[] = {
				{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(ModelVertex, position)},
				{1, 0, VK_FORMAT_R32G32_SFLOAT,	   offsetof(ModelVertex, uv)},
				{2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(ModelVertex, normal)},
				{3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(ModelVertex, tangent)}
			};

			vertexStatInputState = vertexStateInfo(1, &bindings, TK_ARRAYSIZE(attributes), attributes);
		}

		{
			static VkVertexInputBindingDescription bindings[] = {
				{0, sizeof(ModelVertex), VK_VERTEX_INPUT_RATE_VERTEX},
				{1, sizeof(ModelVertexSkeleton), VK_VERTEX_INPUT_RATE_VERTEX}
			};

			static VkVertexInputAttributeDescription attributes[] = {
				{0, 0, VK_FORMAT_R32G32B32_SFLOAT,    offsetof(ModelVertex, position)},
				{1, 0, VK_FORMAT_R32G32_SFLOAT,       offsetof(ModelVertex, uv)},
				{2, 0, VK_FORMAT_R32G32B32_SFLOAT,    offsetof(ModelVertex, normal)},
				{3, 0, VK_FORMAT_R32G32B32_SFLOAT,    offsetof(ModelVertex, tangent)},
				{4, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(ModelVertexSkeleton, bone_weight)},
				{5, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(ModelVertexSkeleton, bone_ID)}
			};

			vertexAnimInputState = vertexStateInfo(TK_ARRAYSIZE(bindings), bindings, TK_ARRAYSIZE(attributes), attributes);
		}

		{
			auto m = std::make_shared<Model>();
			m->filename = "[triangle].tkm";

			std::vector<glm::vec3> vertexs;
			std::vector<glm::vec3> normals;
			std::vector<int> indices;

			addTriangleVertex(vertexs, normals, indices, glm::mat3(1.f), glm::vec3(0.f));

			m->vertex_count = vertexs.size();
			m->vertex = std::make_unique<ModelVertex[]>(m->vertex_count);
			for (int i = 0; i < m->vertex_count; i++)
			{
				m->vertex[i] = {
					vertexs[i],
					glm::vec2(0.f),
					normals[i],
					glm::vec3(0.f)
				};
			}
			m->indice_count = indices.size();
			m->indices = std::make_unique<int[]>(m->indice_count);
			memcpy(m->indices.get(), indices.data(), sizeof(int) * m->indice_count);

			auto g = new Geometry;
			g->material = defaultMaterial;
			g->indiceCount = m->indice_count;
			m->geometries.emplace_back(g);

			_process_model(m.get(), true);

			_models[HASH(m->filename.c_str())] = m;

			triangleModel = m;
		}

		{
			auto m = std::make_shared<Model>();
			m->filename = "[cube].tkm";

			std::vector<glm::vec3> vertexs;
			std::vector<glm::vec3> normals;
			std::vector<int> indices;

			addCubeVertex(vertexs, normals, indices, glm::mat3(1.f), glm::vec3(0.f), 1.f);

			m->vertex_count = vertexs.size();
			m->vertex = std::make_unique<ModelVertex[]>(m->vertex_count);
			for (int i = 0; i < m->vertex_count; i++)
			{
				m->vertex[i] = {
					vertexs[i],
					glm::vec2(0.f),
					normals[i],
					glm::vec3(0.f)
				};
			}
			m->indice_count = indices.size();
			m->indices = std::make_unique<int[]>(m->indice_count);
			memcpy(m->indices.get(), indices.data(), sizeof(int) * m->indice_count);

			auto g = new Geometry;
			g->material = defaultMaterial;
			g->indiceCount = m->indice_count;
			m->geometries.emplace_back(g);

			auto r = m->new_rigidbody();
			r->type = RigidbodyType::dynamic;
			auto s = r->new_shape();
			s->type = ShapeType::box;
			s->setScale(glm::vec3(0.5f));

			_process_model(m.get(), true);

			_models[HASH(m->filename.c_str())] = m;

			cubeModel = m;
		}

		{
			auto m = std::make_shared<Model>();
			m->filename = "[sphere].tkm";

			std::vector<glm::vec3> vertexs;
			std::vector<glm::vec3> normals;
			std::vector<int> indices;

			addSphereVertex(vertexs, normals, indices, glm::mat3(1.f), glm::vec3(0.f), 0.5f, 32, 32);

			m->vertex_count = vertexs.size();
			m->vertex = std::make_unique<ModelVertex[]>(m->vertex_count);
			for (int i = 0; i < m->vertex_count; i++)
			{
				m->vertex[i] = {
					vertexs[i],
					glm::vec2(0.f),
					normals[i],
					glm::vec3(0.f)
				};
			}
			m->indice_count = indices.size();
			m->indices = std::make_unique<int[]>(m->indice_count);
			memcpy(m->indices.get(), indices.data(), sizeof(int) * m->indice_count);

			auto g0 = new Geometry;
			g0->material = defaultMaterial;
			g0->indiceCount = m->indice_count / 2;
			auto g1 = new Geometry;
			g1->material = defaultMaterial;
			g1->indiceBase = g0->indiceCount;
			g1->indiceCount = g0->indiceCount;
			m->geometries.emplace_back(g0);
			m->geometries.emplace_back(g1);

			auto r = m->new_rigidbody();
			r->type = RigidbodyType::dynamic;
			auto s = r->new_shape();
			s->type = ShapeType::sphere;
			s->setScale(glm::vec3(0.5f));

			_process_model(m.get(), true);

			_models[HASH(m->filename.c_str())] = m;

			sphereModel = m;
		}

		{
			auto m = std::make_shared<Model>();
			m->filename = "[cylinder].tkm";

			std::vector<glm::vec3> vertexs;
			std::vector<glm::vec3> normals;
			std::vector<int> indices;

			addCylinderVertex(vertexs, normals, indices, glm::mat3(1.f), glm::vec3(0.f), 0.5f, 0.5f, 32);

			m->vertex_count = vertexs.size();
			m->vertex = std::make_unique<ModelVertex[]>(m->vertex_count);
			for (int i = 0; i < m->vertex_count; i++)
			{
				m->vertex[i] = {
					vertexs[i],
					glm::vec2(0.f),
					normals[i],
					glm::vec3(0.f)
				};
			}
			m->indice_count = indices.size();
			m->indices = std::make_unique<int[]>(m->indice_count);
			memcpy(m->indices.get(), indices.data(), sizeof(int) * m->indice_count);

			auto g = new Geometry;
			g->material = defaultMaterial;
			g->indiceCount = m->indice_count;
			m->geometries.emplace_back(g);

			auto r = m->new_rigidbody();
			r->type = RigidbodyType::dynamic;
			auto s = r->new_shape();
			s->type = ShapeType::capsule;
			s->setScale(glm::vec3(0.5f));

			_process_model(m.get(), true);

			_models[HASH(m->filename.c_str())] = m;

			cylinderModel = m;
		}

		{
			auto m = std::make_shared<Model>();
			m->filename = "[cone].tkm";

			std::vector<glm::vec3> vertexs;
			std::vector<glm::vec3> normals;
			std::vector<int> indices;

			addConeVertex(vertexs, normals, indices, glm::mat3(1.f), glm::vec3(0.f), 0.5f, 0.5f, 32);

			m->vertex_count = vertexs.size();
			m->vertex = std::make_unique<ModelVertex[]>(m->vertex_count);
			for (int i = 0; i < m->vertex_count; i++)
			{
				m->vertex[i] = {
					vertexs[i],
					glm::vec2(0.f),
					normals[i],
					glm::vec3(0.f)
				};
			}
			m->indice_count = indices.size();
			m->indices = std::make_unique<int[]>(m->indice_count);
			memcpy(m->indices.get(), indices.data(), sizeof(int) * m->indice_count);

			auto g = new Geometry;
			g->material = defaultMaterial;
			g->indiceCount = m->indice_count;
			m->geometries.emplace_back(g);

			_process_model(m.get(), true);

			_models[HASH(m->filename.c_str())] = m;

			coneModel = m;
		}

		{
			auto m = std::make_shared<Model>();
			m->filename = "[arrow].tkm";

			std::vector<glm::vec3> vertexs;
			std::vector<glm::vec3> normals;
			std::vector<int> indices;

			glm::mat3 matR = glm::mat3(glm::rotate(glm::radians(-90.f), glm::vec3(0.f, 0.f, 1.f)));

			addCylinderVertex(vertexs, normals, indices, matR, glm::vec3(0.4f, 0.f, 0.f), 0.4f, 0.01f, 32);
			addConeVertex(vertexs, normals, indices, matR, glm::vec3(0.8f, 0.f, 0.f), 0.2f, 0.05f, 32);

			m->vertex_count = vertexs.size();
			m->vertex = std::make_unique<ModelVertex[]>(m->vertex_count);
			for (int i = 0; i < m->vertex_count; i++)
			{
				m->vertex[i] = {
					vertexs[i],
					glm::vec2(0.f),
					normals[i],
					glm::vec3(0.f)
				};
			}
			m->indice_count = indices.size();
			m->indices = std::make_unique<int[]>(m->indice_count);
			memcpy(m->indices.get(), indices.data(), sizeof(int) * m->indice_count);

			auto g = new Geometry;
			g->material = defaultMaterial;
			g->indiceCount = m->indice_count;
			m->geometries.emplace_back(g);

			_process_model(m.get(), true);

			_models[HASH(m->filename.c_str())] = m;

			arrowModel = m;
		}

		{
			auto m = std::make_shared<Model>();
			m->filename = "[torus].tkm";

			std::vector<glm::vec3> vertexs;
			std::vector<glm::vec3> normals;
			std::vector<int> indices;

			glm::mat3 matR = glm::mat3(glm::rotate(glm::radians(-90.f), glm::vec3(0.f, 0.f, 1.f)));

			addTorusVertex(vertexs, normals, indices, matR, glm::vec3(), 1.f, 0.01f, 32, 32);

			m->vertex_count = vertexs.size();
			m->vertex = std::make_unique<ModelVertex[]>(m->vertex_count);
			for (int i = 0; i < m->vertex_count; i++)
			{
				m->vertex[i] = {
					vertexs[i],
					glm::vec2(0.f),
					normals[i],
					glm::vec3(0.f)
				};
			}
			m->indice_count = indices.size();
			m->indices = std::make_unique<int[]>(m->indice_count);
			memcpy(m->indices.get(), indices.data(), sizeof(int) * m->indice_count);

			auto g = new Geometry;
			g->material = defaultMaterial;
			g->indiceCount = m->indice_count;
			m->geometries.emplace_back(g);

			_process_model(m.get(), true);

			_models[HASH(m->filename.c_str())] = m;

			torusModel = m;
		}

		{
			auto m = std::make_shared<Model>();
			m->filename = "[hammer].tkm";

			std::vector<glm::vec3> vertexs;
			std::vector<glm::vec3> normals;
			std::vector<int> indices;

			glm::mat3 matR = glm::mat3(glm::rotate(glm::radians(-90.f), glm::vec3(0.f, 0.f, 1.f)));

			addCylinderVertex(vertexs, normals, indices, matR, glm::vec3(0.45f, 0.f, 0.f), 0.45f, 0.01f, 32);
			int ic0 = indices.size();
			addCubeVertex(vertexs, normals, indices, matR, glm::vec3(0.9f, 0.f, 0.f), 0.1f);
			int ic1 = indices.size();

			m->vertex_count = vertexs.size();
			m->vertex = std::make_unique<ModelVertex[]>(m->vertex_count);
			for (int i = 0; i < m->vertex_count; i++)
			{
				m->vertex[i] = {
					vertexs[i],
					glm::vec2(0.f),
					normals[i],
					glm::vec3(0.f)
				};
			}
			m->indice_count = indices.size();
			m->indices = std::make_unique<int[]>(m->indice_count);
			memcpy(m->indices.get(), indices.data(), sizeof(int) * m->indice_count);

			auto g0 = new Geometry;
			g0->material = defaultMaterial;
			g0->indiceCount = ic0;
			auto g1 = new Geometry;
			g1->material = defaultMaterial;
			g1->indiceBase = ic0;
			g1->indiceCount = ic1 - ic0;
			m->geometries.emplace_back(g0);
			m->geometries.emplace_back(g1);

			_process_model(m.get(), true);

			_models[HASH(m->filename.c_str())] = m;

			hamerModel = m;
		}

		_processVertexAndIndexBuffer();
	}
}
