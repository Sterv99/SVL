#include "camera.h"

#include <glm/gtc/matrix_transform.hpp>

const glm::mat4 const SVL::Camera::view()
{
    return glm::translate(glm::mat4_cast(_orient), _pos);
}

void SVL::Camera::translate(float x, float y, float z)
{
    _pos += glm::vec3(x, y, z) * glm::normalize(_orient);
}
void SVL::Camera::translate(const glm::vec3& v)
{
    _pos += v * glm::normalize(_orient);
}

void SVL::Camera::rotate(float pitch, float yaw, float roll)
{
    _orient = glm::quat(glm::vec3(pitch, yaw, roll)) * glm::normalize(_orient);
}
void SVL::Camera::rotate(const glm::vec3& v)
{
    _orient = glm::quat(v) * glm::normalize(_orient);
}
