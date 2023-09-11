#ifndef MY_WINDOW_H
#define MY_WINDOW_H

#include <SVL/graphics/window.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class MyGLFWwindow
{
public:
	MyGLFWwindow(SVL::Renderer& r, uint32_t width, uint32_t height);
	~MyGLFWwindow();

	GLFWwindow* get_handle() { return window; }
protected:
	GLFWwindow* window;
};

class MyWindow : public MyGLFWwindow, public SVL::Window
{
public:
	MyWindow(SVL::Renderer&, uint32_t width, uint32_t height);
	~MyWindow();
	bool run();

	bool key_pressed[1024] = { false };
	bool mouse_key_pressed[24] = { false };
	double mouse_x, mouse_y;
};


#endif // !WINDOW_H