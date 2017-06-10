#include "object.h"

namespace tke
{
	Object::Object(ObjectType _type, Model *_pModel)
		:type(_type), pModel(_pModel)
	{
		if (pModel->rigidbodies.size() > 0)
			rigidDatas = new RigidData[pModel->rigidbodies.size()];
	}

	Object::~Object()
	{
		delete[]rigidDatas;
	}

	StaticObject::StaticObject(Model *_pModel)
		:Object(ObjectTypeStatic, _pModel)
	{
	}

	StaticObject::~StaticObject() {}

	AnimatedObject::AnimatedObject(Model *_pModel)
		: Object(ObjectTypeAnimated, _pModel)
	{
		boneData = new BoneData[pModel->bones.size()];
		boneMatrix = new glm::mat4[pModel->bones.size()];
		boneMatrixBuffer = new UniformBuffer(sizeof(glm::mat4) * pModel->bones.size());
	}

	AnimatedObject::~AnimatedObject() 
	{
		delete[]boneData;
		delete[]boneMatrix;
		delete boneMatrixBuffer;
	}

	void AnimatedObject::setAnimation(Animation *animation)
	{
		if (!animation && currentAnimation)
		{
			for (int i = 0; i < pModel->bones.size(); i++)
			{
				boneData[i].coord = glm::vec3();
				boneData[i].rotation = glm::mat3();
				boneMatrix[i] = glm::mat4();
			}
			//glNamedBufferSubData(boneMatrixUBO, 0, sizeof(glm::mat4) * pModel->bones.size(), boneMatrix);
		}
		currentAnimation = animation;
		frame = 0;
	}

	void AnimatedObject::sample()
	{
		for (int i = 0; i < pModel->bones.size(); i++)
		{
			boneMatrix[i] = glm::mat4();
			boneData[i].rotation = glm::mat3();
			boneData[i].coord = glm::vec3();
		}
		if (!currentAnimation) return;
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
		if (frame < currentAnimation->frameTotal) frame++;
	}

	void AnimatedObject::calcIK()
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

	void AnimatedObject::fixMatrix()
	{
		for (int i = 0; i < pModel->bones.size(); i++)
		{
			boneMatrix[i] *= glm::translate(-pModel->bones[i].rootCoord);
		}
	}

	void AnimatedObject::updateUBO()
	{
		//glNamedBufferSubData(boneMatrixUBO, 0, sizeof(glm::mat4) * pModel->bones.size(),boneMatrix);
	}
}