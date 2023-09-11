#include "material.h"

#include "renderer.h"
#include "texture.h"
#include "tools.h"

SVL::Material::Material(const Renderer& renderer, MaterialTextures textures, MaterialProperties properties)
	: vk_renderer(renderer), textures(textures), properties(properties)
{
	material_data.size = sizeof(MaterialProperties);

	SVLTools::create_buffer(vk_renderer.device(), vk_renderer.physical_device(), material_data.size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &material_data.buffer, &material_data.memory);

	material_data.descriptor.buffer = material_data.buffer;
	material_data.descriptor.offset = 0;
	material_data.descriptor.range = material_data.size;

	void* data;
	vkMapMemory(vk_renderer.device(), material_data.memory, 0, sizeof(properties), 0, &data);
	memcpy(data, &properties, sizeof(properties));
	vkUnmapMemory(vk_renderer.device(), material_data.memory);
}

SVL::Material::Material(const Material& mat)
	: vk_renderer(mat.vk_renderer), properties(mat.properties), textures(mat.textures), vk_descriptor_set(mat.vk_descriptor_set), diffuse_filename(mat.diffuse_filename)
{
	material_data.size = sizeof(MaterialProperties);

	SVLTools::create_buffer(vk_renderer.device(), vk_renderer.physical_device(), material_data.size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &material_data.buffer, &material_data.memory);

	material_data.descriptor.buffer = material_data.buffer;
	material_data.descriptor.offset = 0;
	material_data.descriptor.range = material_data.size;

	void* data;
	vkMapMemory(vk_renderer.device(), material_data.memory, 0, sizeof(properties), 0, &data);
	memcpy(data, &properties, sizeof(properties));
	vkUnmapMemory(vk_renderer.device(), material_data.memory);
}

SVL::Material::Material(Material&& mat)
: vk_renderer(mat.vk_renderer), properties(std::move(mat.properties)), textures(std::move(mat.textures)), material_data(std::move(mat.material_data)), vk_descriptor_set(std::move(mat.vk_descriptor_set)), diffuse_filename(std::move(mat.diffuse_filename))
{
	mat.del = false;
}

SVL::Material::~Material()
{
	if(del)
	{
		vkFreeMemory(vk_renderer.device(), material_data.memory, nullptr);
		vkDestroyBuffer(vk_renderer.device(), material_data.buffer, nullptr);
	}
}

void SVL::Material::update()
{
	void* data;
	vkMapMemory(vk_renderer.device(), material_data.memory, 0, sizeof(properties), 0, &data);
	memcpy(data, &properties, sizeof(properties));
	vkUnmapMemory(vk_renderer.device(), material_data.memory);
}
