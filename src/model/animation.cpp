#include <iostream>
#include <map>

#include "animation.h"
#include "model.h"
#include "../core.h"

namespace tke
{
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

		void load(Animation *a, const std::string &filename)
		{
			std::ifstream file(filename, std::ios::binary);

			static_assert(sizeof(Header) == 50, "");
			static_assert(sizeof(BoneMotionData) == 111, "");

			Header header;
			file.read((char*)&header, sizeof(Header));

			int count;
			file & count;
			for (int i = 0; i < count; i++)
			{
				BoneMotionData data;
				file.read((char*)&data, sizeof(BoneMotionData));
				auto m = std::make_unique<BoneMotion>();
				m->name = japaneseToChinese(data.name);
				m->frame = data.frame;
				m->coord = glm::vec3(data.coord);
				m->quaternion = glm::vec4(data.quaternion);
				memcpy(m->bezier, data.bezier, 64);
				m->coord.z *= -1.f;
				m->quaternion.z *= -1.f;
				m->quaternion.w *= -1.f;
				a->motions.push_back(std::move(m));
			}
		}
	}

	namespace TKA
	{
		void load(Animation *a, const std::string &filename)
		{
			std::ifstream file(filename, std::ios::binary);

			int count;
			file >> count;
			for (int i = 0; i < count; i++)
			{
				auto m = std::make_unique<BoneMotion>();
				file > m->name;
				file >> m->frame;
				file & m->coord;
				file & m->quaternion;
				file.read(m->bezier, 64);
			}
		}

		void save(Animation *a, const std::string &filename)
		{
			std::ofstream file(filename, std::ios::binary);

			int motionCount = a->motions.size();
			file & motionCount;
			for (auto &m : a->motions)
			{
				file < m->name;
				file & m->frame;
				file & m->coord;
				file & m->quaternion;
				file.write(m->bezier, 64);
			}
		}
	}

	std::map<unsigned int, std::weak_ptr<Animation>> _animations;
	std::shared_ptr<Animation> getAnimation(const std::string &filename)
	{
		auto hash = HASH(filename.c_str());
		auto it = _animations.find(hash);
		if (it != _animations.end())
		{
			auto s = it->second.lock();
			if (s) return s;
		}

		std::experimental::filesystem::path path(filename);
		if (!std::experimental::filesystem::exists(filename))
			return nullptr;

		auto ext = path.extension().string();
		void(*load_func)(Animation *, const std::string &) = nullptr;
		if (ext == ".vmd")
			load_func = &VMD::load;
		else if (ext == ".t3a")
			load_func = &TKA::load;
		else
			return nullptr;

		auto a = std::make_shared<Animation>();
		a->filename = filename;
		load_func(a.get(), filename);

		_animations[hash] = a;
		return a;
	}

	AnimationComponent::AnimationComponent(Model *_model)
	{
		model = _model;
		boneData = new BoneData[model->bones.size()];
		boneMatrix = new glm::mat4[model->bones.size()];
		for (int i = 0; i < model->bones.size(); i++)
			boneMatrix[i] = glm::mat4(1.f);
		boneMatrixBuffer = new UniformBuffer(sizeof(glm::mat4) * model->bones.size());
		boneMatrixBuffer->update(boneMatrix, stagingBuffer, sizeof(glm::mat4) * model->bones.size());
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
			if (model->bones[i].parents != -1) boneMatrix[i] = boneMatrix[model->bones[i].parents] * boneMatrix[i];
		}
	}

	void AnimationComponent::refreshBone(int i)
	{
		assert(model && i < model->bones.size());

		boneMatrix[i] = glm::translate(model->bones[i].relateCoord + boneData[i].coord) * glm::mat4(boneData[i].rotation);
		if (model->bones[i].parents != -1) boneMatrix[i] = boneMatrix[model->bones[i].parents] * boneMatrix[i];

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
			boneMatrixBuffer->update(boneMatrix, stagingBuffer, sizeof(glm::mat4) * model->bones.size());
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

			for (auto &t : currentAnimation->tracks)
			{
				auto pBoneData = &boneData[t->boneID];
				auto it = std::upper_bound(t->motions.rbegin(), t->motions.rend(), currentFrame, [](int frame, BoneMotion *bm) {
					return frame > bm->frame;
				});

				if (it == t->motions.rend()) continue;

				auto pLeftMotion = *it;
				auto pRightMotion = (it == t->motions.rbegin() ? t->motions[0] : *(it - 1));

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

		if (processIK)
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

		boneMatrixBuffer->update(boneMatrix, stagingBuffer, sizeof(glm::mat4) * model->bones.size());
	}
}
