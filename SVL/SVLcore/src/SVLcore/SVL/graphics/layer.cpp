#include "layer.h"
#include "../common/ErrorHandler.h"

#include <glm/gtc/matrix_transform.hpp>

#include "renderer.h"
#include "window.h"
#include "model.h"
#include "camera.h"
#include "tools.h"

SVL::Layer3D::Layer3D(const Window& window, std::string vertex_shader_path, std::string fragment_shader_path, SVLTools::PipelineType pipeline_type)
	: SVL::SecCommand(window.renderer()), vk_renderer(window.renderer()), vk_window(window), vertex_shader_path(vertex_shader_path), fragment_shader_path(fragment_shader_path), pipeline_type(pipeline_type)
{
	proj = glm::perspective(45.0f, (float)vk_window.extent().width / (float)vk_window.extent().height, 0.001f, 256.0f);
	dummy_env = new SVL::Texture(vk_renderer, vk_window.command_pool(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);
}
SVL::Layer3D::~Layer3D()
{
	if(models.size() != 0)
	{
		destroy_command_buffers();
	}
	
	if(models.size() != 0)
	{
		destroy_pipeline();
		destroy_descriptors();
	}
	delete dummy_env;
}

void SVL::Layer3D::update()
{
	if(models.size() == 0) return;

	destroy_command_buffers();
	for(Model* obj : models)
		obj->destroy_custom_pipelines();
	destroy_pipeline();
	destroy_descriptors();

	create_descriptors();
	for(Model* obj : models)
		obj->create_descriptor_sets(vk_descriptor_pool, { ubo_descriptor_set_layout, material_descriptor_set_layout }, environment ? environment : dummy_env);
	create_pipeline();
	for(Model* obj : models)
		obj->create_custom_pipelines(vk_pipeline_layout);
	create_command_buffers(vk_window.framebuffers()->size());
}
void SVL::Layer3D::update_uniforms()
{
	for (uint32_t i = 0; i < models.size(); i++)
	{
		if (camera != nullptr) models[i]->update_uniform(proj, camera->view(), point_lights, glm::vec4(camera->position(), 1.0f));
		else models[i]->update_uniform(proj, glm::mat4(), point_lights, glm::vec4(0.0f));
	}
}

void SVL::Layer3D::add_object(SVL::Model * object)
{
	if(models.size() != 0)
	{
		destroy_command_buffers();
		for(Model* obj : models)
			obj->destroy_custom_pipelines();
		destroy_pipeline();
		destroy_descriptors();
	}

	models.push_back(object);

	create_descriptors();
	for(Model* obj : models)
		obj->create_descriptor_sets(vk_descriptor_pool, { ubo_descriptor_set_layout, material_descriptor_set_layout }, environment ? environment : dummy_env);
	create_pipeline();
	for(Model* obj : models)
		obj->create_custom_pipelines(vk_pipeline_layout);
	create_command_buffers(vk_window.framebuffers()->size());
}
void SVL::Layer3D::add_object(std::vector<SVL::Model*> obj)
{
	if(models.size() != 0)
	{
		destroy_command_buffers();
		for(Model* obj : models)
			obj->destroy_custom_pipelines();
		destroy_pipeline();
		destroy_descriptors();
	}

	models.insert(models.end(), obj.begin(), obj.end());

	create_descriptors();
	for(Model* obj : models)
		obj->create_descriptor_sets(vk_descriptor_pool, { ubo_descriptor_set_layout, material_descriptor_set_layout }, environment ? environment : dummy_env);
	create_pipeline();
	for(Model* obj : models)
		obj->create_custom_pipelines(vk_pipeline_layout);
	create_command_buffers(vk_window.framebuffers()->size());
}
void SVL::Layer3D::del_object(SVL::Model * object)
{
	if(models.size() != 0)
	{
		destroy_command_buffers();
		for(Model* obj : models)
			obj->destroy_custom_pipelines();
		destroy_pipeline();
		destroy_descriptors();
	}

	models.erase(std::remove(models.begin(), models.end(), object), models.end());

	if(models.size() != 0)
	{
		create_descriptors();
		for(Model* obj : models)
			obj->create_descriptor_sets(vk_descriptor_pool, { ubo_descriptor_set_layout, material_descriptor_set_layout }, environment ? environment : dummy_env);
		create_pipeline();
		for(Model* obj : models)
			obj->create_custom_pipelines(vk_pipeline_layout);
		create_command_buffers(vk_window.framebuffers()->size());
	}
}

void SVL::Layer3D::create_descriptors()
{
	//pool
	uint32_t materials = 0, objects_count = 0;
	for (Model * obj : models)
	{
		objects_count += obj->objects_count();
		materials += obj->materials_count();
	}
	std::array<VkDescriptorPoolSize, 2> pool_sizes = {};
	pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	pool_sizes[0].descriptorCount = objects_count*2 + materials;
	pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	pool_sizes[1].descriptorCount = materials*6;
	VkDescriptorPoolCreateInfo pool_info{};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.poolSizeCount = pool_sizes.size();
	pool_info.pPoolSizes = pool_sizes.data();
	pool_info.maxSets = pool_sizes[1].descriptorCount == 0 ? 1 : pool_sizes[1].descriptorCount + objects_count;
	ErrorCheck(vkCreateDescriptorPool(vk_renderer.device(), &pool_info, nullptr, &vk_descriptor_pool));
	//set_layout
	VkDescriptorSetLayoutCreateInfo layout_info{};
	layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	//set0 binding0 - ubos
	std::array<VkDescriptorSetLayoutBinding, 2> ubo_layout_binding = {};
	ubo_layout_binding[0].binding = 0;
	ubo_layout_binding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ubo_layout_binding[0].descriptorCount = 1;
	ubo_layout_binding[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	ubo_layout_binding[0].pImmutableSamplers = nullptr;
	ubo_layout_binding[1].binding = 1;
	ubo_layout_binding[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ubo_layout_binding[1].descriptorCount = 1;
	ubo_layout_binding[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	ubo_layout_binding[1].pImmutableSamplers = nullptr;
	layout_info.bindingCount = ubo_layout_binding.size();
	layout_info.pBindings = ubo_layout_binding.data();
	ErrorCheck(vkCreateDescriptorSetLayout(vk_renderer.device(), &layout_info, nullptr, &ubo_descriptor_set_layout));
	//set1 binding0 - materials
	std::array<VkDescriptorSetLayoutBinding, 7> sampler_layout_bindings = {};
	sampler_layout_bindings[0].binding = 1;
	sampler_layout_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	sampler_layout_bindings[0].descriptorCount = 1;
	sampler_layout_bindings[0].pImmutableSamplers = nullptr;
	sampler_layout_bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	sampler_layout_bindings[1].binding = 0;
	sampler_layout_bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	sampler_layout_bindings[1].descriptorCount = 1;
	sampler_layout_bindings[1].pImmutableSamplers = nullptr;
	sampler_layout_bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	sampler_layout_bindings[2].binding = 2;
	sampler_layout_bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	sampler_layout_bindings[2].descriptorCount = 1;
	sampler_layout_bindings[2].pImmutableSamplers = nullptr;
	sampler_layout_bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	sampler_layout_bindings[3].binding = 3;
	sampler_layout_bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	sampler_layout_bindings[3].descriptorCount = 1;
	sampler_layout_bindings[3].pImmutableSamplers = nullptr;
	sampler_layout_bindings[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	sampler_layout_bindings[4].binding = 4;
	sampler_layout_bindings[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	sampler_layout_bindings[4].descriptorCount = 1;
	sampler_layout_bindings[4].pImmutableSamplers = nullptr;
	sampler_layout_bindings[4].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	sampler_layout_bindings[5].binding = 5;
	sampler_layout_bindings[5].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	sampler_layout_bindings[5].descriptorCount = 1;
	sampler_layout_bindings[5].pImmutableSamplers = nullptr;
	sampler_layout_bindings[5].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	sampler_layout_bindings[6].binding = 6;
	sampler_layout_bindings[6].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	sampler_layout_bindings[6].descriptorCount = 1;
	sampler_layout_bindings[6].pImmutableSamplers = nullptr;
	sampler_layout_bindings[6].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	layout_info.bindingCount = sampler_layout_bindings.size();
	layout_info.pBindings = sampler_layout_bindings.data();
	ErrorCheck(vkCreateDescriptorSetLayout(vk_renderer.device(), &layout_info, nullptr, &material_descriptor_set_layout));
}
void SVL::Layer3D::destroy_descriptors()
{
	vkDestroyDescriptorSetLayout(vk_renderer.device(), material_descriptor_set_layout, nullptr);
	vkDestroyDescriptorSetLayout(vk_renderer.device(), ubo_descriptor_set_layout, nullptr);
	vkDestroyDescriptorPool(vk_renderer.device(), vk_descriptor_pool, nullptr);
}

void SVL::Layer3D::create_pipeline()
{
	//pipeline_layout
	std::array<VkDescriptorSetLayout, 2> layouts = { ubo_descriptor_set_layout, material_descriptor_set_layout };
	VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};
	pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_create_info.setLayoutCount = layouts.size();
	pipeline_layout_create_info.pSetLayouts = layouts.data();
	ErrorCheck(vkCreatePipelineLayout(vk_renderer.device(), &pipeline_layout_create_info, nullptr, &vk_pipeline_layout));

	std::vector<byte> vertex_shader_code = SVLTools::read_file(vertex_shader_path);
	std::vector<byte> fragment_shader_code = SVLTools::read_file(fragment_shader_path);

	vertex_shader_module = SVLTools::create_shader_module(vk_renderer.device(), vertex_shader_code);
	fragment_shader_module = SVLTools::create_shader_module(vk_renderer.device(), fragment_shader_code);

	vk_pipeline = new SVL::Pipeline(vk_renderer, vk_window.render_pass(), vk_pipeline_layout, SVLTools::create_predefined_pipeline(vk_window.extent(), vk_window.get_sample_count(), vertex_shader_module, fragment_shader_module, pipeline_type));
	vk_blend_pipeline = new SVL::Pipeline(vk_renderer, vk_window.render_pass(), vk_pipeline_layout, SVLTools::create_predefined_pipeline(vk_window.extent(), vk_window.get_sample_count(), vertex_shader_module, fragment_shader_module, SVLTools::Blend));
}
void SVL::Layer3D::destroy_pipeline()
{
	delete vk_blend_pipeline;
	delete vk_pipeline;
	vkDestroyShaderModule(vk_renderer.device(), fragment_shader_module, nullptr);
	vkDestroyShaderModule(vk_renderer.device(), vertex_shader_module, nullptr);
	vkDestroyPipelineLayout(vk_renderer.device(), vk_pipeline_layout, nullptr);
}

void SVL::Layer3D::update_command_buffers(VkCommandBufferInheritanceInfo inheritanceInfo, uint32_t index)
{
	if(models.size() == 0) return;

	VkCommandBufferBeginInfo command_buffer_begin_info = {};
	command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	command_buffer_begin_info.pInheritanceInfo = &inheritanceInfo;

	ErrorCheck(vkBeginCommandBuffer(vk_command_buffers[index], &command_buffer_begin_info));

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = vk_window.extent().width;
	viewport.height = vk_window.extent().height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(vk_command_buffers[index], 0, 1, &viewport);

	VkRect2D scissor = {};
	scissor.offset = { 0,0 };
	scissor.extent = vk_window.extent();
	vkCmdSetScissor(vk_command_buffers[index], 0, 1, &scissor);

	for(Model* obj : models)
	{
		obj->render(vk_command_buffers[index], {(*vk_pipeline)(), (*vk_blend_pipeline)()}, vk_pipeline_layout);
	}

	ErrorCheck(vkEndCommandBuffer(vk_command_buffers[index]));
}
