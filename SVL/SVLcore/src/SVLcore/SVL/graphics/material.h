#ifndef MATERIAL_H
#define MATERIAL_H
#include <SVL/definitions.h>
#include <vulkan/vulkan.h>

#include <glm/glm.hpp>
#include <string>

namespace SVL
{
	class Renderer;
	class Texture;
	class DLLDIR Material
	{
	public:
		Material(const Material&);
		Material& operator=(const Material&) = delete;
		Material(Material&&);
		Material& operator=(Material&&) = delete;

		struct MaterialProperties
		{
			bool has_normal_tex;
			unsigned has_ao_tex;
			bool has_height_tex;
		} properties;

		struct MaterialTextures
		{
			Texture* diffuse = nullptr;
			Texture* normal = nullptr;
			Texture* displacement = nullptr;
			Texture* ambient_occulsion = nullptr;
			
			Texture* metalness_roughness = nullptr;
		} textures;

		Material(const Renderer& renderer, MaterialTextures textures, MaterialProperties properties);
		~Material();

		void update();

		struct
		{
			VkBuffer buffer;
			VkDeviceMemory memory;
			VkDescriptorBufferInfo descriptor;
			uint32_t size;
		} material_data;

		VkDescriptorSet vk_descriptor_set = VK_NULL_HANDLE;
		std::string diffuse_filename = "";
	private:
		const class Renderer& vk_renderer;
		bool del = true;
	};
}
#endif
