#include "renderer.h"

#include <SVL/common/ErrorHandler.h>
#include <sstream>

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
	VkDebugUtilsMessageTypeFlagsEXT type,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	std::ostringstream stream;
	stream << "[";
	if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT )
	{
		stream << " VERBOSE ";
	}
	if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
	{
		stream << " INFO ";
	}
	if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		stream << " WARNING ";
	}
	if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		stream << " ERROR ";
	}

	if (type & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT  )
	{
		stream << " GENERAL ";
	}
	if (type & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT  )
	{
		stream << " VALIDATION ";
	}
	if (type & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT )
	{
		stream << " PERFORMANCE ";
	}

	stream << "]: ";
	stream << pCallbackData->pMessage << "\n";
	Log(stream.str());
	return 0;
}

bool SVL::Renderer::check_validation_layer_support()
{
	uint32_t count;
	vkEnumerateInstanceLayerProperties(&count, nullptr);

	std::vector<VkLayerProperties> available(count);
	vkEnumerateInstanceLayerProperties(&count, available.data());

	auto renderer_layers = get_renderer_layers();
	for (const char* name : renderer_layers)
	{
		bool found = false;
		for (const auto& prop : available)
		{
			if (strcmp(name, prop.layerName) == 0)
			{
				found = true;
				break;
			}
		}
		if (!found) return false;
	}
	return true;
}

bool SVL::Renderer::device_check_suitable(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties physical_device_properties{};
	VkPhysicalDeviceFeatures physical_device_features{};
	VkPhysicalDeviceFeatures2 physical_device_features2{};
	vkGetPhysicalDeviceProperties(device, &physical_device_properties);
	vkGetPhysicalDeviceFeatures(device, &physical_device_features);
	vkGetPhysicalDeviceFeatures2(device, &physical_device_features2);

	return physical_device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && physical_device_features.geometryShader && physical_device_features.samplerAnisotropy;
}

std::vector<const char*> SVL::Renderer::get_renderer_layers()
{
	if (debug)
		return { VALIDATION_LAYERS };
	else
		return {};
}

std::vector<const char*> SVL::Renderer::get_instance_extensions()
{
	if (debug)
		return { VK_EXT_DEBUG_UTILS_EXTENSION_NAME, "VK_KHR_surface", VK_KHR_platform_surface };
	else
		return { "VK_KHR_surface", VK_KHR_platform_surface };
}

std::vector<const char*> SVL::Renderer::get_device_extensions()
{
	return { "VK_KHR_swapchain", "VK_EXT_robustness2" };
}

VkResult SVL::Renderer::create_debug_utils_messenger_ext(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* create_info, const VkAllocationCallbacks* p_allocator, VkDebugUtilsMessengerEXT* p_debug_messenger)
{
	auto fnc = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if(fnc != nullptr)
		return fnc(instance, create_info, p_allocator, p_debug_messenger);
	else
		return VK_ERROR_EXTENSION_NOT_PRESENT;
}
void SVL::Renderer::destroy_debug_utils_messenger_ext(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks* p_allocator)
{
	auto fnc = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if(fnc != nullptr)
		fnc(instance, debug_messenger, p_allocator);
}

SVL::Renderer::Renderer(std::string application_name, uint32_t application_version, bool debug)
: application_name(application_name), application_version(application_version), debug(debug)
{
	if(debug && !check_validation_layer_support())
		Error("SVL ERROR: validation layers not available!");
	
	create_instance();
	pick_device();
	create_device();
}
SVL::Renderer::~Renderer()
{
	vkDestroyDevice(vk_device, nullptr);
	if(debug)
		destroy_debug_utils_messenger_ext(vk_instance, debug_messenger, nullptr);
	vkDestroyInstance(vk_instance, nullptr);
}

void SVL::Renderer::create_instance()
{
	VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
	if (debug)
	{
		debug_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debug_create_info.pfnUserCallback = DebugCallback;
		debug_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	}

	VkApplicationInfo application_info{};
	application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	application_info.apiVersion = VK_MAKE_VERSION(1, 0, 3);
	application_info.engineVersion = ENGINE_VERSION;
	application_info.pEngineName = ENGINE_NAME;
	application_info.applicationVersion = application_version;
	application_info.pApplicationName = application_name.c_str();

	VkInstanceCreateInfo instance_create_info{};
	instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_create_info.pApplicationInfo = &application_info;

	auto renderer_layers = get_renderer_layers();
	instance_create_info.enabledLayerCount = static_cast<uint32_t>(renderer_layers.size());
	instance_create_info.ppEnabledLayerNames = renderer_layers.data();
	if (debug)
	{
		instance_create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
	}

	auto instance_extensions = get_instance_extensions();
	instance_create_info.enabledExtensionCount = static_cast<uint32_t>(instance_extensions.size());
	instance_create_info.ppEnabledExtensionNames = instance_extensions.data();

	ErrorCheck(vkCreateInstance(&instance_create_info, nullptr, &vk_instance));

	if (debug)
		ErrorCheck(create_debug_utils_messenger_ext(vk_instance, &debug_create_info, nullptr, &debug_messenger));
}

void SVL::Renderer::pick_device()
{
	//physical device
	uint32_t physical_devices_count = 0;
	ErrorCheck(vkEnumeratePhysicalDevices(vk_instance, &physical_devices_count, nullptr));
	std::vector<VkPhysicalDevice> physical_devices_list(physical_devices_count);
	ErrorCheck(vkEnumeratePhysicalDevices(vk_instance, &physical_devices_count, physical_devices_list.data()));
	for (const auto& device : physical_devices_list)
	{
		if (device_check_suitable(device))
		{
			vk_physical_device = device;
			break;
		}
	}
	if (vk_physical_device == VK_NULL_HANDLE)
		Error("Failed to find suitable GPU!");

	//queue family
	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_device, &queue_family_count, nullptr);
	std::vector<VkQueueFamilyProperties> family_properties_list(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_device, &queue_family_count, family_properties_list.data());

	bool found = false;
	for (uint32_t i = 0; i < queue_family_count; i++)
	{
		if (family_properties_list[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			found = true;
			vk_graphics_family_index = i;
		}
	}
	if (!found)
		Error("SVL ERROR: Queue family supporting graphics not found.");
}

void SVL::Renderer::create_device()
{
	float queue_priorities[]{ 1.0f };
	device_features.shaderClipDistance = VK_TRUE;
	device_features.shaderCullDistance = VK_TRUE;
	device_features.textureCompressionBC = VK_TRUE;
	device_features.fillModeNonSolid = VK_TRUE;
	device_features.samplerAnisotropy = VK_TRUE;
	VkDeviceQueueCreateInfo device_queue_create_info{};
	device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	device_queue_create_info.queueFamilyIndex = vk_graphics_family_index;
	device_queue_create_info.queueCount = 1;
	device_queue_create_info.pQueuePriorities = queue_priorities;
	VkDeviceCreateInfo device_create_info{};
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.queueCreateInfoCount = 1;
	device_create_info.pQueueCreateInfos = &device_queue_create_info;

	auto renderer_layers = get_renderer_layers();
	device_create_info.enabledLayerCount = static_cast<uint32_t>(renderer_layers.size());
	device_create_info.ppEnabledLayerNames = renderer_layers.data();

	auto device_extensions = get_device_extensions();
	device_create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
	device_create_info.ppEnabledExtensionNames = device_extensions.data();

	VkPhysicalDeviceRobustness2FeaturesEXT robustness2feature{};
	robustness2feature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT;
	robustness2feature.nullDescriptor = VK_TRUE;

	VkPhysicalDeviceFeatures2 features2{};
	features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	features2.features = device_features;
	//features2.pNext = &robustness2feature;

	device_create_info.pNext = &features2;
	ErrorCheck(vkCreateDevice(vk_physical_device, &device_create_info, nullptr, &vk_device));
	vkGetDeviceQueue(vk_device, vk_graphics_family_index, 0, &vk_queue);
}

void SVL::Renderer::wait_for_device() const
{
	ErrorCheck(vkDeviceWaitIdle(vk_device));
}
