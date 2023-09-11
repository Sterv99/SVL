#ifndef VERTEX_H
#define VERTEX_H
#include <SVL/definitions.h>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <array>

namespace SVL
{
	class DLLDIR Vertex3D final
	{
	public:
		glm::vec3 position;
		glm::vec3 color;
		glm::vec2 tex_coord;
		glm::vec3 normal;
		glm::vec3 tangent;

		static VkVertexInputBindingDescription binding_descriptor();
		static std::array<VkVertexInputAttributeDescription, 5> attribute_descriptor();

		bool operator== (const Vertex3D & other) const
		{
			return position == other.position && color == other.color && tex_coord == other.tex_coord && normal == other.normal && other.tangent == tangent;
		}
	};
}
#endif