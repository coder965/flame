#include <sstream>
#include <regex>
#include <iostream>
#include <algorithm>

#include "model.h"
#include "../core.h"

namespace tke
{
	void Model::loadData(bool needRigidbody)
	{
		AttributeTree at("data", filename + ".xml");
		if (!at.good)
			return;

		if (needRigidbody)
		{
			for (auto r : rigidbodies)
				delete r;
			rigidbodies.clear();
		}

		at.obtainFromAttributes(this, b);

		for (auto &c : at.children)
		{
			if (c->name == "rigid_body")
			{
				auto r = new Rigidbody;
				rigidbodies.push_back(r);
			}
		}
	}

	void Model::saveData(bool needRigidbody)
	{
		AttributeTree at("data");

		at.addAttributes(this, b);

		if (needRigidbody)
		{
			for (auto r : rigidbodies)
			{
				auto n = new AttributeTreeNode("rigid_body");
				at.add(n);
			}
		}

		at.saveXML(filename + ".xml");
	}

	AnimationBinding *Model::bindAnimation(Animation *a)
	{
		for (auto b : animationBindings)
		{
			if (b->animation == a)
				return b;
		}

		auto b = a->bindTo(this);
		animationBindings.push_back(b);
		return b;
	}

	void Model::setStateAnimation(ModelStateAnimationKind kind, AnimationBinding *b)
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

	void Model::addRigidbody(Rigidbody *pRigidbody)
	{
		static auto magicNumber = 0;
		pRigidbody->id = magicNumber++;
		rigidbodies.push_back(pRigidbody);
	}

	Rigidbody *Model::deleteRigidbody(Rigidbody *pRigidBody)
	{
		for (auto it = rigidbodies.begin(); it != rigidbodies.end(); it++)
		{
			if (*it == pRigidBody)
			{
				if (it > rigidbodies.begin())
					pRigidBody = *(it - 1);
				else
					pRigidBody = nullptr;
				rigidbodies.erase(it);
				break;
			}
		}
		return pRigidBody;
	}

	void Model::addJoint(Joint *pJoint)
	{
		static auto magicNumber = 0;
		pJoint->id = magicNumber++;
		joints.push_back(pJoint);
	}

	static void _add_model(Model *m)
	{
		models.push_back(std::move(std::unique_ptr<Model>(m)));
		needUpdateVertexBuffer = true;
		needUpdateMaterialBuffer = true;
		needUpdateTexture = true;
	}

	void addTriangleVertex(Model *m, glm::mat3 rotation, glm::vec3 center)
	{
		int baseVertex = m->positions.size();

		m->positions.insert(m->positions.end(), { center + rotation * glm::vec3(0.f, 0.f, 0.f), center + rotation * glm::vec3(0.f, 1.f, 0.f), center + rotation * glm::vec3(1.f, 0.f, 0.f) });
		m->normals.insert(m->normals.end(), 3, glm::vec3(0.f, 1.f, 0.f));
		for (int i = 0; i < 3; i++) m->indices.push_back(baseVertex + i);
	}

	void addCubeVertex(Model *m, glm::mat3 rotation, glm::vec3 center, float length)
	{
		int baseVertex = m->positions.size();

		glm::vec3 a = center + rotation * (glm::vec3(0.5f, -0.5f, 0.5f) * length);
		glm::vec3 b = center + rotation * (glm::vec3(0.5f, -0.5f, -0.5f) * length);
		glm::vec3 c = center + rotation * (glm::vec3(0.5f, 0.5f, -0.5f) * length);
		glm::vec3 d = center + rotation * (glm::vec3(0.5f, 0.5f, 0.5f) * length);
		glm::vec3 e = center + rotation * (glm::vec3(-0.5f, -0.5f, 0.5f) * length);
		glm::vec3 f = center + rotation * (glm::vec3(-0.5f, -0.5f, -0.5f) * length);
		glm::vec3 g = center + rotation * (glm::vec3(-0.5f, 0.5f, -0.5f) * length);
		glm::vec3 h = center + rotation * (glm::vec3(-0.5f, 0.5f, 0.5f) * length);

		m->positions.insert(m->positions.end(), {
			a, b, c, d,
			e, f, g, h,
			c, d, g, h,
			a, b, e, f,
			a, d, e, h,
			b, c, f, g
		});

		m->normals.insert(m->normals.end(), 4, rotation * glm::vec3(1.f, 0.f, 0.f));
		m->normals.insert(m->normals.end(), 4, rotation * glm::vec3(-1.f, 0.f, 0.f));
		m->normals.insert(m->normals.end(), 4, rotation * glm::vec3(0.f, 1.f, 0.f));
		m->normals.insert(m->normals.end(), 4, rotation * glm::vec3(0.f, -1.f, 0.f));
		m->normals.insert(m->normals.end(), 4, rotation * glm::vec3(0.f, 0.f, 1.f));
		m->normals.insert(m->normals.end(), 4, rotation * glm::vec3(0.f, 0.f, -1.f));

		std::vector<int> list = {
			3, 0, 1, 3, 1, 2,
			6, 5, 4, 6, 4, 7,
			11, 9, 8, 11, 8, 10,
			12, 14, 15, 12, 15, 13,
			19, 18, 16, 19, 16, 17,
			21, 20, 22, 21, 22, 23
		};

		for (auto &i : list) i += baseVertex;

		m->indices.insert(m->indices.end(), list.begin(), list.end());
	}

	void addSphereVertex(Model *m, glm::mat3 rotation, glm::vec3 center, float radius, int horiSubdiv, int vertSubdiv)
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
				auto index = m->positions.size();
				indexs[level].push_back(index);
				glm::vec3 v = rotation * glm::vec3(cos(ang) * ringRadius, height, sin(ang) * ringRadius);
				m->positions.push_back(center + v);
				m->normals.push_back(glm::normalize(v));
			}
		}

		{
			auto index = m->positions.size();
			indexs[0].push_back(index);
			glm::vec3 v = rotation * glm::vec3(0.f, -radius, 0.f);
			m->positions.push_back(center + v);
			m->normals.push_back(glm::normalize(v));
		}

		{
			auto index = m->positions.size();
			indexs[horiSubdiv].push_back(index);
			glm::vec3 v = rotation * glm::vec3(0.f, radius, 0.f);
			m->positions.push_back(center + v);
			m->normals.push_back(glm::normalize(v));
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

	void addCylinderVertex(Model *m, glm::mat3 rotation, glm::vec3 center, float halfHeight, float radius, int subdiv)
	{
		std::vector<std::vector<int>> indexs;
		indexs.resize(4);

		// top cap
		for (int i = 0; i < subdiv; i++)
		{
			auto ang = glm::radians(i * 360.f / subdiv);
			auto index = m->positions.size();
			indexs[0].push_back(index);
			m->positions.push_back(rotation * glm::vec3(cos(ang) * radius, halfHeight, sin(ang) * radius) + center);
			m->normals.push_back(rotation * glm::vec3(0.f, 1.f, 0.f));
		}

		// bottom cap
		for (int i = 0; i < subdiv; i++)
		{
			auto ang = glm::radians(i * 360.f / subdiv);
			auto index = m->positions.size();
			indexs[1].push_back(index);
			m->positions.push_back(rotation * glm::vec3(cos(ang) * radius, -halfHeight, sin(ang) * radius) + center);
			m->normals.push_back(rotation * glm::vec3(0.f, -1.f, 0.f));
		}

		// top
		for (int i = 0; i < subdiv; i++)
		{
			auto ang = glm::radians(i * 360.f / subdiv);
			auto index = m->positions.size();
			indexs[2].push_back(index);
			m->positions.push_back(rotation * glm::vec3(cos(ang) * radius, halfHeight, sin(ang) * radius) + center);
			m->normals.push_back(rotation * glm::vec3(cos(ang), 0.f, sin(ang)));
		}

		// bottom
		for (int i = 0; i < subdiv; i++)
		{
			auto ang = glm::radians(i * 360.f / subdiv);
			auto index = m->positions.size();
			indexs[3].push_back(index);
			m->positions.push_back(rotation * glm::vec3(cos(ang) * radius, -halfHeight, sin(ang) * radius) + center);
			m->normals.push_back(rotation * glm::vec3(cos(ang), 0.f, sin(ang)));
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
			auto ii = i + 1; if (ii == subdiv) ii = 0;

			m->indices.push_back(indexs[2][i]);
			m->indices.push_back(indexs[2][ii]);
			m->indices.push_back(indexs[3][i]);

			m->indices.push_back(indexs[2][ii]);
			m->indices.push_back(indexs[3][ii]);
			m->indices.push_back(indexs[3][i]);
		}
	}

	void addConeVertex(Model *m, glm::mat3 rotation, glm::vec3 center, float height, float radius, int subdiv)
	{
		std::vector<std::vector<int>> indexs;
		indexs.resize(3);

		// top
		{
			auto index = m->positions.size();
			indexs[0].push_back(index);
			m->positions.push_back(rotation * glm::vec3(0.f, height, 0.f) + center);
			m->normals.push_back(rotation * glm::vec3(0.f, 1.f, 0.f));
		}

		// cap
		for (int i = 0; i < subdiv; i++)
		{
			auto ang = glm::radians(i * 360.f / subdiv);
			auto index = m->positions.size();
			indexs[1].push_back(index);
			m->positions.push_back(rotation * glm::vec3(cos(ang) * radius, 0.f, sin(ang) * radius) + center);
			m->normals.push_back(rotation * glm::vec3(0.f, -1.f, 0.f));
		}

		for (int i = 0; i < subdiv; i++)
		{
			auto ang = glm::radians(i * 360.f / subdiv);
			auto index = m->positions.size();
			indexs[2].push_back(index);
			m->positions.push_back(rotation * glm::vec3(cos(ang) * radius, 0.f, sin(ang) * radius) + center);
			m->normals.push_back(rotation * glm::vec3(cos(ang), 0.f, sin(ang)));
		}

		for (int i = 0; i < subdiv; i++)
		{
			auto ii = i + 1; if (ii == subdiv) ii = 0;

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

	void addTorusVertex(Model *m, glm::mat3 rotation, glm::vec3 center, float radius, float sectionRadius, int axisSubdiv, int heightSubdiv)
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
				auto index = m->positions.size();
				indexs[i].push_back(index);
				m->positions.push_back(rotation * (center + R * (glm::vec3(cos(secang) * sectionRadius + radius, sin(secang) * sectionRadius, 0.f))));
				m->normals.push_back(rotation * R * glm::vec3(cos(secang), sin(secang), 0.f));
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

	Model *triangleModel = nullptr;
	Model *cubeModel = nullptr;
	Model *sphereModel = nullptr;
	Model *cylinderModel = nullptr;
	Model *coneModel = nullptr;
	Model *arrowModel = nullptr;
	Model *torusModel = nullptr;
	Model *hamerModel = nullptr;

	static void _model_after_process(Model *m)
	{
		m->maxCoord = m->positions[0];
		m->minCoord = m->positions[0];
		for (int i = 1; i < m->positions.size(); i++)
		{
			m->maxCoord = glm::max(m->maxCoord, m->positions[i]);
			m->minCoord = glm::min(m->minCoord, m->positions[i]);
		}

		if (m->tangents.size() == 0 && m->uvs.size() > 0)
		{
			m->tangents.resize(m->positions.size());

			for (int i = 0; i < m->indices.size(); i += 3)
			{
				int id[3] = {
					m->indices[i],
					m->indices[i + 1],
					m->indices[i + 2]
				};

				auto u0 = m->uvs[id[1]].s - m->uvs[id[0]].s;
				auto v0 = m->uvs[id[1]].t - m->uvs[id[0]].t;

				auto u1 = m->uvs[id[2]].s - m->uvs[id[0]].s;
				auto v1 = m->uvs[id[2]].t - m->uvs[id[0]].t;

				auto e0 = m->positions[id[1]] - m->positions[id[0]];
				auto e1 = m->positions[id[2]] - m->positions[id[0]];

				auto d = u0 * v1 - u1 * v0;
				if (d == 0.f) continue;

				auto tangent = glm::vec3(v1 * e0.x - v0 * e1.x, v1 * e0.y - v0 * e1.y, v1 * e0.z - v0 * e1.z);
				if (glm::length(tangent) > 0.f)
				{
					tangent = glm::normalize(tangent);
					m->tangents[id[0]] = tangent;
					m->tangents[id[1]] = tangent;
					m->tangents[id[2]] = tangent;
				}
				else
				{
					m->tangents[id[0]] = glm::vec3();
					m->tangents[id[1]] = glm::vec3();
					m->tangents[id[2]] = glm::vec3();
				}
			}
		}

		for (int i = 0; i < m->bones.size(); i++)
		{
			m->bones[i].relateCoord = m->bones[i].rootCoord;
			int parentID = m->bones[i].parents;
			if (parentID != -1)
			{
				m->bones[i].relateCoord -= m->bones[parentID].rootCoord;
				m->bones[parentID].children.push_back(i);
			}
		}

		Animation *a;
		a = getAnimation(m->stand_animation_filename);
		if (a) m->stateAnimations[ModelStateAnimationStand] = m->bindAnimation(a);
		a = getAnimation(m->forward_animation_filename);
		if (a) m->stateAnimations[ModelStateAnimationForward] = m->bindAnimation(a);
		a = getAnimation(m->backward_animation_filename);
		if (a) m->stateAnimations[ModelStateAnimationBackward] = m->bindAnimation(a);
		a = getAnimation(m->leftward_animation_filename);
		if (a) m->stateAnimations[ModelStateAnimationLeftward] = m->bindAnimation(a);
		a = getAnimation(m->rightward_animation_filename);
		if (a) m->stateAnimations[ModelStateAnimationRightward] = m->bindAnimation(a);
		a = getAnimation(m->jump_animation_filename);
		if (a) m->stateAnimations[ModelStateAnimationJump] = m->bindAnimation(a);
	}

	void initGeneralModels()
	{
		{
			triangleModel = new Model;
			triangleModel->name = "triangle";
			triangleModel->filename = "[triangle]";

			addTriangleVertex(triangleModel, glm::mat3(1.f), glm::vec3(0.f));

			auto g = std::make_unique<Geometry>();
			g->material = defaultMaterial;
			g->indiceCount = triangleModel->indices.size();
			triangleModel->geometries.push_back(std::move(g));

			_model_after_process(triangleModel);

			_add_model(triangleModel);
		}

		{
			cubeModel = new Model;
			cubeModel->name = "cube";
			cubeModel->filename = "[cube]";

			addCubeVertex(cubeModel, glm::mat3(1.f), glm::vec3(0.f), 1.f);

			auto g = std::make_unique<Geometry>();
			g->material = defaultMaterial;
			g->indiceCount = cubeModel->indices.size();
			cubeModel->geometries.push_back(std::move(g));

			auto pRigidbody = new Rigidbody(RigidbodyType::dynamic);
			cubeModel->addRigidbody(pRigidbody);
			auto s = new Shape(ShapeType::box);
			s->setScale(glm::vec3(0.5f));
			pRigidbody->addShape(s);

			_model_after_process(cubeModel);

			_add_model(cubeModel);
		}

		{
			sphereModel = new Model;
			sphereModel->name = "sphere";
			sphereModel->filename = "[sphere]";

			addSphereVertex(sphereModel, glm::mat3(1.f), glm::vec3(0.f), 0.5f, 32, 32);

			auto g0 = std::make_unique<Geometry>();
			g0->material = defaultMaterial;
			g0->indiceCount = sphereModel->indices.size() / 2;
			auto g1 = std::make_unique<Geometry>();
			g1->material = defaultMaterial;
			g1->indiceBase = g0->indiceCount;
			g1->indiceCount = g0->indiceCount;
			sphereModel->geometries.push_back(std::move(g0));
			sphereModel->geometries.push_back(std::move(g1));

			auto pRigidbody = new Rigidbody(RigidbodyType::dynamic);
			sphereModel->addRigidbody(pRigidbody);
			auto s = new Shape(ShapeType::sphere);
			s->setScale(glm::vec3(0.5f));
			pRigidbody->addShape(s);

			_model_after_process(sphereModel);

			_add_model(sphereModel);
		}

		{
			cylinderModel = new Model;
			cylinderModel->name = "cylinder";
			cylinderModel->filename = "[cylinder]";

			addCylinderVertex(cylinderModel, glm::mat3(1.f), glm::vec3(0.f), 0.5f, 0.5f, 32);

			auto g = std::make_unique<Geometry>();
			g->material = defaultMaterial;
			g->indiceCount = cylinderModel->indices.size();
			cylinderModel->geometries.push_back(std::move(g));

			auto pRigidbody = new Rigidbody(RigidbodyType::dynamic);
			cylinderModel->addRigidbody(pRigidbody);
			auto s = new Shape(ShapeType::capsule);
			s->setScale(glm::vec3(0.5f));
			pRigidbody->addShape(s);

			_model_after_process(cylinderModel);

			_add_model(cylinderModel);
		}

		{
			coneModel = new Model;
			coneModel->name = "cone";
			coneModel->filename = "[cone]";

			addConeVertex(coneModel, glm::mat3(1.f), glm::vec3(0.f), 0.5f, 0.5f, 32);

			auto g = std::make_unique<Geometry>();
			g->material = defaultMaterial;
			g->indiceCount = coneModel->indices.size();
			coneModel->geometries.push_back(std::move(g));

			_model_after_process(coneModel);

			_add_model(coneModel);
		}

		{
			arrowModel = new Model;
			arrowModel->name = "arrow";
			arrowModel->filename = "[arrow]";

			glm::mat3 matR = glm::mat3(glm::rotate(glm::radians(-90.f), glm::vec3(0.f, 0.f, 1.f)));

			addCylinderVertex(arrowModel, matR, glm::vec3(0.4f, 0.f, 0.f), 0.4f, 0.01f, 32);
			addConeVertex(arrowModel, matR, glm::vec3(0.8f, 0.f, 0.f), 0.2f, 0.05f, 32);

			auto g = std::make_unique<Geometry>();
			g->material = defaultMaterial;
			g->indiceCount = arrowModel->indices.size();
			arrowModel->geometries.push_back(std::move(g));

			_model_after_process(arrowModel);

			_add_model(arrowModel);
		}

		{
			torusModel = new Model;
			torusModel->name = "torus";
			torusModel->filename = "[torus]";

			glm::mat3 matR = glm::mat3(glm::rotate(glm::radians(-90.f), glm::vec3(0.f, 0.f, 1.f)));

			addTorusVertex(torusModel, matR, glm::vec3(), 1.f, 0.01f, 32, 32);

			auto g = std::make_unique<Geometry>();
			g->material = defaultMaterial;
			g->indiceCount = torusModel->indices.size();
			torusModel->geometries.push_back(std::move(g));

			_model_after_process(torusModel);

			_add_model(torusModel);
		}

		{
			hamerModel = new Model;
			hamerModel->name = "hammer";
			hamerModel->filename = "[hammer]";

			glm::mat3 matR = glm::mat3(glm::rotate(glm::radians(-90.f), glm::vec3(0.f, 0.f, 1.f)));

			addCylinderVertex(hamerModel, matR, glm::vec3(0.45f, 0.f, 0.f), 0.45f, 0.01f, 32);
			int ic0 = hamerModel->indices.size();
			addCubeVertex(hamerModel, matR, glm::vec3(0.9f, 0.f, 0.f), 0.1f);
			int ic1 = hamerModel->indices.size();

			auto g0 = std::make_unique<Geometry>();
			g0->material = defaultMaterial;
			g0->indiceCount = ic0;
			auto g1 = std::make_unique<Geometry>();
			g1->material = defaultMaterial;
			g1->indiceBase = ic0;
			g1->indiceCount = ic1 - ic0;
			hamerModel->geometries.push_back(std::move(g0));
			hamerModel->geometries.push_back(std::move(g1));

			_model_after_process(hamerModel);

			_add_model(hamerModel);
		}
	}



	namespace OBJ
	{
		void load(Model *m, const std::string &filename)
		{
			std::ifstream file(filename);

			int currentIndex = 0;

			std::vector<std::pair<std::string, Material*>> materials;

			Geometry *currentGeometry = nullptr;

			std::vector<glm::vec3> rawPositions;
			std::vector<glm::vec2> rawTexcoords;
			std::vector<glm::vec3> rawNormals;
			std::vector<glm::ivec3> rawIndexs;

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
							index = m->positions.size();
							m->positions.push_back(rawPositions[ids[0]]);
							m->uvs.push_back(ids[1] != -1 ? rawTexcoords[ids[1]] : glm::vec2(0.f));
							m->normals.push_back(ids[2] != -1 ? rawNormals[ids[2]] : glm::vec3(0.f));
							rawIndexs.push_back(ids);

						}
						m->indices.push_back(index);
						currentIndex++;
						currentGeometry->indiceCount++;
					}
				}
				else if (token == "usemtl")
				{
					std::string name;
					ss >> name;
					for (auto &_m : materials)
					{
						if (name == _m.first)
						{
							auto g = std::make_unique<Geometry>();
							currentGeometry = g.get();
							currentGeometry->material = _m.second;
							currentGeometry->indiceBase = currentIndex;
							m->geometries.push_back(std::move(g));
							break;
						}
					}
				}
				else if (token == "mtllib")
				{
					std::string libName;
					ss >> libName;

					if (libName != "")
					{
						std::ifstream file(m->filepath + "/" + libName);

						std::string mtlName;
						unsigned char spec, roughness;
						Image *albedoAlphaMap = nullptr;
						Image *normalHeightMap = nullptr;

						while (!file.eof())
						{
							std::string line;
							std::getline(file, line);

							std::stringstream ss(line);
							std::string token;
							ss >> token;

							if (token == "newmtl")
							{
								ss >> mtlName;
							}
							else if (token == "tk_spec")
							{
								ss >> spec;
							}
							else if (token == "tk_roughness")
							{
								ss >> roughness;
							}
							else if (token == "map_Kd")
							{
								std::string filename;
								ss >> filename;
								albedoAlphaMap = addModelTexture(m->filepath + "/" + filename, true);
							}
							else if (token == "map_bump")
							{
								std::string filename;
								ss >> filename;
								normalHeightMap = addModelTexture(m->filepath + "/" + filename);
							}
						}

						auto _m = addModelMaterial(255, 255, 255, 255, spec, roughness, albedoAlphaMap, normalHeightMap, nullptr);
						materials.emplace_back(mtlName, _m);
					}
				}
			}

			m->loadData(true);

			_model_after_process(m);
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

			m->animated = true;

			Header header;
			file.read((char*)&header, sizeof(Header));
			m->name = japaneseToChinese(header.name);
			m->comment = japaneseToChinese(header.comment);

			int vertexCount;
			file & vertexCount;
			assert(vertexCount > 0);
			m->positions.resize(vertexCount);
			m->normals.resize(vertexCount);
			m->uvs.resize(vertexCount);
			m->boneWeights.resize(vertexCount);
			m->boneIDs.resize(vertexCount);
			for (int i = 0; i < vertexCount; i++)
			{
				VertexData data;
				file.read((char*)&data, sizeof(VertexData));
				m->positions[i] = data.position;
				m->positions[i].z *= -1.f;
				m->normals[i] = data.normal;
				m->normals[i].z *= -1.f;
				m->uvs[i] = data.uv;
				m->uvs[i].y = 1.f - m->uvs[i].y;
				float fWeight = data.weight / 100.f;
				m->boneWeights[i].x = fWeight;
				m->boneWeights[i].y = 1.f - fWeight;
				m->boneIDs[i].x = data.boneID0 + 0.5f;
				m->boneIDs[i].y = data.boneID1 + 0.5f;
			}

			int indiceCount;
			file & indiceCount;
			m->indices.resize(indiceCount);
			for (int i = 0; i < indiceCount; i += 3)
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

				auto g = std::make_unique<Geometry>();
				g->material = addModelMaterial(data.diffuse.r * 255, data.diffuse.g * 255, data.diffuse.b * 255, data.diffuse.a * 255,
					0, 255, addModelTexture(m->filepath + "/" + data.mapName, true), nullptr, nullptr);
				g->indiceBase = currentIndiceVertex;
				g->indiceCount = data.indiceCount;

				currentIndiceVertex += data.indiceCount;

				m->geometries.push_back(std::move(g));
			}

			unsigned short boneCount;
			file & boneCount;
			m->bones.resize(boneCount);
			for (int i = 0; i < boneCount; i++)
			{
				BoneData data;
				file.read((char*)&data, sizeof(BoneData));

				m->bones[i].name = japaneseToChinese(data.name);
				m->bones[i].parents = data.parents;
				m->bones[i].type = data.type;
				m->bones[i].rootCoord = data.coord;
				m->bones[i].rootCoord.z *= -1.f;
			}

			unsigned short ikCount;
			file & ikCount;
			m->iks.resize(ikCount);
			for (int i = 0; i < ikCount; i++)
			{
				IkData data;
				file.read((char*)&data, sizeof(IkData));

				m->iks[i].targetID = data.target;
				m->iks[i].effectorID = data.effector;
				m->iks[i].iterations = data.iterations;
				m->iks[i].weight = data.weight;
				m->iks[i].chain.resize(data.chainLength);
				for (int j = 0; j < data.chainLength; j++)
				{
					short boneID;
					file & boneID;
					m->iks[i].chain[j] = boneID;
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

				auto p = new Rigidbody;
				p->name = japaneseToChinese(data.name);
				p->boneID = data.bone;
				p->originCollisionGroupID = data.collisionGroupNumber;
				p->originCollisionFreeFlag = data.collisionGroupMask;
				data.location.z *= -1.f;
				p->setCoord(data.location);
				data.rotation = glm::degrees(data.rotation);
				glm::mat3 rotationMat;
				eulerYxzToMatrix(glm::vec3(-data.rotation.y, -data.rotation.x, data.rotation.z), rotationMat);
				glm::vec4 rotationQuat;
				matrixToQuaternion(rotationMat, rotationQuat);
				p->setQuat(rotationQuat);
				p->type = (RigidbodyType)data.mode;
				//m->addRigidbody(p); // TODO : FIX
				auto q = new Shape;
				switch (data.type)
				{
				case 0: q->type = ShapeType::sphere; break;
				case 1: q->type = ShapeType::box; break;
				case 2: q->type = ShapeType::capsule; break;
				}
				switch (q->type)
				{
				case ShapeType::sphere:
					data.size.y = data.size.z = data.size.x;
					break;
				case ShapeType::capsule:
					data.size.y *= 0.5f;
					data.size.z = data.size.x;
					break;
				}
				q->setScale(data.size);
				auto v = q->getVolume();
				if (v != 0.f) p->density = data.mass / v;
				p->addShape(q);
			}

			unsigned int jointCount;
			file & jointCount;
			for (int i = 0; i < jointCount; i++)
			{
				JointData data;
				file.read((char*)&data, sizeof(JointData));

				auto p = new Joint;
				p->name = japaneseToChinese(data.name);
				p->rigid0ID = data.rigid0;
				p->rigid1ID = data.rigid1;
				p->maxCoord = data.maxCoord;
				p->minCoord = data.minCoord;
				p->maxRotation = data.maxRotation;
				p->minRotation = data.minRotation;
				p->springConstant = data.springConstant;
				p->sprintRotationConstant = data.springRotationConstant;

				data.coord.z *= -1.f;
				p->setCoord(data.coord);
				glm::mat3 rotationMat;
				eulerYxzToMatrix(glm::vec3(-data.rotation.y, -data.rotation.x, data.rotation.z), rotationMat);
				glm::vec4 rotationQuat;
				matrixToQuaternion(rotationMat, rotationQuat);
				p->setQuat(rotationQuat);
				m->addJoint(p);
			}

			m->loadData(false);

			_model_after_process(m);
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
			AttributeTree at("COLLADA", filename);
			AttributeTreeNode *n;
			n = at.firstNode("library_geometries"); assert(n);
			n = n->firstNode("geometry"); assert(n);
			n = n->firstNode("mesh"); assert(n);
			std::vector<std::unique_ptr<Source>> sources;
			VertexInfo vertex_info;
			for (auto &c : n->children)
			{
				if (c->name == "source")
				{
					AttributeTreeNode *n;
					Attribute *a;
					auto s = std::make_unique<Source>();
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
					sources.push_back(std::move(s));
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
							Attribute *a;
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
							assert(element_count_per_vertex > 0);
							assert(element_count_per_vertex <= 3);
							switch (element_count_per_vertex)
							{
							case 1:
							{
								std::regex pattern(R"([0-9]+)");
								auto indice_count = vcount.size() * 3;
								while (std::regex_search(str, match, pattern) && indice_count > 0)
								{
									auto index = std::stoi(match[0].str());
									m->positions.push_back(sources[position_source_index]->v3(index));
									m->uvs.push_back(glm::vec2(0.f));
									m->normals.push_back(glm::vec3(0.f));
									m->indices.push_back(index);
									indice_count--;
									str = match.suffix();
								}
							}
							break;
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
										index = m->positions.size();
										m->positions.push_back(sources[position_source_index]->v3(id[position_offset]));
										m->uvs.push_back(uv_source_index == -1 ? glm::vec2(0.f) : sources[uv_source_index]->v2(id[uv_offset]));
										m->normals.push_back(normal_source_index == -1 ? glm::vec3(0.f) : sources[normal_source_index]->v3(id[normal_offset]));
										ids.push_back(id);

									}
									m->indices.push_back(index);
									indice_count--;
									str = match.suffix();
								}
							}
							break;
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
										index = m->positions.size();
										m->positions.push_back(sources[position_source_index]->v3(id[position_offset]));
										m->uvs.push_back(uv_source_index == -1 ? glm::vec2(0.f) : sources[uv_source_index]->v2(id[uv_offset]));
										m->normals.push_back(normal_source_index == -1 ? glm::vec3(0.f) : sources[normal_source_index]->v3(id[normal_offset]));
										ids.push_back(id);

									}
									m->indices.push_back(index);
									indice_count--;
									str = match.suffix();
								}
							}
							break;
							}
						}
					}
				}
			}

			auto g = std::make_unique<Geometry>();
			g->material = defaultMaterial;
			g->indiceCount = m->indices.size();
			m->geometries.push_back(std::move(g));

			_model_after_process(m);
		}
	}

	namespace TKM
	{
		void load(Model *m, const std::string &filename)
		{
			std::ifstream file(filename, std::ios::binary);

			file & m->animated;

			int vertexCount;
			int indiceCount;

			file & vertexCount;
			file & indiceCount;
			if (vertexCount > 0)
			{
				m->positions.resize(vertexCount);
				m->uvs.resize(vertexCount);
				m->normals.resize(vertexCount);
				m->tangents.resize(vertexCount);
				file.read((char*)m->positions.data(), vertexCount * sizeof(glm::vec3));
				file.read((char*)m->uvs.data(), vertexCount * sizeof(glm::vec2));
				file.read((char*)m->normals.data(), vertexCount * sizeof(glm::vec3));
				file.read((char*)m->tangents.data(), vertexCount * sizeof(glm::vec3));
				if (m->animated)
				{
					m->boneWeights.resize(vertexCount);
					m->boneIDs.resize(vertexCount);
					file.read((char*)m->boneWeights.data(), vertexCount * sizeof(glm::vec4));
					file.read((char*)m->boneIDs.data(), vertexCount * sizeof(glm::ivec4));
				}
			}
			if (indiceCount > 0)
			{
				m->indices.reserve(indiceCount);
				file.read((char*)m->indices.data(), indiceCount * sizeof(int));
			}

			int geometryCount;
			file & geometryCount;
			for (int i = 0; i < geometryCount; i++)
			{
				unsigned char albedoR, albedoG, albedoB, alpha, spec, roughness;
				file & albedoR;
				file & albedoG;
				file & albedoB;
				file & alpha;
				file & spec;
				file & roughness;
				std::string albedoAlphaMapName;
				std::string normalHeightMapName;
				std::string specRoughnessMapName;
				file > albedoAlphaMapName;
				file > normalHeightMapName;
				file > specRoughnessMapName;

				auto g = std::make_unique<Geometry>();
				g->material = addModelMaterial(albedoR, albedoG, albedoB, alpha, spec, roughness,
					addModelTexture(m->filepath + "/" + albedoAlphaMapName, true),
					addModelTexture(m->filepath + "/" + normalHeightMapName, true),
					addModelTexture(m->filepath + "/" + specRoughnessMapName, true));
				file & g->indiceBase;
				file & g->indiceCount;
				file & g->visible;

				m->geometries.push_back(std::move(g));
			}

			int boneCount;
			file & boneCount;
			for (int i = 0; i < boneCount; i++)
			{
				Bone bone;

				char name[20];
				file.read(name, 20);
				bone.name = name;

				file & bone.type;
				file & bone.parents;
				file & bone.rootCoord;

				m->bones.push_back(bone);
			}

			int ikCount;
			file & ikCount;
			m->iks.resize(boneCount);
			for (int i = 0; i < ikCount; i++)
			{
				file & m->iks[i].targetID;
				file & m->iks[i].effectorID;
				file & m->iks[i].iterations;
				file & m->iks[i].weight;

				int chainLength;
				file & chainLength;
				m->iks[i].chain.resize(chainLength);
				file.read((char*)m->iks[i].chain.data(), sizeof(int) * chainLength);
			}

			if (m->animated)
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
				auto p = new Rigidbody;
				int type;
				file & type;
				p->type = (RigidbodyType)type;
				file > p->name;
				file & p->originCollisionGroupID;
				file & p->originCollisionFreeFlag;
				file & p->boneID;
				glm::vec3 coord;
				file & coord;
				p->setCoord(coord);
				glm::vec3 euler;
				file & euler;
				p->setEuler(euler);
				file & p->density;
				file & p->velocityAttenuation;
				file & p->rotationAttenuation;
				file & p->bounce;
				file & p->friction;
				m->addRigidbody(p);

				int shapeCount;
				file & shapeCount;
				for (int j = 0; j < shapeCount; j++)
				{
					auto q = new Shape;
					p->addShape(q);
					glm::vec3 coord;
					file & coord;
					q->setCoord(coord);
					glm::vec3 euler;
					file & euler;
					q->setEuler(euler);
					glm::vec3 scale;
					file & scale;
					q->setScale(scale);
					int type;
					file & type;
					q->type = (ShapeType)type;
				}
			}

			int jointCount = 0;
			file & jointCount;
			for (int i = 0; i < jointCount; i++)
			{
				auto j = new Joint;
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
				m->addJoint(j);
			}

			file & m->bounding_position;
			file & m->bounding_size;

			file & m->controller_height;
			file & m->controller_radius;

			file & m->eye_position;

			_model_after_process(m);
		}

		void save(Model *m, const std::string &filename, bool copyTexture)
		{
			std::experimental::filesystem::path path(filename);

			std::string dstFilepath = path.parent_path().string();
			if (dstFilepath == "")
				dstFilepath = ".";

			std::ofstream file(filename);

			file & m->animated;;

			int vertexCount = m->positions.size();
			int indiceCount = m->indices.size();

			file & vertexCount;
			file & indiceCount;
			if (vertexCount > 0)
			{
				file.write((char*)m->positions.data(), vertexCount * sizeof glm::vec3);
				file.write((char*)m->uvs.data(), vertexCount * sizeof glm::vec2);
				file.write((char*)m->normals.data(), vertexCount * sizeof glm::vec3);
				file.write((char*)m->tangents.data(), vertexCount * sizeof glm::vec3);
				if (m->animated)
				{
					file.write((char*)m->boneWeights.data(), vertexCount * sizeof glm::vec4);
					file.write((char*)m->boneIDs.data(), vertexCount * sizeof glm::ivec4);
				}
			}
			if (indiceCount > 0)
			{
				file.write((char*)m->indices.data(), vertexCount * sizeof(int));
			}

			int geometryCount = m->geometries.size();
			file & geometryCount;
			for (auto &g : m->geometries)
			{
				file & g->material->albedoR;
				file & g->material->albedoG;
				file & g->material->albedoB;
				file & g->material->alpha;
				file & g->material->spec;
				file & g->material->roughness;
				file & g->material->albedoAlphaMap ? g->material->albedoAlphaMap->filename : 0;
				file & g->material->normalHeightMap ? g->material->normalHeightMap->filename : 0;
				file & g->material->specRoughnessMap ? g->material->specRoughnessMap->filename : 0;

				file & g->indiceBase;
				file & g->indiceCount;
				file & g->visible;
				if (copyTexture)
				{
					if (g->material->albedoAlphaMap)
					{
						std::string dst = dstFilepath + "/" + g->material->albedoAlphaMap->filename;
						CopyFile(g->material->albedoAlphaMap->filename.c_str(), dst.c_str(), false);
					}
					if (g->material->normalHeightMap)
					{
						std::string dst = dstFilepath + "/" + g->material->normalHeightMap->filename;
						CopyFile(g->material->normalHeightMap->filename.c_str(), dst.c_str(), false);
					}
					if (g->material->specRoughnessMap)
					{
						std::string dst = dstFilepath + "/" + g->material->specRoughnessMap->filename;
						CopyFile(g->material->specRoughnessMap->filename.c_str(), dst.c_str(), false);
					}
				}
			}

			int boneCount = m->bones.size();
			file & boneCount;
			for (auto &bone : m->bones)
			{
				file & bone.name;
				file & bone.type;
				file & bone.parents;
				file & bone.rootCoord;
			}

			int ikCount = m->iks.size();
			file & ikCount;
			for (auto &ik : m->iks)
			{
				file & ik.targetID;
				file & ik.effectorID;
				file & ik.iterations;
				file & ik.weight;

				int chainSize = ik.chain.size();
				file & chainSize;
				file.write((char*)ik.chain.data(), sizeof(int) * ik.chain.size());
			}

			if (m->animated)
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
			for (auto rb : m->rigidbodies)
			{
				int mode = (int)rb->type;
				file & mode;
				file < rb->name;
				file & rb->originCollisionGroupID;
				file & rb->originCollisionFreeFlag;
				file & rb->boneID;
				file & rb->getCoord();
				file & rb->getEuler();
				file & rb->density;
				file & rb->velocityAttenuation;
				file & rb->rotationAttenuation;
				file & rb->bounce;
				file & rb->friction;

				int shapeCount = rb->shapes.size();
				file & shapeCount;
				for (auto &s : rb->shapes)
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
			for (auto j : m->joints)
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

	Model *createModel(const std::string &_filename)
	{
		auto filename = std::experimental::filesystem::path(_filename).string();
		if (!std::experimental::filesystem::exists(filename))
		{
			std::cout << "Model File Lost:" << filename;
			return nullptr;
		}

		std::experimental::filesystem::path path(filename);
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
		{
			std::cout << "Model Format Not Support:" << ext;
			return nullptr;
		}

		auto m = new Model;
		m->filename = filename;
		m->filepath = path.parent_path().string();
		if (m->filepath == "")
			m->filepath = ".";
		load_func(m, filename);

		_add_model(m);

		return m;
	}

}
