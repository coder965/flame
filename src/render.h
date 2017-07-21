#pragma once

#include <vector>
#include <list>
#include <string>
#include <map>
#include <mutex>

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

	struct Buffer;
	struct StagingBuffer;
	struct VertexBuffer;
	struct IndexBuffer;
	struct IndirectVertexBuffer;
	struct IndirectIndexBuffer;
	struct Image;
	struct ImageView;
	struct DescriptorPool;
	struct DescriptorSetLayout;
	struct DescriptorSet;
	struct Framebuffer;
	struct RenderPass;
	struct Pipeline;

	struct Model;

	struct Instance
	{
		VkInstance v;
		std::mutex mtx;
	};

	struct Device
	{
		VkDevice v;
		std::mutex mtx;

		void waitIdle();
	};

	struct Queue
	{
		VkQueue v;
		std::mutex mtx;

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
		VkBuffer v = 0;
		VkDeviceMemory memory = 0;

		VkBufferUsageFlags usage;
		VkMemoryPropertyFlags memoryProperty;

		Buffer(size_t _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _memoryProperty = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		~Buffer();
		void recreate(size_t _size, void *data = nullptr);
		void update(void *data, StagingBuffer *stagingBuffer, size_t size = 0);
		void *map(size_t offset, size_t _size);
		void unmap();
	};

	struct StagingBuffer : Buffer
	{
		StagingBuffer(size_t _size);
	};

	struct UniformBuffer : Buffer
	{
		UniformBuffer(size_t _size);
	};

	struct VertexBuffer : Buffer
	{
		VertexBuffer(size_t _size = 16, void *data = nullptr);
	};

	struct IndexBuffer : Buffer
	{
		IndexBuffer(size_t _size = 16, void *data = nullptr);
	};

	struct OnceVertexBuffer : Buffer // data use once per frame
	{
		OnceVertexBuffer(size_t _size = 16, void *data = nullptr);
	};

	struct OnceIndexBuffer : Buffer // data use once per frame
	{
		OnceIndexBuffer(size_t _size = 16, void *data = nullptr);
	};

	struct IndirectVertexBuffer : Buffer
	{
		IndirectVertexBuffer(size_t _size = sizeof VkDrawIndirectCommand);
	};

	struct IndirectIndexBuffer : Buffer
	{
		IndirectIndexBuffer(size_t _size = sizeof VkDrawIndexedIndirectCommand);
	};

	struct ImageData
	{
		int fif;
		size_t byte_per_pixel;
		size_t channel;
		size_t cx;
		size_t cy;
		size_t pitch;
		size_t size;
		unsigned char *v;

		~ImageData();
		VkFormat getVkFormat(bool sRGB);
	};

	ImageData *createImageData(const std::string &filename);

	struct ImageView
	{
		Image *image;
		VkImageAspectFlags aspect;
		int baseLevel;
		int levelCount;
		int baseLayer;
		int layerCount;
		VkImageView v;

		ImageView(Image *_image);
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

		int cx = 1, cy = 1;
		unsigned char *data = nullptr;
		size_t bytePerPixel;
		size_t pitch;
		size_t size;
		int level = 1;
		int layer = 1;
		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
		VkImage v = 0;
		VkDeviceMemory memory = 0;
		VkImageLayout layout = VK_IMAGE_LAYOUT_PREINITIALIZED;
		VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;

		std::vector<ImageView*> views;

		std::string filename;

		bool sRGB = false;

		int index = -1;

		Image(int w, int h, VkFormat _format, VkImageUsageFlags usage, int _level = 1, void *_data = nullptr, size_t _size = 0, VkImageAspectFlags aspect = 0);
		Image(Type _type, VkImage _image, int w, int h, VkFormat _format);
		~Image();
		int getWidth(int _level = 0) const;
		int getHeight(int _level = 0) const;
		unsigned char getR(float x, float y);
		void transitionLayout(int _level, VkImageAspectFlags aspect, VkImageLayout _layout);
		void fillData(int _level, void *data, size_t _size, VkImageAspectFlags aspect);
		VkImageView getView(VkImageAspectFlags aspect = 0, int baseLevel = 0, int levelCount = 1, int baseLayer = 0, int layerCount = 1);
	};

	Image *createImage(const std::string &filename, bool sRGB = false, bool saveData = false);

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
		void beginRenderPass(RenderPass *renderPass, Framebuffer *fb, VkClearValue *pClearValue = nullptr);
		void nextSubpass(VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);
		void endRenderPass();
		void setViewportAndScissor(int cx, int cy);
		void setScissor(int x, int y, int cx, int cy);
		void bindVertexBuffer(VertexBuffer *b);
		void bindVertexBuffer(OnceVertexBuffer *b);
		void bindIndexBuffer(IndexBuffer *b);
		void bindIndexBuffer(OnceIndexBuffer *b);
		void bindPipeline(Pipeline *p);
		void bindDescriptorSet();
		void bindDescriptorSet(VkDescriptorSet *sets, int index = 0, int count = 1);
		void execSecondaryCmd(VkCommandBuffer cmd);
		void pushConstant(StageType stage, int offset, int size, void *src);
		void draw(int vertexCount, int firstVertex = 0, int instanceCount = 1, int firstInstance = 0);
		void drawIndex(int indexCount, int firstIndex = 0, int vertexOffset = 0, int instanceCount = 1, int firstInstance = 0);
		void drawModel(Model *m, int mtIndex = -1, int instanceCount = 1, int firstInstance = 0);
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
		void updateBuffer(void *data, size_t size, Buffer *stagingBuffer, VkBuffer &uniformBuffer);
		void copyImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height);
	};
	extern CommandPool *commandPool;

	struct DescriptorSet
	{
		DescriptorPool *pool;
		DescriptorSetLayout *layout;
		VkDescriptorSet v;

		DescriptorSet(DescriptorPool *_pool, DescriptorSetLayout *_layout);
		~DescriptorSet();
		void setBuffer(int binding, int index, Buffer *buffer);
		void setImage(int binding, int index, Image *image, VkSampler sampler, VkImageAspectFlags aspect = 0, int baseLevel = 0, int levelCount = 1, int baseLayer = 0, int layerCount = 1);
	};

	struct DescriptorPool
	{
		VkDescriptorPool v;

		DescriptorPool();
		~DescriptorPool();
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

	Framebuffer *getFramebuffer(Image *i, RenderPass *renderPass, int level = 0);

	Framebuffer *getFramebuffer(int cx, int cy, RenderPass *renderPass, int viewCount, VkImageView *views);

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

	struct RenderPass
	{
		VkRenderPass v;
		std::vector<VkClearValue> clearValues;

		RenderPass(int attachmentCount, VkAttachmentDescription *pAttachments, int subpassCount, VkSubpassDescription *pSubpasses, int dependencyCount = 0, VkSubpassDependency *pDependencies = nullptr);
		~RenderPass();
	};

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
		REFLe image_n_sampler = 1 << 1
	};

	REFLECTABLE struct Descriptor
	{
		REFL_BANK;

		REFLe DescriptorType type = DescriptorType::uniform_buffer;
		REFLv int binding = 0;
		REFLv int count = 0;
		REFLv std::string name;
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

	REFLECTABLE struct ShaderMacro
	{
		REFL_BANK;

		REFLe StageType stage = StageType::null;
		REFLv std::string value;
	};

	struct Model;
	struct ResourceBank
	{
		ResourceBank *parents;

		std::map<std::string, Buffer*> bufferResources;
		std::map<std::string, Image*> imageResources;

		void setBuffer(Buffer *p, const std::string &str);
		void setImage(Image *p, const std::string &str);

		Buffer *getBuffer(const std::string &str);
		Image *getImage(const std::string &str);

		ResourceBank(ResourceBank *_parent);
	};

	extern ResourceBank globalResource;

	struct ShaderModule
	{
		std::string filename;
		std::vector<std::string> defines;

		std::vector<std::vector<Descriptor>> descriptors;
		std::vector<PushConstantRange> pushConstantRanges;
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

		Pipeline *parents;
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

		DescriptorType type = DescriptorType::null;
		VkSampler vkSampler = 0;
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
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
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

		std::vector<ShaderMacro> shaderMacros;

		VkPrimitiveTopology vkPrimitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		VkPolygonMode vkPolygonMode = VK_POLYGON_MODE_FILL;
		VkCullModeFlagBits vkCullMode = VK_CULL_MODE_BACK_BIT;
		std::vector<VkPipelineColorBlendAttachmentState> vkBlendAttachments;
		std::vector<std::vector<VkDescriptorSetLayoutBinding>> vkDescriptors;
		std::vector<DescriptorSetLayout*> descriptorSetLayouts;
		std::vector<VkPushConstantRange> vkPushConstantRanges;
		PipelineLayout *pipelineLayout = nullptr;

		VkPipelineVertexInputStateCreateInfo *pVertexInputState = nullptr;
		RenderPass *renderPass;
		int subpassIndex;
		std::vector<VkDynamicState> vkDynamicStates;
		std::vector<VkPipelineShaderStageCreateInfo> vkStages;
		VkPipeline pipeline = 0;
		DescriptorSet *descriptorSet = nullptr;

		~Pipeline();
		void loadXML(const std::string &filename);
		void saveXML(const std::string &filename);
		void setup(RenderPass *_renderPass, std::uint32_t _subpassIndex, bool need_default_ds);
		DescriptorSet *createDescriptorSet(DescriptorPool *_pool, int index = 0);
		void linkDescriptors(DescriptorSet *set, ResourceBank *resource);
		int descriptorPosition(const std::string &name);
	};
}
