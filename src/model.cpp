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
}
