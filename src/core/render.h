#ifndef __TKE_RENDER_EXEC__
#define __TKE_RENDER_EXEC__

#include "vk.h"
#include "render.abstract.h"
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

	struct Pipeline : PipelineAbstract<StageAbstract>
	{
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

		int descriptorPosition(const std::string &name);
		void loadXML();
		void getLayout();
		void reallocateDescriptorSet();
		VkPipeline getPipeline(int cx, int cy, VkRenderPass renderPass, std::uint32_t subpassIndex);
		void make();
		void create(const char *filename, VkPipelineVertexInputStateCreateInfo *pVertexInputState, VkRenderPass renderPass, std::uint32_t subpassIndex);
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

	VkFramebuffer getFramebuffer(int cx, int cy, VkRenderPass renderPass, std::vector<VkImageView> views);

	struct ShaderModule
	{
		std::string filename;
		VkShaderModule module;
	};

	VkShaderModule getShaderModule(const std::string &filename);

	struct Renderable
	{
		virtual void render(VkCommandBuffer) = 0;
	};

	struct Model;
	struct Drawcall : DrawcallAbstract
	{
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
		template <class T>
		inline Drawcall(VkShaderStageFlags stage, T *data, size_t offset = 0)
		{
			m_pushConstantStage = stage;
			push_constant_offset = offset;
			auto size = sizeof(T);
			m_pushConstantSize = size;
			push_constant_value = malloc(size);
			memcpy(push_constant_value, data, size);
			type = DrawcallType::push_constant;
		}
	};

	struct ImageResourceLink
	{
		Image *m_image;
		int m_baseLevel = 0;
		int m_levelCount = 1;
		int m_baseLayer = 0;
		int m_layerCount = 1;

		int binding = 0;
		int arrayElement = 0;
		ImageResourceLink(Image *image);
	};

	struct DrawAction : DrawActionAbstract<Drawcall>
	{
		VertexBuffer *m_vertexBuffer = nullptr;
		IndexBuffer *m_indexBuffer = nullptr;
		Pipeline *m_pipeline = nullptr;
		std::vector<ImageResourceLink> m_imageResourceLinks;

		VkDescriptorSet m_descriptorSet = 0;

		Renderable *m_pRenderable = nullptr;

		bool show = true;

		DrawAction();
		DrawAction(Pipeline *pipeline);
		DrawAction(Renderable *pRenderable);
		void preprocess(Pipeline* &currentPipeline);
	};

	struct Attachment : AttachmentAbstract
	{
		Image *image = nullptr;
		VkImageAspectFlags aspect = 0;

		VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		VkClearValue clearValue = {};

		int index;

		Attachment();
		Attachment(Image *image, VkClearValue _clearValue = {9999.f});
	};

	struct Dependency : DependencyAbstract
	{
		Dependency();
		Dependency(void *_target);
		Dependency(const std::string &_pass_name);
	};

	struct RenderPass : RenderPassAbstract<Attachment, Dependency, DrawAction>
	{
		VkCommandBuffer secondaryCmd = 0;

		int index = 0;

		RenderPass();
		RenderPass(VkCommandBuffer cmd);
	};

	struct Renderer : RendererAbstract<RenderPass>
	{
		bool containSwapchain = false;

		ResourceBank *pResource = &globalResource;

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

		Renderer();
		Renderer(int _cx, int _cy);
		void pushImage(Attachment *ai);
		void getDescriptorSets();
		void execute(VkCommandBuffer cmd, int index = 0);
		void setup();
	};
}

#endif