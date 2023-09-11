#ifndef IMAGE_H
#define IMAGE_H

#include <SVL/definitions.h>
#include <vulkan/vulkan.h>
#include <vector>

namespace SVL
{
	class Renderer;
	class DLLDIR Image
	{
	public:
		Image(const Image&) = delete;
		Image& operator=(const Image&) = delete;
		Image(Image&&);
		Image& operator=(Image&&) = delete;

		Image(const Renderer&r) : vk_renderer(r) {}
		~Image();

		void create_2D_image(VkExtent2D extent, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkSampleCountFlagBits samples, VkMemoryPropertyFlags properties, VkImageLayout image_layout, uint32_t array_layers, uint32_t mip_levels, VkImageCreateFlags image_flags = VK_NULL_HANDLE);
		void copy_buffer_to_image(VkCommandPool command_pool, VkBuffer buffer, std::vector<VkBufferImageCopy>);
		void copy_buffer_to_image(VkCommandPool command_pool, VkBuffer buffer, VkImageAspectFlags aspect);
		void copy_image_to_buffer(VkCommandPool command_pool, VkBuffer& buffer, std::vector<VkBufferImageCopy>);
		void copy_image_to_buffer(VkCommandPool command_pool, VkBuffer& buffer, VkImageAspectFlags aspect);
		void transition_image_layout(VkCommandPool command_pool, VkImageLayout new_layout, VkImageAspectFlags aspect);

		void destroy();

		VkImage image = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;

		VkExtent2D extent;
		VkFormat format;
		VkSampleCountFlagBits samples;
		VkImageLayout layout;
		uint32_t array_layers, mip_levels;
	protected:
		const class Renderer& vk_renderer;
		
	};

	class DLLDIR ImageView : public Image
	{
	public:
		ImageView(const ImageView&) = delete;
		ImageView& operator=(const ImageView&) = delete;
		ImageView(ImageView&&);
		ImageView& operator=(ImageView&&) = delete;

		ImageView(const Renderer& r) : Image(r) {}
		~ImageView();

		void create_2D_image_view(VkImageAspectFlags aspect);
		void create_cube_image_view(VkImageAspectFlags aspect);

		void destroy();

		VkImageView view = VK_NULL_HANDLE;
	protected:
		VkImageAspectFlags aspect;
	};
}


#endif // !IMAGE_H
