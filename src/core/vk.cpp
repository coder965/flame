#include "vk.h"
#include <assert.h>

namespace tke
{
	namespace vk
	{
		CRITICAL_SECTION cs;

		VkInstance inst;
		VkPhysicalDevice physicalDevice;
		VkDevice device;
		std::vector<VkQueueFamilyProperties> queueFamilyProps;
		uint32_t graphicsQueueIndex;
		uint32_t presentQueueIndex;
		uint32_t transferQueueIndex;
		VkQueue graphicsQueue;
		VkPhysicalDeviceMemoryProperties memProperties;
		VkCommandPool commandPool;
		VkDescriptorPool descriptorPool;
		VkFormat swapchainFormat;

		VkFormat getSwapchainFormat()
		{
			return swapchainFormat;
		}

		void queueWaitIdle()
		{
			EnterCriticalSection(&cs);
			vkQueueWaitIdle(graphicsQueue);
			LeaveCriticalSection(&cs);
		}

		void deviceWaitIdle()
		{
			EnterCriticalSection(&cs);
			vkDeviceWaitIdle(device);
			LeaveCriticalSection(&cs);
		}

		VkCommandBuffer allocateCommandBuffer()
		{
			VkCommandBufferAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandPool = commandPool;
			allocInfo.commandBufferCount = 1;

			VkCommandBuffer cmd;
			EnterCriticalSection(&cs);
			auto res = vkAllocateCommandBuffers(device, &allocInfo, &cmd);
			assert(res == VK_SUCCESS);
			LeaveCriticalSection(&cs);

			return cmd;
		}

		VkCommandBuffer allocateSecondaryCommandBuffer()
		{
			VkCommandBufferAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
			allocInfo.commandPool = commandPool;
			allocInfo.commandBufferCount = 1;

			VkCommandBuffer cmd;
			EnterCriticalSection(&cs);
			auto res = vkAllocateCommandBuffers(device, &allocInfo, &cmd);
			assert(res == VK_SUCCESS);
			LeaveCriticalSection(&cs);

			return cmd;
		}

		void freeCommandBuffer(VkCommandBuffer cmd)
		{
			EnterCriticalSection(&cs);
			vkFreeCommandBuffers(device, commandPool, 1, &cmd);
			LeaveCriticalSection(&cs);
		}

		void queueSubmit(VkSemaphore waitSemaphore, VkSemaphore signalSemaphore, VkCommandBuffer cmd)
		{
			VkSubmitInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			info.pWaitDstStageMask = waitStages;
			info.waitSemaphoreCount = 1;
			info.pWaitSemaphores = &waitSemaphore;
			info.commandBufferCount = 1;
			info.pCommandBuffers = &cmd;
			info.signalSemaphoreCount = 1;
			info.pSignalSemaphores = &signalSemaphore;

			EnterCriticalSection(&cs);
			auto res = vkQueueSubmit(graphicsQueue, 1, &info, VK_NULL_HANDLE);
			assert(res == VK_SUCCESS);
			LeaveCriticalSection(&cs);
		}

		void beginCommandBuffer(VkCommandBuffer cmd, VkCommandBufferUsageFlags flags, VkCommandBufferInheritanceInfo *pInheritance)
		{
			VkCommandBufferBeginInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			info.flags = flags;
			info.pInheritanceInfo = pInheritance;
			auto res = vkBeginCommandBuffer(cmd, &info);
			assert(res == VK_SUCCESS);
		}

		VkCommandBuffer begineOnceCommandBuffer()
		{
			auto cmd = allocateCommandBuffer();

			beginCommandBuffer(cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

			return cmd;
		}

		void endOnceCommandBuffer(VkCommandBuffer cmd)
		{
			VkResult res;

			res = vkEndCommandBuffer(cmd);
			assert(res == VK_SUCCESS);

			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &cmd;

			EnterCriticalSection(&cs);
			res = vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
			assert(res == VK_SUCCESS);
			res = vkQueueWaitIdle(graphicsQueue);
			assert(res == VK_SUCCESS);
			LeaveCriticalSection(&cs);

			freeCommandBuffer(cmd);
		}

		void *mapMemory(VkDeviceMemory memory, size_t offset, size_t size)
		{
			void *map;
			EnterCriticalSection(&cs);
			auto res = vkMapMemory(device, memory, offset, size, 0, &map);
			assert(res == VK_SUCCESS);
			LeaveCriticalSection(&cs);
			return map;
		}

		void unmapMemory(VkDeviceMemory memory)
		{
			EnterCriticalSection(&cs);
			vkUnmapMemory(device, memory);
			LeaveCriticalSection(&cs);
		}

		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
		{
			for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
			{
				if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
					return i;
			}
			return -1;
		}

		void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory)
		{
			VkResult res;

			VkBufferCreateInfo bufferInfo = {};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = size;
			bufferInfo.usage = usage;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			EnterCriticalSection(&cs);

			res = vkCreateBuffer(device, &bufferInfo, nullptr, &buffer);
			assert (res == VK_SUCCESS);

			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

			assert(size <= memRequirements.size);

			VkMemoryAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

			res = vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory);
			assert(res == VK_SUCCESS);

			res = vkBindBufferMemory(device, buffer, bufferMemory, 0);
			assert(res == VK_SUCCESS);

			LeaveCriticalSection(&cs);
		}

		void destroyBuffer(VkBuffer buffer, VkDeviceMemory memory)
		{
			EnterCriticalSection(&cs);
			vkFreeMemory(device, memory, nullptr);
			vkDestroyBuffer(device, buffer, nullptr);
			LeaveCriticalSection(&cs);
		}

		void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, size_t srcOffset, size_t dstOffset)
		{
			auto commandBuffer = begineOnceCommandBuffer();

			VkBufferCopy region = {};
			region.size = size;
			region.srcOffset = srcOffset;
			region.dstOffset = dstOffset;
			vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &region);

			endOnceCommandBuffer(commandBuffer);
		}

		void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, size_t count, VkBufferCopy *ranges)
		{
			auto commandBuffer = begineOnceCommandBuffer();

			vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, count, ranges);

			endOnceCommandBuffer(commandBuffer);
		}

		void updateBuffer(void *data, size_t size, VkBuffer stagingBuffer, VkDeviceMemory stagingMemory, VkBuffer &Buffer)
		{
			EnterCriticalSection(&cs);
			void* map;
			auto res = vkMapMemory(device, stagingMemory, 0, size, 0, &map);
			assert(res == VK_SUCCESS);
			memcpy(map, data, size);
			vkUnmapMemory(device, stagingMemory);
			LeaveCriticalSection(&cs);

			copyBuffer(stagingBuffer, Buffer, size);
		}

		size_t createImage(std::uint32_t w, std::uint32_t h, std::uint32_t mipmapLevels, std::uint32_t arrayLayers, VkFormat format, VkImageUsageFlags usage, VkImage &image, VkDeviceMemory &memory)
		{
			VkResult res;

			VkImageCreateInfo imageInfo = {};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.extent.width = w;
			imageInfo.extent.height = h;
			imageInfo.extent.depth = 1;
			imageInfo.mipLevels = mipmapLevels;
			imageInfo.arrayLayers = arrayLayers;
			imageInfo.format = format;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
			imageInfo.usage = usage;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			EnterCriticalSection(&cs);

			res = vkCreateImage(device, &imageInfo, nullptr, &image);
			assert(res == VK_SUCCESS);

			VkMemoryRequirements memRequirements;
			vkGetImageMemoryRequirements(device, image, &memRequirements);

			VkMemoryAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			res = vkAllocateMemory(device, &allocInfo, nullptr, &memory);
			assert(res == VK_SUCCESS);

			res = vkBindImageMemory(device, image, memory, 0);
			assert(res == VK_SUCCESS);

			LeaveCriticalSection(&cs);

			return memRequirements.size;
		}

		void destroyImage(VkImage image, VkDeviceMemory memory)
		{
			vkFreeMemory(device, memory, nullptr);
			vkDestroyImage(device, image, nullptr);
		}

		void copyImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height)
		{
			auto commandBuffer = begineOnceCommandBuffer();

			VkImageSubresourceLayers subResource = {};
			subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subResource.baseArrayLayer = 0;
			subResource.mipLevel = 0;
			subResource.layerCount = 1;

			VkImageCopy region = {};
			region.srcSubresource = subResource;
			region.dstSubresource = subResource;
			region.srcOffset = { 0, 0, 0 };
			region.dstOffset = { 0, 0, 0 };
			region.extent.width = width;
			region.extent.height = height;
			region.extent.depth = 1;

			vkCmdCopyImage(commandBuffer, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

			endOnceCommandBuffer(commandBuffer);
		}

		VkImageView createImageView(VkImage image, VkImageViewType type, VkImageAspectFlags aspect, VkFormat format, int baseLevel, int levelCount, int baseLayer, int layerCount)
		{
			VkImageViewCreateInfo viewInfo = {};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = image;
			viewInfo.viewType = type;
			viewInfo.format = format;
			viewInfo.subresourceRange.aspectMask = aspect;
			viewInfo.subresourceRange.baseMipLevel = baseLevel;
			viewInfo.subresourceRange.levelCount = levelCount;
			viewInfo.subresourceRange.baseArrayLayer = baseLayer;
			viewInfo.subresourceRange.layerCount = layerCount;

			EnterCriticalSection(&cs);
			VkImageView view;
			auto res = vkCreateImageView(device, &viewInfo, nullptr, &view);
			assert(res == VK_SUCCESS);
			LeaveCriticalSection(&cs);
			return view;
		}

		void transitionImageLayout(VkImage image, VkImageAspectFlags aspect, VkImageLayout oldLayout, VkImageLayout newLayout, int level)
		{
			auto commandBuffer = begineOnceCommandBuffer();

			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = oldLayout;
			barrier.newLayout = newLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = image;
			barrier.subresourceRange.aspectMask = aspect;
			barrier.subresourceRange.baseMipLevel = level;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			if (oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED) barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			if (newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			else if (newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			else if (newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			else if (newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

			endOnceCommandBuffer(commandBuffer);
		}

		VkSampler _colorSampler;
		VkSampler _colorBorderSampler;
		VkSampler _screenUvSampler;
		VkSampler _screenSampler;

		VkSampler getColorSampler()
		{
			return _colorSampler;
		}

		VkSampler getColorBorderSampler()
		{
			return _colorBorderSampler;
		}

		VkSampler getScreenSampler()
		{
			return _screenUvSampler;
		}

		VkSampler getScreenUvSampler()
		{
			return _screenSampler;
		}

		VkPipelineVertexInputStateCreateInfo vertexState(std::uint32_t bindingCount, VkVertexInputBindingDescription *pBindings, std::uint32_t attributeCount, VkVertexInputAttributeDescription *pAttributes)
		{
			VkPipelineVertexInputStateCreateInfo state = {};
			state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			state.vertexBindingDescriptionCount = bindingCount;
			state.pVertexBindingDescriptions = pBindings;
			state.vertexAttributeDescriptionCount = attributeCount;
			state.pVertexAttributeDescriptions = pAttributes;

			return state;
		}

		VkDescriptorSet allocateDescriptorSet(VkDescriptorSetLayout *pLayout)
		{
			VkDescriptorSetAllocateInfo descriptorSetInfo = {};
			descriptorSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			descriptorSetInfo.descriptorPool = descriptorPool;
			descriptorSetInfo.descriptorSetCount = 1;
			descriptorSetInfo.pSetLayouts = pLayout;

			VkDescriptorSet descriptorSet;
			EnterCriticalSection(&cs);
			auto res = vkAllocateDescriptorSets(device, &descriptorSetInfo, &descriptorSet);
			assert(res == VK_SUCCESS);
			LeaveCriticalSection(&cs);

			return descriptorSet;
		}

		void freeDescriptorSet(VkDescriptorSet set)
		{
			EnterCriticalSection(&cs);
			auto res = vkFreeDescriptorSets(device, descriptorPool, 1, &set);
			assert(res == VK_SUCCESS);
			LeaveCriticalSection(&cs);
		}

		VkWriteDescriptorSet writeDescriptorSet(VkDescriptorSet descriptorSet, VkDescriptorType type, uint32_t binding, VkDescriptorImageInfo *pImageInfo, uint32_t dstArrayElement)
		{
			VkWriteDescriptorSet write;
			write = {};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.dstSet = descriptorSet;
			write.dstBinding = binding;
			write.dstArrayElement = dstArrayElement;
			write.descriptorType = type;
			write.descriptorCount = 1;
			write.pImageInfo = pImageInfo;

			return write;
		}

		VkWriteDescriptorSet writeDescriptorSet(VkDescriptorSet descriptorSet, VkDescriptorType type, uint32_t binding, VkDescriptorBufferInfo *pBufferInfo, uint32_t dstArrayElement)
		{
			VkWriteDescriptorSet write;
			write = {};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.dstSet = descriptorSet;
			write.dstBinding = binding;
			write.dstArrayElement = dstArrayElement;
			write.descriptorType = type;
			write.descriptorCount = 1;
			write.pBufferInfo = pBufferInfo;

			return write;
		}

		void updataDescriptorSet(size_t count, VkWriteDescriptorSet *pWrites)
		{
			vkUpdateDescriptorSets(device, count, pWrites, 0, nullptr);
		}

		VkDescriptorSetLayout createDescriptorSetLayout(VkDescriptorSetLayoutCreateInfo *pInfo)
		{
			VkDescriptorSetLayout layout;

			EnterCriticalSection(&cs);
			auto res = vkCreateDescriptorSetLayout(device, pInfo, nullptr, &layout);
			assert(res == VK_SUCCESS);
			LeaveCriticalSection(&cs);

			return layout;
		}

		void destroyDescriptorLayout(VkDescriptorSetLayout layout)
		{
			EnterCriticalSection(&cs);
			vkDestroyDescriptorSetLayout(device, layout, nullptr);
			LeaveCriticalSection(&cs);
		}

		VkShaderModule loadShaderModule(const std::string &filename)
		{
			VkShaderModule shader;
			OnceFileBuffer file(filename);
			VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
			shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			shaderModuleCreateInfo.codeSize = file.length;
			shaderModuleCreateInfo.pCode = (uint32_t*)file.data;
			EnterCriticalSection(&cs);
			auto res = vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &shader);
			assert(res == VK_SUCCESS);
			LeaveCriticalSection(&cs);
			return shader;
		}

		void destroyShaderModule(VkShaderModule shaderModule)
		{
			EnterCriticalSection(&cs);
			vkDestroyShaderModule(device, shaderModule, nullptr);
			LeaveCriticalSection(&cs);
		}

		VkPipelineLayout createPipelineLayout(VkPipelineLayoutCreateInfo *pInfo)
		{
			VkPipelineLayout layout;

			EnterCriticalSection(&cs);
			auto res = vkCreatePipelineLayout(device, pInfo, nullptr, &layout);
			assert(res == VK_SUCCESS);
			LeaveCriticalSection(&cs);

			return layout;
		}

		void destroyPipelineLayout(VkPipelineLayout layout)
		{
			EnterCriticalSection(&cs);
			vkDestroyPipelineLayout(device, layout, nullptr);
			LeaveCriticalSection(&cs);
		}

		VkPipeline createPipeline(VkGraphicsPipelineCreateInfo *pInfo)
		{
			VkPipeline pipeline;

			EnterCriticalSection(&cs);
			auto res = vkCreateGraphicsPipelines(device, 0, 1, pInfo, nullptr, &pipeline);
			assert(res == VK_SUCCESS);
			LeaveCriticalSection(&cs);

			return pipeline;
		}

		void destroyPipeline(VkPipeline pipeline)
		{
			EnterCriticalSection(&cs);
			vkDestroyPipeline(device, pipeline, nullptr);
			LeaveCriticalSection(&cs);
		}

		VkAttachmentDescription colorAttachment(VkFormat format, VkAttachmentLoadOp loadOp)
		{
			VkAttachmentDescription desc = {};
			desc.format = format;
			desc.samples = VK_SAMPLE_COUNT_1_BIT;
			desc.loadOp = loadOp;
			desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			desc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			return desc;
		}

		VkAttachmentDescription depthAttachment(VkFormat format, VkAttachmentLoadOp loadOp)
		{
			VkAttachmentDescription desc = {};
			desc.format = format;
			desc.samples = VK_SAMPLE_COUNT_1_BIT;
			desc.loadOp = loadOp;
			desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			return desc;
		}

		VkAttachmentDescription swapchainAttachment(VkAttachmentLoadOp loadOp)
		{
			VkAttachmentDescription desc = {};
			desc.format = swapchainFormat;
			desc.samples = VK_SAMPLE_COUNT_1_BIT;
			desc.loadOp = loadOp;
			desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			desc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			return desc;
		}

		VkSubpassDescription subpass(int colorCount, VkAttachmentReference *pColors, VkAttachmentReference *pDepth, int inputCount, VkAttachmentReference *pInputs)
		{
			VkSubpassDescription desc = {};
			desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			desc.inputAttachmentCount = inputCount;
			desc.pInputAttachments = pInputs;
			desc.colorAttachmentCount = colorCount;
			desc.pColorAttachments = pColors;
			desc.pDepthStencilAttachment = pDepth;
			return desc;
		}

		VkSubpassDependency subpassDependency(int srcSubpass, int dstSubpass)
		{
			VkSubpassDependency dependency = {};
			dependency.srcSubpass = srcSubpass;
			dependency.dstSubpass = dstSubpass;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			return dependency;
		}

		VkRenderPass createRenderPass(std::uint32_t attachmentCount, VkAttachmentDescription *pAttachments, std::uint32_t subpassCount, VkSubpassDescription *pSubpasses, std::uint32_t dependencyCount, VkSubpassDependency *pDependencies)
		{
			VkRenderPass renderPass;

			VkRenderPassCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			info.attachmentCount = attachmentCount;
			info.pAttachments = pAttachments;
			info.subpassCount = subpassCount;
			info.pSubpasses = pSubpasses;
			info.dependencyCount = dependencyCount;
			info.pDependencies = pDependencies;

			EnterCriticalSection(&cs);
			auto res = vkCreateRenderPass(device, &info, nullptr, &renderPass);
			assert(res == VK_SUCCESS);
			LeaveCriticalSection(&cs);

			return renderPass;
		}

		void destroyRenderPass(VkRenderPass rp)
		{
			EnterCriticalSection(&cs);
			vkDestroyRenderPass(device, rp, nullptr);
			LeaveCriticalSection(&cs);
		}

		VkRenderPassBeginInfo renderPassBeginInfo(VkRenderPass renderPass, VkFramebuffer framebuffer, std::uint32_t cx, std::uint32_t cy, std::uint32_t clearValueCount, VkClearValue *pClearValues)
		{
			VkRenderPassBeginInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			info.renderPass = renderPass;
			info.framebuffer = framebuffer;
			info.renderArea.extent.width = cx;
			info.renderArea.extent.height = cy;
			info.clearValueCount = clearValueCount;
			info.pClearValues = pClearValues;

			return info;
		}

		VkFramebuffer createFramebuffer(std::uint32_t cx, std::uint32_t cy, VkRenderPass renderPass, std::uint32_t attachmentCount, VkImageView *pViews)
		{
			VkFramebuffer framebuffer;

			VkFramebufferCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			info.width = cx;
			info.height = cy;
			info.layers = 1;
			info.renderPass = renderPass;
			info.attachmentCount = attachmentCount;
			info.pAttachments = pViews;

			EnterCriticalSection(&cs);
			auto res = vkCreateFramebuffer(device, &info, nullptr, &framebuffer);
			assert(res == VK_SUCCESS);
			LeaveCriticalSection(&cs);

			return framebuffer;
		}

		void destroyFramebuffer(VkFramebuffer fb)
		{
			EnterCriticalSection(&cs);
			vkDestroyFramebuffer(device, fb, nullptr);
			LeaveCriticalSection(&cs);
		}

		VkFormat createSwapchain(HWND hWnd, int cx, int cy, VkSurfaceKHR &surface, VkSwapchainKHR &swapchain, VkImage *pImages)
		{
			VkResult res;

			EnterCriticalSection(&cs);

			VkWin32SurfaceCreateInfoKHR surfaceInfo = {};
			surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
			surfaceInfo.hinstance = hInst;
			surfaceInfo.hwnd = hWnd;
			res = vkCreateWin32SurfaceKHR(inst, &surfaceInfo, nullptr, &surface);
			assert(res == VK_SUCCESS);

			VkSurfaceCapabilitiesKHR surfaceCapabilities;
			res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);
			assert(res == VK_SUCCESS);

			VkBool32 support;
			res = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, presentQueueIndex, surface, &support);
			assert(res == VK_SUCCESS);

			uint32_t formatCount;
			res = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
			assert(res == VK_SUCCESS);
			std::vector<VkSurfaceFormatKHR> formats;
			formats.resize(formatCount);
			res = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data());
			assert(res == VK_SUCCESS);

			VkSwapchainCreateInfoKHR swapchainInfo = {};
			swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			swapchainInfo.surface = surface;
			swapchainInfo.minImageCount = 2;
			swapchainInfo.imageFormat = swapchainFormat;
			swapchainInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
			swapchainInfo.imageExtent.width = cx;
			swapchainInfo.imageExtent.height = cy;
			swapchainInfo.imageArrayLayers = 1;
			swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			swapchainInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
			swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			swapchainInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
			swapchainInfo.oldSwapchain = VK_NULL_HANDLE;
			swapchainInfo.clipped = true;
			res = vkCreateSwapchainKHR(device, &swapchainInfo, nullptr, &swapchain);
			assert(res == VK_SUCCESS);

			uint32_t swapchainImageCount = 0;
			vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, nullptr);
			vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, pImages);

			LeaveCriticalSection(&cs);

			return swapchainFormat;
		}

		VkSemaphore createSemaphore()
		{
			VkSemaphore semaphore;

			VkSemaphoreCreateInfo semaphoreInfo = {};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			EnterCriticalSection(&cs);
			auto res = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &semaphore);
			assert(res == VK_SUCCESS);
			LeaveCriticalSection(&cs);

			return semaphore;
		}

		std::uint32_t acquireNextImage(VkSwapchainKHR swapchain, VkSemaphore semaphore)
		{
			std::uint32_t index;

			EnterCriticalSection(&cs);
			auto res = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, semaphore, VK_NULL_HANDLE, &index);
			assert(res == VK_SUCCESS);
			LeaveCriticalSection(&cs);

			return index;
		}

		void queuePresent(VkPresentInfoKHR *pInfo)
		{
			EnterCriticalSection(&cs);
			auto res = vkQueuePresentKHR(graphicsQueue, pInfo);
			assert(res == VK_SUCCESS);
			LeaveCriticalSection(&cs);
		}

		static PFN_vkCreateDebugReportCallbackEXT  _my_vkCreateDebugReportCallbackEXT;
		static PFN_vkDebugReportMessageEXT         _my_vkDebugReportMessageEXT;
		static PFN_vkDestroyDebugReportCallbackEXT _my_vkDestroyDebugReportCallbackEXT;

		static VKAPI_ATTR VkBool32 VKAPI_CALL _vkDebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, 
			int32_t messageCode, 
			const char* pLayerPrefix,
			const char* pMessage,
			void* pUserData)
		{
			if (messageCode == 1) return VK_FALSE; // THREADING ERROR, 0.0 what is this
			if (messageCode == 2) return VK_FALSE; // Vertex attribute not consumed by vertex shader, never mind
			if (messageCode == 6) return VK_FALSE; // Image layout should be attachment optimal but got general, never mind
			if (messageCode == 8) return VK_FALSE; // Your fucking computer is not support anisotropy, never mind
			if (messageCode == 13) return VK_FALSE; // Shader expects at least n descriptors but only less provided, never mind
			if (messageCode == 15) return VK_FALSE; // Shader requires VkPhysicalDeviceFeatures::tessellationShader but is not enabled on the device, never mind
			if (messageCode == 53) return VK_FALSE; // You have gave more clear values, never mind
			if (messageCode == 61) return VK_FALSE; // Some descriptor maybe used before any update, never mind

			if (messageCode == 24) return VK_FALSE; // Vertex buffers are bound to command buffer but no vertex buffers are attached to this Pipeline State Object.
			if (messageCode == 59) return VK_FALSE; // Descriptor set encountered the following validation error at vkCmdDrawIndexed() time: Descriptor is being used in draw but has not been updated.
			if (messageCode == 63) return VK_FALSE; // vkBeginCommandBuffer(): Secondary Command Buffers may perform better if a valid framebuffer parameter is specified.
			if (messageCode == 14) return VK_FALSE;
			if (messageCode == 12) return VK_FALSE; // Push constant range covering variable starting at offset not accessible from stage
			return VK_FALSE;
		}

		Err init(const char *appName, bool debug)
		{
			static bool first = true;
			if (!first) return Err::eNoErr;
			first = false;

			InitializeCriticalSection(&cs);

			VkResult res;

			VkApplicationInfo appInfo = {};
			appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			appInfo.pApplicationName = appName;
			appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
			appInfo.pEngineName = "TK Engine";
			appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
			appInfo.apiVersion = VK_API_VERSION_1_0;

			std::vector<const char*> instLayers;
			if (debug) instLayers.push_back("VK_LAYER_LUNARG_standard_validation");

			std::vector<const char*> instExtensions;
			instExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
			instExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
			if (debug) instExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
			VkInstanceCreateInfo instInfo = {};
			instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			instInfo.pApplicationInfo = &appInfo;
			instInfo.enabledExtensionCount = instExtensions.size();
			instInfo.ppEnabledExtensionNames = instExtensions.data();
			instInfo.enabledLayerCount = instLayers.size();
			instInfo.ppEnabledLayerNames = instLayers.data();
			res = vkCreateInstance(&instInfo, nullptr, &inst);
			if (res != VkResult::VK_SUCCESS) { return Err::eContextLost; }

			if (debug)
			{
				_my_vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(inst, "vkCreateDebugReportCallbackEXT"));
				_my_vkDebugReportMessageEXT = reinterpret_cast<PFN_vkDebugReportMessageEXT>(vkGetInstanceProcAddr(inst, "vkDebugReportMessageEXT"));
				_my_vkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(inst, "vkDestroyDebugReportCallbackEXT"));

				VkDebugReportCallbackCreateInfoEXT callbackCreateInfo;
				callbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
				callbackCreateInfo.pNext = nullptr;
				callbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
					VK_DEBUG_REPORT_WARNING_BIT_EXT |
					VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
				callbackCreateInfo.pfnCallback = &_vkDebugCallback;
				callbackCreateInfo.pUserData = nullptr;

				VkDebugReportCallbackEXT callback;
				res = _my_vkCreateDebugReportCallbackEXT(inst, &callbackCreateInfo, nullptr, &callback);
				assert(res == VK_SUCCESS);
			}

			uint32_t gpuCount = 1;
			res = vkEnumeratePhysicalDevices(inst, &gpuCount, &physicalDevice);
			if (res != VkResult::VK_SUCCESS) { return Err::eContextLost; }

			VkPhysicalDeviceProperties prop;
			vkGetPhysicalDeviceProperties(physicalDevice, &prop);

			uint32_t queueFamilyCount;
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
			if (queueFamilyCount < 1) { return Err::eContextLost; }
			queueFamilyProps.resize(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProps.data());

			VkSurfaceKHR surface;
			VkWin32SurfaceCreateInfoKHR surfaceInfo = {};
			surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
			surfaceInfo.hinstance = hInst;
			res = vkCreateWin32SurfaceKHR(inst, &surfaceInfo, nullptr, &surface);
			if (res != VkResult::VK_SUCCESS) { return Err::eContextLost; }

			graphicsQueueIndex = UINT32_MAX;
			presentQueueIndex = UINT32_MAX;
			transferQueueIndex = UINT32_MAX;

			auto pSupportsPresent = (VkBool32 *)malloc(queueFamilyCount * sizeof(VkBool32));
			for (uint32_t i = 0; i < queueFamilyCount; i++)
			{
				res = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &pSupportsPresent[i]);
				assert(res == VK_SUCCESS);
			}

			vkDestroySurfaceKHR(inst, surface, nullptr);

			for (uint32_t i = 0; i < queueFamilyCount; ++i)
			{
				if ((queueFamilyProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
				{
					if (graphicsQueueIndex == UINT32_MAX)
						graphicsQueueIndex = i;
					if (pSupportsPresent[i] == VK_TRUE)
					{
						graphicsQueueIndex = i;
						presentQueueIndex = i;
						break;
					}
				}
			}
			if (presentQueueIndex == UINT32_MAX)
			{
				for (uint32_t i = 0; i < queueFamilyCount; ++i)
				{
					if (pSupportsPresent[i] == VK_TRUE)
					{
						presentQueueIndex = i;
						break;
					}
				}
			}
			for (uint32_t i = 0; i < queueFamilyCount; ++i)
			{
				if ((queueFamilyProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0 && (queueFamilyProps[i].queueFlags & VK_QUEUE_TRANSFER_BIT) != 0)
				{
					transferQueueIndex = i;
					break;
				}
			}
			free(pSupportsPresent);

			if (graphicsQueueIndex == UINT32_MAX || presentQueueIndex == UINT32_MAX) { return Err::eContextLost; }

			float queuePriorities[1] = { 0.0 };
			VkDeviceQueueCreateInfo queueInfo = {};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = graphicsQueueIndex;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = queuePriorities;

			std::vector<VkDeviceQueueCreateInfo> queueInfos;
			queueInfos.push_back(queueInfo);

			if (presentQueueIndex != graphicsQueueIndex)
			{
				queueInfo.queueFamilyIndex = presentQueueIndex;
				queueInfos.push_back(queueInfo);
			}

			if (transferQueueIndex != graphicsQueueIndex)
			{
				queueInfo.queueFamilyIndex = transferQueueIndex;
				queueInfos.push_back(queueInfo);
			}

			std::vector<char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
			VkDeviceCreateInfo deviceInfo = {};
			deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			deviceInfo.pQueueCreateInfos = queueInfos.data();
			deviceInfo.queueCreateInfoCount = queueInfos.size();
			deviceInfo.enabledExtensionCount = deviceExtensions.size();
			deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();
			res = vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &device);
			assert(res == VK_SUCCESS);

			vkGetDeviceQueue(device, graphicsQueueIndex, 0, &graphicsQueue);
			//vkGetDeviceQueue(device, presentQueueIndex, 0, &presentQueue);
			//vkGetDeviceQueue(device, transferQueueIndex, 0, &transferQueue);

			vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

			VkCommandPoolCreateInfo cmdPoolInfo = {};
			cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			cmdPoolInfo.queueFamilyIndex = 0;
			res = vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &commandPool);
			assert(res == VK_SUCCESS);

			VkDescriptorPoolSize descriptorPoolSizes[4];
			descriptorPoolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorPoolSizes[0].descriptorCount = 10000;
			descriptorPoolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			descriptorPoolSizes[1].descriptorCount = 10000;
			descriptorPoolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			descriptorPoolSizes[2].descriptorCount = 10000;
			descriptorPoolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorPoolSizes[3].descriptorCount = 10000;

			VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
			descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			descriptorPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			descriptorPoolInfo.poolSizeCount = 4;
			descriptorPoolInfo.pPoolSizes = descriptorPoolSizes;
			descriptorPoolInfo.maxSets = 256;
			res = vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool);
			assert(res == VK_SUCCESS);

			swapchainFormat = VK_FORMAT_B8G8R8A8_UNORM;

			{
				VkSamplerCreateInfo info = {};
				info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
				info.magFilter = VK_FILTER_LINEAR;
				info.minFilter = VK_FILTER_LINEAR;
				info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				info.anisotropyEnable = VK_TRUE;
				info.maxAnisotropy = 16;
				info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
				info.unnormalizedCoordinates = VK_FALSE;
				info.compareEnable = VK_FALSE;
				info.compareOp = VK_COMPARE_OP_ALWAYS;
				info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
				info.mipLodBias = 0.0f;
				info.minLod = 0.f;
				info.maxLod = 128.f;

				EnterCriticalSection(&cs);
				auto res = vkCreateSampler(device, &info, nullptr, &_colorSampler);
				assert(res == VK_SUCCESS);
				LeaveCriticalSection(&cs);
			}

			{
				VkSamplerCreateInfo info = {};
				info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
				info.magFilter = VK_FILTER_LINEAR;
				info.minFilter = VK_FILTER_LINEAR;
				info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
				info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
				info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
				info.anisotropyEnable = VK_TRUE;
				info.maxAnisotropy = 16;
				info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
				info.unnormalizedCoordinates = VK_FALSE;
				info.compareEnable = VK_FALSE;
				info.compareOp = VK_COMPARE_OP_ALWAYS;
				info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
				info.mipLodBias = 0.0f;
				info.minLod = 0.f;
				info.maxLod = 128.f;

				EnterCriticalSection(&cs);
				auto res = vkCreateSampler(device, &info, nullptr, &_colorBorderSampler);
				assert(res == VK_SUCCESS);
				LeaveCriticalSection(&cs);
			}

			{
				VkSamplerCreateInfo info = {};
				info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
				info.magFilter = VK_FILTER_NEAREST;
				info.minFilter = VK_FILTER_NEAREST;
				info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				info.anisotropyEnable = VK_FALSE;
				info.maxAnisotropy = 0;
				info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
				info.unnormalizedCoordinates = VK_TRUE;
				info.compareEnable = VK_FALSE;
				info.compareOp = VK_COMPARE_OP_ALWAYS;
				info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
				info.mipLodBias = 0.0f;
				info.minLod = 0.0f;
				info.maxLod = 0.0f;

				EnterCriticalSection(&cs);
				auto res = vkCreateSampler(device, &info, nullptr, &_screenUvSampler);
				assert(res == VK_SUCCESS);
				LeaveCriticalSection(&cs);
			}

			{
				VkSamplerCreateInfo samplerInfo = {};
				samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
				samplerInfo.magFilter = VK_FILTER_NEAREST;
				samplerInfo.minFilter = VK_FILTER_NEAREST;
				samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				samplerInfo.anisotropyEnable = VK_FALSE;
				samplerInfo.maxAnisotropy = 0;
				samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
				samplerInfo.unnormalizedCoordinates = VK_FALSE;
				samplerInfo.compareEnable = VK_FALSE;
				samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
				samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
				samplerInfo.mipLodBias = 0.0f;
				samplerInfo.minLod = 0.0f;
				samplerInfo.maxLod = 0.0f;

				EnterCriticalSection(&cs);
				auto res = vkCreateSampler(device, &samplerInfo, nullptr, &_screenSampler);
				assert(res == VK_SUCCESS);
				LeaveCriticalSection(&cs);
			}

			return Err::eNoErr;
		}
	}
}
