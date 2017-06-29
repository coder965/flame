#include "..\src\core.h"

#include "transformer_tool.h"

void TransformerTool::show(VkCommandBuffer cmd, VkEvent waitEvent, VkEvent signalEvent, VkFramebuffer fb)
{
	vkResetCommandBuffer(cmd, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
	tke::beginCommandBuffer(cmd);

	vkCmdWaitEvents(cmd, 1, &waitEvent, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, nullptr, 0, nullptr, 0, nullptr);

	vkCmdBeginRenderPass(cmd, &tke::renderPassBeginInfo(tke::plainRenderPass_image8, fb, tke::resCx, tke::resCy, 0, nullptr), VK_SUBPASS_CONTENTS_INLINE);

	tke::staticVertexBuffer->bind(cmd);
	tke::staticIndexBuffer->bind(cmd);

	vkCmdEndRenderPass(cmd);

	vkCmdResetEvent(cmd, waitEvent, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
	vkCmdSetEvent(cmd, signalEvent, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

	vkEndCommandBuffer(cmd);
}
