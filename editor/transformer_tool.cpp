#include "..\src\core.h"

#include "transformer_tool.h"

void TransformerTool::show(VkCommandBuffer cmd, VkEvent waitEvent, VkEvent signalEvent, tke::Framebuffer *fb)
{
	vkResetCommandBuffer(cmd, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
	tke::beginCommandBuffer(cmd);

	tke::waitEvent(cmd, waitEvent);

	tke::beginRenderPass(cmd, tke::plainRenderPass_image8, fb);

	tke::staticVertexBuffer->bind(cmd);
	tke::staticIndexBuffer->bind(cmd);


	vkCmdEndRenderPass(cmd);

	tke::resetEvent(cmd, waitEvent);
	tke::setEvent(cmd, signalEvent);

	vkEndCommandBuffer(cmd);
}
