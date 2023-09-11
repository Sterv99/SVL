#ifndef PIPELINE_H
#define PIPELINE_H
#include <SVL/definitions.h>

#include <vulkan/vulkan.h>
#include <vector>

namespace SVL
{
	namespace init
	{
		struct PipelineInit
		{
			std::vector<VkPipelineShaderStageCreateInfo> stages{};

			std::vector<VkVertexInputBindingDescription> vertex_bindings{};
			std::vector<VkVertexInputAttributeDescription> vertex_attributes{};
			VkPipelineVertexInputStateCreateInfo vertex_input
			{
				VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
				0,
				0
			};
			VkPipelineInputAssemblyStateCreateInfo input_assembly
			{
				VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
				0,
				0,
				VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
				VK_FALSE
			};
			VkPipelineTessellationStateCreateInfo tessellation
			{

			};
			std::vector<VkViewport> viewports{};
			std::vector<VkRect2D> scissors{};
			VkPipelineViewportStateCreateInfo viewport
			{
				VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
				0,
				0
			};
			VkPipelineRasterizationStateCreateInfo rasterization
			{
				VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
				0,
				0,
				VK_FALSE,
				VK_FALSE,
				VK_POLYGON_MODE_FILL,
				VK_CULL_MODE_NONE,
				VK_FRONT_FACE_COUNTER_CLOCKWISE,
				VK_FALSE,
				0.0f,
				0.0f,
				0.0f,
				1.0f
			};
			VkPipelineMultisampleStateCreateInfo multisample
			{
				VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
				0,
				0
			};
			VkPipelineDepthStencilStateCreateInfo depth_stencil
			{
				VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
				0,
				0,
				VK_TRUE,
				VK_TRUE,
				VK_COMPARE_OP_LESS
			};
			std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachment_states{};
			VkPipelineColorBlendStateCreateInfo color_blend
			{
				VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
				0,
				0,
				VK_FALSE,
				VK_LOGIC_OP_CLEAR
			};
			std::vector<VkDynamicState> dynamic_states{};
			VkPipelineDynamicStateCreateInfo dynamic_state
			{
				VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
				0,
				0
			};

			void update();
		};
	};

	class Renderer;
	class DLLDIR Pipeline
	{
	public:
		Pipeline() = delete;

		Pipeline(const Pipeline&) = delete;
		Pipeline& operator=(const Pipeline&) = delete;
		Pipeline(Pipeline&&) = delete;
		Pipeline& operator=(Pipeline&&) = delete;

		Pipeline(const Renderer& r, VkRenderPass render_pass, VkPipelineLayout pipeline_layout, SVL::init::PipelineInit init);
		~Pipeline();

		const VkPipeline& operator ()() const
		{
			return vk_pipeline;
		}
	private:
		const class Renderer& vk_renderer;
		VkPipeline vk_pipeline;
	};
}

#endif // !PIPELINE_H