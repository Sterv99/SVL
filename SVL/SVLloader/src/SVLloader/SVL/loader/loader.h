#ifndef LOADER_H
#define LOADER_H

#include <SVL/definitions.h>
#include <vulkan/vulkan.h>

#include <string>
#include <unordered_map>

namespace SVL
{
	class Renderer;
	class Window;
	class Model;
	class Texture;
	class DLLDIR Loader final
	{
	public:
		Loader(const Renderer& renderer): vk_renderer(renderer) {}
		~Loader();

		Model load_gltf(VkCommandPool command_pool, std::string filename);
		Texture* load_img(VkFormat format, VkCommandPool command_pool, std::string filename);
		Texture* load_ktx(VkFormat format, VkCommandPool command_pool, std::string filename);
		Texture* load_cubemap_ktx(VkFormat format, VkCommandPool command_pool, std::string filename);
	private:
		const class Renderer& vk_renderer;

		std::unordered_map<std::string, Texture*> textures;
	};
}
#endif