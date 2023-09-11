#ifndef MESH_H
#define MESH_H
#include <SVL/definitions.h>

#include <vector>
#include "vertex.h"

namespace SVL
{
	class DLLDIR Mesh
	{
	public:
		std::vector<Vertex3D> vertices;
		std::vector<uint32_t> indices;
		uint64_t vertex_base = 0;
		uint64_t index_base = 0;
		uint32_t material_id = 0;
	};
}
#endif
