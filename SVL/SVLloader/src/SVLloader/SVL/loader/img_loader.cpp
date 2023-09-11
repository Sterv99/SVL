#include "loader.h"

#include <SVL/graphics/texture.h>
#define STB_IMAGE_IMPLEMENTATION
#include <SVL/external/stb_image.h>

SVL::Texture* SVL::Loader::load_img(VkFormat format, VkCommandPool command_pool, std::string filename)
{
	if(textures.count(filename) > 0)
		return textures[filename];

	int width, height, channels;
	stbi_uc* image = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);

	if (!image)
		return nullptr;

	textures[filename] = new Texture(vk_renderer, command_pool, static_cast<void*>(image), width * height * 4, { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }, format);
	return textures[filename];
}
