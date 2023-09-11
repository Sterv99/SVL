#ifndef MODEL_H
#define MODEL_H

#include <SVL/definitions.h>
#include <vulkan/vulkan.h>

#include <glm/glm.hpp>
#include <vector>
#include <array>

#include "mesh.h"
#include "material.h"
#include "light.h"


namespace SVL
{
	struct UniformBufferObject
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
		glm::vec4 view_pos;
		PointLight point_light[4];
	};
	class Renderer;
	class DLLDIR Model final
	{
	public:
		Model(const Renderer& renderer, VkCommandPool command_pool, SVL::Mesh& mesh, SVL::Texture& texture);
		Model(const Renderer& renderer, VkCommandPool command_pool, std::vector<Mesh> meshes, std::vector<Material> materials);
		~Model();

		Model(const Model&) = default;
		Model& operator=(const Model&) = default;
		Model(Model&&) = default;
		Model& operator=(Model&&) = delete;

		const VkDescriptorSet descriptor_set() { return vk_descriptor_set; }
		const glm::mat4 ubo_model() { return model; }
		const std::vector<Mesh> get_meshes() { return meshes; }
		std::vector<Material>& get_materials() { return materials; }
		const bool can_render() { return _can_render; }

		virtual const uint32_t objects_count() { return 1; }
		virtual const uint32_t materials_count() { return materials.size(); }

		virtual void create_descriptor_sets(VkDescriptorPool descriptor_pool, std::array<VkDescriptorSetLayout, 2> layouts, Texture* environment = nullptr);
		virtual void render(VkCommandBuffer command_buffer, std::array<VkPipeline, 2> pipelines, VkPipelineLayout pipeline_layout);
		virtual void update_uniform(glm::mat4 projection, glm::mat4 view, std::array<PointLight, 4> point_lights, glm::vec4 view_pos);

		void set_ubo_model(glm::mat4 model);

		//virtual void handle_input(InputType, Input) {};
		virtual void update() {};
		virtual void create_custom_pipelines(VkPipelineLayout pipeline_layout) {};
		virtual void destroy_custom_pipelines() {};
	protected:
		const class Renderer& vk_renderer;
		bool _can_render = false;
		bool _destroy = true;

		std::vector<Mesh> meshes;
		std::vector<Material> materials;

		glm::mat4 model;

		struct
		{
			VkBuffer buffer;
			VkDeviceMemory memory;
			VkDescriptorBufferInfo descriptor;
			uint32_t size;
		}vk_uniform_data;

		struct
		{
			uint64_t size = 0;
			VkBuffer buffer = VK_NULL_HANDLE;
			VkDeviceMemory memory = VK_NULL_HANDLE;
		}vk_vertices, vk_indices;

		VkDescriptorSet vk_descriptor_set = VK_NULL_HANDLE;
	};
}

#endif
