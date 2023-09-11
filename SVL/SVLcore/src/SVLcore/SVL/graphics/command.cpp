#include "command.h"

#include <SVL/common/ErrorHandler.h>
#include "renderer.h"

SVL::Command::Command(const Renderer& renderer)
	: vk_renderer(renderer)
{
	VkCommandPoolCreateInfo command_pool_info{};
	command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	command_pool_info.queueFamilyIndex = vk_renderer.graphics_family_index();
	command_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	ErrorCheck(vkCreateCommandPool(vk_renderer.device(), &command_pool_info, nullptr, &vk_command_pool));
}

SVL::Command::~Command()
{
	vkDestroyCommandPool(vk_renderer.device(), vk_command_pool, nullptr);
}

void SVL::Command::destroy_command_buffers()
{
	vkFreeCommandBuffers(vk_renderer.device(), vk_command_pool, vk_command_buffers.size(), vk_command_buffers.data());
	vk_command_buffers.clear();
}



void SVL::PrimCommand::create_command_buffers(uint32_t size)
{
	vk_command_buffers.resize(size);

	VkCommandBufferAllocateInfo command_buffer_allocate_info = {};
	command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_allocate_info.commandPool = vk_command_pool;
	command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	command_buffer_allocate_info.commandBufferCount = vk_command_buffers.size();

	ErrorCheck(vkAllocateCommandBuffers(vk_renderer.device(), &command_buffer_allocate_info, vk_command_buffers.data()));
}

void SVL::SecCommand::create_command_buffers(uint32_t size)
{
	vk_command_buffers.resize(size);

	VkCommandBufferAllocateInfo command_buffer_allocate_info = {};
	command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_allocate_info.commandPool = vk_command_pool;
	command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
	command_buffer_allocate_info.commandBufferCount = vk_command_buffers.size();

	ErrorCheck(vkAllocateCommandBuffers(vk_renderer.device(), &command_buffer_allocate_info, vk_command_buffers.data()));
}