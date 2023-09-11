#include "pipeline.h"

#include "renderer.h"
#include <SVL/common/ErrorHandler.h>

void SVL::init::PipelineInit::update()
{
	vertex_input.vertexBindingDescriptionCount = vertex_bindings.size();
	vertex_input.pVertexBindingDescriptions = vertex_bindings.data();
	vertex_input.vertexAttributeDescriptionCount = vertex_attributes.size();
	vertex_input.pVertexAttributeDescriptions = vertex_attributes.data();

	viewport.viewportCount = viewports.size();
	viewport.pViewports = viewports.data();
	viewport.scissorCount = scissors.size();
	viewport.pScissors = scissors.data();

	color_blend.attachmentCount = color_blend_attachment_states.size();
	color_blend.pAttachments = color_blend_attachment_states.data();

	dynamic_state.dynamicStateCount = dynamic_states.size();
	dynamic_state.pDynamicStates = dynamic_states.data();
}

SVL::Pipeline::Pipeline(const Renderer& r, VkRenderPass render_pass, VkPipelineLayout pipeline_layout, SVL::init::PipelineInit init)
	: vk_renderer(r)
{
	init.update();

	VkGraphicsPipelineCreateInfo pipeline_info = {};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.stageCount = init.stages.size();
	pipeline_info.pStages = init.stages.data();
	pipeline_info.pVertexInputState = &init.vertex_input;
	pipeline_info.pInputAssemblyState = &init.input_assembly;
	pipeline_info.pTessellationState = &init.tessellation;
	pipeline_info.pViewportState = &init.viewport;
	pipeline_info.pRasterizationState = &init.rasterization;
	pipeline_info.pMultisampleState = &init.multisample;
	pipeline_info.pColorBlendState = &init.color_blend;
	pipeline_info.pDepthStencilState = &init.depth_stencil;
	pipeline_info.pDynamicState = &init.dynamic_state;
	pipeline_info.layout = pipeline_layout;
	pipeline_info.renderPass = render_pass;
	pipeline_info.subpass = 0;
	pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

	ErrorCheck(vkCreateGraphicsPipelines(vk_renderer.device(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &vk_pipeline));
}

SVL::Pipeline::~Pipeline()
{
	vkDestroyPipeline(vk_renderer.device(), vk_pipeline, nullptr);
}