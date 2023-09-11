#ifndef COMMAND_H
#define COMMAND_H

#include <SVL/definitions.h>
#include <vulkan/vulkan.h>
#include <vector>

namespace SVL
{
	class Renderer;
	class DLLDIR Command
	{
	public:
		Command(const Renderer&);
		~Command();

		const VkCommandPool command_pool() const { return vk_command_pool; }
		const std::vector<VkCommandBuffer> command_buffers() { return vk_command_buffers; }
		const VkCommandBuffer command_buffer(uint32_t i) { return vk_command_buffers[i]; }
	protected:
		const class Renderer& vk_renderer;

		VkCommandPool vk_command_pool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer> vk_command_buffers;

		virtual void create_command_buffers(uint32_t) {}
		void destroy_command_buffers();
	};

	class DLLDIR PrimCommand : public Command
	{
	public:
		PrimCommand(const Renderer& r) : SVL::Command(r) {}
		virtual void update_command_buffers(uint32_t) {}
	protected:
		void create_command_buffers(uint32_t);
	};

	class DLLDIR SecCommand : public Command
	{
	public:
		SecCommand(const Renderer& r) : SVL::Command(r) {}
		virtual void update() {}
		virtual void update_command_buffers(VkCommandBufferInheritanceInfo, uint32_t) {}
	protected:
		void create_command_buffers(uint32_t);
	};
}

#endif // !COMMAND_H