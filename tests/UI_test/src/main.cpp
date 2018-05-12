#include <flame/surface.h>
#include <flame/system.h>
#include <flame/filesystem.h>
#include <flame/math.h>
#include <flame/graphics/device.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/shader.h>
#include <flame/graphics/pipeline.h>
#include <flame/graphics/framebuffer.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/semaphore.h>
#include <flame/graphics/queue.h>
#include <flame/UI/UI.h>

#include <Windows.h>

int main(int argc, char **args)
{
	using namespace flame;
	using namespace glm;

	vec2 res(1280, 720);

	auto sm = create_surface_manager();
	auto s = sm->create_surface(res.x, res.y, SurfaceStyleFrame,
		"Hello");

	auto d = graphics::create_device(false);

	auto sc = graphics::create_swapchain(d, s->get_win32_handle(), s->cx, s->cy);

	auto rp_ui = graphics::create_renderpass(d);
	rp_ui->add_attachment(sc->format, true);
	rp_ui->add_subpass({0}, -1);
	rp_ui->build();

	graphics::Framebuffer *fbs_ui[2];
	graphics::Commandbuffer *cbs_ui[2];
	for (auto i = 0; i < 2; i++)
	{
		fbs_ui[i] = create_framebuffer(d, res.x, res.y, rp_ui);
		fbs_ui[i]->set_view_swapchain(0, sc, i);
		fbs_ui[i]->build();

		cbs_ui[i] = d->cp->create_commandbuffer();
		cbs_ui[i]->begin();
		cbs_ui[i]->end();
	}

	auto image_avalible = graphics::create_semaphore(d);
	auto ui_finished = graphics::create_semaphore(d);

	auto ui = UI::create_instance(d, rp_ui);

	std::function<void()> f1 = [&](){
		ui->button("123");
	};

	std::function<void()> f2;
	memcpy(&f2, &f1, sizeof(f1));

	std::filesystem::copy_file("../tests/UI_test/windows/test.cpp", "test.cpp", 
		std::filesystem::copy_options::update_existing);

	LongString output;
	exec("", "cl test.cpp -LD -MD -I \"D:\\flame\\src\" -link"

		" \"C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.17134.0\\um\\x64\\kernel32.lib\""
		" \"C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.17134.0\\ucrt\\x64\\ucrt.lib\""
		" \"C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.17134.0\\um\\x64\\uuid.lib\""
		" \"D:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Community\\VC\\Tools\\MSVC\\14.14.26428\\lib\\x64\\msvcrt.lib\""
		" \"D:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Community\\VC\\Tools\\MSVC\\14.14.26428\\lib\\x64\\vcruntime.lib\""
		" \"D:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Community\\VC\\Tools\\MSVC\\14.14.26428\\lib\\x64\\oldnames.lib\""
		" \"D:\\flame\\bin\\flame_graphics.lib\""
		" \"D:\\flame\\bin\\flame_UI.lib\""
		/*
		" \"C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.17134.0\\um\\x64\\kernel32.lib\""
		" \"C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.17134.0\\ucrt\\x64\\ucrt.lib\""
		" \"C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.17134.0\\um\\x64\\uuid.lib\""
		" \"D:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Enterprise\\VC\\Tools\\MSVC\\14.14.26428\\lib\\x64\\msvcrt.lib\""
		" \"D:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Enterprise\\VC\\Tools\\MSVC\\14.14.26428\\lib\\x64\\vcruntime.lib\""
		" \"D:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Enterprise\\VC\\Tools\\MSVC\\14.14.26428\\lib\\x64\\oldnames.lib\""
		" \"D:\\flame\\bin\\flame_graphics.lib\""
		" \"D:\\flame\\bin\\flame_UI.lib\""
		*/
		, &output);

	auto lib = LoadLibrary("test.dll");
	auto fun = (void(*)(void*i))GetProcAddress(lib, "say_hello");

	sm->run([&](){
		ui->begin(res.x, res.y, sm->elapsed_time, s->mouse_x, s->mouse_y,
			(s->mouse_buttons[0] & KeyStateDown) != 0,
			(s->mouse_buttons[1] & KeyStateDown) != 0,
			(s->mouse_buttons[2] & KeyStateDown) != 0,
			s->mouse_scroll);
		ui->begin_window("Hello");
		fun(ui);
		f1();
		f2();
		ui->end_window();
		ui->end();

		for (auto i = 0; i < 2; i++)
		{
			cbs_ui[i]->begin();
			ui->record_commandbuffer(cbs_ui[i], rp_ui, fbs_ui[i]);
			cbs_ui[i]->end();
		}

		auto index = sc->acquire_image(image_avalible);

		d->q->submit(cbs_ui[index], image_avalible, ui_finished);
		d->q->wait_idle();
		d->q->present(index, sc, ui_finished);

		static long long last_fps = 0;
		if (last_fps != sm->fps)
			printf("%lld\n", sm->fps);
		last_fps = sm->fps;
	});

	return 0;
}
