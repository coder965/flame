#pragma once

#include <vector>
#include <memory>

#include "../math/math.h"

namespace tke
{
	struct Bone
	{
		std::string name;
		char type = -1;

		int parents = -1;

		glm::vec3 rootCoord = glm::vec3(0.f);
		glm::vec3 relateCoord = glm::vec3(0.f);

		std::vector<int> children;
	};

	struct BoneIK
	{
		int targetID = -1;
		int effectorID = -1;
		unsigned short iterations = 0;
		float weight = 0.f;
		std::vector<int> chain;
	};

	struct BoneMotion
	{
		std::string name;
		int frame = 0;
		glm::vec3 coord = glm::vec3(0.f);
		glm::vec4 quaternion = glm::vec4(0.f, 0.f, 0.f, 1.f);
		char bezier[64] = {};
	};

	struct BoneMotionTrack
	{
		int boneID;
		std::vector<BoneMotion*> motions;
	};

	struct Animation
	{
		std::string filename;
		std::vector<std::unique_ptr<BoneMotion>> motions;
	};

	std::shared_ptr<Animation> getAnimation(const std::string &filename);

	struct AnimationBinding
	{
		std::shared_ptr<Animation> animation;
		int frameTotal;
		std::vector<std::unique_ptr<BoneMotionTrack>> tracks;
	};

	struct Model;
	struct UniformBuffer;
	struct AnimationComponent
	{
		struct BoneData
		{
			glm::mat3 rotation = glm::mat3(1.f);
			glm::vec3 coord = glm::vec3(0.f);
		};

		Model *model;
		AnimationBinding *currentAnimation = nullptr;
		float currentFrame = 0.f;
		float currentTime = 0.f;
		BoneData *boneData = nullptr;
		glm::mat4 *boneMatrix = nullptr;
		UniformBuffer *boneMatrixBuffer = nullptr;
		bool processIK = true;

		AnimationComponent(Model *_model);
		~AnimationComponent();
		void refreshBone();
		void refreshBone(int i);
		void setAnimation(AnimationBinding *animation);
		void update();
	};
}
