#include "texture.h"
#include "renderer.h"
#include "tools.h"
#include "../common/ErrorHandler.h"


SVL::Texture::Texture(const Renderer& renderer, ImageView&& image)
	: vk_renderer(renderer), image(std::move(image))
{
	VkSamplerCreateInfo sampler_info{};
	sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_info.magFilter = VK_FILTER_LINEAR;
	sampler_info.minFilter = VK_FILTER_LINEAR;
	sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_info.anisotropyEnable = VK_TRUE;
	sampler_info.maxAnisotropy = 16.0f;
	sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	sampler_info.unnormalizedCoordinates = VK_FALSE;
	sampler_info.compareEnable = VK_FALSE;
	sampler_info.compareOp = VK_COMPARE_OP_NEVER;
	sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler_info.mipLodBias = 0.0f;
	sampler_info.minLod = 0.0f;
	sampler_info.maxLod = 1.0f;
	ErrorCheck(vkCreateSampler(vk_renderer.device(), &sampler_info, nullptr, &image_sampler));
	//descriptor
	descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	descriptor.imageView = image.view;
	descriptor.sampler = image_sampler;
}

SVL::Texture::Texture(const Renderer& renderer, VkCommandPool command_pool, const void* image_data, size_t size, VkExtent2D extent, VkFormat format)
	: vk_renderer(renderer), image(renderer)
{
	//buffer
	VkBuffer buffer;
	VkDeviceMemory buffer_memory;
	SVLTools::create_buffer(vk_renderer.device(), vk_renderer.physical_device(), size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &buffer, &buffer_memory);
	void* data;
	ErrorCheck(vkMapMemory(vk_renderer.device(), buffer_memory, 0, size, 0, &data));
	memcpy(data, image_data, size);
	vkUnmapMemory(vk_renderer.device(), buffer_memory);
	//image
	image.create_2D_image({ extent.width, extent.height }, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_SAMPLE_COUNT_1_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_LAYOUT_UNDEFINED, 1, 1);
	image.transition_image_layout(command_pool, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);
	//copy buffer to image
	image.copy_buffer_to_image(command_pool, buffer, VK_IMAGE_ASPECT_COLOR_BIT);
	image.transition_image_layout(command_pool, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);
	//destroy buffer
	vkFreeMemory(vk_renderer.device(), buffer_memory, nullptr);
	vkDestroyBuffer(vk_renderer.device(), buffer, nullptr);
	//view
	image.create_2D_image_view(VK_IMAGE_ASPECT_COLOR_BIT);
	//sampler
	VkSamplerCreateInfo sampler_info{};
	sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_info.magFilter = VK_FILTER_LINEAR;
	sampler_info.minFilter = VK_FILTER_LINEAR;
	sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_info.anisotropyEnable = VK_TRUE;
	sampler_info.maxAnisotropy = 16.0f;
	sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	sampler_info.unnormalizedCoordinates = VK_FALSE;
	sampler_info.compareEnable = VK_FALSE;
	sampler_info.compareOp = VK_COMPARE_OP_NEVER;
	sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler_info.mipLodBias = 0.0f;
	sampler_info.minLod = 0.0f;
	sampler_info.maxLod = 1.0f;
	ErrorCheck(vkCreateSampler(vk_renderer.device(), &sampler_info, nullptr, &image_sampler));
	//descriptor
	descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	descriptor.imageView = image.view;
	descriptor.sampler = image_sampler;
}

SVL::Texture::Texture(const Renderer& renderer, VkCommandPool command_pool, VkFormat format, VkImageCreateFlags img_flags)
	: vk_renderer(renderer), image(renderer)
{
	uint8_t* image_data;
	size_t size;
	unsigned faces = 1;
	if (img_flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
	{
		image_data = new uint8_t[]{ 128, 128, 128, 255, 128, 128, 128, 255, 128, 128, 128, 255, 128, 128, 128, 255, 128, 128, 128, 255, 128, 128, 128, 255 };
		size = 24;
		faces = 6;
	}
	else
	{
		image_data = new uint8_t[]{ 128, 128, 128, 255 };
		size = 4;
	}

	VkExtent2D extent{1,1};

	//buffer
	VkBuffer buffer;
	VkDeviceMemory buffer_memory;
	SVLTools::create_buffer(vk_renderer.device(), vk_renderer.physical_device(), size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &buffer, &buffer_memory);
	void* data;
	ErrorCheck(vkMapMemory(vk_renderer.device(), buffer_memory, 0, size, 0, &data));
	memcpy(data, image_data, size);
	vkUnmapMemory(vk_renderer.device(), buffer_memory);
	//image
	image.create_2D_image({ extent.width, extent.height }, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_SAMPLE_COUNT_1_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_LAYOUT_UNDEFINED, 6, 1, img_flags);
	image.transition_image_layout(command_pool, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);
	//copy buffer to image
	std::vector<VkBufferImageCopy> bufferCopyRegions;
	uint32_t offset = 0;
	for (uint32_t face = 0; face < faces; face++)
	{
		VkBufferImageCopy bufferCopyRegion = {};
		bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopyRegion.imageSubresource.mipLevel = 0;
		bufferCopyRegion.imageSubresource.baseArrayLayer = face;
		bufferCopyRegion.imageSubresource.layerCount = 1;
		bufferCopyRegion.imageExtent.width = 1;
		bufferCopyRegion.imageExtent.height = 1;
		bufferCopyRegion.imageExtent.depth = 1;
		bufferCopyRegion.bufferOffset = offset;

		bufferCopyRegions.push_back(bufferCopyRegion);
		offset += 4;
	}
	image.copy_buffer_to_image(command_pool, buffer, bufferCopyRegions);
	image.transition_image_layout(command_pool, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);
	//destroy buffer
	vkFreeMemory(vk_renderer.device(), buffer_memory, nullptr);
	vkDestroyBuffer(vk_renderer.device(), buffer, nullptr);
	delete image_data;
	//view
	if (img_flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
		image.create_cube_image_view(VK_IMAGE_ASPECT_COLOR_BIT);
	else
		image.create_2D_image_view(VK_IMAGE_ASPECT_COLOR_BIT);
	//sampler
	VkSamplerCreateInfo sampler_info{};
	sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_info.magFilter = VK_FILTER_LINEAR;
	sampler_info.minFilter = VK_FILTER_LINEAR;
	sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_info.anisotropyEnable = VK_TRUE;
	sampler_info.maxAnisotropy = 16.0f;
	sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	sampler_info.unnormalizedCoordinates = VK_FALSE;
	sampler_info.compareEnable = VK_FALSE;
	sampler_info.compareOp = VK_COMPARE_OP_NEVER;
	sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler_info.mipLodBias = 0.0f;
	sampler_info.minLod = 0.0f;
	sampler_info.maxLod = 1.0f;
	ErrorCheck(vkCreateSampler(vk_renderer.device(), &sampler_info, nullptr, &image_sampler));
	//descriptor
	descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	descriptor.imageView = image.view;
	descriptor.sampler = image_sampler;
}

SVL::Texture::Texture(Texture&& tex)
: image(std::move(tex.image)), image_sampler(std::move(tex.image_sampler)), descriptor(std::move(tex.descriptor)), vk_renderer(tex.vk_renderer)
{
	tex.del = false;
}

SVL::Texture::~Texture()
{
	if(del)
	{
		vkDestroySampler(vk_renderer.device(), image_sampler, nullptr);
		image.destroy();
	}
}
