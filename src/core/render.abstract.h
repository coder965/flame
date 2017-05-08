#ifndef __TKE_RENDER__
#define __TKE_RENDER__

#include <vector>
#include <list>
#include <string>
#include <memory>
#include <experimental/filesystem>

#include "utils.h"

namespace tke
{
	REFLECTABLE enum class StageFlags : int
	{
		null,
		REFLe vert = 1 << 0,
		REFLe tesc = 1 << 1,
		REFLe tese = 1 << 2,
		REFLe geom = 1 << 3,
		REFLe frag = 1 << 4
	};

	inline StageFlags StageFlagByExt(const std::string &ext)
	{
		if (ext == ".vert") return StageFlags::vert;
		if (ext == ".tesc") return StageFlags::tesc;
		if (ext == ".tese") return StageFlags::tese;
		if (ext == ".geom") return StageFlags::geom;
		if (ext == ".frag") return StageFlags::frag;
		return StageFlags::vert;
	}

	REFLECTABLE enum class AspectFlags : int
	{
		null,
		REFLe color = 1 << 0,
		REFLe depth = 1 << 1,
		REFLe stencil = 1 << 2,
		REFLe depth_stencil = 1 << 3
	};

	REFLECTABLE enum class RenderPassType : int
	{
		null,
		REFLe draw_action = 1 << 0,
		REFLe call_secondary_cmd = 1 << 1
	};

	REFLECTABLE enum class DrawActionType : int
	{
		null,
		REFLe draw_action = 1 << 0,
		REFLe call_fuction = 1 << 1
	};

	REFLECTABLE enum class DrawcallType : int
	{
		null,
		REFLe vertex = 1 << 0,
		REFLe index = 1 << 1,
		REFLe indirect_vertex = 1 << 2,
		REFLe indirect_index = 1 << 3,
		REFLe push_constant = 1 << 4
	};

	REFLECTABLE enum class PushConstantType : int
	{
		null,
		REFLe int_t = 1 << 0,
		REFLe float_t = 1 << 1,
		REFLe vec2_t = 1 << 2,
		REFLe vec3_t = 1 << 3,
		REFLe vec4_t = 1 << 4
	};

	REFLECTABLE enum class PrimitiveTopology : int
	{
		null,
		REFLe triangle_list = 1 << 0,
		REFLe line_list = 1 << 1,
		REFLe line_strip = 1 << 2,
		REFLe patch_list = 1 << 3
	};

	REFLECTABLE enum class PolygonMode : int
	{
		null,
		REFLe fill = 1 << 0,
		REFLe line = 1 << 1
	};

	REFLECTABLE enum class CullMode : int
	{
		null,
		REFLe none = 1 << 0,
		REFLe front = 1 << 1,
		REFLe back = 1 << 2,
		REFLe front_and_back = 1 << 3
	};

	REFLECTABLE enum class BlendFactor : int
	{
		null,
		REFLe zero = 1 << 0,
		REFLe one = 1 << 1,
		REFLe src_alpha = 1 << 2,
		REFLe one_minus_src_alpha = 1 << 3
	};

	REFLECTABLE struct BlendAttachment
	{
		REFL_BANK;

		REFLv bool enable = false;
		REFLe BlendFactor src_color = BlendFactor::zero;
		REFLe BlendFactor dst_color = BlendFactor::zero;
		REFLe BlendFactor src_alpha = BlendFactor::zero;
		REFLe BlendFactor dst_alpha = BlendFactor::zero;
	};

	REFLECTABLE enum class DescriptorType : int
	{
		null,
		REFLe uniform_buffer = 1 << 0,
		REFLe storage_buffer = 1 << 1,
		REFLe storage_image = 1 << 2,
		REFLe sampler = 1 << 3,
		REFLe input_attachment = 1 << 4
	};

	REFLECTABLE struct Descriptor
	{
		REFL_BANK;

		REFLe DescriptorType type = DescriptorType::uniform_buffer;
		REFLv int binding = 0;
		REFLv int count = 0;
	};

	REFLECTABLE struct PushConstantRange
	{
		REFL_BANK;

		REFLv int offset = 0;
		REFLv int size = 0;
	};

	REFLECTABLE struct StageAbstract
	{
		REFL_BANK;

		REFLv std::string filename;
		std::string filepath;
		StageFlags type;

		std::vector<Descriptor> descriptors;
		std::vector<PushConstantRange> pushConstantRanges;
	};

	REFLECTABLE enum class SamplerType : int
	{
		null,
		REFLe none = 1 << 0,
		REFLe screen = 1 << 1,
		REFLe screen_uv = 1 << 2,
		REFLe color = 1 << 3
	};

	REFLECTABLE struct LinkResource
	{
		REFL_BANK;

		REFLe DescriptorType type = DescriptorType::uniform_buffer;
		REFLv int binding = 0;
		REFLv int array_element = 0;
		REFLv std::string name;
		REFLe SamplerType sampler = SamplerType::none;
	};

	template <class StageType>
	REFLECTABLE struct PipelineAbstract
	{
		REFL_BANK;

		REFLv std::string name;
		std::string filename;
		std::string filepath;

		REFLv int cx = 0;
		REFLv int cy = 0;

		REFLv int patch_control_points = 0;
		REFLv bool depth_test = false;
		REFLv bool depth_write = false;
		REFLv bool depth_clamp = false;

		REFLe PrimitiveTopology primitive_topology = PrimitiveTopology::triangle_list;
		REFLe PolygonMode polygon_mode = PolygonMode::fill;
		REFLe CullMode cull_mode = CullMode::back;

		std::vector<BlendAttachment> blendAttachments;
		std::vector<StageType*> stages;
		std::vector<LinkResource> links;

		~PipelineAbstract()
		{
			for (auto s : stages) delete s;
		}
		void setFilename(const std::string &_filename)
		{
			filename = _filename;
			std::experimental::filesystem::path p(filename);
			filepath = p.parent_path().string();
		}
		void loadXML()
		{
			blendAttachments.clear();
			stages.clear();
			links.clear();

			AttributeTree at("pipeline");
			at.loadXML(filename);
			at.obtainFromAttributes(this, b);

			for (auto c : at.children)
			{
				if (c->name == "stage")
				{
					auto s = new StageType;
					c->obtainFromAttributes(s, s->b);
					std::experimental::filesystem::path p(s->filename);
					s->filepath = p.parent_path().string();
					auto ext = p.extension().string();
					s->type = StageFlagByExt(ext);

					for (auto c : c->children)
					{
						if (c->name == "descriptor")
						{
							Descriptor d;
							c->obtainFromAttributes(&d, d.b);
							s->descriptors.push_back(d);
						}
						else if (c->name == "push_constant")
						{
							PushConstantRange pc;
							c->obtainFromAttributes(&pc, pc.b);
							s->pushConstantRanges.push_back(pc);
						}
					}

					stages.push_back(s);
				}
				else if (c->name == "blend_attachment")
				{
					BlendAttachment ba;
					c->obtainFromAttributes(&ba, ba.b);
					blendAttachments.push_back(ba);
				}
				else if (c->name == "link")
				{
					LinkResource l;
					c->obtainFromAttributes(&l, l.b);
					links.push_back(l);
				}
			}
		}
		void saveXML()
		{
			AttributeTree at("pipeline");
			at.addAttributes(this, b);

			for (auto s : stages)
			{
				auto n = new AttributeTreeNode("stage");
				n->addAttributes(s, s->b);
				at.children.push_back(n);

				for (auto &d : s->descriptors)
				{
					auto nn = new AttributeTreeNode("descriptor");
					nn->addAttributes(&d, d.b);
					n->children.push_back(n);
				}
				for (auto &p : s->pushConstantRanges)
				{
					auto nn = new AttributeTreeNode("push_constant");
					nn->addAttributes(&p, p.b);
					n->children.push_back(n);
				}
			}
			for (auto &b : blendAttachments)
			{
				auto n = new AttributeTreeNode("blend_attachment");
				n->addAttributes(&b, b.b);
				at.children.push_back(n);
			}
			for (auto &l : links)
			{
				auto n = new AttributeTreeNode("link");
				n->addAttributes(&l, l.b);
				at.children.push_back(n);
			}

			at.saveXML(filename);
		}
	};

	REFLECTABLE struct DrawcallAbstract : Element
	{
		Container *parent = nullptr;

		REFL_BANK;

		REFLv std::string name;

		REFLv int instance_count = 1;
		REFLv int first_instance = 0;

		REFLe DrawcallType type = DrawcallType::vertex;

		REFLv int vertex_count = 0;
		REFLv int first_vertex = 0;

		REFLv int index_count = 0;
		REFLv int first_index = 0;
		REFLv int vertex_offset = 0;

		REFLv std::string indirect_vertex_buffer_name;
		REFLv std::string indirect_index_buffer_name;

		REFLv int first_indirect = 0;
		REFLv int indirect_count = 0;

		REFLv StageFlags push_constant_stage = StageFlags::vert;
		REFLv int push_constant_offset = 0;
		REFLe PushConstantType push_constant_type = PushConstantType::int_t;
		void *push_constant_value = nullptr;

		~DrawcallAbstract()
		{
			delete push_constant_value;
		}
		void loadFromAt(AttributeTreeNode *n)
		{
			n->obtainFromAttributes(this, b);
		}
		void saveToAt(AttributeTreeNode *n)
		{
			n->addAttributes(this, b);
		}
	};

	template <class DrawcallType>
	REFLECTABLE struct DrawActionAbstract : Element, Container
	{
		Container *parent = nullptr;

		REFL_BANK;

		REFLv std::string  name;

		REFLe DrawActionType type = DrawActionType::draw_action;

		REFLv std::string vertex_buffer_name;
		REFLv std::string index_buffer_name;
		REFLv std::string pipeline_name;

		REFLv std::string renderable_name;

		REFLv int cx = 0;
		REFLv int cy = 0;

		std::list<DrawcallType> drawcalls;
		
		template <class... _Valty>
		DrawcallType *addDrawcall(_Valty&&... _Val)
		{
			drawcalls.emplace_back(_Val...);
			auto d = &drawcalls.back();
			d->parent = this;
			return d;
		}
		void loadFromAt(AttributeTreeNode *n)
		{
			n->obtainFromAttributes(this, b);

			auto drawcallsNode = n->firstNode("drawcalls");
			if (drawcallsNode)
			{
				for (auto nn : drawcallsNode->children)
				{
					if (nn->name == "drawcall")
					{
						auto d = addDrawcall();
						d->loadFromAt(nn);
					}
				}
			}
		}
		void saveToAt(AttributeTreeNode *n)
		{
			n->addAttributes(this, b);

			auto drawcallsNode = new AttributeTreeNode("drawcalls");
			n->children.push_back(drawcallsNode);
			for (auto &d : drawcalls)
			{
				auto n = new AttributeTreeNode("drawcall");
				drawcallsNode->children.push_back(n);
				d.saveToAt(n);
			}
		}
		void maintain(int row) override
		{
			maintainList(drawcalls);
		}
	};

	REFLECTABLE struct AttachmentAbstract : Element
	{
		Container *parent;

		REFL_BANK;

		REFLv std::string name;

		REFLv std::string image_name;
		REFLv int level = 0;
		REFLv int layer = 0;

		REFLe AspectFlags aspect = AspectFlags::color;

		REFLv bool clear = false;
		REFLv float clear_r = 0.f;
		REFLv float clear_g = 0.f;
		REFLv float clear_b = 0.f;
		REFLv float clear_a = 0.f;
		REFLv float clear_depth = 1.f;
		REFLv int clear_stencil = 0;
		void loadFromAt(AttributeTreeNode *n)
		{
			n->obtainFromAttributes(this, b);
		}
		void saveToAt(AttributeTreeNode *n)
		{
			n->addAttributes(this, b);
		}
	};

	REFLECTABLE struct DependencyAbstract : Element
	{
		Container *parent;

		REFL_BANK;

		REFLv std::string pass_name;
		void *target = nullptr;
		void loadFromAt(AttributeTreeNode *n)
		{
			n->obtainFromAttributes(this, b);
		}
		void saveToAt(AttributeTreeNode *n)
		{
			n->addAttributes(this, b);
		}
	};

	enum class RenderPassElement
	{
		eAction,
		eColorAttachment,
		eDepthStencilAttachment,
		eDependency
	};

	template <class AttachmentType, class DependencyType, class DrawActionType>
	REFLECTABLE struct RenderPassAbstract : Element, Container
	{
		Container *parent;

		std::list<AttachmentType> colorAttachments;
		AttachmentType *depthStencilAttachment = nullptr;

		std::list<DependencyType> dependencies;

		std::list<DrawActionType> actions;

		REFL_BANK;

		REFLe RenderPassType type = RenderPassType::draw_action;

		REFLv std::string name;

		REFLv std::string secondary_cmd_name;

		~RenderPassAbstract() { delete depthStencilAttachment; }
		template <class... _Valty>
		AttachmentType *addColorAttachment(_Valty&&... _Val)
		{
			colorAttachments.emplace_back(_Val...);
			auto p = &colorAttachments.back();
			p->parent = this;
			return p;
		}
		template <class... _Valty>
		AttachmentType *addDepthStencilAttachment(_Valty&&... _Val)
		{
			auto p = new AttachmentType(_Val...);
			p->parent = this;
			depthStencilAttachment = p;
			return p;
		}
		template <class... _Valty>
		DependencyType *addDependency(_Valty&&... _Val)
		{
			dependencies.emplace_back(_Val...);
			auto p = &dependencies.back();
			p->parent = this;
			return p;
		}
		template <class... _Valty>
		DrawActionType *addAction(_Valty&&... _Val)
		{
			actions.emplace_back(_Val...);
			auto p = &actions.back();
			p->parent = this;
			return p;
		}
		void loadFromAt(AttributeTreeNode *n)
		{
			n->obtainFromAttributes(this, b);

			auto attachmentNode = n->firstNode("attachments");
			if (attachmentNode)
			{
				auto colorNode = attachmentNode->firstNode("color");
				if (colorNode)
				{
					for (auto n : colorNode->children)
					{
						if (n->name == "attachment")
						{
							auto attachment = addColorAttachment();
							attachment->loadFromAt(n);
						}
					}
				}
				auto depthStencilNode = attachmentNode->firstNode("depth_stencil");
				if (depthStencilNode)
				{
					auto attachmentNode = depthStencilNode->firstNode("attachment");
					if (attachmentNode)
					{
						depthStencilAttachment = addDepthStencilAttachment();
						depthStencilAttachment->loadFromAt(attachmentNode);
					}
				}
			}
			auto dependenciesNode = n->firstNode("dependencies");
			if (dependenciesNode)
			{
				for (auto n : dependenciesNode->children)
				{
					if (n->name == "dependency")
					{
						auto dependency = addDependency();
						dependency->loadFromAt(n);
					}
				}
			}
			auto actionsNode = n->firstNode("actions");
			if (actionsNode)
			{
				for (auto n : actionsNode->children)
				{
					if (n->name == "action")
					{
						auto action = addAction();
						action->loadFromAt(n);
					}
				}
			}
		}
		void saveToAt(AttributeTreeNode *n)
		{
			n->addAttributes(this, b);

			auto attachmentNode = new AttributeTreeNode("attachments");
			n->children.push_back(attachmentNode);
			auto colorNode = new AttributeTreeNode("color");
			attachmentNode->children.push_back(colorNode);
			for (auto &c : colorAttachments)
			{
				auto n = new AttributeTreeNode("attachment");
				colorNode->children.push_back(n);
				c.saveToAt(n);
			}
			auto depthStencilNode = new AttributeTreeNode("depth_stencil");
			attachmentNode->children.push_back(depthStencilNode);
			if (depthStencilAttachment)
			{
				auto n = new AttributeTreeNode("attachment");
				depthStencilNode->children.push_back(n);
				depthStencilAttachment->saveToAt(n);
			}
			auto dependenciesNode = new AttributeTreeNode("dependencies");
			for (auto &d : dependencies)
			{
				auto n = new AttributeTreeNode("dependency");
				dependenciesNode->children.push_back(n);
				d.saveToAt(n);
			}
			auto actionsNode = new AttributeTreeNode("actions");
			for (auto &a : actions)
			{
				auto n = new AttributeTreeNode("action");
				actionsNode->children.push_back(n);
				a.saveToAt(n);
			}
		}
		void maintain(int row) override
		{
			switch (row)
			{
			case RenderPassElement::eAction:
				maintainList(actions);
				break;
			case RenderPassElement::eColorAttachment:
				maintainList(colorAttachments);
				break;
			case RenderPassElement::eDepthStencilAttachment:
				if (depthStencilAttachment)
				{
					if (depthStencilAttachment->mark == Element::eMarkClear)
					{
						delete depthStencilAttachment;
						depthStencilAttachment = nullptr;
					}
				}
				break;
			case RenderPassElement::eDependency:
				maintainList(dependencies);
				break;
			}
		}
	};

	template <class RenderPassType>
	REFLECTABLE struct RendererAbstract : Container
	{
		REFL_BANK;

		REFLv std::string name;
		std::string filename;

		REFLv int cx = 0;
		REFLv int cy = 0;

		std::list<RenderPassType> passes;

		template <class... _Valty>
		RenderPassType *addPass(_Valty&&... _Val)
		{
			passes.emplace_back(_Val...);
			auto p = &passes.back();
			p->parent = this;
			return p;
		}
		void loadXML()
		{
			AttributeTree at("renderer");
			at.loadXML(filename);

			at.obtainFromAttributes(this, b);

			auto passesNode = at.firstNode("passes");
			if (passesNode)
			{
				for (auto c : passesNode->children)
				{
					if (c->name == "pass")
					{
						auto pass = addPass();
						pass->loadFromAt(c);
					}
				}
			}

			for (auto &p : passes)
			{
				for (auto &d : p.dependencies)
				{
					for (auto &pp : passes)
					{
						if (d.pass_name == pp.name)
						{
							d.target = &pp;
							break;
						}
					}
				}
			}
		}
		void saveXML()
		{
			for (auto &p : passes)
			{
				for (auto &d : p.dependencies)
				{
					if (d.target)
					{
						auto pp = (RenderPassType*)d.target;
						d.pass_name = pp->name;
					}
				}
			}

			AttributeTree at("renderer");

			at.addAttributes(this, b);

			auto passesNode = new AttributeTreeNode("passes");
			at.children.push_back(passesNode);
			for (auto &p : passes)
			{
				auto n = new AttributeTreeNode("pass");
				passesNode->children.push_back(n);
				p.saveToAt(n);
			}

			at.saveXML(filename);
		}
		void maintain(int row) override
		{
			maintainList(passes);
		}
	};
}

#endif