#ifndef WINDOW_H
#define WINDOW_H

#include <SVL/definitions.h>
#include <vulkan/vulkan.h>

#include <vector>
#include <functional>

#include "command.h"
#include "image.h"
#include "render_pass.h"

namespace SVL
{
	enum AntiAliasing
	{
		None,
		MSAA_2, MSAA_4, MSAA_8, MSAA_16, MSAA_32, MSAA_64
	};

	class Renderer;
	class DLLDIR Window : public PrimCommand
	{
	public:
		Window(const Renderer& renderer, std::function<VkSurfaceKHR()> surface_create_fnc, uint32_t width, uint32_t height);
		~Window();

		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;
		Window(Window&&) = delete;
		Window& operator=(Window&&) = delete;

		void update();
		void draw();

		void add_command(SecCommand*);
		void del_command(SecCommand*);

		void set_extent(uint32_t width, uint32_t height);
		void set_msaa(AntiAliasing aa);

		const Renderer& renderer() const { return vk_renderer; }
		const VkExtent2D extent() const { return vk_extent; }
		const VkSurfaceFormatKHR surface_format() const { return vk_surface_format; }
		const uint32_t image_count() const { return vk_swapchain_image_count; }
		const VkRenderPass& render_pass() const { return (*vk_render_pass)(); }
		const std::vector<VkFramebuffer>* framebuffers() const { return &vk_framebuffers; }
		
		const VkSampleCountFlagBits get_sample_count() const;
	protected:
		virtual void pre_draw() {}
		virtual void post_draw() {}
		virtual void pre_update_command_buffers(VkCommandBuffer, VkCommandBufferInheritanceInfo inheritanceInfo, uint32_t index) {}
		virtual void post_update_command_buffers(VkCommandBuffer, VkCommandBufferInheritanceInfo inheritanceInfo, uint32_t index) {}
	private:
		const class Renderer& vk_renderer;
		VkExtent2D vk_extent;

		VkSurfaceKHR vk_surface;
		VkBool32 WSI_supported = false;
		VkSurfaceFormatKHR vk_surface_format;
		VkSurfaceCapabilitiesKHR vk_surface_capabilities;

		VkSwapchainKHR vk_swapchain{};
		uint32_t vk_swapchain_image_count = NUM_SWAPCHAIN_IMAGE;
		std::vector<VkImage> vk_swapchain_images;
		std::vector<VkImageView> vk_swapchain_image_views;

		ImageView depth;

		ImageView multisample_color, multisample_depth;
		
		AntiAliasing aa = AntiAliasing::None;

		RenderPass* vk_render_pass;

		std::vector<VkFramebuffer> vk_framebuffers;

		VkSemaphore vk_image_available_semaphore;
		VkSemaphore vk_render_finished_semaphore;
		VkFence vk_fence;

		std::vector<SecCommand*> vk_sec_command_buffers;
			

		void create_surface();
		const std::function<VkSurfaceKHR()> create_vk_surface;
		void destroy_surface();

		void create_swapchain();
		void destroy_swapchain();

		void create_depth_resources();
		void destroy_depth_resources();

		void create_resolve_resources();
		void destroy_resolve_resources();

		void create_renderpass();
		void destroy_renderpass();

		void create_framebuffers();
		void destroy_framebuffers();

		void create_semaphores();
		void destroy_semaphores();

		void update_command_buffers(uint32_t index);
	};
}
#endif // !window_h
