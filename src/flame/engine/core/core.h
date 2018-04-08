#pragma once

#include <functional>
#include <string>
#include <vector>

#include <flame/global.h>
#include <flame/engine/graphics/graphics.h>

namespace flame
{
	struct Node;

	extern long long now_ns;
	extern double elapsed_time;

	struct Profile
	{
		std::string name;
		long long time;
	};

	extern std::vector<Profile> profiles;

	extern long long p_total_time;
	extern long long p_head_time;
	extern long long p_ui_begin_time;
	extern long long p_ui_end_time;
	extern long long p_render_time;
	extern long long p_tail_time;

	void begin_profile(const std::string &name);
	void end_profile();

	extern unsigned long long total_frame_count;
	extern uint32_t FPS;

	extern Node *root_node;

	int init(int _resolution_x, int _resolution_y, int debug_level,
		int window_cx, int window_cy, int window_style, const std::string &window_title);
	void add_after_frame_event(const std::function<void()> &e); // once
	void add_to_draw_list(VkCommandBuffer cb);
}
