#ifndef GUI_H
#define GUI_H

#include <SVL/SVL.h>

#include "external/imgui/imgui_impl_glfw.h"

struct GUIShareData
{
	SVL::Renderer& renderer;
	SVL::Loader loader;
	VkCommandPool command_pool;

	SVL::Layer3D cubemap_layer, object_layer;

	SVL::Camera cam;

	float cam_sensitivity = 5.0f;
	float mouse_sensitivity = 0.002f;

	GUIShareData(SVL::Renderer& r, SVL::Window& w)
		: renderer(r),
		loader(r),
		command_pool(w.command_pool()),
		cubemap_layer(w, "../resources/shaders/cubemap/vert.spv", "../resources/shaders/cubemap/frag.spv", SVLTools::Cubemap),
		object_layer(w, "../resources/shaders/object/vert.spv", "../resources/shaders/object/frag.spv"),
		cam()
	{
		w.add_command(&cubemap_layer);
		w.add_command(&object_layer);

		object_layer.set_camera(&cam);
		cubemap_layer.set_camera(&cam);
	}

	~GUIShareData()
	{
		for (auto obj : cubemap_layer.get_objects())
		{
			delete obj;
		}
		for (auto obj : object_layer.get_objects())
		{
			delete obj;
		}
	}

	void update()
	{
		cubemap_layer.update_uniforms();
		object_layer.update_uniforms();
	}
};

class GLFWwindow;
class MyGUI : public SVL::SecCommand
{
public:
	MyGUI(const SVL::Window& window, GLFWwindow*);
	~MyGUI();

	void update() override;
	void update_command_buffers(VkCommandBufferInheritanceInfo, uint32_t) override;

	void new_frame();
	void draw(GUIShareData& data);

	bool want_capture();
private:
	const class SVL::Window& window;
	VkDescriptorPool vk_descriptor_pool;

	void create_descriptor_pool();
	void destroy_descriptor_pool();
};

#endif // !GUI_H
