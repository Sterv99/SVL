#ifndef TEXTURE_H
#define TEXTURE_H
#include <SVL/definitions.h>
#include <vulkan/vulkan.h>

#include <SVL/graphics/image.h>

namespace SVL
{
	class Renderer;
	class DLLDIR Texture final
	{
	public:
		Texture(const Renderer&, ImageView&& image);
		Texture(const Renderer& renderer, VkCommandPool command_pool, const void* image_data, size_t size, VkExtent2D extent, VkFormat format);
		Texture(const Renderer& renderer, VkCommandPool command_pool, VkFormat format, VkImageCreateFlags img_flags = 0);//empty texture
		~Texture();

		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;
		Texture(Texture&&);
		Texture& operator=(Texture&&) = delete;

		ImageView image;
		VkSampler image_sampler = VK_NULL_HANDLE;
		VkDescriptorImageInfo descriptor{};
	private:
		const class Renderer& vk_renderer;
		bool del = true;
	};
}
#endif
