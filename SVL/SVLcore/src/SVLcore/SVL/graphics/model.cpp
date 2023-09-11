#include "model.h"

#include "../common/ErrorHandler.h"
#include "tools.h"

#include "window.h"
#include "layer.h"
#include "vertex.h"
#include "texture.h"
#include "light.h"
#include "renderer.h"

#include <chrono>
#include <unordered_map>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

SVL::Model::Model(const Renderer& renderer, VkCommandPool command_pool, SVL::Mesh & mesh, SVL::Texture & texture)
	:vk_renderer(renderer), model(glm::mat4(1.0f))
{
	vk_uniform_data.size = sizeof(UniformBufferObject);

	SVLTools::create_buffer(vk_renderer.device(), vk_renderer.physical_device(), vk_uniform_data.size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &vk_uniform_data.buffer, &vk_uniform_data.memory);

	vk_uniform_data.descriptor.buffer = vk_uniform_data.buffer;
	vk_uniform_data.descriptor.offset = 0;
	vk_uniform_data.descriptor.range = vk_uniform_data.size;

	meshes.push_back(mesh);

	Material::MaterialTextures textures;
	textures.diffuse = &texture;
	materials.push_back(Material(renderer, textures, {}));


	std::vector<SVL::Vertex3D> vertices = mesh.vertices;
	std::vector<uint32_t> indices = mesh.indices;

	vk_vertices.size = sizeof(vertices[0]) * vertices.size();
	SVLTools::create_buffer_and_memory(vk_renderer.device(), vk_renderer.physical_device(), vk_renderer.queue(), command_pool, vk_vertices.size, vertices.data(),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		vk_vertices.buffer, vk_vertices.memory
	);

	vk_indices.size = sizeof(indices[0]) * indices.size();
	SVLTools::create_buffer_and_memory(vk_renderer.device(), vk_renderer.physical_device(), vk_renderer.queue(), command_pool, vk_indices.size, indices.data(),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		vk_indices.buffer, vk_indices.memory
	);

	_destroy = false;
	_can_render = true;
}

SVL::Model::Model(const Renderer& renderer, VkCommandPool command_pool, std::vector<Mesh> meshes, std::vector<Material> materials)
	:vk_renderer(renderer), meshes(meshes), materials(materials), model(glm::mat4(1.0f))
{
	vk_uniform_data.size = sizeof(UniformBufferObject);

	SVLTools::create_buffer(vk_renderer.device(), vk_renderer.physical_device(), vk_uniform_data.size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &vk_uniform_data.buffer, &vk_uniform_data.memory);

	vk_uniform_data.descriptor.buffer = vk_uniform_data.buffer;
	vk_uniform_data.descriptor.offset = 0;
	vk_uniform_data.descriptor.range = vk_uniform_data.size;


	std::vector<SVL::Vertex3D> vertices;
	std::vector<uint32_t> indices;

	uint32_t offset = 0;
	for (Mesh mesh : meshes)
	{
		vertices.insert(vertices.end(), mesh.vertices.begin(), mesh.vertices.end());
		indices.insert(indices.end(), mesh.indices.begin(), mesh.indices.end());
		mesh.index_base = offset;
		offset += mesh.indices.size();
	}

	vk_vertices.size = sizeof(vertices[0]) * vertices.size();
	SVLTools::create_buffer_and_memory(vk_renderer.device(), vk_renderer.physical_device(), vk_renderer.queue(), command_pool, vk_vertices.size, vertices.data(),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		vk_vertices.buffer, vk_vertices.memory
		);

	vk_indices.size = sizeof(indices[0]) * indices.size();
	SVLTools::create_buffer_and_memory(vk_renderer.device(), vk_renderer.physical_device(), vk_renderer.queue(), command_pool, vk_indices.size, indices.data(),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		vk_indices.buffer, vk_indices.memory
	);

	_can_render = true;
}

SVL::Model::~Model()
{
	vkFreeMemory(vk_renderer.device(), vk_indices.memory, nullptr);
	vkDestroyBuffer(vk_renderer.device(), vk_indices.buffer, nullptr);

	vkFreeMemory(vk_renderer.device(), vk_vertices.memory, nullptr);
	vkDestroyBuffer(vk_renderer.device(), vk_vertices.buffer, nullptr);

	vkFreeMemory(vk_renderer.device(), vk_uniform_data.memory, nullptr);
	vkDestroyBuffer(vk_renderer.device(), vk_uniform_data.buffer, nullptr);
}

void SVL::Model::create_descriptor_sets(VkDescriptorPool descriptor_pool, std::array<VkDescriptorSetLayout, 2> layouts, Texture* environment)
{
	//set0
	VkDescriptorSetAllocateInfo allocate_info = {};
	allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocate_info.descriptorPool = descriptor_pool;
	allocate_info.descriptorSetCount = 1;
	allocate_info.pSetLayouts = &layouts[0];
	ErrorCheck(vkAllocateDescriptorSets(vk_renderer.device(), &allocate_info, &vk_descriptor_set));
	//ubo
	std::vector<VkWriteDescriptorSet> descriptor_writes = {};
	VkWriteDescriptorSet uniform_set{};
	uniform_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	uniform_set.dstSet = vk_descriptor_set;
	uniform_set.dstBinding = 0;
	uniform_set.dstArrayElement = 0;
	uniform_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniform_set.descriptorCount = 1;
	uniform_set.pBufferInfo = &vk_uniform_data.descriptor;
	descriptor_writes.push_back(uniform_set);
	//ubo in fragment
	uniform_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	uniform_set.dstSet = vk_descriptor_set;
	uniform_set.dstBinding = 1;
	uniform_set.dstArrayElement = 0;
	uniform_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniform_set.descriptorCount = 1;
	uniform_set.pBufferInfo = &vk_uniform_data.descriptor;
	descriptor_writes.push_back(uniform_set);

	vkUpdateDescriptorSets(vk_renderer.device(), descriptor_writes.size(), descriptor_writes.data(), 0, nullptr);

	for (Material& material : materials)
	{
		//set1
		VkDescriptorSetAllocateInfo allocate_info = {};
		allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocate_info.descriptorPool = descriptor_pool;
		allocate_info.descriptorSetCount = 1;
		allocate_info.pSetLayouts = &layouts[1];
		ErrorCheck(vkAllocateDescriptorSets(vk_renderer.device(), &allocate_info, &material.vk_descriptor_set));
		//material
		std::vector<VkWriteDescriptorSet> descriptor_writes = {};
		VkWriteDescriptorSet descriptor_write{};

		descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_write.dstSet = material.vk_descriptor_set;
		descriptor_write.dstBinding = 0;
		descriptor_write.dstArrayElement = 0;
		descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptor_write.descriptorCount = 1;
		descriptor_write.pBufferInfo = &material.material_data.descriptor;
		descriptor_writes.push_back(descriptor_write);

		descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_write.dstSet = material.vk_descriptor_set;
		descriptor_write.dstBinding = 1;
		descriptor_write.dstArrayElement = 0;
		descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptor_write.descriptorCount = 1;
		descriptor_write.pImageInfo = &material.textures.diffuse->descriptor;
		descriptor_writes.push_back(descriptor_write);

		descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_write.dstSet = material.vk_descriptor_set;
		descriptor_write.dstBinding = 2;
		descriptor_write.dstArrayElement = 0;
		descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptor_write.descriptorCount = 1;
		if (material.textures.normal != nullptr)descriptor_write.pImageInfo = &material.textures.normal->descriptor;
		else descriptor_write.pImageInfo = &material.textures.diffuse->descriptor;
		descriptor_writes.push_back(descriptor_write);

		descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_write.dstSet = material.vk_descriptor_set;
		descriptor_write.dstBinding = 3;
		descriptor_write.dstArrayElement = 0;
		descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptor_write.descriptorCount = 1;
		if (material.textures.metalness_roughness)descriptor_write.pImageInfo = &material.textures.metalness_roughness->descriptor;
		else descriptor_write.pImageInfo = &material.textures.diffuse->descriptor;
		descriptor_writes.push_back(descriptor_write);

		descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_write.dstSet = material.vk_descriptor_set;
		descriptor_write.dstBinding = 4;
		descriptor_write.dstArrayElement = 0;
		descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptor_write.descriptorCount = 1;
		if (material.textures.ambient_occulsion != nullptr)descriptor_write.pImageInfo = &material.textures.ambient_occulsion->descriptor;
		else descriptor_write.pImageInfo = &material.textures.diffuse->descriptor;
		descriptor_writes.push_back(descriptor_write);

		descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_write.dstSet = material.vk_descriptor_set;
		descriptor_write.dstBinding = 5;
		descriptor_write.dstArrayElement = 0;
		descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptor_write.descriptorCount = 1;
		descriptor_write.pImageInfo = &environment->descriptor;
		descriptor_writes.push_back(descriptor_write);

		descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_write.dstSet = material.vk_descriptor_set;
		descriptor_write.dstBinding = 6;
		descriptor_write.dstArrayElement = 0;
		descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptor_write.descriptorCount = 1;
		if (material.textures.displacement != nullptr)descriptor_write.pImageInfo = &material.textures.displacement->descriptor;
		else descriptor_write.pImageInfo = &material.textures.diffuse->descriptor;
		descriptor_writes.push_back(descriptor_write);

		vkUpdateDescriptorSets(vk_renderer.device(), descriptor_writes.size(), descriptor_writes.data(), 0, nullptr);
	}
}

void SVL::Model::render(VkCommandBuffer command_buffer, std::array<VkPipeline, 2> pipelines, VkPipelineLayout pipeline_layout)
{
	if (!_can_render) return;
	VkDeviceSize offsets[] = { 0 };

	vkCmdBindVertexBuffers(command_buffer, 0, 1, &vk_vertices.buffer, offsets);
	vkCmdBindIndexBuffer(command_buffer, vk_indices.buffer, 0, VK_INDEX_TYPE_UINT32);

	for (size_t i = 0; i < meshes.size(); i++)
	{
		VkDescriptorSet descriptor_sets[2];
		descriptor_sets[0] = vk_descriptor_set;
		descriptor_sets[1] = materials[meshes[i].material_id].vk_descriptor_set;
		
		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[0]);//defined pipeline
		vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 2, descriptor_sets, 0, nullptr);
		vkCmdDrawIndexed(command_buffer, meshes[i].indices.size(), 1, meshes[i].index_base, meshes[i].vertex_base, 0);
	}
}

void SVL::Model::update_uniform(glm::mat4 projection, glm::mat4 view, std::array<SVL::PointLight, 4> lights, glm::vec4 view_pos)
{
	UniformBufferObject ubo{};
	
	ubo.proj = projection;
	ubo.view = view;
	ubo.model = model;
	
	std::copy(std::begin(lights), std::end(lights), std::begin(ubo.point_light));

	ubo.view_pos = view_pos * -1.0f;

	void* data;
	vkMapMemory(vk_renderer.device(), vk_uniform_data.memory, 0, sizeof(UniformBufferObject), 0, &data);
	memcpy(data, &ubo, sizeof(UniformBufferObject));
	vkUnmapMemory(vk_renderer.device(), vk_uniform_data.memory);
}

void SVL::Model::set_ubo_model(glm::mat4 model)
{
	this->model = model;
}
