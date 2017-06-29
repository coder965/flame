#include "transformer_tool.h"

void TransformerTool::show(VkCommandBuffer cmd, VkEvent waitEvent, VkEvent signalEvent)
{
	vkResetCommandBuffer(cmd, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
	tke::beginCommandBuffer(cmd);

	vkCmdWaitEvents(cmd, 1, &waitEvent, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, nullptr, 0, nullptr, 0, nullptr);



	vkCmdResetEvent(cmd, waitEvent, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
	vkCmdSetEvent(cmd, signalEvent, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
}