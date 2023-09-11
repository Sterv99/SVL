#ifndef LAYER_H
#define LAYER_H

#include <SVL/definitions.h>
#include <vulkan/vulkan.h>

#include <map>
#include <array>
#include <glm/glm.hpp>

#include "tools.h"
#include "light.h"
#include "command.h"
#include "pipeline.h"
#include "texture.h"

namespace SVL
{
	class Renderer;
	class Window;
	class Model;
	class Camera;
	class DLLDIR Layer3D : public SecCommand
	{
	public:
		Layer3D(const Window& window, std::string vertex_shader_path, std::string fragment_shader_path, SVLTools::PipelineType pipeline_type = SVLTools::Solid);
		~Layer3D();

		Layer3D(const Layer3D&) = delete;
		Layer3D& operator=(const Layer3D&) = delete;
		Layer3D(Layer3D&&) = delete;
		Layer3D& operator=(Layer3D&&) = delete;

		void add_object(SVL::Model* object);
		void add_object(std::vector<SVL::Model*> models);
		void del_object(SVL::Model* object);

		void update();
		void update_uniforms();
		void update_command_buffers(VkCommandBufferInheritanceInfo inheritanceInfo, uint32_t index);

		void set_projection(glm::mat4 projection) { this->proj = projection; }
		void set_camera(Camera * camera) { this->camera = camera; }
		void set_environment_tex(Texture* env) { this->environment = env; }

		glm::mat4 ubo_projection() { return proj; }
		Camera * get_camera() { return camera; }
		VkPipelineLayout pipeline_layout() { return vk_pipeline_layout; }
		const std::vector<Model*>& get_objects() const { return models; }

		std::array<PointLight, 4> point_lights;
	private:
		const class Renderer& vk_renderer;
		const class Window& vk_window;

		std::string vertex_shader_path;
		std::string fragment_shader_path;
		SVLTools::PipelineType pipeline_type;

		std::vector<SVL::Model*> models;

		glm::mat4 proj;

		
		Camera * camera = nullptr;

		Texture* environment = nullptr;
		Texture* dummy_env;

		VkDescriptorPool vk_descriptor_pool = VK_NULL_HANDLE;
		VkDescriptorSetLayout ubo_descriptor_set_layout = VK_NULL_HANDLE;
		VkDescriptorSetLayout material_descriptor_set_layout = VK_NULL_HANDLE;
		VkPipelineLayout vk_pipeline_layout = VK_NULL_HANDLE;

		SVL::Pipeline* vk_pipeline;
		VkShaderModule vertex_shader_module = VK_NULL_HANDLE;
		VkShaderModule fragment_shader_module = VK_NULL_HANDLE;

		SVL::Pipeline* vk_blend_pipeline;

		void create_descriptors();
		void destroy_descriptors();

		void create_pipeline();
		void destroy_pipeline();
	};
}
#endif
