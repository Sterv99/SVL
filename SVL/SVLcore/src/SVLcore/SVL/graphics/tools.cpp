#include "tools.h"
#include "../common/ErrorHandler.h"
#include "vertex.h"
#include <fstream>
#include <sstream>
#include <iomanip>

#pragma region Pipeline
SVL::init::PipelineInit SVLTools::create_predefined_pipeline(VkExtent2D extent, VkSampleCountFlagBits samples, VkShaderModule vertex_shader_module, VkShaderModule fragment_shader_module, PipelineType type)
{
	SVL::init::PipelineInit init;

	init.vertex_bindings.push_back(SVL::Vertex3D::binding_descriptor());

	auto vertex_att = SVL::Vertex3D::attribute_descriptor();
	init.vertex_attributes.insert(init.vertex_attributes.end(), vertex_att.begin(), vertex_att.end());

	VkPipelineShaderStageCreateInfo vertexShaderStageInfo = {};
	vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderStageInfo.module = vertex_shader_module;
	vertexShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragmentShaderStageInfo = {};
	fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderStageInfo.module = fragment_shader_module;
	fragmentShaderStageInfo.pName = "main";

	init.stages.push_back(vertexShaderStageInfo);
	init.stages.push_back(fragmentShaderStageInfo);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = extent.width;
	viewport.height = extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	init.viewports.push_back(viewport);

	VkRect2D scissor{};
	scissor.offset = { 0,0 };
	scissor.extent = extent;
	init.scissors.push_back(scissor);


	init.multisample.rasterizationSamples = samples;

	/*
	VkStencilOpState noOPStencilState = {};
	noOPStencilState.failOp = VK_STENCIL_OP_KEEP;
	noOPStencilState.passOp = VK_STENCIL_OP_KEEP;
	noOPStencilState.depthFailOp = VK_STENCIL_OP_KEEP;
	noOPStencilState.compareOp = VK_COMPARE_OP_ALWAYS;
	noOPStencilState.compareMask = 0;
	noOPStencilState.writeMask = 0;
	noOPStencilState.reference = 0;*/

	init.dynamic_states.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	init.dynamic_states.push_back(VK_DYNAMIC_STATE_SCISSOR);

	VkPipelineColorBlendAttachmentState colorBlend_attachment_state{};
	colorBlend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlend_attachment_state.blendEnable = VK_FALSE;
	if (type == Blend)
	{
		colorBlend_attachment_state.blendEnable = VK_TRUE;
		colorBlend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;

		init.depth_stencil.depthTestEnable = VK_TRUE;
		init.depth_stencil.depthWriteEnable = VK_FALSE;
		init.depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	}
	else if (type == FontBlend)
	{
		init.depth_stencil.depthTestEnable = VK_TRUE;
		init.depth_stencil.depthWriteEnable = VK_FALSE;
		init.depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		
		colorBlend_attachment_state.blendEnable = VK_TRUE;
		colorBlend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;//VK_BLEND_FACTOR_ONE VK_BLEND_FACTOR_DST_ALPHA
		colorBlend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;//VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA VK_BLEND_FACTOR_ONE
		colorBlend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;//VK_BLEND_FACTOR_ONE VK_BLEND_FACTOR_ZERO
		colorBlend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;//VK_BLEND_FACTOR_ZERO VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
		colorBlend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
	}
	else if (type == Wireframe)
	{
		init.rasterization.cullMode = VK_CULL_MODE_BACK_BIT;
		init.rasterization.polygonMode = VK_POLYGON_MODE_LINE;
		init.rasterization.lineWidth = 1.0f;
	}
	else if(type == Cubemap)
	{
		init.rasterization.cullMode = VK_CULL_MODE_BACK_BIT;

		init.depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		init.depth_stencil.depthTestEnable = VK_FALSE;
		init.depth_stencil.depthWriteEnable = VK_FALSE;
	}

	init.color_blend_attachment_states.push_back(colorBlend_attachment_state);

	return init;
}
VkShaderModule SVLTools::create_shader_module(VkDevice device, const std::vector<byte>& code)
{
	VkShaderModule shader_module = VK_NULL_HANDLE;
	VkShaderModuleCreateInfo shader_module_create_info = {};
	shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shader_module_create_info.codeSize = code.size();
	shader_module_create_info.pCode = (uint32_t*)code.data();
	ErrorCheck(vkCreateShaderModule(device, &shader_module_create_info, nullptr, &shader_module));
	return shader_module;
}
#pragma endregion

#pragma region Image
VkImageView SVLTools::create_2D_image_view(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspect_flags, uint32_t mip_levels)
{
	VkImageView image_view = VK_NULL_HANDLE;
	VkImageViewCreateInfo view_info = {};
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.image = image;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	view_info.format = format;
	view_info.subresourceRange.aspectMask = aspect_flags;
	view_info.subresourceRange.baseMipLevel = 0;
	view_info.subresourceRange.levelCount = mip_levels;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount = 1;
	ErrorCheck(vkCreateImageView(device, &view_info, nullptr, &image_view));
	return image_view;
}
#pragma endregion

#pragma region Buffer
void SVLTools::create_buffer_and_memory(VkDevice device, VkPhysicalDevice physical_device, VkQueue queue, VkCommandPool command_pool, VkDeviceSize buffer_size, const void * source, VkBufferUsageFlags staging_usage, VkMemoryPropertyFlags staging_properties, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer & buffer, VkDeviceMemory & buffer_memory)
{
	VkBuffer staging_buffer = buffer;
	VkDeviceMemory staging_memory = buffer_memory;
	create_buffer(device, physical_device, buffer_size, staging_usage, staging_properties, &staging_buffer, &staging_memory);
	void* data;
	ErrorCheck(vkMapMemory(device, staging_memory, 0, buffer_size, 0, &data));
	memcpy(data, source, (size_t)buffer_size);
	vkUnmapMemory(device, staging_memory);
	create_buffer(device, physical_device, buffer_size, usage, properties, &buffer, &buffer_memory);
	copy_buffer(device, queue, command_pool, staging_buffer, buffer, buffer_size);
	//destroy_staging
	vkFreeMemory(device, staging_memory, nullptr);
	vkDestroyBuffer(device, staging_buffer, nullptr);
}
void SVLTools::create_buffer(VkDevice device, VkPhysicalDevice physical_device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer * buffer, VkDeviceMemory * buffer_memory)
{
	VkBufferCreateInfo buffer_info = {};
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.size = size;
	buffer_info.usage = usage;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	ErrorCheck(vkCreateBuffer(device, &buffer_info, nullptr, buffer));
	VkMemoryRequirements memory_requirements;
	vkGetBufferMemoryRequirements(device, *buffer, &memory_requirements);
	VkMemoryAllocateInfo memory_allocate_info = {};
	memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memory_allocate_info.allocationSize = memory_requirements.size;
	memory_allocate_info.memoryTypeIndex = find_memory_type(physical_device, memory_requirements.memoryTypeBits, properties);
	ErrorCheck(vkAllocateMemory(device, &memory_allocate_info, nullptr, buffer_memory));
	ErrorCheck(vkBindBufferMemory(device, *buffer, *buffer_memory, 0));
}
void SVLTools::copy_buffer(VkDevice device, VkQueue queue, VkCommandPool command_pool, VkBuffer source_buffer, VkBuffer destination_buffer, VkDeviceSize size)
{
	VkCommandBuffer command_buffer = VK_NULL_HANDLE;
	begin_single_time_commands(device, command_pool, command_buffer);
	VkBufferCopy copy_region = {};
	copy_region.size = size;
	vkCmdCopyBuffer(command_buffer, source_buffer, destination_buffer, 1, &copy_region);
	end_single_time_commands(device, queue, command_pool, command_buffer);
}
#pragma endregion

#pragma region SingleTimeCommands
void SVLTools::begin_single_time_commands(VkDevice device, VkCommandPool command_pool, VkCommandBuffer & command_buffer)
{
	VkCommandBufferAllocateInfo allocation_info = {};
	allocation_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocation_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocation_info.commandPool = command_pool;
	allocation_info.commandBufferCount = 1;
	ErrorCheck(vkAllocateCommandBuffers(device, &allocation_info, &command_buffer));
	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	ErrorCheck(vkBeginCommandBuffer(command_buffer, &begin_info));
}
void SVLTools::end_single_time_commands(VkDevice device, VkQueue queue, VkCommandPool command_pool, VkCommandBuffer command_buffer)
{
	vkEndCommandBuffer(command_buffer);
	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffer;
	ErrorCheck(vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE));
	ErrorCheck(vkQueueWaitIdle(queue));
	vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);
}
#pragma endregion

#pragma region Other
uint32_t SVLTools::find_memory_type(VkPhysicalDevice physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memory_properties;
	vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);
	for (uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i)
	{
		if ((type_filter & (1 << i)) &&
			((memory_properties.memoryTypes[i].propertyFlags & properties) == properties))
			return i;
	}
	Error("SVL ERROR: failed to find suitable memory type");
	return VK_FORMAT_UNDEFINED;
}

VkFormat SVLTools::find_supported_format(VkPhysicalDevice physical_device, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates)
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physical_device, format, &props);
		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) return format;
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) return format;
	}
	Error("SVL ERROR: failed to find supported format.");
	return VK_FORMAT_UNDEFINED;
}/*
void SVLTools::transition_image_layout(VkDevice device, VkQueue queue, VkCommandPool command_pool, VkImage image, VkImageLayout old_layout, VkImageLayout new_layout, VkImageSubresourceRange subresource_range)
{
	VkCommandBuffer command_buffer = VK_NULL_HANDLE;
	begin_single_time_commands(device, command_pool, command_buffer);
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = old_layout;
	barrier.newLayout = new_layout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange = subresource_range;

	VkPipelineStageFlags src_stage, dst_stage;
	if(old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if(old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if(old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dst_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else
	{
		Error("unsupported layout transition");
	}
	vkCmdPipelineBarrier(command_buffer, src_stage, dst_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	end_single_time_commands(device, queue, command_pool, command_buffer);
}*/
std::vector<byte> SVLTools::read_file(const std::string file_name)
{
	std::ifstream file(file_name, std::ios::ate | std::ios::binary);
	if (!file.is_open())
	{
		Error("SVL ERROR: Cannot open file: " + file_name);
	}
	size_t fileSize = (size_t)file.tellg();
	std::vector<byte> buffer(fileSize);
	file.seekg(0);
	file.read((char*)buffer.data(), fileSize);
	file.close();
	return buffer;
}
std::string SVLTools::file_to_hex(const std::string file_name)
{
	std::stringstream s;
	std::ifstream input(file_name, std::ios::binary);
	input >> std::noskipws;
	byte x;
	while (input >> x)
	{
		s << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (int)x;
		//s << std::hex << std::setw(2) << (int)x;
	}
	return s.str();
}
std::vector<byte> SVLTools::hex_to_bytes(std::string hex)
{
	std::vector<byte> bytes;
	for (unsigned int i = 0; i < hex.length(); i += 2)
	{
		std::string byte_str = hex.substr(i, 2);
		byte b = (char)strtol(byte_str.c_str(), NULL, 16);
		bytes.push_back(b);
	}
	return bytes;
}
std::vector<std::string> SVLTools::split(std::string source, const char symbol)
{
	std::vector<std::string> list;
	std::string tmp;
	for (char v : source)
	{
		if (v != symbol)
		{
			tmp += v;
		}
		else
		{
			if (tmp != "") list.push_back(tmp);
			tmp = "";
		}
	}
	if (tmp != "") list.push_back(tmp);
	return list;
}
#pragma endregion
