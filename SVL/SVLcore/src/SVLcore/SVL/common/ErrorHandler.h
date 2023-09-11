#ifndef ERROR_HANDLER
#define ERROR_HANDLER
#include <iostream>
#include <time.h>
#include <assert.h>
#include <SVL/definitions.h>
#include <vulkan/vulkan.h>
static void Log(std::string msg)
{
	time_t t = time(0);
	tm now;
	localtime_s(&now, &t);
	//now = localtime(&t);

	std::cout << "<" << now.tm_hour << ":" << now.tm_min << ":" << now.tm_sec << " " << now.tm_mday << "." << now.tm_mon + 1 << "." << now.tm_year + 1900 << ">" << msg.c_str() << std::endl;
}
static void Error(std::string msg)
{
	Log(msg);
	assert(0 && "Internal runtime error.");
	std::exit(-1);
}
static VkResult ErrorCheck(VkResult result)
{
	if (result < 0)
	{
		switch (result)
		{
		case VK_ERROR_OUT_OF_HOST_MEMORY:
			Log("VK_ERROR_OUT_OF_HOST_MEMORY");
			break;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
			Log("VK_ERROR_OUT_OF_DEVICE_MEMORY");
			break;
		case VK_ERROR_INITIALIZATION_FAILED:
			Log("VK_ERROR_INITIALIZATION_FAILED");
			break;
		case VK_ERROR_DEVICE_LOST:
			Log("VK_ERROR_DEVICE_LOST");
			break;
		case VK_ERROR_MEMORY_MAP_FAILED:
			Log("VK_ERROR_MEMORY_MAP_FAILED");
			break;
		case VK_ERROR_LAYER_NOT_PRESENT:
			Log("VK_ERROR_LAYER_NOT_PRESENT");
			break;
		case VK_ERROR_EXTENSION_NOT_PRESENT:
			Log("VK_ERROR_EXTENSION_NOT_PRESENT");
			break;
		case VK_ERROR_FEATURE_NOT_PRESENT:
			Log("VK_ERROR_FEATURE_NOT_PRESENT");
			break;
		case VK_ERROR_INCOMPATIBLE_DRIVER:
			Log("VK_ERROR_INCOMPATIBLE_DRIVER");
			break;
		case VK_ERROR_TOO_MANY_OBJECTS:
			Log("VK_ERROR_TOO_MANY_OBJECTS");
			break;
		case VK_ERROR_FORMAT_NOT_SUPPORTED:
			Log("VK_ERROR_FORMAT_NOT_SUPPORTED");
			break;
		case VK_ERROR_SURFACE_LOST_KHR:
			Log("VK_ERROR_SURFACE_LOST_KHR");
			break;
		case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
			Log("VK_ERROR_NATIVE_WINDOW_IN_USE_KHR");
			break;
		case VK_SUBOPTIMAL_KHR:
			Log("VK_SUBOPTIMAL_KHR");
			break;
		case VK_ERROR_OUT_OF_DATE_KHR:
			Log("VK_ERROR_OUT_OF_DATE_KHR");
			break;
		case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
			Log("VK_ERROR_INCOMPATIBLE_DISPLAY_KHR");
			break;
		case VK_ERROR_VALIDATION_FAILED_EXT:
			Log("VK_ERROR_VALIDATION_FAILED_EXT");
			break;
		case VK_ERROR_INVALID_SHADER_NV:
			Log("VK_ERROR_INVALID_SHADER_NV");
			break;
		default:
			break;
		}
		assert(0 && "Vulkan runtime error.");
	}
	return result;
}
#endif // !ERROR_HANDLER
