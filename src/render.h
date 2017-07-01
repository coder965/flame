#pragma once

#include <vector>
#include <list>
#include <string>
#include <map>
#include <memory>
#include <experimental/filesystem>

#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan\vulkan.h>

#include "utils.h"

namespace tke
{
	REFLECTABLE enum class StageType : int
	{
		null,
		REFLe vert = 1 << 0,
		REFLe tesc = 1 << 1,
		REFLe tese = 1 << 2,
		REFLe geom = 1 << 3,
		REFLe frag = 1 << 4
	};

	struct VertexBuffer;
	struct IndexBuffer;
	struct IndirectVertexBuffer;
	struct IndirectIndexBuffer;
	struct Framebuffer;
	struct Pipeline;
	struct Drawcall;
	struct Dependency;
	struct Attachment;
	struct DrawAction;
	struct RenderPass;
	struct Renderer;

	struct Instance
	{
		VkInstance v;
		CriticalSection cs;
	};

	struct Device
	{
		VkDevice v;
		CriticalSection cs;

		void waitIdle();
	};

	struct Queue
	{
		VkQueue v;
		CriticalSection cs;

		void waitIdle();
		void submit(int count, VkCommandBuffer *cmds, VkSemaphore waitSemaphore = 0, VkSemaphore signalSemaphore = 0, VkFence fence = 0);
	};

	extern Instance inst;
	extern VkPhysicalDevice physicalDevice;
	extern VkPhysicalDeviceProperties physicalDeviceProperties;
	extern Device device;
	extern Queue graphicsQueue;

	struct Buffer
	{
		size_t size = 0;
		VkBuffer buffer = 0;
		VkDeviceMemory memory = 0;

		VkBufferUsageFlags usage;
		VkMemoryPropertyFlags memoryProperty;

		Buffer(size_t _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _memoryProperty = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		~Buffer();
		void recreate(size_t _size);
	};

	struct StagingBuffer : Buffer
	{
		StagingBuffer(size_t _size);
		void *map(size_t offset, size_t _size);
		void unmap();
	};

	struct NonStagingBufferAbstract : Buffer
	{
		NonStagingBufferAbstract(size_t _size, VkBufferUsageFlags usage, void *data = nullptr);
		void recreate(size_t size, void *data = nullptr);
		void update(void *data, StagingBuffer &stagingBuffer, size_t size = 0);
	};

	struct ShaderManipulatableBufferAbstract : NonStagingBufferAbstract
	{
		VkDescriptorBufferInfo m_info;

		ShaderManipulatableBufferAbstract(size_t _size, VkBufferUsageFlags usage);
	};

	struct UniformBuffer : ShaderManipulatableBufferAbstract
	{
		UniformBuffer(size_t _size);
	};

	struct VertexBuffer : NonStagingBufferAbstract
	{
		VertexBuffer(size_t _size = 16, void *data = nullptr);
	};

	struct IndexBuffer : NonStagingBufferAbstract
	{
		IndexBuffer(size_t _size = 16, void *data = nullptr);
	};

	struct IndirectVertexBuffer : NonStagingBufferAbstract
	{
		IndirectVertexBuffer(size_t _size = sizeof VkDrawIndirectCommand);
	};

	struct IndirectIndexBuffer : NonStagingBufferAbstract
	{
		IndirectIndexBuffer(size_t _size = sizeof VkDrawIndexedIndirectCommand);
	};

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

		size_t size;
		int width = 1, height = 1;
		int level = 1;
		int layer = 1;
		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
		VkImage image = 0;
		VkDeviceMemory memory = 0;
		VkImageLayout layout = VK_IMAGE_LAYOUT_PREINITIALIZED;
		VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;

		std::vector<View> views;
		std::list<VkDescriptorImageInfo> infos;

		std::string filename;

		bool sRGB = false;

		int index = -1;

		Image(int w, int h, VkFormat _format, VkImageUsageFlags usage, int _level = 1, void *data = nullptr, size_t _size = 0, VkImageAspectFlags aspect = 0);
		Image(Type _type, VkImage _image, int w, int h, VkFormat _format);
		~Image();
		int getWidth(int _level = 0) const;
		int getHeight(int _level = 0) const;
		void transitionLayout(int _level, VkImageAspectFlags aspect, VkImageLayout _layout);
		void fillData(int _level, void *data, size_t _size, VkImageAspectFlags aspect);
		VkImageView getView(VkImageAspectFlags aspect = 0, int baseLevel = 0, int levelCount = 1, int baseLayer = 0, int layerCount = 1);
		VkDescriptorImageInfo *getInfo(VkSampler sampler, VkImageAspectFlags aspect = 0, int baseLevel = 0, int levelCount = 1, int baseLayer = 0, int layerCount = 1);
	};

	Image *createImage(const std::string &filename, bool sRGB, bool saveData = false);

	struct CommandPool;

	struct CommandBuffer
	{
		CommandPool *pool;
		VkCommandBuffer v;
		Pipeline *currentPipeline = nullptr;

		CommandBuffer(CommandPool *_pool, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		~CommandBuffer();
		void reset();
		void begin(VkCommandBufferUsageFlags flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, VkCommandBufferInheritanceInfo *pInheritance = nullptr);
		void end();
		void beginRenderPass(VkRenderPass renderPass, Framebuffer *fb, int clearValueCount = 0, VkClearValue *pClearValues = nullptr);
		void nextSubpass(VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);
		void endRenderPass();
		void setViewportAndScissor(int cx, int cy);
		void setScissor(int x, int y, int cx, int cy);
		void bindVertexBuffer(VertexBuffer *b);
		void bindIndexBuffer(IndexBuffer *b);
		void bindPipeline(Pipeline *p);
		void bindDescriptorSet();
		void bindDescriptorSet(VkDescriptorSet set);
		void execSecondaryCmd(VkCommandBuffer cmd);
		void pushConstant(StageType stage, int offset, int size, void *src);
		void draw(int vertexCount, int firstVertex = 0, int instanceCount = 1, int firstInstance = 0);
		void drawIndex(int indexCount, int firstIndex = 0, int vertexOffset = 0, int instanceCount = 1, int firstInstance = 0);
		void drawIndirect(IndirectVertexBuffer *b, int count, int offset = 0);
		void drawIndirectIndex(IndirectIndexBuffer *b, int count, int offset = 0);
		void waitEvents(size_t count, VkEvent *e);
		void setEvent(VkEvent e);
		void resetEvent(VkEvent e);
	};

	struct CommandPool
	{
		VkCommandPool v;

		CommandPool();
		~CommandPool();
		CommandBuffer *begineOnce();
		void endOnce(CommandBuffer *cb);
		void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, size_t srcOffset = 0, size_t dstOffset = 0);
		void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, size_t count, VkBufferCopy *ranges);
		void updateBuffer(void *data, size_t size, StagingBuffer &stagingBuffer, VkBuffer &uniformBuffer);
		void copyImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height);
	};
	extern CommandPool *commandPool;

	struct DescriptorPool
	{
		VkDescriptorPool v;
		std::vector<VkWriteDescriptorSet> writes;

		DescriptorPool();
		~DescriptorPool();
		VkDescriptorSet allocate(VkDescriptorSetLayout *pLayout);
		void free(VkDescriptorSet set);
		void addWrite(VkDescriptorSet descriptorSet, VkDescriptorType type, uint32_t binding, VkDescriptorImageInfo *pImageInfo, uint32_t dstArrayElement = 0);
		void addWrite(VkDescriptorSet descriptorSet, VkDescriptorType type, uint32_t binding, VkDescriptorBufferInfo *pBufferInfo, uint32_t dstArrayElement = 0);
		void update();
	};
	extern DescriptorPool *descriptorPool;

	struct Framebuffer
	{
		int cx, cy;
		std::vector<VkImageView> views;
		VkFramebuffer v;
		int refCount = 1;

		~Framebuffer();
	};

	Framebuffer *getFramebuffer(Image *i, VkRenderPass renderPass, int level = 0);

	Framebuffer *getFramebuffer(int cx, int cy, VkRenderPass renderPass, std::vector<VkImageView> &views);

	void releaseFramebuffer(Framebuffer *f);

	extern VkFormat swapchainFormat;
	extern VkSampler plainSampler;
	extern VkSampler plainUnnormalizedSampler;
	extern VkSampler colorSampler;
	extern VkSampler colorBorderSampler;

	VkFence createFence();
	void destroyFence(VkFence fence);

	VkEvent createEvent();
	void destroyEvent(VkEvent event);

	VkSemaphore createSemaphore();
	void destroySemaphore(VkSemaphore semaphore);

	void waitFence(VkFence);

	VkPipelineVertexInputStateCreateInfo vertexStateInfo(std::uint32_t bindingCount, VkVertexInputBindingDescription *pBindings, std::uint32_t attributeCount, VkVertexInputAttributeDescription *pAttributes);

	VkAttachmentDescription colorAttachmentDesc(VkFormat format, VkAttachmentLoadOp loadOp);
	VkAttachmentDescription depthAttachmentDesc(VkFormat format, VkAttachmentLoadOp loadOp);
	VkAttachmentDescription swapchainAttachmentDesc(VkAttachmentLoadOp loadOp);
	VkSubpassDescription subpassDesc(int colorCount, VkAttachmentReference *pColors, VkAttachmentReference *pDepth = nullptr, int inputCount = 0, VkAttachmentReference *pInputs = nullptr);
	VkSubpassDependency subpassDependency(int srcSubpass, int dstSubpass);
	VkRenderPass createRenderPass(std::uint32_t attachmentCount, VkAttachmentDescription *pAttachments, std::uint32_t subpassCount, VkSubpassDescription *pSubpasses, std::uint32_t dependencyCount, VkSubpassDependency *pDependencies);
	void destroyRenderPass(VkRenderPass rp);

	Err initRender(bool debug);

	REFLECTABLE enum class Format : int
	{
		null,
		REFLe R8G8B8A8 = 1 << 0,
		REFLe R16G16B16A16 = 1 << 1,
		REFLe R32G32B32A32 = 1 << 2
	};

	inline VkFormat vkFormat(Format f)
	{
		switch (f)
		{
		case Format::R8G8B8A8:
			return VK_FORMAT_R8G8B8A8_UNORM;
		case Format::R16G16B16A16:
			return VK_FORMAT_R16G16B16A16_SFLOAT;
		case Format::R32G32B32A32:
			return VK_FORMAT_R32G32B32A32_SFLOAT;
		}
		return VK_FORMAT_UNDEFINED;
	}

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

	REFLECTABLE struct UniformBufferInfo
	{
		REFL_BANK;

		REFLv std::string name;
		REFLv int size = 0;

		UniformBuffer *p = nullptr;
	};

	REFLECTABLE struct ImageInfo
	{
		REFL_BANK;

		REFLv std::string name;
		REFLv std::string file_name;
		REFLv bool sRGB = true;
		REFLv int cx = 0;
		REFLv int cy = 0;
		REFLe Format format = Format::null;

		Image *p = nullptr;
	};

	REFLECTABLE enum class VertexInputType : int
	{
		null,
		REFLe zero = 1 << 0,
		REFLe plain2d = 1 << 1,
		REFLe normal = 1 << 2,
		REFLe animated = 1 << 3,
		REFLe line = 1 << 4
	};

	REFLECTABLE enum class DynamicStateType : int
	{
		null,
		REFLe viewport = 1 << 0,
		REFLe scissor = 1 << 1
	};

	REFLECTABLE struct DynamicState
	{
		REFL_BANK;

		REFLe DynamicStateType type = DynamicStateType::null;
	};

	REFLECTABLE struct PipelineInfo
	{
		REFL_BANK;

		REFLv std::string file_name;

		Pipeline *p = nullptr;
		int subpassIndex = -1;
	};

	REFLECTABLE struct ShaderMacro
	{
		REFL_BANK;

		REFLv std::string pipeline_name;
		REFLe StageType stage = StageType::null;
		REFLv std::string value;
	};

	struct Model;
	struct ResourceBank
	{
		ResourceBank *parent;

		std::vector<UniformBufferInfo> privateBuffers;
		std::vector<ImageInfo> privateImages;
		std::vector<PipelineInfo> privatePipelines;
		std::vector<ShaderMacro> shaderMacros;

		std::map<std::string, Buffer*> bufferResources;
		std::map<std::string, Image*> imageResources;
		std::map<std::string, Model*> modelResources;
		std::map<std::string, Pipeline*> pipelineResources;
		std::map<std::string, VkCommandBuffer> cmdResources;
		std::map<std::string, int*> intResources;

		void setBuffer(Buffer *p, const std::string &str);
		void setImage(Image *p, const std::string &str);
		void setModel(Model *p, const std::string &str);
		void setPipeline(Pipeline *p);
		void setCmd(VkCommandBuffer p, const std::string &str);
		void setInt(int *p, const std::string &str);

		Buffer *getBuffer(const std::string &str);
		Image *getImage(const std::string &str);
		Model *getModel(const std::string &str);
		Pipeline *getPipeline(const std::string &str);
		VkCommandBuffer getCmd(const std::string &str);
		int *getInt(const std::string &str);

		ResourceBank(ResourceBank *_parent);
		~ResourceBank();
	};

	extern ResourceBank globalResource;

	struct ShaderModule
	{
		std::string filename;
		std::vector<std::string> defines;
		VkShaderModule v;
		int refCount = 1;

		~ShaderModule();
	};

	REFLECTABLE struct Stage
	{
		REFL_BANK;

		REFLv std::string filename;
		std::string filepath;
		StageType type;

		std::vector<Descriptor> descriptors;
		std::vector<PushConstantRange> pushConstantRanges;

		Pipeline *parent;
		ShaderModule *module = nullptr;

		Stage(Pipeline *_parent);
		void create();
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

	struct DescriptorSetLayout
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings;
		VkDescriptorSetLayout v;
		int refCount = 1;

		~DescriptorSetLayout();
	};

	struct PipelineLayout
	{
		VkDescriptorSetLayout descriptorLayout;
		std::vector<VkPushConstantRange> pushConstantRanges;
		VkPipelineLayout v;
		int refCount = 1;

		~PipelineLayout();
	};

	struct ResourceBank;
	REFLECTABLE struct Pipeline
	{
		REFL_BANK;

		REFLv std::string name;
		std::string filename;
		std::string filepath;

		REFLv int cx = 0;
		REFLv int cy = 0;

		REFLe VertexInputType vertex_input_type = VertexInputType::zero;
		REFLv int patch_control_points = 0;
		REFLv bool depth_test = false;
		REFLv bool depth_write = false;
		REFLv bool depth_clamp = false;

		REFLe PrimitiveTopology primitive_topology = PrimitiveTopology::triangle_list;
		REFLe PolygonMode polygon_mode = PolygonMode::fill;
		REFLe CullMode cull_mode = CullMode::back;

		std::vector<BlendAttachment> blendAttachments;
		std::vector<DynamicState> dynamicStates;
		std::vector<LinkResource> links;
		Stage *stages[5] = {};

		ResourceBank *pResource = nullptr;

		std::vector<ShaderMacro> shaderMacros;

		VkPrimitiveTopology vkPrimitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		VkPolygonMode vkPolygonMode = VK_POLYGON_MODE_FILL;
		VkCullModeFlagBits vkCullMode = VK_CULL_MODE_BACK_BIT;
		std::vector<VkPipelineColorBlendAttachmentState> vkBlendAttachments;
		std::vector<VkDescriptorSetLayoutBinding> vkDescriptors;
		std::vector<VkPushConstantRange> vkPushConstantRanges;
		std::vector<VkPipelineShaderStageCreateInfo> vkStages;

		VkPipelineVertexInputStateCreateInfo *pVertexInputState = nullptr;
		VkRenderPass renderPass;
		int subpassIndex;
		std::vector<VkDynamicState> vkDynamicStates;
		DescriptorSetLayout *descriptorSetLayout = nullptr;
		PipelineLayout *pipelineLayout = nullptr;
		VkPipeline pipeline = 0;
		VkDescriptorSet descriptorSet = 0;

		Pipeline();
		~Pipeline();
		void loadXML(const std::string &filename);
		void saveXML(const std::string &filename);
		void setup(VkRenderPass _renderPass, std::uint32_t _subpassIndex);
		void updateDescriptors();
		int descriptorPosition(const std::string &name);
	};

	struct Model;
	REFLECTABLE struct Drawcall : Element
	{
		DrawAction *parent = nullptr;

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

		REFLv std::string model_name;
		Model *model = nullptr;

		REFLv std::string indirect_vertex_buffer_name;
		REFLv std::string indirect_index_buffer_name;

		REFLv int first_indirect = 0;

		REFLv int indirect_count = 0;
		REFLv std::string indirect_count_name;
		int *p_indirect_count = nullptr;

		REFLv StageType push_constant_stage = StageType::null;
		REFLv int push_constant_offset = 0;
		REFLe PushConstantType push_constant_type = PushConstantType::int_t;
		void *push_constant_value = nullptr;
		size_t push_constant_size = 0;

		IndirectVertexBuffer *m_indirectVertexBuffer = nullptr;
		IndirectIndexBuffer *m_indirectIndexBuffer = nullptr;

		Drawcall();
		Drawcall(int vertexCount, int firstVertex, int instanceCount, int firstInstance);
		Drawcall(int indexCount, int firstIndex, int vertexOffset, int instanceCount, int firstInstance);
		Drawcall(Model *p, int instanceCount = 1, int firstInstance = 0);
		Drawcall(IndirectVertexBuffer *indirectVertexBuffer, uint32_t firstIndirect = 0, uint32_t indirectCount = 0);
		Drawcall(IndirectIndexBuffer *indirectIndexBuffer, uint32_t firstIndirect = 0, uint32_t indirectCount = 0);
		Drawcall(StageType stage, void *data, size_t size, size_t offset = 0);
		~Drawcall();
		void loadFromAt(AttributeTreeNode *n);
		void saveToAt(AttributeTreeNode *n);
	};

	typedef void(*PF_RenderFunc)(VkCommandBuffer);
	REFLECTABLE struct DrawAction : Element, Container
	{
		RenderPass *parent = nullptr;

		REFL_BANK;

		REFLv std::string name;

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
		Pipeline *pipeline = nullptr;

		VkDescriptorSet descriptorSet = 0;

		PF_RenderFunc m_pRenderFunc = nullptr;

		bool show = true;

		Drawcall *findDrawcall(const std::string &n);

		DrawAction();
		DrawAction(Pipeline *pipeline);
		DrawAction(PF_RenderFunc pRenderFunc);

		template <class... _Valty>
		inline Drawcall *addDrawcall(_Valty&&... _Val)
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
		RenderPass *parent;

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
		RenderPass *parent;

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
		Renderer *parent;

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
		inline Attachment *addAttachment(_Valty&&... _Val)
		{
			attachments.emplace_back(_Val...);
			auto p = &attachments.back();
			p->parent = this;
			return p;
		}

		template <class... _Valty>
		inline Dependency *addDependency(_Valty&&... _Val)
		{
			dependencies.emplace_back(_Val...);
			auto p = &dependencies.back();
			p->parent = this;
			return p;
		}

		template <class... _Valty>
		inline DrawAction *addAction(_Valty&&... _Val)
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

		Framebuffer *vkFramebuffer[2] = {};

		RenderPass *findRenderPass(const std::string &n);

		Renderer(int _cx = -1, int _cy = -1);
		~Renderer();

		template <class... _Valty>
		inline RenderPass *addPass(_Valty&&... _Val)
		{
			passes.emplace_back(_Val...);
			auto p = &passes.back();
			p->parent = this;
			return p;
		}

		void loadXML(const std::string &_filename);
		void saveXML();
		void maintain(int row) override;
		void getDescriptorSets();
		void setup();
		void updateDescriptors();
		void execute(CommandBuffer *cb, int index = 0);
	};

	struct ImageData
	{
		int fif;
		size_t bpp;
		size_t channel;
		size_t cx;
		size_t cy;
		size_t pitch;
		size_t size;
		unsigned char *data;

		~ImageData();
		void swapChannel(size_t channel0, size_t channel1);
		void format();
		VkFormat getVkFormat(bool sRGB);
	};

	ImageData *createImageData(const std::string &filename);
}
