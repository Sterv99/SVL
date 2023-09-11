#include "window.h"
#include "renderer.h"
#include "tools.h"

#include <SVL/common/ErrorHandler.h>
#include <array>


SVL::Window::Window(const Renderer& r, std::function<VkSurfaceKHR()> create_surface_fnc, uint32_t width, uint32_t height)
	: PrimCommand(r), vk_renderer(r), vk_extent({ width, height }), create_vk_surface(create_surface_fnc), depth(r), multisample_color(r), multisample_depth(r)
{
	if(width == 0 || height == 0)
		Error("invalid resolution");
	create_surface();
	create_swapchain();
	create_depth_resources();
	create_resolve_resources();
	create_renderpass();
	create_framebuffers();
	create_command_buffers(vk_swapchain_image_count);
	create_semaphores();
}

SVL::Window::~Window()
{
	destroy_semaphores();
	destroy_command_buffers();
	destroy_framebuffers();
	destroy_renderpass();
	destroy_resolve_resources();
	destroy_depth_resources();
	destroy_swapchain();
	destroy_surface();
}

void SVL::Window::update()
{
	if(vk_extent.width == 0 || vk_extent.height == 0)
		return;
	
	ErrorCheck(vkQueueWaitIdle(vk_renderer.queue()));

	destroy_command_buffers();
	destroy_framebuffers();
	destroy_renderpass();
	destroy_resolve_resources();
	destroy_depth_resources();
	destroy_swapchain();
	destroy_surface();

	create_surface();
	create_swapchain();
	create_depth_resources();
	create_resolve_resources();
	create_renderpass();
	create_framebuffers();
	for(SecCommand* c : vk_sec_command_buffers)
	{
		c->update();
	}
	create_command_buffers(vk_swapchain_image_count);
}


void SVL::Window::draw()
{
	if (vk_extent.width == 0 || vk_extent.height == 0)
		return;
	//if (window_layers.size() == 0) return;

	pre_draw();

	uint32_t image_index;
	VkResult result = vkAcquireNextImageKHR(vk_renderer.device(), vk_swapchain, UINT64_MAX, vk_image_available_semaphore, VK_NULL_HANDLE, &image_index);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		update();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		Error("SVL ERROR: failed to acquire swapchain image.");
	}

	update_command_buffers(image_index);

	VkSubmitInfo main_submit{};
	main_submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkPipelineStageFlags wait_stages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	main_submit.waitSemaphoreCount = 1;
	main_submit.pWaitSemaphores = &vk_image_available_semaphore;
	main_submit.pWaitDstStageMask = &wait_stages;
	main_submit.commandBufferCount = 1;
	main_submit.pCommandBuffers = &vk_command_buffers[image_index];

	main_submit.signalSemaphoreCount = 1;
	main_submit.pSignalSemaphores = &vk_render_finished_semaphore;

	ErrorCheck(vkQueueSubmit(vk_renderer.queue(), 1, &main_submit, vk_fence));

	VkResult fence_result;
	do
	{
		fence_result = vkWaitForFences(vk_renderer.device(), 1, &vk_fence, VK_TRUE, UINT64_MAX);
	} while (fence_result == VK_TIMEOUT);
	vkResetFences(vk_renderer.device(), 1, &vk_fence);

	VkPresentInfoKHR present_info{};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = &vk_render_finished_semaphore;
	present_info.swapchainCount = 1;
	present_info.pSwapchains = &vk_swapchain;
	present_info.pImageIndices = &image_index;

	result = vkQueuePresentKHR(vk_renderer.queue(), &present_info);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		update();
	}
	else if (result != VK_SUCCESS)
	{
		Error("SVL ERROR: failed to present swapchain image.");
	}

	post_draw();
}


void SVL::Window::set_extent(uint32_t width, uint32_t height)
{
	vk_extent = {width, height};
	update();
}
void SVL::Window::set_msaa(AntiAliasing aa)
{
	this->aa = aa;
	this->update();
}

void SVL::Window::add_command(SecCommand* com)
{
	vk_sec_command_buffers.push_back(com);
}

void SVL::Window::del_command(SecCommand* com)
{
	vk_sec_command_buffers.erase(std::remove(vk_sec_command_buffers.begin(), vk_sec_command_buffers.end(), com), vk_sec_command_buffers.end());
}

void SVL::Window::create_surface()
{
	//surface
	vk_surface = create_vk_surface();
	//wsi
	ErrorCheck(vkGetPhysicalDeviceSurfaceSupportKHR(vk_renderer.physical_device(), vk_renderer.graphics_family_index(), vk_surface, &WSI_supported));
	if (!WSI_supported)
	{
		Error("SVL ERROR: WSI is not supported.");
	}
	//surface_formats
	uint32_t format_count = 0;
	ErrorCheck(vkGetPhysicalDeviceSurfaceFormatsKHR(vk_renderer.physical_device(), vk_surface, &format_count, nullptr));
	if (format_count == 0)
	{
		Error("SVL ERROR: Surface formats missing.");
	}
	std::vector<VkSurfaceFormatKHR> formats(format_count);
	ErrorCheck(vkGetPhysicalDeviceSurfaceFormatsKHR(vk_renderer.physical_device(), vk_surface, &format_count, formats.data()));
	if (formats[0].format == VK_FORMAT_UNDEFINED)
	{
		vk_surface_format.format = VK_FORMAT_B8G8R8A8_UNORM;
		vk_surface_format.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	}
	else
	{
		vk_surface_format = formats[0];
	}
}
void SVL::Window::destroy_surface()
{
	vkDestroySurfaceKHR(vk_renderer.instance(), vk_surface, nullptr);
}

void SVL::Window::create_swapchain()
{
	//surface_capabilities
	ErrorCheck(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_renderer.physical_device(), vk_surface, &vk_surface_capabilities));
	if (vk_surface_capabilities.currentExtent.width < UINT32_MAX)
	{
		vk_extent.width = vk_surface_capabilities.currentExtent.width;
		vk_extent.height = vk_surface_capabilities.currentExtent.height;
	}
	//swapchain
	VkSwapchainKHR new_swapchain;
	if (vk_swapchain_image_count < vk_surface_capabilities.minImageCount + 1) vk_swapchain_image_count = vk_surface_capabilities.minImageCount + 1;
	if (vk_surface_capabilities.maxImageCount > 0)
	{
		if (vk_swapchain_image_count > vk_surface_capabilities.maxImageCount) vk_swapchain_image_count = vk_surface_capabilities.maxImageCount;
	}

	VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
	uint32_t present_mode_count = 0;
	ErrorCheck(vkGetPhysicalDeviceSurfacePresentModesKHR(vk_renderer.physical_device(), vk_surface, &present_mode_count, nullptr));
	std::vector<VkPresentModeKHR> present_mode_list(present_mode_count);
	ErrorCheck(vkGetPhysicalDeviceSurfacePresentModesKHR(vk_renderer.physical_device(), vk_surface, &present_mode_count, present_mode_list.data()));

	for (auto m : present_mode_list)
	{
		if (m == VK_PRESENT_MODE_MAILBOX_KHR) present_mode = m;
	}
	VkSwapchainCreateInfoKHR swapchain_create_info{};
	swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_create_info.surface = vk_surface;
	swapchain_create_info.minImageCount = vk_swapchain_image_count;
	swapchain_create_info.imageFormat = vk_surface_format.format;
	swapchain_create_info.imageColorSpace = vk_surface_format.colorSpace;
	swapchain_create_info.imageExtent = vk_extent;
	swapchain_create_info.imageArrayLayers = 1;
	swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchain_create_info.queueFamilyIndexCount = 0;
	swapchain_create_info.pQueueFamilyIndices = nullptr;
	swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchain_create_info.presentMode = present_mode;
	swapchain_create_info.clipped = VK_TRUE;
	//swapchain_create_info.oldSwapchain = vk_swapchain;

	ErrorCheck(vkCreateSwapchainKHR(vk_renderer.device(), &swapchain_create_info, nullptr, &new_swapchain));
	ErrorCheck(vkGetSwapchainImagesKHR(vk_renderer.device(), new_swapchain, &vk_swapchain_image_count, nullptr));
	//swapchain images
	vk_swapchain_images.resize(vk_swapchain_image_count);
	vk_swapchain_image_views.resize(vk_swapchain_image_count);

	ErrorCheck(vkGetSwapchainImagesKHR(vk_renderer.device(), new_swapchain, &vk_swapchain_image_count, vk_swapchain_images.data()));
	for (uint32_t i = 0; i < vk_swapchain_image_count; i++)
	{
		vk_swapchain_image_views[i] = SVLTools::create_2D_image_view(vk_renderer.device(), vk_swapchain_images[i], vk_surface_format.format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}
	this->vk_swapchain = new_swapchain;
}
void SVL::Window::destroy_swapchain()
{
	for (uint32_t i = 0; i < vk_swapchain_image_views.size(); i++)
	{
		vkDestroyImageView(vk_renderer.device(), vk_swapchain_image_views[i], nullptr);
	}
	vk_swapchain_image_views.clear();
	vk_swapchain_images.clear();
	vkDestroySwapchainKHR(vk_renderer.device(), vk_swapchain, nullptr);
}

void SVL::Window::create_depth_resources()
{
	VkFormat depth_format = SVLTools::find_supported_format(vk_renderer.physical_device(), { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

	depth.create_2D_image(vk_extent, depth_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_SAMPLE_COUNT_1_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_LAYOUT_UNDEFINED, 1, 1);
	depth.transition_image_layout(vk_command_pool, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);
	depth.create_2D_image_view(VK_IMAGE_ASPECT_DEPTH_BIT);
}
void SVL::Window::destroy_depth_resources()
{
	depth.destroy();
}

void SVL::Window::create_resolve_resources()
{
	multisample_color.create_2D_image(vk_extent, vk_surface_format.format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, get_sample_count(), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_LAYOUT_UNDEFINED, 1, 1);
	//multisample_color.transition_image_layout(vk_command_pool, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	multisample_color.create_2D_image_view(VK_IMAGE_ASPECT_COLOR_BIT);

	VkFormat depth_format = SVLTools::find_supported_format(vk_renderer.physical_device(), { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

	multisample_depth.create_2D_image(vk_extent, depth_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, get_sample_count(), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_LAYOUT_UNDEFINED, 1, 1);
	//multisample_depth.transition_image_layout(vk_command_pool, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	multisample_depth.create_2D_image_view(VK_IMAGE_ASPECT_DEPTH_BIT);
}
void SVL::Window::destroy_resolve_resources()
{
	multisample_depth.destroy();
	multisample_color.destroy();
}

void SVL::Window::create_renderpass()
{
	if (aa != None)
	{
		std::vector<VkAttachmentDescription> attachments
		{
			{
				0,
				vk_surface_format.format,
				get_sample_count(),
				VK_ATTACHMENT_LOAD_OP_CLEAR,
				VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
			},
			{
				0,
				vk_surface_format.format,
				VK_SAMPLE_COUNT_1_BIT,
				VK_ATTACHMENT_LOAD_OP_CLEAR,
				VK_ATTACHMENT_STORE_OP_STORE,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
			},
			{
				0,
				SVLTools::find_supported_format(vk_renderer.physical_device(), { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT),
				get_sample_count(),
				VK_ATTACHMENT_LOAD_OP_CLEAR,
				VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
			}
		};
		attachments.push_back(
			{
				0,
				attachments[2].format,
				VK_SAMPLE_COUNT_1_BIT,
				VK_ATTACHMENT_LOAD_OP_CLEAR,
				VK_ATTACHMENT_STORE_OP_STORE,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
			}
		);

		VkAttachmentReference color_reference{};
		color_reference.attachment = 0;
		color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depth_reference{};
		depth_reference.attachment = 2;
		depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference resolve_reference{};
		resolve_reference.attachment = 1;
		resolve_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		std::vector<VkSubpassDescription> subpasses
		{
			{
				0,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				0,
				nullptr,
				1,
				&color_reference,
				&resolve_reference,
				&depth_reference,
				0,
				nullptr
			}
		};

		std::vector<VkSubpassDependency> subpass_dependencies
		{
			{
				VK_SUBPASS_EXTERNAL,
				0,
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_ACCESS_MEMORY_READ_BIT,
				VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				VK_DEPENDENCY_BY_REGION_BIT
			},
			{
				0,
				VK_SUBPASS_EXTERNAL,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				VK_ACCESS_MEMORY_READ_BIT,
				VK_DEPENDENCY_BY_REGION_BIT
			}
		};

		vk_render_pass = new RenderPass(vk_renderer, attachments, subpasses, subpass_dependencies);
	}
	else
	{
		std::vector<VkAttachmentDescription> attachments
		{
			{
				0,
				vk_surface_format.format,
				VK_SAMPLE_COUNT_1_BIT,
				VK_ATTACHMENT_LOAD_OP_CLEAR,
				VK_ATTACHMENT_STORE_OP_STORE,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
			},
			{
				0, 
				SVLTools::find_supported_format(vk_renderer.physical_device(), { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT),
				VK_SAMPLE_COUNT_1_BIT,
				VK_ATTACHMENT_LOAD_OP_CLEAR,
				VK_ATTACHMENT_STORE_OP_STORE,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
			}
		};

		VkAttachmentReference color_reference{};
		color_reference.attachment = 0;
		color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depth_reference{};
		depth_reference.attachment = 1;
		depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		std::vector<VkSubpassDescription> subpasses
		{
			{
				0,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				0,
				nullptr,
				1,
				&color_reference,
				nullptr,
				&depth_reference,
				0,
				nullptr
			} 
		};

		std::vector<VkSubpassDependency> subpass_dependencies
		{
			{
				VK_SUBPASS_EXTERNAL,
				0,
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_ACCESS_MEMORY_READ_BIT,
				VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				VK_DEPENDENCY_BY_REGION_BIT
			}
		};

		vk_render_pass = new RenderPass(vk_renderer, attachments, subpasses, subpass_dependencies);
	}
}
void SVL::Window::destroy_renderpass()
{
	delete vk_render_pass;
}

void SVL::Window::create_framebuffers()
{
	if (aa != None)
	{
		std::array<VkImageView, 4> attachments;
		attachments[0] = multisample_color.view;
		attachments[2] = multisample_depth.view;
		attachments[3] = depth.view;

		VkFramebufferCreateInfo framebuffer_info = {};
		framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_info.renderPass = (*vk_render_pass)();
		framebuffer_info.attachmentCount = attachments.size();
		framebuffer_info.pAttachments = attachments.data();
		framebuffer_info.width = vk_extent.width;
		framebuffer_info.height = vk_extent.height;
		framebuffer_info.layers = 1;

		vk_framebuffers.resize(vk_swapchain_image_views.size());
		for (size_t i = 0; i < vk_swapchain_image_views.size(); i++)
		{
			attachments[1] = vk_swapchain_image_views[i];

			ErrorCheck(vkCreateFramebuffer(vk_renderer.device(), &framebuffer_info, nullptr, &vk_framebuffers[i]));
		}
	}
	else
	{
		std::array<VkImageView, 2> attachments;
		attachments[1] = depth.view;

		VkFramebufferCreateInfo framebuffer_info = {};
		framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_info.renderPass = (*vk_render_pass)();
		framebuffer_info.attachmentCount = attachments.size();
		framebuffer_info.pAttachments = attachments.data();
		framebuffer_info.width = vk_extent.width;
		framebuffer_info.height = vk_extent.height;
		framebuffer_info.layers = 1;

		vk_framebuffers.resize(vk_swapchain_image_views.size());
		for (size_t i = 0; i < vk_swapchain_image_views.size(); i++)
		{
			attachments[0] = vk_swapchain_image_views[i];

			ErrorCheck(vkCreateFramebuffer(vk_renderer.device(), &framebuffer_info, nullptr, &vk_framebuffers[i]));
		}
	}
}
void SVL::Window::destroy_framebuffers()
{
	for (size_t i = 0; i < vk_framebuffers.size(); i++)
	{
		vkDestroyFramebuffer(vk_renderer.device(), vk_framebuffers[i], nullptr);
	}
	vk_framebuffers.clear();
}

void SVL::Window::update_command_buffers(uint32_t index)
{
	if (vk_extent.width == 0 || vk_extent.height == 0) return;

	VkCommandBufferBeginInfo command_buffer_begin_info{};
	command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	VkRenderPassBeginInfo render_pass_begin_info{};
	render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_begin_info.renderPass = (*vk_render_pass)();
	render_pass_begin_info.renderArea.offset = { 0, 0 };
	render_pass_begin_info.renderArea.extent = vk_extent;

	std::vector<VkClearValue> clear_values{};
	if (aa != None)
	{
		clear_values.resize(4);
		clear_values[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		clear_values[1].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		clear_values[2].depthStencil = { 1.0f, 0 };
		clear_values[3].depthStencil = { 1.0f, 0 };
	}
	else
	{
		clear_values.resize(2);
		clear_values[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		clear_values[1].depthStencil = { 1.0f, 0 };
	}
	render_pass_begin_info.clearValueCount = (uint32_t)clear_values.size();
	render_pass_begin_info.pClearValues = clear_values.data();
	render_pass_begin_info.framebuffer = vk_framebuffers[index];

	ErrorCheck(vkBeginCommandBuffer(vk_command_buffers[index], &command_buffer_begin_info));

	vkCmdBeginRenderPass(vk_command_buffers[index], &render_pass_begin_info, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

	VkCommandBufferInheritanceInfo inheritance_info{};
	inheritance_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	inheritance_info.renderPass = (*vk_render_pass)();
	inheritance_info.framebuffer = vk_framebuffers[index];
	//inheritance_info.framebuffer = VK_NULL_HANDLE;

	pre_update_command_buffers(vk_command_buffers[index], inheritance_info, index);

	std::vector<VkCommandBuffer> secondary_cmd_buffers;
	for(SecCommand* com : vk_sec_command_buffers)
	{
		com->update_command_buffers(inheritance_info, index);
		if(!com->command_buffers().empty())
			secondary_cmd_buffers.push_back(com->command_buffers()[index]);
	}
	vkCmdExecuteCommands(vk_command_buffers[index], secondary_cmd_buffers.size(), secondary_cmd_buffers.data());
	
	post_update_command_buffers(vk_command_buffers[index], inheritance_info, index);

	vkCmdEndRenderPass(vk_command_buffers[index]);

	ErrorCheck(vkEndCommandBuffer(vk_command_buffers[index]));
}

void SVL::Window::create_semaphores()
{
	VkSemaphoreCreateInfo semaphore_info{};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	ErrorCheck(vkCreateSemaphore(vk_renderer.device(), &semaphore_info, nullptr, &vk_image_available_semaphore));
	ErrorCheck(vkCreateSemaphore(vk_renderer.device(), &semaphore_info, nullptr, &vk_render_finished_semaphore));
	VkFenceCreateInfo fence_info{};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	ErrorCheck(vkCreateFence(vk_renderer.device(), &fence_info, nullptr, &vk_fence));
}
void SVL::Window::destroy_semaphores()
{
	vkDestroyFence(vk_renderer.device(), vk_fence, nullptr);
	vkDestroySemaphore(vk_renderer.device(), vk_render_finished_semaphore, nullptr);
	vkDestroySemaphore(vk_renderer.device(), vk_image_available_semaphore, nullptr);
}

const VkSampleCountFlagBits SVL::Window::get_sample_count() const
{
	switch (aa)
	{
	case SVL::MSAA_2:
		return VK_SAMPLE_COUNT_2_BIT;
	case SVL::MSAA_4:
		return VK_SAMPLE_COUNT_4_BIT;
	case SVL::MSAA_8:
		return VK_SAMPLE_COUNT_8_BIT;
	case SVL::MSAA_16:
		return VK_SAMPLE_COUNT_16_BIT;
	case SVL::MSAA_32:
		return VK_SAMPLE_COUNT_32_BIT;
	case SVL::MSAA_64:
		return VK_SAMPLE_COUNT_64_BIT;
	default:
		return VK_SAMPLE_COUNT_1_BIT;
	}
}

