#ifndef __TKE_RENDER__
#define __TKE_RENDER__

#include <vector>
#include <string>
#include <memory>
#include <experimental/filesystem>

#include "utils.h"

namespace tke
{
	REFLECTABLE enum class StageFlags : int
	{
		REFLe vert,
		REFLe tesc,
		REFLe tese,
		REFLe geom,
		REFLe frag
	};

	REFLECTABLE enum class AspectFlags : int
	{
		REFLe color,
		REFLe depth,
		REFLe stencil,
		REFLe depth_stencil
	};

	REFLECTABLE enum class RenderPassType : int
	{
		REFLe draw_action,
		REFLe call_secondary_cmd
	};

	REFLECTABLE enum class DrawActionType : int
	{
		REFLe draw_action,
		REFLe call_fuction
	};

	REFLECTABLE enum class DrawcallType : int
	{
		REFLe vertex,
		REFLe index,
		REFLe indirect_vertex,
		REFLe indirect_index,
		REFLe push_constant
	};

	REFLECTABLE enum class PushConstantType : int
	{
		REFLe int_t,
		REFLe float_t,
		REFLe vec2_t,
		REFLe vec3_t,
		REFLe vec4_t
	};

	REFLECTABLE enum class PrimitiveTopology : int
	{
		REFLe triangle_list,
		REFLe line_list,
		REFLe line_strip,
		REFLe patch_list
	};

	REFLECTABLE enum class PolygonMode : int
	{
		REFLe fill,
		REFLe line
	};

	REFLECTABLE enum class CullMode : int
	{
		REFLe none,
		REFLe front,
		REFLe back,
		REFLe front_and_back
	};

	REFLECTABLE enum class BlendFactor : int
	{
		REFLe zero,
		REFLe one,
		REFLe src_alpha,
		REFLe one_minus_src_alpha
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
		REFLe uniform_buffer,
		REFLe storage_buffer,
		REFLe storage_image,
		REFLe sampler,
		REFLe input_attachment
	};

	REFLECTABLE struct Descriptor
	{
		REFL_BANK;

		REFLe DescriptorType type = DescriptorType::uniform_buffer;
		REFLv int binding = 0;
		REFLv int count = 0;
		REFLe StageFlags stage = StageFlags::vert;
	};

	REFLECTABLE struct PushConstantRange
	{
		REFL_BANK;

		REFLv int offset = 0;
		REFLv int size = 0;
		REFLe StageFlags stage = StageFlags::vert;
	};

	REFLECTABLE struct StageAbstract
	{
		REFL_BANK;

		REFLv std::string filename;
		std::string filepath;
		StageFlags type;
	};

	REFLECTABLE enum class SamplerType : int
	{
		REFLe none,
		REFLe screen,
		REFLe screen_uv,
		REFLe color
	};

	REFLECTABLE struct LinkResource
	{
		REFL_BANK;

		REFLe DescriptorType descriptorType;
		REFLv int binding = 0;
		REFLv int arrayElement = 0;
		REFLv std::string name;
		REFLe SamplerType sampler = SamplerType::none;
	};

	template <class StageType>
	REFLECTABLE struct PipelineAbstract
	{
		REFL_BANK;

		REFLv std::string name;
		REFLv std::string filename;
		REFLv std::string filepath;

		REFLv int m_cx;
		REFLv int m_cy;

		REFLv int patchControlPoints = 0;
		REFLv bool depthTest = false;
		REFLv bool depthWrite = false;
		REFLv bool depthClamp = false;

		REFLe PrimitiveTopology primitiveTopology = PrimitiveTopology::triangle_list;
		REFLe PolygonMode polygonMode = PolygonMode::fill;
		REFLe CullMode cullMode = CullMode::back;
		std::vector<BlendAttachment> blendAttachments;
		std::vector<Descriptor> descriptors;
		std::vector<PushConstantRange> pushConstantRanges;
		std::vector<StageType> stages;
		std::vector<LinkResource> links;

		inline void setFilename(const std::string &_filename)
		{
			filename = _filename;
			std::experimental::filesystem::path p(filename);
			filepath = p.parent_path().string();
		}
		inline void loadXML()
		{
			blendAttachments.clear();
			descriptors.clear();
			pushConstantRanges.clear();
			stages.clear();
			links.clear();

			AttributeTree at("pipeline");
			at.loadXML(filename);
			at.obtainFromAttributes(this, b);

			for (auto c : at.children)
			{
				if (c->name == "blend_attachment")
				{
					BlendAttachment ba;
					c->obtainFromAttributes(&ba, ba.b);
					blendAttachments.push_back(ba);
				}
				else if (c->name == "descriptor")
				{
					Descriptor d;
					c->obtainFromAttributes(&d, d.b);
					descriptors.push_back(d);
					for (auto cc : c->children)
					{
						if (cc->name == "link")
						{
							LinkResource lr;
							lr.descriptorType = d.type;
							lr.binding = d.binding;
							cc->obtainFromAttributes(&lr, lr.b);
							links.push_back(lr);
						}
					}
				}
				else if (c->name == "push_constant")
				{
					PushConstantRange pc;
					c->obtainFromAttributes(&pc, pc.b);
					pushConstantRanges.push_back(pc);
				}
				else if (c->name == "shader")
				{
					for (auto cc : c->children)
					{
						if (cc->name == "stage")
						{
							StageType s;
							cc->obtainFromAttributes(&s, s.b);
							std::experimental::filesystem::path p(s.filename);
							s.filepath = p.parent_path();
							auto ext = p.extension().string();
							if (ext == ".vert")
								s.type = StageFlags::vert;
							else if (ext == ".tesc")
								s.type = StageFlags::tesc;
							else if (ext == ".tese")
								s.type = StageFlags::tese;
							else if (ext == ".geom")
								s.type = StageFlags::geom;
							else if (ext == ".frag")
								s.type = StageFlags::frag;
							stages.push_back(s);
						}
					}
				}
			}
		}
		inline void saveXML()
		{
			AttributeTree at("pipeline");
			at.addAttributes(this, b);

			for (auto &b : blendAttachments)
			{
				auto n = new AttributeTreeNode("blend_attachment");
				n->addAttributes(&b, b.b);
				at.children.push_back(n);
			}
			for (auto &d : descriptors)
			{
				auto n = new AttributeTreeNode("descriptor");
				n->addAttributes(&d, d.b);
				at.children.push_back(n);
			}

			at.saveXML(filename);
		}
	};

	REFLECTABLE struct DrawcallAbstract
	{
		void *parent = nullptr;

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
		inline void setIndex(int _indexCount, int _firstIndex, int _vertexOffset)
		{
			index_count = _indexCount;
			first_index = _firstIndex;
			vertex_offset = _vertexOffset;
			type = DrawcallType::index;
		}
		inline void loadFromAt(AttributeTreeNode *n)
		{
			n->obtainFromAttributes(this, b);
		}
		inline void saveToAt(AttributeTreeNode *n)
		{
			n->addAttributes(this, b);
		}
	};

	template <class DrawcallType>
	REFLECTABLE struct DrawActionAbstract
	{
		void *parent = nullptr;

		REFL_BANK;

		REFLv std::string  name;

		REFLe DrawActionType type = DrawActionType::draw_action;

		REFLv std::string vertex_buffer_name;
		REFLv std::string index_buffer_name;
		REFLv std::string pipeline_name;

		REFLv std::string renderable_name;

		REFLv int cx = 0;
		REFLv int cy = 0;

		std::vector<std::shared_ptr<DrawcallType>> m_drawcalls;

		inline void addDrawcall(std::shared_ptr<DrawcallType> drawcall)
		{
			drawcall->parent = this;
			m_drawcalls.push_back(drawcall);
		}
		inline void removeDrawcall(DrawcallType *p)
		{
			auto it = std::find_if(m_drawcalls.begin(), m_drawcalls.end(), [&](std::shared_ptr<Drawcall> const& p) {
				return p.get() == drawcall;
			});
			if (it != m_drawcalls.end())
				m_drawcalls.erase(it);
		}
		inline int upDrawcall(DrawcallType *c)
		{
			for (int i = 0; i < m_drawcalls.size(); i++)
			{
				if (m_drawcalls[i].get() == c)
				{
					if (i > 0) std::swap(m_drawcalls[i], m_drawcalls[i - 1]);
					return i;
				}
			}
			return -1;
		}
		inline int downDrawcall(DrawcallType *c)
		{
			for (int i = 0; i < m_drawcalls.size(); i++)
			{
				if (m_drawcalls[i].get() == c)
				{
					if (i < m_drawcalls.size() - 1) std::swap(m_drawcalls[i], m_drawcalls[i + 1]);
					return i;
				}
			}
			return -1;
		}
		inline void clearDrawcalls()
		{
			m_drawcalls.clear();
		}
		inline void loadFromAt(AttributeTreeNode *n)
		{
			n->obtainFromAttributes(this, b);

			auto drawcallsNode = n->firstNode("drawcalls");
			if (drawcallsNode)
			{
				for (auto nn : drawcallsNode->children)
				{
					if (nn->name == "drawcall")
					{
						auto drawcall = std::make_shared<DrawcallType>();
						addDrawcall(drawcall);
						drawcall->loadFromAt(nn);
					}
				}
			}
		}
		inline void saveToAt(AttributeTreeNode *n)
		{
			n->addAttributes(this, b);

			auto drawcallsNode = new AttributeTreeNode("drawcalls");
			n->children.push_back(drawcallsNode);
			for (auto &d : m_drawcalls)
			{
				auto n = new AttributeTreeNode("drawcall");
				drawcallsNode->children.push_back(n);
				d->saveToAt(n);
			}
		}
	};

	REFLECTABLE struct AttachmentAbstract
	{
		void *parent;

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
		inline void loadFromAt(AttributeTreeNode *n)
		{
			n->obtainFromAttributes(this, b);
		}
		inline void saveToAt(AttributeTreeNode *n)
		{
			n->addAttributes(this, b);
		}
	};

	REFLECTABLE struct DependencyAbstract
	{
		void *parent;

		REFL_BANK;

		REFLv std::string pass_name;
		void *target = nullptr;
		inline void loadFromAt(AttributeTreeNode *n)
		{
			n->obtainFromAttributes(this, b);
		}
		inline void saveToAt(AttributeTreeNode *n)
		{
			n->addAttributes(this, b);
		}
	};

	template <class AttachmentType, class DependencyType, class DrawActionType>
	REFLECTABLE struct RenderPassAbstract
	{
		void *parent;

		std::vector<std::shared_ptr<AttachmentType>> colorAttachments;
		std::shared_ptr<AttachmentType> depthStencilAttachment = nullptr;

		std::vector<std::shared_ptr<DependencyType>> dependencies;

		std::vector<std::shared_ptr<DrawActionType>> actions;

		REFL_BANK;

		REFLe RenderPassType type = RenderPassType::draw_action;

		REFLv std::string name;

		REFLv std::string secondary_cmd_name;

		void addColorAttachment(std::shared_ptr<AttachmentType> p)
		{
			p->parent = this;
			colorAttachments.push_back(p);
		}
		void removeColorAttachment(AttachmentType *attachment)
		{
			auto it = std::find_if(colorAttachments.begin(), colorAttachments.end(), [&](std::shared_ptr<Attachment> const& p) {
				return p.get() == attachment;
			});
			if (it != colorAttachments.end())
				colorAttachments.erase(it);
		}
		int upColorAttachment(AttachmentType *a)
		{
			for (int i = 0; i < colorAttachments.size(); i++)
			{
				if (colorAttachments[i].get() == a)
				{
					if (i > 0) std::swap(colorAttachments[i], colorAttachments[i - 1]);
					return i;
				}
			}
			return -1;
		}
		int downColorAttachment(AttachmentType *a)
		{
			for (int i = 0; i < colorAttachments.size(); i++)
			{
				if (colorAttachments[i].get() == a)
				{
					if (i < colorAttachments.size() - 1) std::swap(colorAttachments[i], colorAttachments[i + 1]);
					return i;
				}
			}
			return -1;
		}
		void addDepthStencilAttachment(std::shared_ptr<AttachmentType> p)
		{
			p->parent = this;
			depthStencilAttachment = p;
		}
		void removeDepthStencilAttachment()
		{
			depthStencilAttachment.reset();
		}
		void addDependency(std::shared_ptr<DependencyType> p)
		{
			p->parent = this;
			dependencies.push_back(p);
		}
		void removeDependency(DependencyType *dependency)
		{
			auto it = std::find_if(dependencies.begin(), dependencies.end(), [&](std::shared_ptr<Dependency> const& p) {
				return p.get() == dependency;
			});
			if (it != dependencies.end())
				dependencies.erase(it);
		}
		int upDependency(DependencyType *d)
		{
			for (int i = 0; i < dependencies.size(); i++)
			{
				if (dependencies[i].get() == d)
				{
					if (i > 0) std::swap(dependencies[i], dependencies[i - 1]);
					return i;
				}
			}
			return -1;
		}
		int downDependency(DependencyType *d)
		{
			for (int i = 0; i < dependencies.size(); i++)
			{
				if (dependencies[i].get() == d)
				{
					if (i < dependencies.size() - 1) std::swap(dependencies[i], dependencies[i + 1]);
					return i;
				}
			}
			return -1;
		}
		void addAction(std::shared_ptr<DrawActionType> action)
		{
			action->parent = this;
			actions.push_back(action);
		}
		void removeAction(DrawActionType *action)
		{
			auto it = std::find_if(actions.begin(), actions.end(), [&](std::shared_ptr<DrawAction> const& p) {
				return p.get() == action;
			});
			if (it != actions.end())
				actions.erase(it);
		}
		int upAction(DrawActionType *a)
		{
			for (int i = 0; i < actions.size(); i++)
			{
				if (actions[i].get() == a)
				{
					if (i > 0) std::swap(actions[i], actions[i - 1]);
					return i;
				}
			}
			return -1;
		}
		int downAction(DrawActionType *a)
		{
			for (int i = 0; i < actions.size(); i++)
			{
				if (actions[i].get() == a)
				{
					if (i < actions.size() - 1) std::swap(actions[i], actions[i + 1]);
					return i;
				}
			}
			return -1;
		}
		inline void loadFromAt(AttributeTreeNode *n)
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
							auto attachment = std::make_shared<AttachmentType>();
							addColorAttachment(attachment);
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
						depthStencilAttachment = std::make_shared<AttachmentType>();
						addDepthStencilAttachment(depthStencilAttachment);
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
						auto dependency = std::make_shared<DependencyType>();
						addDependency(dependency);
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
						auto action = std::make_shared<DrawActionType>();
						addAction(action);
						action->loadFromAt(n);
					}
				}
			}
		}
		inline void saveToAt(AttributeTreeNode *n)
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
				c->saveToAt(n);
			}
			auto depthStencilNode = new AttributeTreeNode("depth_stencil");
			attachmentNode->children.push_back(depthStencilNode);
			if (depthStencilAttachment.get())
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
				d->saveToAt(n);
			}
			auto actionsNode = new AttributeTreeNode("actions");
			for (auto &a : actions)
			{
				auto n = new AttributeTreeNode("action");
				actionsNode->children.push_back(n);
				a->saveToAt(n);
			}
		}
	};

	template <class RenderPassType>
	REFLECTABLE struct RendererAbstract
	{
		REFL_BANK;

		REFLv int cx = 0;
		REFLv int cy = 0;

		REFLv std::string name;
		REFLv std::string filename;

		std::vector<std::shared_ptr<RenderPassType>> passes;

		void addPass(std::shared_ptr<RenderPassType> p)
		{
			p->parent = this;
			passes.push_back(p);
		}
		void removePass(RenderPassType *pass)
		{
			auto it = std::find_if(passes.begin(), passes.end(), [&](std::shared_ptr<RenderPass> const& p) {
				return p.get() == pass;
			});
			if (it != passes.end())
				passes.erase(it);
		}
		int upPass(RenderPassType *p)
		{
			for (int i = 0; i < passes.size(); i++)
			{
				if (passes[i].get() == p)
				{
					if (i > 0) std::swap(passes[i], passes[i - 1]);
					return i;
				}
			}
			return -1;
		}
		int downPass(RenderPassType *p)
		{
			for (int i = 0; i < passes.size(); i++)
			{
				if (passes[i].get() == p)
				{
					if (i < passes.size() - 1) std::swap(passes[i], passes[i + 1]);
					return i;
				}
			}
			return -1;
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
						auto pass = std::make_shared<RenderPassType>();
						addPass(pass);
						pass->loadFromAt(c);
					}
				}
			}

			for (auto p : passes)
			{
				for (auto d : p->dependencies)
				{
					for (auto &pp : passes)
					{
						if (d->pass_name == pp->name)
						{
							d->target = pp.get();
							break;
						}
					}
				}
			}
		}
		void saveXML()
		{
			for (auto p : passes)
			{
				for (auto d : p->dependencies)
				{
					if (d->target)
					{
						auto pp = (RenderPassType)d->target;
						d->pass_name = pp->name;
					}
				}
			}
			n->children.push_back(new AttributeTreeNode(de->target->name));

			AttributeTree at("renderer");

			at.addAttributes(this, b);

			auto passesNode = new AttributeTreeNode("passes");
			at.children.push_back(passesNode);
			for (auto &p : passes)
			{
				auto n = new AttributeTreeNode("pass");
				passesNode->children.push_back(n);
				p->saveToAt(n);
			}

			at.saveXML(filename);
		}
	};
}

#endif