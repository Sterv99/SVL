#ifndef CAMERA_H
#define CAMERA_H

#include <SVL/definitions.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace SVL
{
	class DLLDIR Camera final
	{
	public:
		Camera() = default;
		Camera(const Camera &) = default;
		Camera& operator =(const Camera&) = default;
		Camera(Camera&&) = default;
		Camera& operator=(Camera&&) = default;

		Camera(const glm::vec3& pos, const glm::quat& orient) : _pos(pos), _orient(orient) {}

		const glm::vec3 position() const { return _pos; }
		const glm::quat orientation() const { return _orient; }

		const glm::mat4 const view();

		void translate(const glm::vec3& v);
		void translate(float x, float y, float z);

		void rotate(const glm::vec3& v);
		void rotate(float pitch, float yaw, float roll);
	private:
		glm::vec3 _pos;
		glm::quat _orient;
	};
}
#endif
