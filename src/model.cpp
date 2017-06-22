#include <algorithm>
#include <experimental/filesystem>
#include <sstream>

#include "core.h"
#include "scene.h"

namespace tke
{
	std::vector<Animation*> animations;

	AnimationBinding::~AnimationBinding()
	{
		for (auto t : pTracks)
			delete t;
	}

	char *shapeNames[] = {
		"Box",
		"Sphere",
		"Capsule",
		"Plane",
		"Convex Mesh",
		"Triangle Mesh",
		"Height Field"
	};

	Shape::Shape()
	{
	}

	Shape::Shape(ShapeType _type)
		:type(_type)
	{
	}

	char *Shape::getTypeName(Type _type)
	{
		return shapeNames[(int)_type];
	}

	char *Shape::getTypeName()
	{
		return shapeNames[(int)type];
	}

	float Shape::getVolume() const
	{
		auto size = getScale();
		switch (type)
		{
		case ShapeTypeBox:
			return size.x * size.y * size.z * 8.f;
		case ShapeTypeSphere:
			return 4.f * size.x * size.x * size.x * M_PI / 3.f;
		case ShapeTypeCapsule:
			return 4.f * size.x * size.x * size.x * M_PI / 3.f + M_PI * size.x * size.x * size.y;
		}
		return 0.f;
	}

	Rigidbody::Rigidbody()
	{
	}

	Rigidbody::Rigidbody(RigidbodyType _type)
		:type(_type)
	{
	}

	Rigidbody::~Rigidbody()
	{
		for (auto s : shapes)
			delete s;
	}

	void Rigidbody::addShape(Shape *pShape)
	{
		static auto magicNumber = 0;
		pShape->id = magicNumber++;
		shapes.push_back(pShape);
	}

	Shape *Rigidbody::deleteShape(Shape *pShape)
	{
		for (auto it = shapes.begin(); it != shapes.end(); it++)
		{
			if (*it == pShape)
			{
				if (it > shapes.begin())
					pShape = *(it - 1);
				else
					pShape = nullptr;
				shapes.erase(it);
				break;
			}
		}
		return pShape;
	}

	void Model::loadDat(const std::string &filename)
	{
		std::ifstream file(filename);
		if (!file.good()) return;
		int count;
		file >> count;
		for (auto p : rigidbodies) delete p;
		rigidbodies.clear();
		for (int i = 0; i < count; i++)
		{
			auto rb = new Rigidbody;
			file.read((char*)rb, sizeof(Rigidbody));

			rigidbodies.push_back(rb);
		}
		file >> eyePosition;
	}

	void Model::saveDat(const std::string &filename)
	{
		std::ofstream file(filename);
		if (!file.good()) return;
		file << rigidbodies.size();
		for (int i = 0; i < rigidbodies.size(); i++)
			file.write((char*)&rigidbodies[i], sizeof(Rigidbody));

		file << eyePosition;
	}

	AnimationBinding *Model::bindAnimation(Animation *pAnimationTemplate)
	{
		auto pAnimation = new AnimationBinding;
		pAnimation->pTemplate = pAnimationTemplate;
		for (auto &motion : pAnimationTemplate->motions)
		{
			pAnimation->frameTotal = glm::max(pAnimation->frameTotal, motion.frame);

			int boneID = -1;
			for (int iBone = 0; iBone < bones.size(); iBone++)
			{
				if (motion.name.compare(bones[iBone].name) == 0)
				{
					boneID = iBone;
					break;
				}
			}
			if (boneID == -1) continue;

			BoneMotionTrack *pTrack = nullptr;
			for (auto t : pAnimation->pTracks)
			{
				if (t->boneID == boneID)
				{
					pTrack = t;
					break;
				}
			}
			if (!pTrack)
			{
				pTrack = new BoneMotionTrack;
				pTrack->boneID = boneID;
				pTrack->pMotions.push_back(&motion);
				pAnimation->pTracks.push_back(pTrack);
			}
			else
			{
				bool inserted = false;
				for (int i = 0; i < pTrack->pMotions.size(); i++)
				{
					if (pTrack->pMotions[i]->frame > motion.frame)
					{
						pTrack->pMotions.insert(pTrack->pMotions.begin() + i, &motion);
						inserted = true;
						break;
					}
				}
				if (!inserted) pTrack->pMotions.push_back(&motion);
			}
		}
		animations.push_back(pAnimation);
		return pAnimation;
	}

	Image *Model::getImage(const char *name)
	{
		for (auto pImage : pImages)
		{
			if (pImage->filename.compare(name) == 0)
				return pImage;
		}
		return nullptr;
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

	std::vector<Model*> models;
	static void _add_model(Model *m)
	{
		for (auto src : m->pImages)
		{
			bool found = false;
			for (auto i : textures)
			{
				if (i == src)
				{
					found = true;
					break;
				}
			}
			if (!found)
			{
				src->index = textures.size();
				textures.push_back(src);
			}
		}
		for (auto mt : m->materials)
		{
			MaterialShaderStruct stru;
			stru.albedoAlphaCompress = mt->albedoR + (mt->albedoG << 8) + (mt->albedoB << 16) + (mt->alpha << 24);
			stru.specRoughnessCompress = mt->spec + (mt->roughness << 8);

			auto albedoAlphaMapIndex = mt->albedoAlphaMap ? mt->albedoAlphaMap->index + 1 : 0;
			auto normalHeightMapIndex = mt->normalHeightMap ? mt->normalHeightMap->index + 1 : 0;
			auto specRoughnessMapIndex = mt->specRoughnessMap ? mt->specRoughnessMap->index + 1 : 0;

			int sameIndex = -1;
			int materialIndex = 0;
			for (auto &mt : materials)
			{
				auto storeAlbedoAlphaMapIndex = mt.mapIndex & 0xff;
				auto storeNormalHeightMapIndex = (mt.mapIndex >> 8) & 0xff;
				auto storeSpecRoughnessMapIndex = (mt.mapIndex >> 16) & 0xff;

				bool theSameAlbedoAlpha = false;
				bool theSameNormalHeight = false;
				bool theSameSpecRoughness = false;

				if (albedoAlphaMapIndex != 0 && albedoAlphaMapIndex == storeAlbedoAlphaMapIndex)
					theSameAlbedoAlpha = true;
				else if (albedoAlphaMapIndex == 0 && storeAlbedoAlphaMapIndex == 0 &&
					stru.albedoAlphaCompress == mt.albedoAlphaCompress)
					theSameAlbedoAlpha = true;
				if (normalHeightMapIndex == storeNormalHeightMapIndex)
					theSameNormalHeight = true;
				if (specRoughnessMapIndex != 0 && specRoughnessMapIndex == storeSpecRoughnessMapIndex)
					theSameSpecRoughness = true;
				else if (specRoughnessMapIndex == 0 && storeSpecRoughnessMapIndex == 0 &&
					stru.specRoughnessCompress == mt.specRoughnessCompress)
					theSameSpecRoughness = true;

				if (theSameAlbedoAlpha && theSameNormalHeight && theSameSpecRoughness)
				{
					sameIndex = materialIndex;
					break;
				}

				materialIndex++;
			}
			if (sameIndex == -1)
			{
				mt->sceneIndex = materials.size();
				stru.mapIndex = albedoAlphaMapIndex + (normalHeightMapIndex << 8) + (specRoughnessMapIndex << 16);
				materials.push_back(stru);
			}
			else
			{
				mt->sceneIndex = sameIndex;
			}
		}
		models.push_back(m);
		needUpdateVertexBuffer = true;
		needUpdateMaterialBuffer = true;
		needUpdateTexture = true;
	}

	void clearModel()
	{
		for (auto m : models)
			delete m;
		models.clear();
	}

	AnimationComponent::AnimationComponent(Model *_model)
	{
		model = _model;
		boneData = new BoneData[model->bones.size()];
		boneMatrix = new glm::mat4[model->bones.size()];
		boneMatrixBuffer = new UniformBuffer(sizeof(glm::mat4) * model->bones.size());
		boneMatrixBuffer->update(boneMatrix, *stagingBuffer, sizeof(glm::mat4) * model->bones.size());
	}

	AnimationComponent::~AnimationComponent()
	{
		delete[]boneData;
		delete[]boneMatrix;
		delete boneMatrixBuffer;
	}

	void AnimationComponent::refreshBone()
	{
		assert(model);

		for (int i = 0; i < model->bones.size(); i++)
		{
			boneMatrix[i] = glm::translate(model->bones[i].relateCoord + boneData[i].coord) * glm::mat4(boneData[i].rotation);
			if (model->bones[i].parent != -1) boneMatrix[i] = boneMatrix[model->bones[i].parent] * boneMatrix[i];
		}
	}

	void AnimationComponent::refreshBone(int i)
	{
		assert(model && i < model->bones.size());

		boneMatrix[i] = glm::translate(model->bones[i].relateCoord + boneData[i].coord) * glm::mat4(boneData[i].rotation);
		if (model->bones[i].parent != -1) boneMatrix[i] = boneMatrix[model->bones[i].parent] * boneMatrix[i];

		for (auto child : model->bones[i].children)
			refreshBone(child);
	}

	void AnimationComponent::setAnimation(AnimationBinding *animation)
	{
		if (!animation && currentAnimation)
		{
			for (int i = 0; i < model->bones.size(); i++)
			{
				boneData[i].coord = glm::vec3();
				boneData[i].rotation = glm::mat3();
				boneMatrix[i] = glm::mat4();
			}
			boneMatrixBuffer->update(boneMatrix, *stagingBuffer, sizeof(glm::mat4) * model->bones.size());
		}
		currentAnimation = animation;
		currentFrame = 0;
		currentTime = nowTime;
	}

	void AnimationComponent::update()
	{
		const float dist = 1000.f / 60.f;

		if (currentAnimation)
		{
			for (int i = 0; i < model->bones.size(); i++)
			{
				boneMatrix[i] = glm::mat4();
				boneData[i].rotation = glm::mat3();
				boneData[i].coord = glm::vec3();
			}

			for (auto pTrack : currentAnimation->pTracks)
			{
				auto pBoneData = &boneData[pTrack->boneID];
				auto it = std::upper_bound(pTrack->pMotions.rbegin(), pTrack->pMotions.rend(), currentFrame, [](int frame, BoneMotion *bm) {
					return frame > bm->frame;
				});

				if (it == pTrack->pMotions.rend()) continue;

				auto pLeftMotion = *it;
				auto pRightMotion = (it == pTrack->pMotions.rbegin() ? pTrack->pMotions[0] : *(it - 1));

				auto beta = 0.f;
				if (pLeftMotion != pRightMotion) beta = (currentFrame - pLeftMotion->frame) / (pRightMotion->frame - pLeftMotion->frame);

				tke::quaternionToMatrix(glm::normalize((1.f - beta) * pLeftMotion->quaternion + beta * pRightMotion->quaternion), pBoneData->rotation);
				pBoneData->coord = pLeftMotion->coord + (pRightMotion->coord - pLeftMotion->coord) * beta;
			}

			currentFrame += (nowTime - currentTime) / dist;
			if (currentFrame >= currentAnimation->frameTotal)
				currentFrame = currentAnimation->frameTotal - 1.f;
			currentTime = nowTime;
		}

		refreshBone();

		if (processedIK)
		{
			//	for (int i = 0; i < pModel->iks.size(); i++)
			//	{
			//		auto &ik = pModel->iks[i];
			//		auto t = glm::vec3(boneMatrix[ik.targetID][3]);
			//		//t.z *= -1.f;
			//		for (int times = 0; times < ik.iterations; times++)
			//		{
			//			for (int index = 0; index < ik.chain.size(); index++)
			//			{
			//				//index = iChain;
			//				auto e = glm::vec3(boneMatrix[ik.effectorID][3]);
			//				//e.z *= -1.f;
			//				if (glm::length(t - e) < 0.0001f)
			//				{
			//					goto nextIk;
			//				}

			//				auto boneID = ik.chain[index];

			//				auto p = glm::vec3(boneMatrix[boneID][3]);
			//				//p.z *= -1.f;

			//				auto pe = glm::normalize(e - p);
			//				auto pt = glm::normalize(t - p);
			//				auto theDot = glm::dot(pe, pt);
			//				theDot = glm::clamp(theDot, 0.f, 1.f);
			//				auto theAcos = glm::acos(theDot);
			//				auto ang = glm::degrees(theAcos);
			//				if (glm::abs(ang) > 0.5f)
			//				{
			//					auto n = glm::normalize(glm::cross(pe, pt));
			//					if (glm::abs(n.z) < 1.f)
			//					{
			//						n.z = 0;
			//						n = glm::normalize(n);
			//					}
			//					boneData[boneID].rotation = glm::mat3(glm::rotate(ang, n)) * boneData[boneID].rotation;
			//					//refreshBone(ik.effectorID, boneData, outMat);
			//					pModel->refreshBone(boneID, boneData, boneMatrix);
			//					p = glm::vec3(boneMatrix[boneID][3]);
			//					e = glm::vec3(boneMatrix[ik.effectorID][3]);
			//					pe = glm::normalize(e - p);
			//					auto dot = glm::dot(pe, pt);
			//					int cut = 1;
			//				}
			//				//break;
			//			}
			//		}
			//	nextIk:
			//		//break;
			//		continue;
			//	}
			//}
		}

		for (int i = 0; i < model->bones.size(); i++)
			boneMatrix[i] *= glm::translate(-model->bones[i].rootCoord);

		boneMatrixBuffer->update(boneMatrix, *stagingBuffer, sizeof(glm::mat4) * model->bones.size());
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
			glm::mat3 R = glm::mat3(glm::rotate(-ang, glm::vec3(0.f, 1.f, 0.f)));
			for (int j = 0; j < heightSubdiv; j++)
			{
				auto secang = glm::radians(j * 360.f / heightSubdiv);
				auto index = m->positions.size();
				indexs[i].push_back(index);
				m->positions.push_back(center + R * (glm::vec3(cos(secang) * sectionRadius + radius, sin(secang) * sectionRadius, 0.f)));
				m->normals.push_back(R * glm::vec3(cos(secang), sin(secang), 0.f));
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
			int parentID = m->bones[i].parent;
			if (parentID != -1)
			{
				m->bones[i].relateCoord -= m->bones[parentID].rootCoord;
				m->bones[parentID].children.push_back(i);
			}
		}
	}

	void initGeneralModels()
	{
		{
			triangleModel = new Model;
			triangleModel->name = "triangle";

			addTriangleVertex(triangleModel, glm::mat3(), glm::vec3());

			auto mt = new Material;
			mt->indiceCount = triangleModel->indices.size();
			triangleModel->materials.push_back(mt);

			_model_after_process(triangleModel);

			_add_model(triangleModel);

			globalResource.setModel(triangleModel, "Triangle.Model");
		}

		{
			cubeModel = new Model;
			cubeModel->name = "cube";

			addCubeVertex(cubeModel, glm::mat3(), glm::vec3(), 1.f);

			auto mt = new Material;
			mt->indiceCount = cubeModel->indices.size();
			cubeModel->materials.push_back(mt);

			auto pRigidbody = new Rigidbody(RigidbodyTypeDynamic);
			cubeModel->addRigidbody(pRigidbody);
			auto pShape = new Shape(ShapeTypeBox);
			pRigidbody->addShape(pShape);
			pShape->setScale(glm::vec3(0.5f));

			_model_after_process(cubeModel);

			_add_model(cubeModel);

			globalResource.setModel(cubeModel, "Cube.Model");
		}

		{
			sphereModel = new Model;
			sphereModel->name = "sphere";

			addSphereVertex(sphereModel, glm::mat3(), glm::vec3(), 0.5f, 32, 32);

			auto mt0 = new Material;
			mt0->indiceCount = sphereModel->indices.size() / 2;
			sphereModel->materials.push_back(mt0);
			auto mt1 = new Material;
			mt1->indiceBase = sphereModel->indices.size() / 2;
			mt1->indiceCount = sphereModel->indices.size() / 2;
			sphereModel->materials.push_back(mt1);

			auto pRigidbody = new Rigidbody(RigidbodyTypeDynamic);
			sphereModel->addRigidbody(pRigidbody);
			auto pShape = new Shape(ShapeTypeSphere);
			pRigidbody->addShape(pShape);
			pShape->setScale(glm::vec3(0.5f));

			_model_after_process(sphereModel);

			_add_model(sphereModel);

			globalResource.setModel(sphereModel, "Sphere.Model");
		}

		{
			cylinderModel = new Model;
			cylinderModel->name = "cylinder";

			addCylinderVertex(cylinderModel, glm::mat3(), glm::vec3(), 0.5f, 0.5f, 32);

			auto mt = new Material;
			mt->indiceCount = cylinderModel->indices.size();
			cylinderModel->materials.push_back(mt);

			auto pRigidbody = new Rigidbody(RigidbodyTypeDynamic);
			cylinderModel->addRigidbody(pRigidbody);
			auto pShape = new Shape(ShapeTypeCapsule);
			pRigidbody->addShape(pShape);
			pShape->setScale(glm::vec3(0.5f));

			_model_after_process(cylinderModel);

			_add_model(cylinderModel);

			globalResource.setModel(cylinderModel, "Cylinder.Model");
		}

		{
			coneModel = new Model;
			coneModel->name = "cone";

			addConeVertex(coneModel, glm::mat3(), glm::vec3(), 0.5f, 0.5f, 32);

			auto mt = new Material;
			mt->indiceCount = coneModel->indices.size();
			coneModel->materials.push_back(mt);

			_model_after_process(coneModel);

			_add_model(coneModel);

			globalResource.setModel(coneModel, "Cone.Model");
		}

		{
			arrowModel = new Model;
			arrowModel->name = "arrow";

			glm::mat3 matR = glm::mat3(glm::rotate(-90.f, glm::vec3(0.f, 0.f, 1.f)));

			addCylinderVertex(arrowModel, matR, glm::vec3(0.4f, 0.f, 0.f), 0.4f, 0.01f, 32);
			addConeVertex(arrowModel, matR, glm::vec3(0.8f, 0.f, 0.f), 0.2f, 0.05f, 32);

			auto mt = new Material;
			mt->indiceCount = arrowModel->indices.size();
			arrowModel->materials.push_back(mt);

			_model_after_process(arrowModel);

			_add_model(arrowModel);

			globalResource.setModel(arrowModel, "Arrow.Model");
		}

		{
			torusModel = new Model;
			torusModel->name = "torus";

			glm::mat3 matR = glm::mat3(glm::rotate(-90.f, glm::vec3(0.f, 0.f, 1.f)));

			addTorusVertex(torusModel, matR, glm::vec3(), 1.f, 0.01f, 32, 32);

			auto mt = new Material;
			mt->indiceCount = torusModel->indices.size();
			torusModel->materials.push_back(mt);

			_model_after_process(torusModel);

			_add_model(torusModel);

			globalResource.setModel(torusModel, "Torus.Model");
		}

		{
			hamerModel = new Model;
			hamerModel->name = "hammer";

			glm::mat3 matR = glm::mat3(glm::rotate(-90.f, glm::vec3(0.f, 0.f, 1.f)));

			addCylinderVertex(hamerModel, matR, glm::vec3(0.45f, 0.f, 0.f), 0.45f, 0.01f, 32);
			int ic0 = hamerModel->indices.size();
			addCubeVertex(hamerModel, matR, glm::vec3(0.9f, 0.f, 0.f), 0.1f);
			int ic1 = hamerModel->indices.size();

			auto mt0 = new Material;
			mt0->indiceCount = ic0;
			hamerModel->materials.push_back(mt0);
			auto mt1 = new Material;
			mt1->indiceBase = ic0;
			mt1->indiceCount = ic1 - ic0;
			hamerModel->materials.push_back(mt1);

			_model_after_process(hamerModel);

			_add_model(hamerModel);

			globalResource.setModel(hamerModel, "Hamer.Model");
		}
	}

	namespace OBJ
	{
		struct obj_ctype : std::ctype<char>
		{
			mask my_table[table_size];
			obj_ctype(size_t refs = 0)
				: std::ctype<char>(my_table, false, refs)
			{
				std::copy_n(classic_table(), table_size, my_table);
				my_table['/'] = (mask)space;
			}
		};
		std::stringstream obj_line_ss;
		struct Init
		{
			Init()
			{
				std::locale x(std::locale::classic(), new obj_ctype);
				obj_line_ss.imbue(x);
			}
		};
		static Init _init;

		void load(Model *m, std::ifstream &file)
		{
			reportMinorProgress(0);

			int currentIndex = 0;
			Material *pmt = nullptr;

			std::vector<glm::vec3> rawPositions;
			std::vector<glm::vec2> rawTexcoords;
			std::vector<glm::vec3> rawNormals;
			std::vector<glm::ivec3> rawIndexs;

			reportMinorProgress(10);

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
						obj_line_ss.clear();
						obj_line_ss << token;
						glm::ivec3 ids;
						for (int j = 0; j < 3; j++)
						{
							obj_line_ss >> ids[j];
							ids[j]--;
						}

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

							if (ids[0] < rawPositions.size()) m->positions.push_back(rawPositions[ids[0]]);
							else m->positions.push_back(glm::vec3(0.f));
							if (ids[1] != -1 && ids[1] < rawTexcoords.size()) m->uvs.push_back(rawTexcoords[ids[1]]);
							else m->uvs.push_back(glm::vec2(0.f));
							if (ids[2] != -1 && ids[2] < rawNormals.size()) m->normals.push_back(rawNormals[ids[2]]);
							else m->normals.push_back(glm::vec3(0.f));

							rawIndexs.push_back(ids);

						}
						m->indices.push_back(index);
						currentIndex++;
						pmt->indiceCount++;
					}
				}
				else if (token == "usemtl")
				{
					std::string name;
					ss >> name;
					for (auto mt : m->materials)
					{
						if (name == mt->name)
						{
							pmt = mt;
							pmt->indiceBase = currentIndex;
							break;
						}
					}
				}
				else if (token == "mtllib")
				{
					std::string mtlName;
					ss >> mtlName;

					if (mtlName != "")
					{
						std::ifstream file(m->filepath + "/" + mtlName);

						while (!file.eof())
						{
							std::string line;
							std::getline(file, line);

							std::stringstream ss(line);
							std::string token;
							ss >> token;

							if (token == "newmtl")
							{
								pmt = new Material;

								std::string mtlName;
								ss >> mtlName;
								pmt->name = mtlName;

								m->materials.push_back(pmt);
							}
							else if (token == "tk_spec")
							{
								float spec;
								ss >> spec;
								pmt->spec = 255.f * spec;
							}
							else if (token == "tk_roughness")
							{
								float roughness;
								ss >> roughness;
								pmt->roughness = 255.f * roughness;
							}
							else if (token == "map_Kd")
							{
								std::string filename;
								ss >> filename;
								auto pImage = m->getImage(filename.c_str());
								if (!pImage)
								{
									pImage = createImage(m->filepath + "/" + filename, true, true);
									if (pImage) m->pImages.push_back(pImage);
								}
								pmt->albedoAlphaMap = pImage;
							}
							else if (token == "map_bump")
							{
								std::string filename;
								ss >> filename;
								auto pImage = m->getImage(filename.c_str());
								if (!pImage)
								{
									pImage = createImage(m->filepath + "/" + filename, false, false);
									if (pImage) m->pImages.push_back(pImage);
								}
								pmt->normalHeightMap = pImage;
							}
						}
					}
				}
			}

			reportMinorProgress(50);

			_model_after_process(m);

			reportMinorProgress(100);
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
			short parent;
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

		void load(Model *m, std::ifstream &file)
		{
			static_assert(sizeof(Header) == 283, "");
			static_assert(sizeof(VertexData) == 38, "");
			static_assert(sizeof(MaterialData) == 70, "");
			static_assert(sizeof(BoneData) == 39, "");
			static_assert(sizeof(MorphHeadData) == 25, "");
			static_assert(sizeof(MorphData) == 16, "");
			static_assert(sizeof(RigidData) == 83, "");
			static_assert(sizeof(JointData) == 124, "");

			reportMinorProgress(0);

			m->animated = true;

			Header header;
			file.read((char*)&header, sizeof(Header));
			m->name = japaneseToChinese(header.name);
			m->comment = japaneseToChinese(header.comment);

			int vertexCount;
			file >> vertexCount;
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
			file >> indiceCount;
			m->indices.resize(indiceCount);
			for (int i = 0; i < indiceCount; i += 3)
			{
				unsigned short indice;
				file >> indice;
				m->indices[i + 0] = indice;
				file >> indice;
				m->indices[i + 2] = indice;
				file >> indice;
				m->indices[i + 1] = indice;
			}

			int materialCount;
			file >> materialCount;
			int currentIndiceVertex = 0;
			for (int i = 0; i < materialCount; i++)
			{
				MaterialData data;
				file.read((char*)&data, sizeof(MaterialData));

				auto pmt = new Material;
				pmt->name = std::to_string(i);

				pmt->albedoR = data.diffuse.r * 255;
				pmt->albedoG = data.diffuse.g * 255;
				pmt->albedoB = data.diffuse.b * 255;
				pmt->alpha = data.diffuse.a * 255;
				pmt->indiceBase = currentIndiceVertex;
				pmt->indiceCount = data.indiceCount;

				auto pImage = m->getImage(data.mapName);
				if (!pImage)
				{
					pImage = createImage(m->filepath + "/" + data.mapName, true, true);
					if (pImage) m->pImages.push_back(pImage);
				}
				pmt->albedoAlphaMap = pImage;


				currentIndiceVertex += data.indiceCount;

				m->materials.push_back(pmt);
			}

			unsigned short boneCount;
			file >> boneCount;
			m->bones.resize(boneCount);
			for (int i = 0; i < boneCount; i++)
			{
				BoneData data;
				file.read((char*)&data, sizeof(BoneData));

				m->bones[i].name = japaneseToChinese(data.name);
				m->bones[i].parent = data.parent;
				m->bones[i].type = data.type;
				m->bones[i].rootCoord = data.coord;
				m->bones[i].rootCoord.z *= -1.f;
			}

			unsigned short ikCount;
			file >> ikCount;
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
					file >> boneID;
					m->iks[i].chain[j] = boneID;
				}
			}

			unsigned short morphsCount;
			file >> morphsCount;
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
			file >> dispMorphsListLength;
			for (int i = 0; i < dispMorphsListLength; i++)
			{
				unsigned short id;
				file >> id;
			}
			char dispBoneListLength;
			file >> dispBoneListLength;
			for (int i = 0; i < dispBoneListLength; i++)
			{
				char name[50];
				file.read(name, 50);
			}

			unsigned int dispBoneCount;
			file >> dispBoneCount;
			for (int i = 0; i < dispBoneCount; i++)
			{
				unsigned short boneIndex;
				char index;
				file >> boneIndex;
				file >> index;
			}

			char endFlag;
			file >> endFlag;
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
			file >> rigidCount;
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
				m->addRigidbody(p);
				auto q = new Shape;
				p->addShape(q);
				switch (data.type)
				{
				case 0: q->type = ShapeTypeSphere; break;
				case 1: q->type = ShapeTypeBox; break;
				case 2: q->type = ShapeTypeCapsule; break;
				}
				switch (q->type)
				{
				case ShapeTypeSphere:
					data.size.y = data.size.z = data.size.x;
					break;
				case ShapeTypeCapsule:
					data.size.y *= 0.5f;
					data.size.z = data.size.x;
					break;
				}
				q->setScale(data.size);
				auto v = q->getVolume();
				if (v != 0.f) p->density = data.mass / v;
			}

			unsigned int jointCount;
			file >> jointCount;
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

			_model_after_process(m);

			reportMinorProgress(100);
		}
	}

	namespace VMD
	{
#pragma pack(1)
		struct Header
		{
			char str[30];
			char modelName[20];
		};

		struct BoneMotionData
		{
			char name[15];
			int frame;
			glm::vec3 coord;
			glm::vec4 quaternion;
			char bezier[64];
		};
#pragma pack()

		void load(Animation *a, std::ifstream &file)
		{
			static_assert(sizeof(Header) == 50, "");
			static_assert(sizeof(BoneMotionData) == 111, "");

			Header header;
			file.read((char*)&header, sizeof(Header));
			a->name = japaneseToChinese(header.modelName);
			a->comment = japaneseToChinese(header.str);

			int count;
			file >> count;
			a->motions.resize(count);
			for (int i = 0; i < count; i++)
			{
				BoneMotionData data;
				file.read((char*)&data, sizeof(BoneMotionData));
				a->motions[i].name = japaneseToChinese(data.name);
				a->motions[i].frame = data.frame;
				a->motions[i].coord = glm::vec3(data.coord);
				a->motions[i].quaternion = glm::vec4(data.quaternion);
				memcpy(a->motions[i].bezier, data.bezier, 64);

				a->motions[i].coord.z *= -1.f;
				a->motions[i].quaternion.z *= -1.f;
				a->motions[i].quaternion.w *= -1.f;
			}
		}
	}

	namespace TKM
	{
		static AnimationBinding *_loadAnimation(std::ifstream &file, Model *pModel)
		{
			std::string animName;
			file >> animName;
			for (auto a : animations)
			{
				if (a->name == animName == 0)
					return pModel->bindAnimation(a);
			}
			return nullptr;
		}

		void load(Model *m, std::ifstream &file)
		{
			reportMinorProgress(0);

			int textureCount = 0;
			file >> textureCount;
			for (int i = 0; i < textureCount; i++)
			{
				std::string filename;
				file >> filename;
				bool sRGB;
				file >> sRGB;
				auto pImage = createImage(m->filepath + "/" + filename, sRGB, false);
				if (pImage) m->pImages.push_back(pImage);
			}

			file >> m->animated;

			reportMinorProgress(30);

			int vertexCount;
			int indiceCount;

			file >> vertexCount;
			file >> indiceCount;
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

			reportMinorProgress(50);

			int materialCount;
			file >> materialCount;
			for (int i = 0; i < materialCount; i++)
			{
				auto pmt = new Material;

				file >> pmt->indiceBase;
				file >> pmt->indiceCount;

				file >> pmt->visible;

				file >> pmt->name;
				file >> pmt->albedoR;
				file >> pmt->albedoG;
				file >> pmt->albedoB;
				file >> pmt->alpha;
				file >> pmt->spec;
				file >> pmt->roughness;
				std::string name;
				file >> name;
				pmt->albedoAlphaMap = m->getImage(name.c_str());
				file >> name;
				pmt->normalHeightMap = m->getImage(name.c_str());
				file >> name;
				pmt->specRoughnessMap = m->getImage(name.c_str());

				m->materials.push_back(pmt);
			}

			reportMinorProgress(80);

			int boneCount;
			file >> boneCount;
			for (int i = 0; i < boneCount; i++)
			{
				Bone bone;

				char name[20];
				file.read(name, 20);
				bone.name = name;

				file >> bone.type;
				file >> bone.parent;
				file >> bone.rootCoord;

				m->bones.push_back(bone);
			}

			int ikCount;
			file >> ikCount;
			m->iks.resize(boneCount);
			for (int i = 0; i < ikCount; i++)
			{
				file >> m->iks[i].targetID;
				file >> m->iks[i].effectorID;
				file >> m->iks[i].iterations;
				file >> m->iks[i].weight;

				int chainLength;
				file >> chainLength;
				m->iks[i].chain.resize(chainLength);
				file.read((char*)m->iks[i].chain.data(), sizeof(int) * chainLength);
			}

			if (m->animated)
			{
				m->animationStand = _loadAnimation(file, m);
				m->animationForward = _loadAnimation(file, m);
				m->animationLeft = _loadAnimation(file, m);
				m->animationRight = _loadAnimation(file, m);
				m->animationBackward = _loadAnimation(file, m);
				m->animationJump = _loadAnimation(file, m);
			}

			int rigidbodyCount;
			file >> rigidbodyCount;
			for (int i = 0; i < rigidbodyCount; i++)
			{
				auto p = new Rigidbody;
				int type;
				file >> type;
				p->type = (RigidbodyType)type;
				file >> p->name;
				file >> p->originCollisionGroupID;
				file >> p->originCollisionFreeFlag;
				file >> p->boneID;
				glm::vec3 coord;
				file >> coord;
				p->setCoord(coord);
				glm::vec3 euler;
				file >> euler;
				p->setEuler(euler);
				file >> p->density;
				file >> p->velocityAttenuation;
				file >> p->rotationAttenuation;
				file >> p->bounce;
				file >> p->friction;
				m->addRigidbody(p);

				int shapeCount;
				file >> shapeCount;
				for (int j = 0; j < shapeCount; j++)
				{
					auto q = new Shape;
					p->addShape(q);
					glm::vec3 coord;
					file >> coord;
					q->setCoord(coord);
					glm::vec3 euler;
					file >> euler;
					q->setEuler(euler);
					glm::vec3 scale;
					file >> scale;
					q->setScale(scale);
					int type;
					file >> type;
					q->type = (ShapeType)type;
				}
			}

			int jointCount = 0;
			file >> jointCount;
			for (int i = 0; i < jointCount; i++)
			{
				auto j = new Joint;
				glm::vec3 coord;
				file >> coord;
				j->setCoord(coord);
				glm::vec3 euler;
				file >> euler;
				j->setEuler(euler);
				file >> j->rigid0ID;
				file >> j->rigid1ID;
				file >> j->maxCoord;
				file >> j->minCoord;
				file >> j->maxRotation;
				file >> j->minRotation;
				file >> j->springConstant;
				file >> j->sprintRotationConstant;
				m->addJoint(j);
			}

			file >> m->boundingPosition;
			file >> m->boundingSize;

			file >> m->eyePosition;

			_model_after_process(m);

			reportMinorProgress(100);
		}

		static void _saveAnimation(std::ofstream &file, Model *pModel, AnimationBinding *pAnim)
		{
			std::string animName;
			if (pAnim)
				animName = pAnim->pTemplate->name;
			file << animName;
		}

		void save(Model *m, const std::string &filename, bool copyTexture)
		{
			std::experimental::filesystem::path p(filename);

			std::string dstFilepath = p.parent_path().string();
			if (dstFilepath == "")
				dstFilepath = ".";

			std::ofstream file(filename);

			file << m->pImages.size();
			for (auto pImage : m->pImages)
			{
				file << pImage->filename;
				file << pImage->sRGB;
				if (copyTexture)
				{
					std::string srcFilename = m->filepath + "/" + pImage->filename;
					std::string dstFilename = dstFilepath + "/" + pImage->filename;
					CopyFile(srcFilename.c_str(), dstFilename.c_str(), false);
				}
			}

			file << m->animated;;

			int vertexCount = m->positions.size();
			int indiceCount = m->indices.size();

			file << vertexCount;
			file << indiceCount;
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

			file << m->materials.size();
			for (auto mt : m->materials)
			{
				file << mt->indiceBase;
				file << mt->indiceCount;

				file << mt->visible;

				file << mt->name;

				file << mt->albedoR;
				file << mt->albedoG;
				file << mt->albedoB;
				file << mt->alpha;
				file << mt->spec;
				file << mt->roughness;
				if (mt->albedoAlphaMap) file << mt->albedoAlphaMap->filename;
				else file << 0;
				if (mt->normalHeightMap) file << mt->normalHeightMap->filename;
				else file << 0;
				if (mt->specRoughnessMap) file << mt->specRoughnessMap->filename;
				else file << 0;
			}

			file << m->bones.size();
			for (auto &bone : m->bones)
			{
				file << bone.name;
				file << bone.type;
				file << bone.parent;
				file << bone.rootCoord;
			}

			file << m->iks.size();
			for (auto &ik : m->iks)
			{
				file << ik.targetID;
				file << ik.effectorID;
				file << ik.iterations;
				file << ik.weight;

				file << ik.chain.size();
				file.write((char*)ik.chain.data(), sizeof(int) * ik.chain.size());
			}

			if (m->animated)
			{
				_saveAnimation(file, m, m->animationStand);
				_saveAnimation(file, m, m->animationForward);
				_saveAnimation(file, m, m->animationLeft);
				_saveAnimation(file, m, m->animationRight);
				_saveAnimation(file, m, m->animationBackward);
				_saveAnimation(file, m, m->animationJump);
			}


			file << m->rigidbodies.size();
			for (auto rb : m->rigidbodies)
			{
				int mode = (int)rb->type;
				file << mode;
				file << rb->name;
				file << rb->originCollisionGroupID;
				file << rb->originCollisionFreeFlag;
				file << rb->boneID;
				file << rb->getCoord();
				file << rb->getEuler();
				file << rb->density;
				file << rb->velocityAttenuation;
				file << rb->rotationAttenuation;
				file << rb->bounce;
				file << rb->friction;

				file << rb->shapes.size();
				for (auto s : rb->shapes)
				{
					file << s->getCoord();
					file << s->getEuler();
					file << s->getScale();
					int type = (int)s->type;
					file << type;
				}
			}

			file << m->joints.size();
			for (auto j : m->joints)
			{
				file << j->getCoord();
				file << j->getEuler();
				file << j->rigid0ID;
				file << j->rigid1ID;
				file << j->maxCoord;
				file << j->minCoord;
				file << j->maxRotation;
				file << j->minRotation;
				file << j->springConstant;
				file << j->sprintRotationConstant;
			}

			file << m->boundingPosition;
			file << m->boundingSize;

			file << m->eyePosition;
		}
	}

	namespace TKA
	{
		void load(Animation *a, std::ifstream &file)
		{
			int count;
			file >> count;
			a->motions.resize(count);
			for (int i = 0; i < count; i++)
			{
				int nameSize;
				file >> nameSize;
				a->motions[i].name.resize(nameSize);
				file.read((char*)a->motions[i].name.data(), nameSize);
				file >> a->motions[i].frame;
				file.read((char*)&a->motions[i].coord, sizeof(glm::vec3));
				file.read((char*)&a->motions[i].quaternion, sizeof(glm::vec4));
				file.read(a->motions[i].bezier, 64);
			}
		}

		void save(Animation *a, const std::string &filename)
		{
			std::ofstream file(filename, std::ios::binary);

			file << a->motions.size();
			for (auto &motion : a->motions)
			{
				file << motion.name.size();
				file.write(motion.name.c_str(), motion.name.size());
				file << motion.frame;
				file.write((char*)&motion.coord, sizeof(glm::vec3));
				file.write((char*)&motion.quaternion, sizeof(glm::vec4));
				file.write(motion.bezier, 64);
			}
		}
	}

	Model *createModel(const std::string &filename)
	{
		std::ifstream file(filename, std::ios::binary);
		if (!file.good())
		{
			report("Model File Lost:" + filename);
			return nullptr;
		}

		std::experimental::filesystem::path p(filename);
		auto ext = p.extension().string();
		void(*load_func)(Model *, std::ifstream &) = nullptr;
		if (ext == ".obj")
		{
			// obj need file open in text mode
			file.close();
			file.open(filename);
			load_func = &OBJ::load;
		}
		else if (ext == ".pmd")
			load_func = &PMD::load;
		else if (ext == ".tkm")
			load_func = &TKM::load;
		else
		{
			report("Model Format Not Support:" + ext);
			return nullptr;
		}

		auto m = new Model;
		m->filename = p.filename().string();
		m->filepath = p.parent_path().string();
		if (m->filepath == "")
			m->filepath = ".";
		load_func(m, file);

		_add_model(m);

		return m;
	}

	Animation *createAnimation(const std::string &filename)
	{
		std::ifstream file(filename, std::ios::binary);
		if (!file.good())
		{
			report("Animation File Lost:" + filename);
			return nullptr;
		}

		std::experimental::filesystem::path p(filename);
		auto ext = p.extension().string();
		void(*load_func)(Animation *, std::ifstream &) = nullptr;
		if (ext == ".vmd")
			load_func = &VMD::load;
		else if (ext == ".t3a")
			load_func = &TKA::load;
		else
		{
			report("Animation Format Not Support:%s" + ext);
			return nullptr;
		}
		auto a = new Animation;
		a->filename = p.filename().string();
		a->filepath = p.parent_path().string();
		if (a->filepath == "")
			a->filepath = ".";
		load_func(a, file);

		animations.push_back(a);

		return a;
	}
}
