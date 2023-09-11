#include "render_pass.h"

#include "renderer.h"
#include <SVL/common/ErrorHandler.h>

SVL::RenderPass::RenderPass(const Renderer& r, SVL::init::RenderPassInit init)
	: RenderPass(r, init.attachments, init.subpasses, init.dependencies)
{
}

SVL::RenderPass::RenderPass(const Renderer& r, std::vector<VkAttachmentDescription> attachments, std::vector<VkSubpassDescription> subpasses, std::vector<VkSubpassDependency> dependencies)
	: vk_renderer(r)
{
	VkRenderPassCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	ci.attachmentCount = attachments.size();
	ci.pAttachments = attachments.data();
	ci.subpassCount = subpasses.size();
	ci.pSubpasses = subpasses.data();
	ci.dependencyCount = dependencies.size();
	ci.pDependencies = dependencies.data();

	ErrorCheck(vkCreateRenderPass(vk_renderer.device(), &ci, nullptr, &vk_render_pass));
}

SVL::RenderPass::~RenderPass()
{
	vkDestroyRenderPass(vk_renderer.device(), vk_render_pass, nullptr);
}

