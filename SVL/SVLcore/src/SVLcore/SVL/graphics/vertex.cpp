#include "vertex.h"

VkVertexInputBindingDescription SVL::Vertex3D::binding_descriptor()
{
	VkVertexInputBindingDescription binding_descriptor{};
	binding_descriptor.binding = 0;
	binding_descriptor.stride = sizeof(Vertex3D);
	binding_descriptor.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return binding_descriptor;
}
std::array<VkVertexInputAttributeDescription, 5> SVL::Vertex3D::attribute_descriptor()
{
	std::array<VkVertexInputAttributeDescription, 5> attribute_descriptions{};

	attribute_descriptions[0].binding = 0;
	attribute_descriptions[0].location = 0;
	attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribute_descriptions[0].offset = offsetof(Vertex3D, position);

	attribute_descriptions[1].binding = 0;
	attribute_descriptions[1].location = 1;
	attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribute_descriptions[1].offset = offsetof(Vertex3D, color);

	attribute_descriptions[2].binding = 0;
	attribute_descriptions[2].location = 2;
	attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	attribute_descriptions[2].offset = offsetof(Vertex3D, tex_coord);

	attribute_descriptions[3].binding = 0;
	attribute_descriptions[3].location = 3;
	attribute_descriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribute_descriptions[3].offset = offsetof(Vertex3D, normal);

	attribute_descriptions[4].binding = 0;
	attribute_descriptions[4].location = 4;
	attribute_descriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribute_descriptions[4].offset = offsetof(Vertex3D, tangent);

	return attribute_descriptions;
}