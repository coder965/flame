#ifndef __TKE_VK__
#define __TKE_VK__

#include <vector>
#include <string>

#define VK_USE_PLATFORM_WIN32_KHR 1
#include <vulkan\vulkan.h>

#include "utils.h"

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

		void *map(size_t offset, size_t size);
		void unmap();
	};

	struct NonStagingBufferAbstract : Buffer
	{
		void create(size_t size, VkBufferUsageFlags usage, void *data = nullptr);
		void update(void *data, StagingBuffer &stagingBuffer, size_t size = 0);
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

	struct IndirectVertexBuffer : NonStagingBufferAbstract
	{
		size_t stride();
		void create(size_t size);
	};

	struct IndirectIndexBuffer : NonStagingBufferAbstract
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
		std::list<VkDescriptorImageInfo> infos;

		std::string filename;

		bool m_sRGB = false;

		int sceneIndex = -1;

		unsigned char *m_data = nullptr;

		int getWidth(int mipmapLevel = 0) const;
		int getHeight(int mipmapLevel = 0) const;
		void transitionLayout(int level, VkImageAspectFlags aspect, VkImageLayout layout);
		void fillData(int level, std::uint8_t *data, size_t size, VkImageAspectFlags aspect);
		void create(int w, int h, VkFormat format, VkImageUsageFlags usage, std::uint8_t *data = nullptr, size_t size = 0, VkImageAspectFlags aspect = 0);
		void destroy();
		VkImageView getView(VkImageAspectFlags aspect = 0, int baseLevel = 0, int levelCount = 1, int baseLayer = 0, int layerCount = 1);
		VkDescriptorImageInfo *getInfo(VkSampler sampler, VkImageAspectFlags aspect = 0, int baseLevel = 0, int levelCount = 1, int baseLayer = 0, int layerCount = 1);
		unsigned char getPixel(int x, int y, int off) const;
	};

	struct CommandPool
	{
		VkCommandPool pool;

		void create();
		void destroy();
		VkCommandBuffer allocate();
		VkCommandBuffer allocateSecondary();
		void free(VkCommandBuffer cmd);

		VkCommandBuffer begineOnce();
		void endOnce(VkCommandBuffer cmd);

		void cmdCopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, size_t srcOffset = 0, size_t dstOffset = 0);
		void cmdCopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, size_t count, VkBufferCopy *ranges);
		void cmdUpdateBuffer(void *data, size_t size, StagingBuffer &stagingBuffer, VkBuffer &uniformBuffer);

		void cmdCopyImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height);
	};
	extern CommandPool commandPool;

	struct DescriptorSetLayout
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings;
		VkDescriptorSetLayout v;
	};

	VkDescriptorSetLayout createDescriptorSetLayout(std::vector<VkDescriptorSetLayoutBinding> &bindings);
	void destroyDescriptorSetLayout(VkDescriptorSetLayout v);

	struct PipelineLayout
	{
		VkDescriptorSetLayout descriptorLayout;
		std::vector<VkPushConstantRange> pushConstantRanges;
		VkPipelineLayout v;
	};

	VkPipelineLayout createPipelineLayout(VkDescriptorSetLayout descriptorLayout, std::vector<VkPushConstantRange> &pushConstantRanges);
	void destroyPipelineLayout(VkPipelineLayout v);

	struct Framebuffer
	{
		std::vector<VkImageView> views;
		VkFramebuffer v;
	};

	VkFramebuffer createFramebuffer(int cx, int cy, VkRenderPass renderPass, std::vector<VkImageView> &views);
	void destroyFramebuffer(VkFramebuffer v);

	struct ShaderModule
	{
		std::string filename;
		VkShaderModule v;
	};

	VkShaderModule createShaderModule(const std::string &filename);
	void destroyShaderModule(VkShaderModule v);

	namespace vk
	{
		extern VkFormat swapchainFormat;

		void queueWaitIdle();
		void deviceWaitIdle();

		void queueSubmit(VkSemaphore waitSemaphore, VkCommandBuffer cmd, VkSemaphore signalSemaphore);
		void queueSubmit(VkSemaphore waitSemaphore, int count, VkCommandBuffer *cmds, VkSemaphore signalSemaphore);
		void queueSubmitFence(VkSemaphore waitSemaphore, int count, VkCommandBuffer *cmds, VkFence fence);
		void beginCommandBuffer(VkCommandBuffer cmd, VkCommandBufferUsageFlags flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, VkCommandBufferInheritanceInfo *pInheritance = nullptr);

		extern VkSampler plainSampler;
		extern VkSampler plainUnnormalizedSampler;
		extern VkSampler colorSampler;
		extern VkSampler colorBorderSampler;

		VkPipelineVertexInputStateCreateInfo vertexState(std::uint32_t bindingCount, VkVertexInputBindingDescription *pBindings, std::uint32_t attributeCount, VkVertexInputAttributeDescription *pAttributes);

		struct DescriptrPool
		{
			VkDescriptorPool pool;
			std::vector<VkWriteDescriptorSet> writes;

			void create();
			VkDescriptorSet allocate(VkDescriptorSetLayout *pLayout);
			void free(VkDescriptorSet set);
			void addWrite(VkDescriptorSet descriptorSet, VkDescriptorType type, uint32_t binding, VkDescriptorImageInfo *pImageInfo, uint32_t dstArrayElement = 0);
			void addWrite(VkDescriptorSet descriptorSet, VkDescriptorType type, uint32_t binding, VkDescriptorBufferInfo *pBufferInfo, uint32_t dstArrayElement = 0);
			void update();
		};
		extern DescriptrPool descriptorPool;

		VkPipeline createPipeline(VkGraphicsPipelineCreateInfo *pInfo);
		void destroyPipeline(VkPipeline pipeline);

		VkAttachmentDescription colorAttachment(VkFormat format, VkAttachmentLoadOp loadOp);
		VkAttachmentDescription depthAttachment(VkFormat format, VkAttachmentLoadOp loadOp);
		VkAttachmentDescription swapchainAttachment(VkAttachmentLoadOp loadOp);
		VkSubpassDescription subpass(int colorCount, VkAttachmentReference *pColors, VkAttachmentReference *pDepth = nullptr, int inputCount = 0, VkAttachmentReference *pInputs = nullptr);
		VkSubpassDependency subpassDependency(int srcSubpass, int dstSubpass);
		VkRenderPass createRenderPass(std::uint32_t attachmentCount, VkAttachmentDescription *pAttachments, std::uint32_t subpassCount, VkSubpassDescription *pSubpasses, std::uint32_t dependencyCount, VkSubpassDependency *pDependencies);
		void destroyRenderPass(VkRenderPass rp);
		VkRenderPassBeginInfo renderPassBeginInfo(VkRenderPass renderPass, VkFramebuffer framebuffer, std::uint32_t cx, std::uint32_t cy, std::uint32_t clearValueCount, VkClearValue *pClearValues);

		void createSwapchain(HWND hWnd, int cx, int cy, VkSurfaceKHR &surface, VkSwapchainKHR &swapchain, VkImage *pImages);
		void destroySwapchain(VkSurfaceKHR surface, VkSwapchainKHR swapchain);

		VkFence createFence();
		void destroyFence(VkFence fence);

		VkEvent createEvent();
		void destroyEvent(VkEvent event);

		VkSemaphore createSemaphore();
		void destroySemaphore(VkSemaphore semaphore);

		void waitFence(VkFence);

		std::uint32_t acquireNextImage(VkSwapchainKHR swapchain, VkSemaphore semaphore);

		void queuePresent(VkPresentInfoKHR *pInfo);

		Err init(const char *appName, bool debug);
	}
}

#endif