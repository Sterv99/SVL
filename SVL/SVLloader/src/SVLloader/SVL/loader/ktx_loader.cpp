#include "loader.h"

#include <SVL/common/ErrorHandler.h>
#include <SVL/graphics/image.h>
#include <SVL/graphics/renderer.h>
#include <SVL/graphics/texture.h>
#include <SVL/graphics/tools.h>


#define GLM_ENABLE_EXPERIMENTAL
#include <gli/texture2d.hpp>
#include <gli/texture_cube.hpp>
#include <gli/load.hpp>

SVL::Texture* SVL::Loader::load_ktx(VkFormat format, VkCommandPool command_pool, std::string filename)
{
	if(textures.count(filename) > 0)
		return textures[filename];

	gli::texture2d tex2d = gli::texture2d(gli::load(filename));

	VkBuffer buffer;
	VkDeviceMemory buffer_memory;
	SVLTools::create_buffer(vk_renderer.device(), vk_renderer.physical_device(), tex2d.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &buffer, &buffer_memory);
	void* data;
	ErrorCheck(vkMapMemory(vk_renderer.device(), buffer_memory, 0, tex2d.size(), 0, &data));
	memcpy(data, tex2d.data(), tex2d.size());
	vkUnmapMemory(vk_renderer.device(), buffer_memory);
	//image
	ImageView image(vk_renderer);
	image.create_2D_image({ static_cast<uint32_t>(tex2d[0].extent().x), static_cast<uint32_t>(tex2d[0].extent().y) }, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_SAMPLE_COUNT_1_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_LAYOUT_UNDEFINED, static_cast<uint32_t>(tex2d.layers()), static_cast<uint32_t>(tex2d.levels()));
	image.transition_image_layout(command_pool, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);
	//copy buffer to image
	std::vector<VkBufferImageCopy> bufferCopyRegions;
	uint32_t offset = 0;
	for (uint32_t level = 0; level < tex2d.levels(); level++)
	{
		VkBufferImageCopy bufferCopyRegion = {};
		bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopyRegion.imageSubresource.mipLevel = level;
		bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
		bufferCopyRegion.imageSubresource.layerCount = 1;
		bufferCopyRegion.imageExtent.width = tex2d[level].extent().x;
		bufferCopyRegion.imageExtent.height = tex2d[level].extent().y;
		bufferCopyRegion.imageExtent.depth = 1;
		bufferCopyRegion.bufferOffset = offset;

		bufferCopyRegions.push_back(bufferCopyRegion);
		offset += tex2d[level].size();
	}
	image.copy_buffer_to_image(command_pool, buffer, bufferCopyRegions);
	image.transition_image_layout(command_pool, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);
	//view
	image.create_2D_image_view(VK_IMAGE_ASPECT_COLOR_BIT);

	return textures[filename] = new Texture(vk_renderer, std::move(image));
}

SVL::Texture* SVL::Loader::load_cubemap_ktx(VkFormat format, VkCommandPool command_pool, std::string filename)
{
	if(textures.count(filename) > 0)
		return textures[filename];

	gli::texture_cube tex_cube = gli::texture_cube(gli::load(filename));

	VkBuffer buffer;
	VkDeviceMemory buffer_memory;
	SVLTools::create_buffer(vk_renderer.device(), vk_renderer.physical_device(), tex_cube.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &buffer, &buffer_memory);
	void* data;
	ErrorCheck(vkMapMemory(vk_renderer.device(), buffer_memory, 0, tex_cube.size(), 0, &data));
	memcpy(data, tex_cube.data(), tex_cube.size());
	vkUnmapMemory(vk_renderer.device(), buffer_memory);
	//image
	ImageView image(vk_renderer);
	image.create_2D_image({ static_cast<uint32_t>(tex_cube[0].extent().x), static_cast<uint32_t>(tex_cube[0].extent().y) }, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_SAMPLE_COUNT_1_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_LAYOUT_UNDEFINED, 6, static_cast<uint32_t>(tex_cube.levels()), VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);
	image.transition_image_layout(command_pool, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);
	//copy buffer to image
	std::vector<VkBufferImageCopy> bufferCopyRegions;
	uint32_t offset = 0;
	for (uint32_t face = 0; face < tex_cube.faces(); face++)
	{
		for (uint32_t level = 0; level < tex_cube.levels(); level++)
		{
			VkBufferImageCopy bufferCopyRegion = {};
			bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			bufferCopyRegion.imageSubresource.mipLevel = level;
			bufferCopyRegion.imageSubresource.baseArrayLayer = face;
			bufferCopyRegion.imageSubresource.layerCount = 1;
			bufferCopyRegion.imageExtent.width = tex_cube[face][level].extent().x;
			bufferCopyRegion.imageExtent.height = tex_cube[face][level].extent().y;
			bufferCopyRegion.imageExtent.depth = 1;
			bufferCopyRegion.bufferOffset = offset;

			bufferCopyRegions.push_back(bufferCopyRegion);
			offset += tex_cube[face][level].size();
		}
	}
	image.copy_buffer_to_image(command_pool, buffer, bufferCopyRegions);
	image.transition_image_layout(command_pool, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);
	//view
	image.create_cube_image_view(VK_IMAGE_ASPECT_COLOR_BIT);

	return textures[filename] = new Texture(vk_renderer, std::move(image));
}
