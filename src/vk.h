#ifndef __TKE_VK__
#define __TKE_VK__

#include <vector>
#include <string>

#define VK_USE_PLATFORM_WIN32_KHR 1
#include <vulkan\vulkan.h>

#include "utils.h"

namespace tke
{
	namespace vk
	{
		extern VkFormat swapchainFormat;

		void queueWaitIdle();
		void deviceWaitIdle();

		void queueSubmit(VkSemaphore waitSemaphore, VkCommandBuffer cmd, VkSemaphore signalSemaphore);
		void queueSubmit(VkSemaphore waitSemaphore, int count, VkCommandBuffer *cmds, VkSemaphore signalSemaphore);
		void queueSubmitFence(VkSemaphore waitSemaphore, int count, VkCommandBuffer *cmds, VkFence fence);
		void beginCommandBuffer(VkCommandBuffer cmd, VkCommandBufferUsageFlags flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, VkCommandBufferInheritanceInfo *pInheritance = nullptr);

		struct CommandPool
		{
			VkCommandPool pool;

			void create();
			VkCommandBuffer allocate();
			VkCommandBuffer allocateSecondary();
			void free(VkCommandBuffer cmd);

			VkCommandBuffer begineOnce();
			void endOnce(VkCommandBuffer cmd);
		};
		extern CommandPool commandPool;

		void *mapMemory(VkDeviceMemory memory, size_t offset, size_t size);
		void unmapMemory(VkDeviceMemory memory);
		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

		void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);
		void destroyBuffer(VkBuffer buffer, VkDeviceMemory memory);
		void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, size_t srcOffset = 0, size_t dstOffset = 0);
		void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, size_t count, VkBufferCopy *ranges);
		void updateBuffer(void *data, size_t size, VkBuffer stagingBuffer, VkDeviceMemory stagingMemory, VkBuffer &uniformBuffer);

		size_t createImage(std::uint32_t w, std::uint32_t h, std::uint32_t mipmapLevels, std::uint32_t arrayLayers, VkFormat format, VkImageUsageFlags usage, VkImage &image, VkDeviceMemory &memory);
		void destroyImage(VkImage image, VkDeviceMemory memory);
		void copyImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height);
		VkImageView createImageView(VkImage image, VkImageViewType type, VkImageAspectFlags aspect, VkFormat format, int baseLevel, int levelCount, int baseLayer, int layerCount);
		void transitionImageLayout(VkImage image, VkImageAspectFlags aspect, VkImageLayout oldLayout, VkImageLayout newLayout, int level);

		extern VkSampler plainSampler;
		extern VkSampler plainUnnormalizedSampler;
		extern VkSampler colorSampler;
		extern VkSampler colorBorderSampler;

		VkPipelineVertexInputStateCreateInfo vertexState(std::uint32_t bindingCount, VkVertexInputBindingDescription *pBindings, std::uint32_t attributeCount, VkVertexInputAttributeDescription *pAttributes);

		VkDescriptorSetLayout createDescriptorSetLayout(VkDescriptorSetLayoutCreateInfo *pInfo);
		void destroyDescriptorLayout(VkDescriptorSetLayout layout);

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

		VkShaderModule loadShaderModule(const std::string &filename);
		void destroyShaderModule(VkShaderModule shaderModule);

		VkPipelineLayout createPipelineLayout(VkPipelineLayoutCreateInfo *pInfo);
		void destroyPipelineLayout(VkPipelineLayout layout);
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

		VkFramebuffer createFramebuffer(std::uint32_t cx, std::uint32_t cy, VkRenderPass renderPass, std::uint32_t attachmentCount, VkImageView *pViews);
		void destroyFramebuffer(VkFramebuffer fb);

		void createSwapchain(HWND hWnd, int cx, int cy, VkSurfaceKHR &surface, VkSwapchainKHR &swapchain, VkImage *pImages);

		VkFence createFence();
		VkEvent createEvent();
		VkSemaphore createSemaphore();

		void waitFence(VkFence);

		std::uint32_t acquireNextImage(VkSwapchainKHR swapchain, VkSemaphore semaphore);

		void queuePresent(VkPresentInfoKHR *pInfo);

		Err init(const char *appName, bool debug);
	}
}

#endif