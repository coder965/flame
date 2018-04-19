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

struct UBO
{
	glm::mat4 proj;
	glm::mat4 view;
	glm::mat4 model;
};

UBO *ubo;

struct Bone
{
	glm::mat4 mat;
	int parent;
	std::vector<int> children;
};

std::vector<Bone> bones;

glm::mat4 *bone_matrix;
glm::mat4 *_bone_matrix;

glm::vec2 *bone_pos;

static void update_bone_pos(flame::Model *m, int bone_id, glm::vec2 *dst)
{
	auto &b = bones[bone_id];

	auto mat = b.mat;

	if (b.parent != -1)
		mat = bones[b.parent].mat * mat;

	_bone_matrix[bone_id] = mat;

	if (b.parent != -1)
	{
		auto p0 = _bone_matrix[b.parent][3] * 0.001f;
		auto p1 = _bone_matrix[bone_id][3] * 0.001f;
		p0 = ubo->proj * ubo->view * p0;
		p0 /= p0.w;
		p1 = ubo->proj * ubo->view * p1;
		p1 /= p1.w;
		p0.z = p1.z = 0.f;
		if (glm::length(p0 - p1) > 0.001f)
		{
			auto w = glm::normalize(
				glm::cross(glm::vec3(p1) - glm::vec3(p0), glm::vec3(0.f, 1.f, 0.f)));
			w *= 0.005f;
			bone_pos[bone_id * 3 + 0].x = p0.x + w.x;
			bone_pos[bone_id * 3 + 0].y = p0.y + w.y;
			bone_pos[bone_id * 3 + 1].x = p0.x - w.x;
			bone_pos[bone_id * 3 + 1].y = p0.y - w.y;
			bone_pos[bone_id * 3 + 2].x = p1.x;
			bone_pos[bone_id * 3 + 2].y = p1.y;
		}
	}

	for (auto i = 0; i < b.children.size(); i++)
		update_bone_pos(m, b.children[i], dst);
}

int main(int argc, char **args)
{
	using namespace flame;
	using namespace graphics;
	using namespace glm;

	auto sm = create_surface_manager();
	auto s = sm->create_surface(1280, 720, SurfaceStyleFrame,
		"Hello");

	auto d = create_device(true, 1280, 720);

	auto sc = create_swapchain(d, s->get_win32_handle(), s->cx, s->cy);

	auto q = create_queue(d);
	auto cp = create_commandpool(d);

	auto ub = create_buffer(d, sizeof(mat4) * 3, BufferUsageUniformBuffer, 
		MemPropHost | MemPropHostCoherent);
	ub->map();
	ubo = (UBO*)ub->mapped;
	ubo->proj = mat4(
		vec4(1.f, 0.f, 0.f, 0.f),
		vec4(0.f, -1.f, 0.f, 0.f),
		vec4(0.f, 0.f, 1.f, 0.f),
		vec4(0.f, 0.f, 0.f, 1.f)
	) * perspective(radians(d->fovy), d->aspect, d->near_plane, d->far_plane);
	ubo->view = lookAt(vec3(0.f, 5.f, 0.f), vec3(0.f), vec3(0.f, 0.f, 1.f));
	ubo->model = translate(vec3(0.f, 0.f, -2.f)) * scale(vec3(0.001f));

	Format depth_format;
	depth_format.v = Format::Depth16;

	auto depth_tex = create_texture(d, -1, -1, 1, 1, depth_format, 
		TextureUsageAttachment, MemPropDevice);
	auto depth_tex_view = create_textureview(d, depth_tex);

	auto rp = create_renderpass(d);
	rp->add_attachment_swapchain(sc, true);
	rp->add_attachment(depth_format, true);
	rp->add_subpass({0}, 1);
	rp->build();

	auto pipeline = create_pipeline(d, rp, 0);
	pipeline->set_vertex_attributes({{
			VertexAttributeFloat3, 
			VertexAttributeFloat2,
			VertexAttributeFloat3,
			VertexAttributeFloat4,
			VertexAttributeFloat4
	}});
	pipeline->set_size(-1, -1);
	pipeline->set_depth_test(true);
	pipeline->set_depth_write(true);
	pipeline->add_shader("test/test_skeleton.vert", {});
	pipeline->add_shader("test/test.frag", {});
	pipeline->build();

	auto pipeline_line = create_pipeline(d, rp, 0);
	pipeline_line->set_vertex_attributes({{
			VertexAttributeFloat2
		}});
	pipeline_line->set_polygon_mode(PolygonModeLine);
	pipeline_line->set_size(-1, -1);
	pipeline_line->set_cull_mode(CullModeNone);
	pipeline_line->add_shader("test/line2d.vert", {});
	pipeline_line->add_shader("test/line2d.frag", {});
	pipeline_line->build();

	auto dp = create_descriptorpool(d);
	auto ds = dp->create_descriptorset(pipeline, 0);
	ds->set_uniformbuffer(0, 0, ub);

	auto m = load_model("../../Vulkan/data/models/goblin.dae");
	auto mvs = m->get_vertex_semantics();
	auto mvc = m->get_vertex_count();
	auto mic = m->get_indice_count();

	auto vb = create_buffer(d, mvc * m->get_vertex_size(0), BufferUsageVertexBuffer | 
		BufferUsageTransferDst, MemPropDevice);
	auto ib = create_buffer(d, mic * sizeof(int), BufferUsageIndexBuffer |
		BufferUsageTransferDst, MemPropDevice);
	auto sb = create_buffer(d, std::max(vb->size, ib->size), BufferUsageTransferSrc, 
		MemPropHost | MemPropHostCoherent);
	sb->map();
	{
		auto c = cp->create_commandbuffer();
		c->begin(true);
		memcpy(sb->mapped, m->get_vertexes(0), vb->size);
		c->copy_buffer(sb, vb, 0, 0, vb->size);
		c->end();
		q->submit(c, nullptr, nullptr);
		q->wait_idle();
		c->begin(true);
		memcpy(sb->mapped, m->get_indices(), ib->size);
		c->copy_buffer(sb, ib, 0, 0, ib->size);
		c->end();
		q->submit(c, nullptr, nullptr);
		q->wait_idle();
		cp->destroy_commandbuffer(c);
	}
	sb->unmap();

	bones.resize(m->get_bone_count());

	for (auto i = 0; i < bones.size(); i++)
	{
		m->get_bone_matrix(i, &bones[i].mat[0][0]);
		bones[i].mat = glm::transpose(bones[i].mat);
		bones[i].parent = m->get_bone_parent(i);
		bones[i].children.resize(m->get_bone_children_count(i));
		for (auto j = 0; j < bones[i].children.size(); j++)
			bones[i].children[j] = m->get_bone_child(i, j);
	}

	auto vb_bone_pos = create_buffer(d, bones.size() * 3 * sizeof(vec2), BufferUsageVertexBuffer,
		MemPropHost | MemPropHostCoherent);
	vb_bone_pos->map();
	bone_pos = (vec2*)vb_bone_pos->mapped;
	for (auto i = 0; i < bones.size(); i++)
	{
		bone_pos[i * 3 + 0].x = -10.f;
		bone_pos[i * 3 + 0].y = -10.f;
		bone_pos[i * 3 + 1].x = -10.f;
		bone_pos[i * 3 + 1].y = -10.f;
		bone_pos[i * 3 + 2].x = -10.f;
		bone_pos[i * 3 + 2].y = -10.f;
	}

	auto ub_bone = create_buffer(d, sizeof(mat4) * bones.size(), BufferUsageUniformBuffer,
		MemPropHost | MemPropHostCoherent);
	ub_bone->map();
	bone_matrix = (mat4*)ub_bone->mapped;
	for (auto i = 0; i < bones.size(); i++)
		bone_matrix[i] = mat4(1.f);
	_bone_matrix = new mat4[bones.size()];

	update_bone_pos(m, m->get_bone_root(), bone_pos);

	ds->set_uniformbuffer(2, 0, ub_bone);

	auto sampler = create_sampler(d, FilterLinear, FilterLinear,
		false);

	auto m_map = create_texture_from_file(d, cp, q, "../../Vulkan/data/textures/goblin_bc3_unorm.ktx");
	auto m_map_view = create_textureview(d, m_map);
	ds->set_texture(1, 0, m_map_view, sampler);

	Framebuffer *fbs[2];
	Commandbuffer *cbs[2];
	for (auto i = 0; i < 2; i++)
	{
		fbs[i] = create_framebuffer(d);
		fbs[i]->set_size(-1, -1);
		fbs[i]->set_renderpass(rp);
		fbs[i]->set_view_swapchain(0, sc, i);
		fbs[i]->set_view(1, depth_tex_view);
		fbs[i]->build();

		cbs[i] = cp->create_commandbuffer();
		cbs[i]->begin();
		cbs[i]->begin_renderpass(rp, fbs[i]);
		cbs[i]->bind_pipeline(pipeline);
		cbs[i]->bind_descriptorset(ds);
		cbs[i]->bind_vertexbuffer(vb);
		cbs[i]->bind_indexbuffer(ib, IndiceTypeUint);
		cbs[i]->draw_indexed(m->get_indice_count(), 0);
		cbs[i]->bind_pipeline(pipeline_line);
		cbs[i]->bind_vertexbuffer(vb_bone_pos);
		cbs[i]->draw(bones.size() * 3);
		cbs[i]->end_renderpass();
		cbs[i]->end();
	}

	auto image_avalible = create_semaphore(d);
	auto render_finished = create_semaphore(d);

	auto x_ang = 0.f;
	auto view_changed = false;
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
			ubo->model = translate(vec3(0.f, 0.f, -2.f)) * rotate(radians(x_ang), vec3(0.f, 0.f, 1.f)) * scale(vec3(0.001f));

			view_changed = false;
		}

		auto index = sc->acquire_image(image_avalible);
		q->submit(cbs[index], image_avalible, render_finished);
		q->present(index, sc, render_finished);

		//static long long last_ns = 0;
		//auto t = get_now_ns();
		//if (t - last_ns >= 10000000)
		//{
		//	static int B = 0;
		//	vec4 color(0.5f, 0.7f, (B / 255.f), 1.f);
		//	memcpy(ub->mapped, &color, sizeof(vec4));
		//	B++;
		//	if (B == 256)
		//		B = 0;
		//	last_ns = t;
		//}

		static long long last_fps = 0;
		if (last_fps != sm->fps)
			printf("%lld\n", sm->fps);
		last_fps = sm->fps;
	});

	return 0;
}