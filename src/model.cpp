#include <algorithm>
#include <experimental/filesystem>

#include "core.h"
#include "scene.h"

namespace tke
{
	Animation::~Animation()
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
		case Type::eBox:
			return size.x * size.y * size.z * 8.f;
		case Type::eSphere:
			return 4.f * size.x * size.x * size.x * M_PI / 3.f;
		case Type::eCapsule:
			return 4.f * size.x * size.x * size.x * M_PI / 3.f + M_PI * size.x * size.x * size.y;
		}
		return 0.f;
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

	void Model::createTangent()
	{
		tangents.resize(positions.size());

		for (int i = 0; i < indices.size(); i += 3)
		{
			int id[3];
			id[0] = indices[i];
			id[1] = indices[i + 1];
			id[2] = indices[i + 2];

			auto u0 = uvs[id[1]].s - uvs[id[0]].s;
			auto v0 = uvs[id[1]].t - uvs[id[0]].t;

			auto u1 = uvs[id[2]].s - uvs[id[0]].s;
			auto v1 = uvs[id[2]].t - uvs[id[0]].t;

			auto e0 = positions[id[1]] - positions[id[0]];
			auto e1 = positions[id[2]] - positions[id[0]];

			auto d = u0 * v1 - u1 * v0;
			if (d == 0.f) continue;

			tangents[id[0]] = glm::normalize(glm::vec3(v1 * e0.x - v0 * e1.x, v1 * e0.y - v0 * e1.y, v1 * e0.z - v0 * e1.z));
			tangents[id[1]] = tangents[id[0]];
			tangents[id[2]] = tangents[id[0]];
		}
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
		file >> mainWeaponPosition;
	}

	void Model::saveDat(const std::string &filename)
	{
		std::ofstream file(filename);
		if (!file.good()) return;
		file << rigidbodies.size();
		for (int i = 0; i < rigidbodies.size(); i++)
			file.write((char*)&rigidbodies[i], sizeof(Rigidbody));

		file << eyePosition;
		file << mainWeaponPosition;
	}

	Animation *Model::bindAnimation(AnimationTemplate *pAnimationTemplate)
	{
		auto pAnimation = new Animation;
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

	void Model::arrangeBone()
	{
		for (int i = 0; i < bones.size(); i++)
		{
			bones[i].relateCoord = bones[i].rootCoord;
			int parentID = bones[i].parent;
			if (parentID != -1)
			{
				bones[i].relateCoord -= bones[parentID].rootCoord;
				bones[parentID].children.push_back(i);
			}
		}
	}

	void Model::refreshBone(BoneData *boneData, glm::mat4 *outMat)
	{
		for (int i = 0; i < bones.size(); i++)
		{
			auto pBone = &bones[i];
			outMat[i] = glm::translate(pBone->relateCoord + boneData[i].coord) * glm::mat4(boneData[i].rotation);
			int parentID = pBone->parent;
			if (parentID != -1)
			{
				outMat[i] = outMat[parentID] * outMat[i];
			}
		}
	}

	void Model::refreshBone(int boneID, BoneData *boneData, glm::mat4 *outMat)
	{
		auto pBone = &bones[boneID];
		outMat[boneID] = glm::translate(pBone->relateCoord + boneData[boneID].coord) * glm::mat4(boneData[boneID].rotation);
		int parentID = pBone->parent;
		if (parentID != -1)
		{
			outMat[boneID] = outMat[parentID] * outMat[boneID];
		}
		for (auto child : pBone->children)
		{
			refreshBone(child, boneData, outMat);
		}
	}

	void Model::addRigidbody(Rigidbody *pRigidbody)
	{
		static auto magicNumber = 0;
		pRigidbody->id = magicNumber++;
		if (pRigidbody->mode == Rigidbody::Mode::eStatic)
		{
			staticRigidbodies.push_back(pRigidbody);
		}
		else
		{
			dynamicRigidbodies.push_back(pRigidbody);
		}
		rigidbodies.push_back(pRigidbody);
	}

	Rigidbody *Model::deleteRigidbody(Rigidbody *pRigidBody)
	{
		if (pRigidBody->mode == Rigidbody::Mode::eStatic)
		{
			for (auto it = staticRigidbodies.begin(); it != staticRigidbodies.end(); it++)
			{
				if (*it == pRigidBody)
				{
					staticRigidbodies.erase(it);
					break;
				}
			}
		}
		else
		{
			for (auto it = dynamicRigidbodies.begin(); it != dynamicRigidbodies.end(); it++)
			{
				if (*it == pRigidBody)
				{
					dynamicRigidbodies.erase(it);
					break;
				}
			}
		}
		for (auto it = rigidbodies.begin(); it != rigidbodies.end(); it++)
		{
			if (*it == pRigidBody)
			{
				if (it > rigidbodies.begin())
				{
					pRigidBody = *(it - 1);
				}
				else
				{
					pRigidBody = nullptr;
				}
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

	AnimationSolver::AnimationSolver(Model *_pModel)
	{
		pModel = _pModel;
		boneData = new BoneData[pModel->bones.size()];
		boneMatrix = new glm::mat4[pModel->bones.size()];
		//glCreateBuffers(1, &boneMatrixUBO);
		//glNamedBufferStorage(boneMatrixUBO, sizeof(glm::mat4) * pModel->bones.size(), boneMatrix, GL_DYNAMIC_STORAGE_BIT);
	}

	AnimationSolver::~AnimationSolver()
	{
		delete[]boneData;
		delete[]boneMatrix;
		//glDeleteBuffers(1, &boneMatrixUBO);
	}

	void AnimationSolver::setAnimation(Animation *_pAnimation)
	{
		if (!_pAnimation && pAnimation)
		{
			for (int i = 0; i < pModel->bones.size(); i++)
			{
				boneData[i].coord = glm::vec3();
				boneData[i].rotation = glm::mat3();
				boneMatrix[i] = glm::mat4();
			}
			//glNamedBufferSubData(boneMatrixUBO, 0, sizeof(glm::mat4) * pModel->bones.size(), boneMatrix);
		}
		pAnimation = _pAnimation;
		frame = 0;
	}

	void AnimationSolver::sample()
	{
		for (int i = 0; i < pModel->bones.size(); i++)
		{
			boneMatrix[i] = glm::mat4();
			boneData[i].rotation = glm::mat3();
			boneData[i].coord = glm::vec3();
		}
		if (!pAnimation) return;
		//for (auto pTrack : pAnimation->pTracks)
		//{
		//	auto pBoneData = &boneData[pTrack->boneID];
		//	auto it = std::upper_bound(pTrack->pMotions.rbegin(), pTrack->pMotions.rend(), frame, [](int frame, BoneMotion *bm) {
		//		return frame > bm->frame;
		//	});

		//	if (it == pTrack->pMotions.rend()) continue;

		//	auto pLeftMotion = *it;
		//	auto pRightMotion = (it == pTrack->pMotions.rbegin() ? pTrack->pMotions[0] : *(it - 1));

		//	auto beta = 0.f;
		//	if (pLeftMotion != pRightMotion) beta = (frame - pLeftMotion->frame) / (pRightMotion->frame - pLeftMotion->frame);

		//	tk::Math::quaternionToMatrix(glm::normalize((1.f - beta) * pLeftMotion->quaternion + beta * pRightMotion->quaternion), pBoneData->rotation);
		//	pBoneData->coord = pLeftMotion->coord + (pRightMotion->coord - pLeftMotion->coord) * beta;

		//}
		if (frame < pAnimation->frameTotal) frame++;
	}

	void AnimationSolver::calcIK()
	{
		for (int i = 0; i < pModel->iks.size(); i++)
		{
			auto &ik = pModel->iks[i];
			auto t = glm::vec3(boneMatrix[ik.targetID][3]);
			//t.z *= -1.f;
			for (int times = 0; times < ik.iterations; times++)
			{
				for (int index = 0; index < ik.chain.size(); index++)
				{
					//index = iChain;
					auto e = glm::vec3(boneMatrix[ik.effectorID][3]);
					//e.z *= -1.f;
					if (glm::length(t - e) < 0.0001f)
					{
						goto nextIk;
					}

					auto boneID = ik.chain[index];

					auto p = glm::vec3(boneMatrix[boneID][3]);
					//p.z *= -1.f;

					auto pe = glm::normalize(e - p);
					auto pt = glm::normalize(t - p);
					//if (times + 1 == ikIteration)
					//{
					//	tke3DebugLine l;
					//	l.p0.coord = p;
					//	l.p0.color = glm::vec3(1, 0, 0);
					//	l.p1.coord = e;
					//	l.p1.color = glm::vec3(0, 1, 0);
					//	tke3_debugBuffer.lines.push_back(l);
					//	l.p0.coord = p;
					//	l.p0.color = glm::vec3(1, 0, 0);
					//	l.p1.coord = t;
					//	l.p1.color = glm::vec3(0, 0, 1);
					//	tke3_debugBuffer.lines.push_back(l);
					//}
					auto theDot = glm::dot(pe, pt);
					theDot = glm::clamp(theDot, 0.f, 1.f);
					auto theAcos = glm::acos(theDot);
					auto ang = glm::degrees(theAcos);
					if (glm::abs(ang) > 0.5f)
					{
						auto n = glm::normalize(glm::cross(pe, pt));
						if (glm::abs(n.z) < 1.f)
						{
							n.z = 0;
							n = glm::normalize(n);
						}
						boneData[boneID].rotation = glm::mat3(glm::rotate(ang, n)) * boneData[boneID].rotation;
						//refreshBone(ik.effectorID, boneData, outMat);
						pModel->refreshBone(boneID, boneData, boneMatrix);
						p = glm::vec3(boneMatrix[boneID][3]);
						e = glm::vec3(boneMatrix[ik.effectorID][3]);
						pe = glm::normalize(e - p);
						auto dot = glm::dot(pe, pt);
						int cut = 1;
					}
					//break;
				}
			}
		nextIk:
			//break;
			continue;
		}
	}

	void AnimationSolver::fixMatrix()
	{
		for (int i = 0; i < pModel->bones.size(); i++)
		{
			boneMatrix[i] *= glm::translate(-pModel->bones[i].rootCoord);
		}
	}

	void AnimationSolver::updateUBO()
	{
		//glNamedBufferSubData(boneMatrixUBO, 0, sizeof(glm::mat4) * pModel->bones.size(),boneMatrix);
	}
}
