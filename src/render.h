#ifndef __TKE_RENDER__
#define __TKE_RENDER__

#include <vector>
#include <list>
#include <string>
#include <memory>
#include <experimental/filesystem>

#include "vk.h"
#include "resource.h"

namespace tke
{
	struct Buffer
	{
		size_t m_size = 0;
		VkBuffer m_buffer = 0;
		VkDeviceMemory m_memory = 0;

		void create(size_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryProperty = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		void destory();
	};

	struct StagingBuffer : Buffer
	{
		void create(size_t size);
	};

	struct NonStagingBufferAbstract : Buffer
	{
		void create(size_t size, VkBufferUsageFlags usage, void *data = nullptr);
		void update(void *data, StagingBuffer *pStagingBuffer, size_t size = 0);
	};

	struct ShaderManipulatableBufferAbstract : NonStagingBufferAbstract
	{
		VkDescriptorBufferInfo m_info;
		void create(size_t size, VkBufferUsageFlags usage);
	};

	struct UniformBuffer : ShaderManipulatableBufferAbstract
	{
		void create(size_t size);
	};

	struct VertexBuffer : NonStagingBufferAbstract
	{
		void create(size_t size, void *data = nullptr);
		void bind(VkCommandBuffer cmd);
	};

	struct IndexBuffer : NonStagingBufferAbstract
	{
		void create(size_t size, void *data = nullptr);
		void bind(VkCommandBuffer cmd);
	};

	struct VertexIndirectBuffer : NonStagingBufferAbstract
	{
		size_t stride();
		void create(size_t size);
	};

	struct IndexedIndirectBuffer : NonStagingBufferAbstract
	{
		size_t stride();
		void create(size_t size);
	};


	REFLECTABLE enum class ImageFormat : int
	{
		null,
		REFLe R8G8B8A8 = 1 << 0,
		REFLe R16G16B16A16 = 1 << 1
	};

	inline VkFormat vkFormat(ImageFormat f)
	{
		switch (f)
		{
		case ImageFormat::R8G8B8A8:
			return VK_FORMAT_R8G8B8A8_UNORM;
		case ImageFormat::R16G16B16A16:
			return VK_FORMAT_R16G16B16A16_SFLOAT;
		}
		return VK_FORMAT_UNDEFINED;
	}

	struct Image
	{
		enum Type
		{
			eColor,
			eSwapchain,
			eDepth,
			eDepthStencil
		};
		Type type = eColor;
		inline bool isColorType() { return type == eColor || type == eSwapchain; }
		inline bool isDepthStencilType() { return type == eDepth || type == eDepthStencil; }

		struct View
		{
			VkImageAspectFlags aspect;
			int baseLevel;
			int levelCount;
			int baseLayer;
			int layerCount;
			VkImageView view;
		};

		size_t m_size;
		int m_width = 1, m_height = 1;
		int m_mipmapLevels = 1;
		int m_arrayLayers = 1;
		VkFormat m_format = VK_FORMAT_R8G8B8A8_UNORM;
		VkImage m_image = 0;
		VkDeviceMemory m_memory = 0;
		VkImageLayout m_layout = VK_IMAGE_LAYOUT_PREINITIALIZED;
		VkImageViewType m_viewType = VK_IMAGE_VIEW_TYPE_2D;

		std::vector<View> views;
		std::vector<VkDescriptorImageInfo*> infos;

		std::string filename;

		bool m_sRGB = false;

		int sceneIndex = -1;

		unsigned char *m_data = nullptr;

		~Image();
		int getWidth(int mipmapLevel = 0) const;
		int getHeight(int mipmapLevel = 0) const;
		void transitionLayout(int level, VkImageAspectFlags aspect, VkImageLayout layout);
		void fillData(int level, std::uint8_t *data, size_t size, VkImageAspectFlags aspect);
		void create(int w, int h, VkFormat format, VkImageUsageFlags usage, std::uint8_t *data = nullptr, size_t size = 0, VkImageAspectFlags aspect = 0);
		VkImageView getView(VkImageAspectFlags aspect = 0, int baseLevel = 0, int levelCount = 1, int baseLayer = 0, int layerCount = 1);
		VkDescriptorImageInfo *getInfo(VkSampler sampler, VkImageAspectFlags aspect = 0, int baseLevel = 0, int levelCount = 1, int baseLayer = 0, int layerCount = 1);
		unsigned char getPixel(int x, int y, int off) const;
	};

	REFLECTABLE enum class StageType : int
	{
		null,
		REFLe vert = 1 << 0,
		REFLe tesc = 1 << 1,
		REFLe tese = 1 << 2,
		REFLe geom = 1 << 3,
		REFLe frag = 1 << 4
	};

	const int StageTypes[] = {
		(int)StageType::vert,
		(int)StageType::tesc,
		(int)StageType::tese,
		(int)StageType::geom,
		(int)StageType::frag
	};

	const std::string StageNames[] = {
		"vert",
		"tesc",
		"tese",
		"geom",
		"frag"
	};

	inline int StageIndexByType(StageType t)
	{
		for (int i = 0; i < 5; i++)
		{
			if (StageTypes[i] == (int)t)
				return i;
		}
		return -1;
	}

	inline const std::string StageNameByType(StageType t)
	{
		return StageNames[StageIndexByType(t)];
	}

	inline StageType StageFlagByExt(const std::string &ext)
	{
		if (ext == ".vert") return StageType::vert;
		if (ext == ".tesc") return StageType::tesc;
		if (ext == ".tese") return StageType::tese;
		if (ext == ".geom") return StageType::geom;
		if (ext == ".frag") return StageType::frag;
		return StageType::null;
	}

	inline VkShaderStageFlags vkStage(StageType f)
	{
		VkShaderStageFlags v = 0;
		if ((int)f & (int)StageType::vert) v |= VK_SHADER_STAGE_VERTEX_BIT;
		if ((int)f & (int)StageType::tesc) v |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		if ((int)f & (int)StageType::tese) v |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		if ((int)f & (int)StageType::geom) v |= VK_SHADER_STAGE_GEOMETRY_BIT;
		if ((int)f & (int)StageType::frag) v |= VK_SHADER_STAGE_FRAGMENT_BIT;
		return v;
	}

	REFLECTABLE enum class AspectFlags : int
	{
		null,
		REFLe color = 1 << 0,
		REFLe depth = 1 << 1,
		REFLe stencil = 1 << 2,
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

	REFLECTABLE struct PushConstantRange
	{
		REFL_BANK;

		REFLv int offset = 0;
		REFLv int size = 0;
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
		REFLe image_n_sampler = 1 << 3,
		REFLe input_attachment = 1 << 4
	};

	REFLECTABLE struct Descriptor
	{
		REFL_BANK;

		REFLe DescriptorType type = DescriptorType::uniform_buffer;
		REFLv int binding = 0;
		REFLv int count = 0;
		REFLv std::string name;
	};

	struct Pipeline;

	REFLECTABLE struct Stage
	{
		REFL_BANK;

		REFLv std::string filename;
		std::string filepath;
		StageType type;

		std::vector<Descriptor> descriptors;
		std::vector<PushConstantRange> pushConstantRanges;

		Pipeline *parent;
		ExtType *ext = nullptr;
		Stage(Pipeline *_parent);
		~Stage();
	};

	REFLECTABLE enum class SamplerType : int
	{
		null,
		REFLe none = 1 << 0,
		REFLe plain = 1 << 1,
		REFLe plain_unnormalized = 1 << 2,
		REFLe color = 1 << 3,
		REFLe color_border = 1 << 4
	};

	REFLECTABLE struct LinkResource
	{
		REFL_BANK;

		REFLv int binding = -1;
		REFLv int array_element = 0;
		REFLv std::string descriptor_name;
		REFLv std::string resource_name;
		REFLe SamplerType sampler = SamplerType::none;
	};

	REFLECTABLE enum class VertexInputType : int
	{
		null,
		REFLe zero = 1 << 0,
		REFLe normal = 1 << 1
	};

	REFLECTABLE struct Pipeline
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
		Stage *stages[5] = {};
		std::vector<LinkResource> links;

		ResourceBank *pResource = &globalResource;

		VkPrimitiveTopology vkPrimitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		VkPolygonMode vkPolygonMode = VK_POLYGON_MODE_FILL;
		VkCullModeFlagBits vkCullMode = VK_CULL_MODE_BACK_BIT;
		std::vector<VkPipelineColorBlendAttachmentState> vkBlendAttachments;
		std::vector<VkDescriptorSetLayoutBinding> vkDescriptors;
		std::vector<VkPushConstantRange> vkPushConstantRanges;
		std::vector<VkPipelineShaderStageCreateInfo> vkStages;

		VkPipelineVertexInputStateCreateInfo *m_pVertexInputState;
		VkRenderPass m_renderPass;
		int m_subpassIndex;
		std::vector<VkDynamicState> m_dynamics;

		VkDescriptorSetLayout m_descriptorSetLayout = 0;
		VkPipelineLayout m_pipelineLayout = 0;
		VkPipeline m_pipeline = 0;
		VkDescriptorSet m_descriptorSet = 0;

		ExtType *ext = nullptr;

		inline bool isFullOfStage()
		{
			for (int i = 0; i < 5; i++)
			{
				if (stages[i])
					return false;
			}
			return true;
		}

		~Pipeline();
		void loadXML(const std::string &_filename);
		void saveXML();
		int descriptorPosition(const std::string &name);
		void create(VkPipelineVertexInputStateCreateInfo *pVertexInputState, VkRenderPass renderPass, std::uint32_t subpassIndex);
		void create(const std::string &filename, VkPipelineVertexInputStateCreateInfo *pVertexInputState, VkRenderPass renderPass, std::uint32_t subpassIndex);
	};

	struct DescriptorSetLayout
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings;
		VkDescriptorSetLayout layout;
	};

	VkDescriptorSetLayout getDescriptorSetLayout(std::vector<VkDescriptorSetLayoutBinding> &bindings);

	struct PipelineLayout
	{
		VkDescriptorSetLayout descriptorLayout;
		std::vector<VkPushConstantRange> pushConstantRanges;
		VkPipelineLayout layout;
	};

	VkPipelineLayout getPipelineLayout(VkDescriptorSetLayout descriptorLayout, std::vector<VkPushConstantRange> &pushConstantRanges);

	struct Framebuffer
	{
		std::vector<VkImageView> views;
		VkFramebuffer framebuffer;
	};

	VkFramebuffer getFramebuffer(int cx, int cy, VkRenderPass renderPass, std::vector<VkImageView> &views);

	struct ShaderModule
	{
		std::string filename;
		VkShaderModule module;
	};

	VkShaderModule getShaderModule(const std::string &filename);

	typedef void (*PF_RenderFunc)(VkCommandBuffer);

	struct Model;
	REFLECTABLE struct Drawcall : Element
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

		REFLv std::string model_name;

		REFLv StageType push_constant_stage = StageType::null;
		REFLv int push_constant_offset = 0;
		REFLe PushConstantType push_constant_type = PushConstantType::int_t;
		void *push_constant_value = nullptr;

		VertexIndirectBuffer *m_vertexIndirectBuffer = nullptr;
		IndexedIndirectBuffer *m_indexedIndirectBuffer = nullptr;
		VkShaderStageFlags m_pushConstantStage;
		size_t m_pushConstantSize;

		Drawcall();
		Drawcall(int vertexCount, int firstVertex, int instanceCount, int firstInstance);
		Drawcall(int indexCount, int firstIndex, int vertexOffset, int instanceCount, int firstInstance);
		Drawcall(Model *p, int instanceCount = 1, int firstInstance = 0);
		Drawcall(VertexIndirectBuffer *vertexIndirectBuffer, uint32_t firstIndirect = 0, uint32_t indirectCount = 0);
		Drawcall(IndexedIndirectBuffer *m_indexedIndirectBuffer, uint32_t firstIndirect = 0, uint32_t indirectCount = 0);
		Drawcall(VkShaderStageFlags stage, void *data, size_t size, size_t offset = 0);
		~Drawcall();
		void loadFromAt(AttributeTreeNode *n);
		void saveToAt(AttributeTreeNode *n);
	};

	REFLECTABLE struct DrawAction : Element, Container
	{
		Container *parent = nullptr;

		REFL_BANK;

		REFLv std::string  name;

		REFLe DrawActionType type = DrawActionType::draw_action;

		REFLv std::string vertex_buffer_name;
		REFLv std::string index_buffer_name;
		REFLv std::string pipeline_name;

		REFLv std::string render_func_name;

		REFLv int cx = 0;
		REFLv int cy = 0;

		std::list<Drawcall> drawcalls;

		VertexBuffer *m_vertexBuffer = nullptr;
		IndexBuffer *m_indexBuffer = nullptr;
		Pipeline *m_pipeline = nullptr;

		VkDescriptorSet m_descriptorSet = 0;

		PF_RenderFunc m_pRenderFunc = nullptr;

		bool show = true;

		Drawcall *findDrawcall(const std::string &n);

		DrawAction();
		DrawAction(Pipeline *pipeline);
		DrawAction(PF_RenderFunc pRenderFunc);
		template <class... _Valty>
		Drawcall *addDrawcall(_Valty&&... _Val)
		{
			drawcalls.emplace_back(_Val...);
			auto d = &drawcalls.back();
			d->parent = this;
			return d;
		}
		void loadFromAt(AttributeTreeNode *n);
		void saveToAt(AttributeTreeNode *n);
		void maintain(int row) override;
		void preprocess(Pipeline* &currentPipeline);
	};

	REFLECTABLE struct Attachment : Element
	{
		Container *parent;

		REFL_BANK;

		REFLv std::string name;

		REFLv std::string image_name;
		REFLv int level = 0;
		REFLv int layer = 0;

		REFLe AspectFlags aspect = AspectFlags::null;

		REFLv bool clear = false;
		REFLv float clear_r = 0.f;
		REFLv float clear_g = 0.f;
		REFLv float clear_b = 0.f;
		REFLv float clear_a = 0.f;
		REFLv float clear_depth = 1.f;
		REFLv int clear_stencil = 0;

		Image *image = nullptr;

		VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		VkClearValue clearValue = {};

		int index;

		Attachment();
		Attachment(Image *image, VkClearValue _clearValue = {9999.f});
		void loadFromAt(AttributeTreeNode *n);
		void saveToAt(AttributeTreeNode *n);
	};

	REFLECTABLE struct Dependency : Element
	{
		Container *parent;

		REFL_BANK;

		REFLv std::string pass_name;
		void *target = nullptr;

		Dependency();
		Dependency(void *_target);
		Dependency(const std::string &_pass_name);
		void loadFromAt(AttributeTreeNode *n);
		void saveToAt(AttributeTreeNode *n);
	};

	enum class RenderPassElement
	{
		eAction,
		eAttachment,
		eDependency
	};

	REFLECTABLE struct RenderPass : Element, Container
	{
		Container *parent;

		std::list<Attachment> attachments;

		std::list<Dependency> dependencies;

		std::list<DrawAction> actions;

		REFL_BANK;

		REFLe RenderPassType type = RenderPassType::draw_action;

		REFLv std::string name;

		REFLv std::string secondary_cmd_name;

		VkCommandBuffer secondaryCmd = 0;

		int index = 0;

		Attachment *findAttachment(const std::string &n);
		DrawAction *findAction(const std::string &n);

		RenderPass();
		RenderPass(VkCommandBuffer cmd);
		template <class... _Valty>
		Attachment *addAttachment(_Valty&&... _Val)
		{
			attachments.emplace_back(_Val...);
			auto p = &attachments.back();
			p->parent = this;
			return p;
		}
		template <class... _Valty>
		Dependency *addDependency(_Valty&&... _Val)
		{
			dependencies.emplace_back(_Val...);
			auto p = &dependencies.back();
			p->parent = this;
			return p;
		}
		template <class... _Valty>
		DrawAction *addAction(_Valty&&... _Val)
		{
			actions.emplace_back(_Val...);
			auto p = &actions.back();
			p->parent = this;
			return p;
		}
		void loadFromAt(AttributeTreeNode *n);
		void saveToAt(AttributeTreeNode *n);
		void maintain(int row) override;
	};

	REFLECTABLE struct BufferResource
	{
		REFL_BANK;

		REFLv std::string name;
		REFLv int size = 0;

		UniformBuffer *p = nullptr;
	};

	REFLECTABLE struct ImageResource
	{
		REFL_BANK;

		REFLv std::string name;
		REFLv std::string file_name;
		REFLv bool sRGB = true;
		REFLv int cx = 0;
		REFLv int cy = 0;
		REFLe ImageFormat format = ImageFormat::null;

		Image *p = nullptr;
	};

	REFLECTABLE struct PipelineResource
	{
		REFL_BANK;

		REFLv std::string name;
		REFLv std::string file_name;
		REFLe VertexInputType vertex_input_type = VertexInputType::zero;

		Pipeline *p = nullptr;
		int subpassIndex = -1;
	};

	REFLECTABLE struct Renderer : Container
	{
		REFL_BANK;

		REFLv std::string name;
		std::string filename;

		REFLv int cx = 0;
		REFLv int cy = 0;

		REFLv std::string vertex_buffer_name;
		REFLv std::string index_buffer_name;

		std::list<RenderPass> passes;

		std::vector<BufferResource> bufferResources;
		std::vector<ImageResource> imageResources;
		std::vector<PipelineResource> pipelineResources;

		bool containSwapchain = false;

		ResourceBank resource;

		VertexBuffer *initVertexBuffer = nullptr;
		IndexBuffer *initIndexBuffer = nullptr;
		Pipeline *initPipeline = nullptr;
		VkDescriptorSet initDescriptorSet = 0;

		VertexBuffer *currentVertexBuffer = nullptr;
		IndexBuffer *currentIndexBuffer = nullptr;
		Pipeline *currentPipeline = nullptr;
		VkDescriptorSet currentDescriptorSet = 0;

		std::vector<VkAttachmentDescription> vkAttachments;
		std::vector<VkImageView> vkViews[2];
		std::vector<VkClearValue> vkClearValues;

		VkRenderPass vkRenderPass = 0;

		VkFramebuffer vkFramebuffer[2] = {};

		RenderPass *findRenderPass(const std::string &n);

		ExtType *ext = nullptr;

		Renderer();
		Renderer(int _cx, int _cy);
		~Renderer();
		template <class... _Valty>
		RenderPass *addPass(_Valty&&... _Val)
		{
			passes.emplace_back(_Val...);
			auto p = &passes.back();
			p->parent = this;
			return p;
		}
		void loadXML(const std::string &_filename);
		void saveXML();
		void maintain(int row) override;
		void pushImage(Attachment *ai);
		void getDescriptorSets();
		void setup();
		void execute(VkCommandBuffer cmd, int index = 0);
	};
}

#endif