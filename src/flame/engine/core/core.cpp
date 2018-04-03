#include <map>
#include <regex>
#include <list>
#include <chrono>
#include <stack>

#include <flame/global.h>
#include <flame/system.h>
#include <flame/engine/core/core.h>
#include <flame/engine/core/input.h>
#include <flame/engine/core/surface.h>
#include <flame/engine/graphics/synchronization.h>
#include <flame/engine/entity/entity.h>
#include <flame/engine/entity/node.h>
#include <flame/engine/physics/physics.h>
#include <flame/engine/sound/sound.h>
#include <flame/engine/ui/ui.h>

namespace flame
{
	long long now_ns;
	double elapsed_time;

	long long get_now_ns()
	{
		return std::chrono::time_point_cast<std::chrono::nanoseconds>(
			std::chrono::system_clock::now()
			).time_since_epoch().count();
	}

	std::stack<Profile> profile_stack;
	std::vector<Profile> profiles;

	long long p_total_time = 1;
	long long p_head_time;
	long long p_ui_begin_time;
	long long p_ui_end_time;
	long long p_render_time;
	long long p_tail_time;

	void begin_profile(const std::string &name)
	{
		Profile p;
		p.name = name;
		p.time = get_now_ns();
		profile_stack.push(p);
	}

	void end_profile()
	{
		profiles.push_back(profile_stack.top());
		profile_stack.pop();
		profiles.back().time = get_now_ns() - profiles.back().time;
	}

	unsigned long long total_frame_count = 0;
	uint32_t FPS;

	Node *root_node;

	static std::vector<VkCommandBuffer> draw_list;
	
	static VkSemaphore render_finished;

	int init(int _resolution_x, int _resolution_y, int debug_level,
		int window_cx, int window_cy, int window_style, const std::string &window_title)
	{
		printf("%d\n", GetCurrentProcessId());

		auto init_start_time = GetTickCount();

#ifdef _MSVC_LANG
		SetProcessDPIAware();
#endif
		init_graphics(debug_level > 0, _resolution_x, _resolution_y);
		new Surface(window_cx, window_cy, window_style, window_title);
		render_finished = createSemaphore();
		ui::init();
		init_sound();
#if FLAME_ENABLE_PHYSICS
		init_physics();
#endif
		init_entity();
		root_node = new Node;
		root_node->name = "root";

		printf("\n=====INFO=====\nengine init finished - %d ms\n==============\n", GetTickCount() - init_start_time);

		return NoErr;
	}

	static std::list<std::function<void()>> _destroy_listeners;

	void add_destroy_listener(const std::function<void()> &e)
	{
		_destroy_listeners.push_back(e);
	}

	void remove_destroy_listener(const std::function<void()> &e)
	{
		for (auto it = _destroy_listeners.begin(); it != _destroy_listeners.end(); it++)
		{
			if (TK_GET_ADDRESS(*it) == TK_GET_ADDRESS(e))
			{
				_destroy_listeners.erase(it);
				return;
			}
		}
	}

	static std::list<std::function<void()>> _after_frame_events;

	static std::mutex _after_frame_event_mtx;

	void add_after_frame_event(const std::function<void()> &e)
	{
		_after_frame_event_mtx.lock();
		_after_frame_events.push_back(e);
		_after_frame_event_mtx.unlock();
	}

	void add_to_draw_list(VkCommandBuffer cb)
	{
		draw_list.push_back(cb);
	}

	static long long _last_sec_time;

	void run(PF_EVENT0 render_func)
	{
		assert(render_func);

		now_ns = get_now_ns();
		_last_sec_time = now_ns;

		static long long last_time_ns;

		for (;;)
		{
			profiles.clear();

			MSG msg;
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
					return;
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			last_time_ns = now_ns;
			now_ns = get_now_ns();
			elapsed_time = (now_ns - last_time_ns) / 1000000000.0;

			static unsigned int frame_count = 0;
			frame_count++;
			total_frame_count++;
			bool do_copy_profile = false;
			if (now_ns - _last_sec_time >= 1000000000)
			{
				FPS = std::max(frame_count, 1U);
				_last_sec_time = now_ns;
				frame_count = 0;
				do_copy_profile = true;
			}

			{
				input_on_frame_begin();

				root_node->update();

				surface->acquire_image();

				ui::begin();

				render_func();

				ui::end();

				if (!draw_list.empty())
				{
					vk_queue_submit(draw_list.size(), draw_list.data(), surface->image_available, render_finished);
					draw_list.clear();
				}

				surface->present(render_finished);

				int cut = 1;

				input_on_frame_end();

				_after_frame_event_mtx.lock();
				for (auto &e : _after_frame_events)
					e();
				_after_frame_events.clear();
				_after_frame_event_mtx.unlock();
				//end_profile();
			}
		}

		for (auto &e : _destroy_listeners)
			e();
	}
}
