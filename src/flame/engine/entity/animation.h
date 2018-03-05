#pragma once

#include <vector>
#include <memory>

#include <flame/common/math.h>

namespace flame
{
	struct KeyFrame
	{
		int frame = 0;
		glm::vec3 coord = glm::vec3(0.f);
		glm::vec4 quaternion = glm::vec4(0.f, 0.f, 0.f, 1.f);
	};

	struct AnimationTrack
	{
		std::string bone_name;
		std::vector<std::unique_ptr<KeyFrame>> keyframes;

		KeyFrame *new_keyframe(); // only for sorted keyframes
		KeyFrame *new_keyframe(int frame);
		void remove_keyframe(KeyFrame *k);
	};

	struct Animation
	{
		std::string filename;
		std::vector<std::unique_ptr<AnimationTrack>> tracks;

		AnimationTrack *new_track();
		void remove_track(AnimationTrack *t);
	};

	std::shared_ptr<Animation> getAnimation(const std::string &filename);

	struct AnimationBinding
	{
		std::shared_ptr<Animation> animation;
		int total_frame;
		std::vector<std::pair<int, AnimationTrack*>> tracks; // first: bone index, second: pointer to Animation's track
	};

	struct Model;
	struct Buffer;

	std::shared_ptr<AnimationBinding> get_animation_binding(Model *m, std::shared_ptr<Animation> anim);

	struct AnimationRunner
	{
		struct BoneData
		{
			glm::mat3 rotation = glm::mat3(1.f);
			glm::vec3 coord = glm::vec3(0.f);
		};

		Model *model;
		std::unique_ptr<BoneData[]> bone_data;
		std::unique_ptr<glm::mat4[]> bone_matrix;
		std::unique_ptr<Buffer> bone_buffer;

		AnimationBinding *curr_anim = nullptr;
		float curr_frame = 0.f;
		std::vector<int> curr_frame_index;
		bool enable_IK;

		AnimationRunner(Model *_model);
		void reset_bones();
		void refresh_bone();
		void refresh_bone(int i);
		void set_animation(AnimationBinding *animation);
		void update();
	};
}
