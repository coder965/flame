#include <map>
#include <regex>
#include <list>
#include <chrono>
#include <stack>

#include <flame/global.h>
#include <flame/system.h>
#include <flame/surface.h>
#include <flame/engine/core/core.h>
#include <flame/engine/graphics/texture.h>
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

	static SurfaceManager *surface_manager;
	static Surface *surface;
	static VkSurfaceKHR vk_surface;
	static VkSwapchainKHR vk_swapchain;
	static VkImage window_images[2];
	static std::unique_ptr<TextureView> window_image_views[2];
	static uint curr_window_image_index;
	static VkSemaphore image_available;

	static void create_swapchain()
	{
		VkWin32SurfaceCreateInfoKHR surface_info;
		surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surface_info.flags = 0;
		surface_info.pNext = nullptr;
		surface_info.hinstance = (HINSTANCE)get_hinst();
		surface_info.hwnd = (HWND)get_surface_win32_handle(surface);
		vk_chk_res(vkCreateWin32SurfaceKHR(vk_instance, &surface_info, nullptr, &vk_surface));

		VkBool32 surface_supported;
		vkGetPhysicalDeviceSurfaceSupportKHR(vk_physical_device, 0, vk_surface, &surface_supported);
		assert(surface_supported);

		VkSurfaceCapabilitiesKHR surface_capabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_physical_device, vk_surface, &surface_capabilities);
		auto s_size = get_surface_size(surface);
		assert(s_size.x >= surface_capabilities.minImageExtent.width);
		assert(s_size.y >= surface_capabilities.minImageExtent.height);
		assert(s_size.x <= surface_capabilities.maxImageExtent.width);
		assert(s_size.y <= surface_capabilities.maxImageExtent.height);

		unsigned int surface_format_count = 0;
		std::vector<VkSurfaceFormatKHR> surface_formats;
		vkGetPhysicalDeviceSurfaceFormatsKHR(vk_physical_device, vk_surface,
			&surface_format_count, nullptr);
		assert(surface_format_count > 0);
		surface_formats.resize(surface_format_count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(vk_physical_device, vk_surface,
			&surface_format_count, surface_formats.data());

		swapchain_format = surface_formats[0].format;

		VkSwapchainCreateInfoKHR swapchain_info;
		swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchain_info.flags = 0;
		swapchain_info.pNext = nullptr;
		swapchain_info.surface = vk_surface;
		swapchain_info.minImageCount = 2;
		swapchain_info.imageFormat = swapchain_format;
		swapchain_info.imageColorSpace = surface_formats[0].colorSpace;
		swapchain_info.imageExtent.width = s_size.x;
		swapchain_info.imageExtent.height = s_size.y;
		swapchain_info.imageArrayLayers = 1;
		swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchain_info.queueFamilyIndexCount = 0;
		swapchain_info.pQueueFamilyIndices = nullptr;
		swapchain_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchain_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
		swapchain_info.clipped = true;
		swapchain_info.oldSwapchain = 0;
		vk_chk_res(vkCreateSwapchainKHR(vk_device, &swapchain_info, nullptr, &vk_swapchain));

		uint imageCount = 0;
		vkGetSwapchainImagesKHR(vk_device, vk_swapchain, &imageCount, nullptr);
		vkGetSwapchainImagesKHR(vk_device, vk_swapchain, &imageCount, window_images);

		for (int i = 0; i < 2; i++)
			window_image_views[i] = std::make_unique<TextureView>(window_images[i], swapchain_format, VK_IMAGE_ASPECT_COLOR_BIT);
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

		surface_manager = create_surface_manager();
		surface = create_surface(surface_manager, window_cx, window_cy, window_style, window_title);

#ifdef _MSVC_LANG
		SetProcessDPIAware();
#endif
		init_graphics(debug_level > 0, _resolution_x, _resolution_y);
		image_available = createSemaphore();
		render_finished = createSemaphore();
		create_swapchain();
		add_resize_listener(surface, [](Surface*, int, int){
			create_swapchain();
		});
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
				root_node->update();

				vk_chk_res(vkAcquireNextImageKHR(vk_device, vk_swapchain, UINT64_MAX, image_available, VK_NULL_HANDLE, &curr_window_image_index));

				ui::begin();

				render_func();

				ui::end();

				if (!draw_list.empty())
				{
					vk_queue_submit(draw_list.size(), draw_list.data(), image_available, render_finished);
					draw_list.clear();
				}

				VkPresentInfoKHR present_info;
				present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
				present_info.pNext = nullptr;
				present_info.pResults = nullptr;
				present_info.waitSemaphoreCount = 1;
				present_info.pWaitSemaphores = &render_finished;
				present_info.swapchainCount = 1;
				present_info.pSwapchains = &vk_swapchain;
				present_info.pImageIndices = &curr_window_image_index;
				vk_chk_res(vkQueuePresentKHR(vk_graphics_queue, &present_info));
				vk_queue_wait_idle();

				_after_frame_event_mtx.lock();
				for (auto &e : _after_frame_events)
					e();
				_after_frame_events.clear();
				_after_frame_event_mtx.unlock();
				//end_profile();
			}
		}
	}
}
