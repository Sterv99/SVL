#include "window.h"
#include "gui.h"

int main()
{
	SVL::Renderer renderer("SVL Demo", MAKE_VERSION(1, 0, 0), false);

	MyWindow window(renderer, 1856, 1046);
	double last_x, last_y;
	last_x = window.extent().width / 2;
	last_y = window.extent().height / 2;
	window.set_msaa(SVL::AntiAliasing::MSAA_8);

	GUIShareData data(renderer, window);

	MyGUI gui(window, window.get_handle());
	window.add_command(&gui);

	double now_time, last_time = glfwGetTime();
	while (window.run())
	{
		gui.new_frame();
		now_time = glfwGetTime();

		if (!gui.want_capture())
		{
			if (window.key_pressed[GLFW_KEY_W]) data.cam.translate(0.0f, 0.0f, data.cam_sensitivity * (now_time - last_time));
			if (window.key_pressed[GLFW_KEY_S]) data.cam.translate(0.0f, 0.0f, -data.cam_sensitivity * (now_time - last_time));
			if (window.key_pressed[GLFW_KEY_A]) data.cam.translate(data.cam_sensitivity * (now_time - last_time), 0.0f, 0.0f);
			if (window.key_pressed[GLFW_KEY_D]) data.cam.translate(-data.cam_sensitivity * (now_time - last_time), 0.0f, 0.0f);
			double x_offset = window.mouse_x - last_x;
			double y_offset = window.mouse_y - last_y;
			last_x = window.mouse_x;
			last_y = window.mouse_y;
			if (window.mouse_key_pressed[GLFW_MOUSE_BUTTON_1])
			{
				double yaw = x_offset * data.mouse_sensitivity;
				double pitch = y_offset * data.mouse_sensitivity;

				if (pitch > 89.0f) pitch = 89.0f;
				else if (pitch < -89.0f) pitch = -89.0f;

				data.cam.rotate(glm::vec3(pitch, yaw, 0.0f));
			}
		}
		last_time = now_time;

		data.update();
		gui.draw(data);
		window.draw();
	}
	return 0;
}