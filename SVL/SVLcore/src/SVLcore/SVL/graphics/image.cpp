#include "image.h"

#include <SVL/common/ErrorHandler.h>
#include "renderer.h"
#include "window.h"
#include <SVL/graphics/tools.h>

SVL::Image::Image(Image&& i)
	: image(std::move(i.image)), memory(std::move(i.memory)), vk_renderer(i.vk_renderer), extent(std::move(i.extent)), format(std::move(i.format)), samples(std::move(i.samples)), layout(std::move(i.layout)), array_layers(std::move(i.array_layers)), mip_levels(std::move(i.mip_levels))
{
}

SVL::Image::~Image()
{
}

void SVL::Image::destroy()
{
	vkFreeMemory(vk_renderer.device(), memory, nullptr);
	vkDestroyImage(vk_renderer.device(), image, nullptr);
}

SVL::ImageView::ImageView(ImageView&& i)
	: Image(std::move(i)), view(std::move(i.view)), aspect(std::move(i.aspect))
{
}

SVL::ImageView::~ImageView()
{
}

void SVL::ImageView::destroy()
{
	vkDestroyImageView(vk_renderer.device(), view, nullptr);
	Image::destroy();
}

void SVL::Image::create_2D_image(VkExtent2D extent, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkSampleCountFlagBits samples, VkMemoryPropertyFlags properties, VkImageLayout image_layout, uint32_t array_layers, uint32_t mip_levels, VkImageCreateFlags image_flags)
{
	this->extent = extent;
	this->format = format;
	this->samples = samples;
	this->layout = image_layout;
	this->mip_levels = mip_levels;
	this->array_layers = array_layers;

	VkImageCreateInfo image_info = {};
	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.imageType = VK_IMAGE_TYPE_2D;
	image_info.extent = { extent.width, extent.height, 1 };
	image_info.mipLevels = mip_levels;
	image_info.arrayLayers = array_layers;
	image_info.format = format;
	image_info.tiling = tiling;
	image_info.initialLayout = image_layout;
	image_info.usage = usage;
	image_info.samples = samples;
	image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_info.flags = image_flags;
	ErrorCheck(vkCreateImage(vk_renderer.device(), &image_info, nullptr, &image));

	VkMemoryRequirements memory_requirements;
	vkGetImageMemoryRequirements(vk_renderer.device(), image, &memory_requirements);

	VkMemoryAllocateInfo allocate_info = {};
	allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocate_info.allocationSize = memory_requirements.size;
	allocate_info.memoryTypeIndex = SVLTools::find_memory_type(vk_renderer.physical_device(), memory_requirements.memoryTypeBits, properties);
	ErrorCheck(vkAllocateMemory(vk_renderer.device(), &allocate_info, nullptr, &memory));
	ErrorCheck(vkBindImageMemory(vk_renderer.device(), image, memory, 0));
}

void SVL::Image::copy_buffer_to_image(VkCommandPool command_pool, VkBuffer buffer, std::vector<VkBufferImageCopy> buffer_copy_regions)
{
	VkCommandBuffer command_buffer = VK_NULL_HANDLE;
	SVLTools::begin_single_time_commands(vk_renderer.device(), command_pool, command_buffer);

	vkCmdCopyBufferToImage(command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, buffer_copy_regions.size(), buffer_copy_regions.data());

	SVLTools::end_single_time_commands(vk_renderer.device(), vk_renderer.queue(), command_pool, command_buffer);
}

void SVL::Image::copy_buffer_to_image(VkCommandPool command_pool, VkBuffer buffer, VkImageAspectFlags aspect)
{
	VkBufferImageCopy buffer_copy_region{};
	buffer_copy_region.imageSubresource.aspectMask = aspect;
	buffer_copy_region.imageSubresource.mipLevel = 0;
	buffer_copy_region.imageSubresource.baseArrayLayer = 0;
	buffer_copy_region.imageSubresource.layerCount = array_layers;
	buffer_copy_region.imageExtent.width = extent.width;
	buffer_copy_region.imageExtent.height = extent.height;
	buffer_copy_region.imageExtent.depth = 1;

	Image::copy_buffer_to_image(command_pool, buffer, { buffer_copy_region });
}

void SVL::Image::copy_image_to_buffer(VkCommandPool command_pool, VkBuffer& buffer, std::vector<VkBufferImageCopy> buffer_copy_regions)
{
	VkCommandBuffer command_buffer = VK_NULL_HANDLE;
	SVLTools::begin_single_time_commands(vk_renderer.device(), command_pool, command_buffer);

	vkCmdCopyImageToBuffer(command_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer, buffer_copy_regions.size(), buffer_copy_regions.data());

	SVLTools::end_single_time_commands(vk_renderer.device(), vk_renderer.queue(), command_pool, command_buffer);
}

void SVL::Image::copy_image_to_buffer(VkCommandPool command_pool, VkBuffer& buffer, VkImageAspectFlags aspect)
{
	VkBufferImageCopy buffer_copy_region{};
	buffer_copy_region.imageSubresource.aspectMask = aspect;
	buffer_copy_region.imageSubresource.mipLevel = 0;
	buffer_copy_region.imageSubresource.baseArrayLayer = 0;
	buffer_copy_region.imageSubresource.layerCount = array_layers;
	buffer_copy_region.imageExtent.width = extent.width;
	buffer_copy_region.imageExtent.height = extent.height;
	buffer_copy_region.imageExtent.depth = 1;

	Image::copy_image_to_buffer(command_pool, buffer, { buffer_copy_region });
}

void SVL::Image::transition_image_layout(VkCommandPool command_pool, VkImageLayout new_layout, VkImageAspectFlags aspect)
{
	if (image == VK_NULL_HANDLE || memory == VK_NULL_HANDLE)
		Error("transition_image_layout - null image or memory!");

	VkImageSubresourceRange subresource_range{};
	subresource_range.aspectMask = aspect;
	subresource_range.baseMipLevel = 0;
	subresource_range.levelCount = mip_levels;
	subresource_range.layerCount = array_layers;

	VkCommandBuffer command_buffer = VK_NULL_HANDLE;
	SVLTools::begin_single_time_commands(vk_renderer.device(), command_pool, command_buffer);
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = layout;
	barrier.newLayout = new_layout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange = subresource_range;

	VkPipelineStageFlags src_stage, dst_stage;
	if (layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dst_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else if (layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		src_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
	{
		Error("unsupported layout transition");
	}

	vkCmdPipelineBarrier(command_buffer, src_stage, dst_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	SVLTools::end_single_time_commands(vk_renderer.device(), vk_renderer.queue(), command_pool, command_buffer);

	layout = new_layout;
}

void SVL::ImageView::create_2D_image_view(VkImageAspectFlags aspect)
{
	this->aspect = aspect;

	if (image == VK_NULL_HANDLE || memory == VK_NULL_HANDLE)
		Error("create_2D_image_view - null image or memory!");

	VkImageViewCreateInfo view_info = {};
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.image = image;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	view_info.format = format;
	view_info.subresourceRange.aspectMask = aspect;
	view_info.subresourceRange.baseMipLevel = 0;
	view_info.subresourceRange.levelCount = mip_levels;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount = array_layers;
	ErrorCheck(vkCreateImageView(vk_renderer.device(), &view_info, nullptr, &view));
}

void SVL::ImageView::create_cube_image_view(VkImageAspectFlags aspect)
{
	this->aspect = aspect;

	if (image == VK_NULL_HANDLE || memory == VK_NULL_HANDLE)
		Error("create_2D_image_view - null image or memory!");

	VkImageViewCreateInfo view_info = {};
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.image = image;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	view_info.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	view_info.format = format;
	view_info.subresourceRange.aspectMask = aspect;
	view_info.subresourceRange.baseMipLevel = 0;
	view_info.subresourceRange.levelCount = mip_levels;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount = array_layers;
	ErrorCheck(vkCreateImageView(vk_renderer.device(), &view_info, nullptr, &view));
}
