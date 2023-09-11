#ifndef TOOLS_H
#define TOOLS_H

#include <SVL/definitions.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <string>

#include "pipeline.h"

typedef unsigned char byte;

namespace SVLTools
{
	enum PipelineType
	{
		Solid,
		Blend,
		FontBlend,
		Wireframe,
		Cubemap
	};

	DLLDIR ::SVL::init::PipelineInit create_predefined_pipeline(VkExtent2D extent, VkSampleCountFlagBits samples, VkShaderModule vertex_shader_module, VkShaderModule fragment_shader_module, PipelineType type);
	DLLDIR VkShaderModule create_shader_module(VkDevice device, const std::vector<byte>& code);

	DLLDIR VkImageView create_2D_image_view(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspect_flags, uint32_t mip_levels);

	DLLDIR void create_buffer_and_memory(VkDevice device, VkPhysicalDevice physical_device, VkQueue queue, VkCommandPool command_pool, VkDeviceSize buffer_size, const void * source, VkBufferUsageFlags staging_usage, VkMemoryPropertyFlags staging_properties, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer & buffer, VkDeviceMemory & buffer_memory);
	DLLDIR void create_buffer(VkDevice device, VkPhysicalDevice physical_device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* buffer_memory);
	DLLDIR void copy_buffer(VkDevice device, VkQueue queue, VkCommandPool command_pool, VkBuffer source_buffer, VkBuffer destination_buffer, VkDeviceSize size);

	DLLDIR void begin_single_time_commands(VkDevice device, VkCommandPool command_pool, VkCommandBuffer & command_buffer);
	DLLDIR void end_single_time_commands(VkDevice device, VkQueue queue, VkCommandPool command_pool, VkCommandBuffer command_buffer);

	DLLDIR uint32_t find_memory_type(VkPhysicalDevice physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties);
	DLLDIR VkFormat find_supported_format(VkPhysicalDevice physical_device, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	DLLDIR std::vector<byte> read_file(const std::string file_name);
	DLLDIR std::string file_to_hex(const std::string file_name);
	DLLDIR std::vector<byte> hex_to_bytes(std::string hex);

	DLLDIR std::vector<std::string> split(std::string source, const char symbol);
}
#endif
