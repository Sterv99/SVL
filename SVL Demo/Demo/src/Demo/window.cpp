#include "window.h"
#include <SVL/common/ErrorHandler.h>
#include <SVL/graphics/renderer.h>
#include <SVL/graphics/tools.h>

MyGLFWwindow::MyGLFWwindow(SVL::Renderer& r, uint32_t width, uint32_t height)
{
	glfwInit();
	glfwDefaultWindowHints();
	glfwSwapInterval(1);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(width, height, r.app_name().c_str(), nullptr, nullptr);
}

MyGLFWwindow::~MyGLFWwindow()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

static void framebuffer_resize_callback(GLFWwindow* window, int width, int height)
{
	MyWindow* w = reinterpret_cast<MyWindow*>(glfwGetWindowUserPointer(window));
	w->set_extent(width, height);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	MyWindow* w = reinterpret_cast<MyWindow*>(glfwGetWindowUserPointer(window));
	//const char* s = glfwGetKeyName(key, scancode);
	switch (action)
	{
	case GLFW_PRESS:
		w->key_pressed[key] = true;
		break;
	case GLFW_RELEASE:
		w->key_pressed[key] = false;
		break;
	case GLFW_REPEAT:
		break;
	}
}

static void mouse_key_callback(GLFWwindow* window, int button, int action, int mods)
{
	MyWindow* w = reinterpret_cast<MyWindow*>(glfwGetWindowUserPointer(window));
	glfwGetCursorPos(window, &w->mouse_x, &w->mouse_y);

	switch (action)
	{
	case GLFW_PRESS:
		w->mouse_key_pressed[button] = true;
		break;
	case GLFW_RELEASE:
		w->mouse_key_pressed[button] = false;
		break;
	}
}

static void cursos_position_callback(GLFWwindow* window, double x, double y)
{
	MyWindow* w = reinterpret_cast<MyWindow*>(glfwGetWindowUserPointer(window));
	w->mouse_x = x;
	w->mouse_y = y;
}


MyWindow::MyWindow(SVL::Renderer& r, uint32_t width, uint32_t height)
:	MyGLFWwindow(r, width, height),
	SVL::Window(r, [&]()
		{
			VkSurfaceKHR surface;
			glfwCreateWindowSurface(r.instance(), window, NULL, &surface);
			return surface;
		}, width, height)
{
	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, framebuffer_resize_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_key_callback);
	glfwSetCursorPosCallback(window, cursos_position_callback);
}

MyWindow::~MyWindow()
{

}

bool MyWindow::run()
{
	glfwPollEvents();
	return !glfwWindowShouldClose(window);
}
