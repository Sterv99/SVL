#ifndef LIGHT_H
#define LIGHT_H
#include <SVL/definitions.h>
#include <glm/glm.hpp>
namespace SVL
{
	struct PointLight
	{
		glm::vec4 position = glm::vec4(0.0f); //3d pos, empty
		glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f); //rgb, enabled
		glm::vec4 params = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f); //attenuation, empty, empty, empty
	};
}
#endif
