#include <flame/time.h>
#include <flame/system.h>
#include <flame/surface.h>
#include <flame/image.h>
#include <flame/math.h>
#include <flame/model/model.h>
#include <flame/graphics/device.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/pipeline.h>
#include <flame/graphics/descriptor.h>
#include <flame/graphics/buffer.h>
#include <flame/graphics/texture.h>
#include <flame/graphics/sampler.h>
#include <flame/graphics/framebuffer.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/semaphore.h>
#include <flame/graphics/queue.h>

#include <algorithm>

int main(int argc, char **args)
{
	using namespace flame;
	using namespace graphics;
	using namespace glm;

	auto near_plane = 0.1f;
	auto far_plane = 1000.f;
	auto fovy = 60.f;

	vec2 res(1280, 720);

	auto aspect = (float)res.x / res.y;

	auto sm = create_surface_manager();
	auto s = sm->create_surface(res.x, res.y, SurfaceStyleFrame,
		"Hello");

	auto d = create_device(true);

	auto sc = create_swapchain(d, s->get_win32_handle(), s->cx, s->cy);

	auto q = create_queue(d);
	auto cp = create_commandpool(d);

	struct UBO_terrain
	{
		vec3 coord;
		float dummy0;
		ivec2 count;
		float size;
		float height;
		vec2 resolution;
		float tessellation_factor;
		float dummy1;
		mat4 view_matrix;
		mat4 proj_matrix;
	};

	auto ub_terrain = create_buffer(d, sizeof(UBO_terrain), BufferUsageUniformBuffer,
		MemPropHost | MemPropHostCoherent);
	ub_terrain->map();

	auto ubo_terrain = (UBO_terrain*)ub_terrain->mapped;

	ubo_terrain->coord = vec3(-32.f, 0.f, -32.f);
	ubo_terrain->count = ivec2(64);
	ubo_terrain->size = 1.f;
	ubo_terrain->height = 1.f;
	ubo_terrain->resolution = vec2(1280.f, 720.f);
	ubo_terrain->tessellation_factor = 0.1f;
	ubo_terrain->proj_matrix = mat4(
		vec4(1.f, 0.f, 0.f, 0.f),
		vec4(0.f, -1.f, 0.f, 0.f),
		vec4(0.f, 0.f, 1.f, 0.f),
		vec4(0.f, 0.f, 0.f, 1.f)
	) * perspective(radians(fovy), aspect, near_plane, far_plane);

	Format depth_format;
	depth_format.v = Format::Depth16;

	auto depth_tex = create_texture(d, res.x, res.y, 1, 1, depth_format, 
		TextureUsageAttachment, MemPropDevice);
	auto depth_tex_view = create_textureview(d, depth_tex);

	auto rp = create_renderpass(d);
	rp->add_attachment_swapchain(sc, true);
	rp->add_attachment(depth_format, true);
	rp->add_subpass({0}, 1);
	rp->build();

	auto p = create_pipeline(d, rp, 0);
	p->set_size(res.x, res.y);
	p->set_patch_control_points(4);
	p->set_depth_test(true);
	p->set_depth_write(true);
	p->set_cull_mode(CullModeFront);
	p->set_primitive_topology(PrimitiveTopologyPatchList);
	p->add_shader("test/terrain.vert", {});
	p->add_shader("test/terrain.tesc", {});
	p->add_shader("test/terrain.tese", {});
	p->add_shader("test/terrain.geom", {});
	p->add_shader("test/terrain.frag", {});
	p->build();

	auto dp = create_descriptorpool(d);
	auto ds = dp->create_descriptorset(p, 0);
	ds->set_uniformbuffer(0, 0, ub_terrain);

	auto sampler = create_sampler(d, FilterLinear, FilterLinear,
		false);

	auto m_map = create_texture_from_file(d, cp, q, "1.png");
	auto m_map_view = create_textureview(d, m_map);
	ds->set_texture(1, 0, m_map_view, sampler);

	Framebuffer *fbs[2];
	Commandbuffer *cbs[2];
	for (auto i = 0; i < 2; i++)
	{
		fbs[i] = create_framebuffer(d, res.x, res.y, rp);
		fbs[i]->set_view_swapchain(0, sc, i);
		fbs[i]->set_view(1, depth_tex_view);
		fbs[i]->build();

		cbs[i] = cp->create_commandbuffer();
		cbs[i]->begin();
		cbs[i]->begin_renderpass(rp, fbs[i]);
		cbs[i]->bind_pipeline(p);
		cbs[i]->bind_descriptorset(ds);
		cbs[i]->draw(4, ubo_terrain->count.x * ubo_terrain->count.y, 0);
		cbs[i]->end_renderpass();
		cbs[i]->end();
	}

	auto image_avalible = create_semaphore(d);
	auto render_finished = create_semaphore(d);

	auto x_ang = 0.f;
	auto view_changed = true;
	s->add_mousemove_listener([&](Surface *s, int, int){
		if (s->mouse_buttons[0] & KeyStateDown)
		{
			x_ang += s->mouse_disp_x;
			view_changed = true;
		}
	});

	sm->run([&](){
		if (view_changed)
		{
			ubo_terrain->view_matrix = lookAt(vec3(0.f, 50.f, 50.f), vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f)) *
				rotate(radians(x_ang), vec3(0.f, 1.f, 0.f));

			view_changed = false;
		}

		auto index = sc->acquire_image(image_avalible);
		q->submit(cbs[index], image_avalible, render_finished);
		q->present(index, sc, render_finished);

		static long long last_fps = 0;
		if (last_fps != sm->fps)
			printf("%lld\n", sm->fps);
		last_fps = sm->fps;
	});

	return 0;
}
