#ifndef RENDER_PASS_H
#define RENDER_PASS_H
#include <SVL/definitions.h>

#include <vulkan/vulkan.h>
#include <vector>

namespace SVL
{
	namespace init
	{
		struct RenderPassInit
		{
			std::vector<VkAttachmentDescription> attachments;
			std::vector<VkSubpassDescription> subpasses;
			std::vector<VkSubpassDependency> dependencies;
		};
	};

	class Renderer;
	class DLLDIR RenderPass
	{
	public:
		RenderPass() = delete;

		RenderPass(const RenderPass&) = delete;
		RenderPass& operator=(const RenderPass&) = delete;
		RenderPass(RenderPass&&) = delete;
		RenderPass& operator=(RenderPass&&) = delete;

		RenderPass(const Renderer& r, SVL::init::RenderPassInit init);
		RenderPass(const Renderer& r, std::vector<VkAttachmentDescription> attachments, std::vector<VkSubpassDescription> subpasses, std::vector<VkSubpassDependency> dependencies);
		~RenderPass();

		const VkRenderPass& operator ()() const
		{
			return vk_render_pass;
		}
	private:
		const class Renderer& vk_renderer;
		VkRenderPass vk_render_pass;
	};
}

#endif // !RENDER_PASS_H