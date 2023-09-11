#ifndef RENDERER_H
#define RENDERER_H

#include <SVL/definitions.h>
#include <vulkan/vulkan.h>

#include <vector>
#include <string>

namespace SVL
{
	class DLLDIR Renderer
	{
	public:
		Renderer(std::string application_name, uint32_t application_version, bool debug = true);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) = delete;
		Renderer& operator = (const Renderer&) = delete;
		Renderer& operator = (Renderer&&) = delete;

		const VkInstance instance() const { return vk_instance; }
		const VkPhysicalDevice physical_device() const { return vk_physical_device; }
		const uint32_t graphics_family_index() const { return vk_graphics_family_index; }
		const VkDevice device() const { return vk_device; }
		const VkQueue queue() const { return vk_queue; }
		const std::string app_name() const { return application_name; }

		void wait_for_device() const;
	private:
		const std::string application_name;
		const uint32_t application_version;
		const bool debug;

		VkDebugUtilsMessengerEXT debug_messenger;
		VkInstance vk_instance;

		VkPhysicalDevice vk_physical_device;
		uint32_t vk_graphics_family_index;

		VkDevice vk_device;
		VkQueue vk_queue;

		VkPhysicalDeviceFeatures device_features{};
		

		bool check_validation_layer_support();

		virtual bool device_check_suitable(VkPhysicalDevice device);
		virtual std::vector<const char*> get_renderer_layers();
		virtual std::vector<const char*> get_instance_extensions();
		virtual std::vector<const char*> get_device_extensions();

		VkResult create_debug_utils_messenger_ext(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* create_info, const VkAllocationCallbacks* p_allocator, VkDebugUtilsMessengerEXT* p_debug_messenger);
		void destroy_debug_utils_messenger_ext(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks* p_allocator);

		void create_instance();
		void pick_device();
		void create_device();
	};
}

#endif //RENDERER_H
