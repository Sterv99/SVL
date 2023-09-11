#include "loader.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <SVL/external/tiny_obj_loader.h>

#include <SVL/graphics/renderer.h>
#include <SVL/graphics/window.h>
#include <SVL/graphics/object.h>
#include <SVL/graphics/vertex.h>
//#include "../graphics/mesh.h"
//#include "../graphics/material.h"

SVL::Object SVL::Loader::load_obj(VkCommandPool command_pool, std::string filename, std::string mtl_dir)
{
	std::string err;
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	uint32_t mat_id = 0;

	if (mtl_dir == "") tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename.c_str());
	else tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename.c_str(), mtl_dir.c_str());

	std::vector<Mesh> meshes;
	std::vector<Material> mats;

	int i = 0;

	for (tinyobj::shape_t shape : shapes)
	{
		SVL::Mesh obj_mesh;
		uint32_t index_offset = 0;
		for (int face_vertices : shape.mesh.num_face_vertices)
		{
			for (size_t v = 0; v < face_vertices; v++)
			{
				tinyobj::index_t index = shape.mesh.indices[index_offset + v];

				SVL::Vertex3D vertex{};
				vertex.position.x = attrib.vertices[3 * index.vertex_index + 0];
				vertex.position.y = attrib.vertices[3 * index.vertex_index + 1];
				vertex.position.z = attrib.vertices[3 * index.vertex_index + 2];
				if (index.texcoord_index != -1)
				{
					vertex.tex_coord.x = attrib.texcoords[2 * index.texcoord_index + 0];
					vertex.tex_coord.y = 1.0f - attrib.texcoords[2 * index.texcoord_index + 1];
				}
				else vertex.tex_coord = glm::vec2(0.0f);
				if (index.normal_index != -1)
				{
					vertex.normal.x = attrib.normals[3 * index.normal_index + 0];
					vertex.normal.y = attrib.normals[3 * index.normal_index + 1];
					vertex.normal.z = attrib.normals[3 * index.normal_index + 2];
				}
				vertex.color = { 1.0f, 1.0f, 1.0f };

				obj_mesh.vertices.push_back(vertex);
				obj_mesh.indices.push_back(obj_mesh.indices.size());
			}
			index_offset += face_vertices;
		}
		if(shape.mesh.material_ids.size() != 0) obj_mesh.material_id = shape.mesh.material_ids[0];
		meshes.push_back(obj_mesh);
	}

	for (tinyobj::material_t material : materials)
	{
		Material::MaterialProperties properties{};
		Material::MaterialTextures textures{};

		if(material.diffuse_texname != "") textures.diffuse = load_img(VK_FORMAT_R8G8B8A8_UNORM, command_pool, mtl_dir + material.diffuse_texname);//VK_FORMAT_R8G8B8A8_UNORM
		//else mat->diffuse = SVL::load_ktx("resources/cringe.ktx", VK_FORMAT_R8G8B8A8_UNORM, window->command_pool(), renderer);

		//properties.diffuse = glm::vec4(material.diffuse[0], material.diffuse[1], material.diffuse[2], 0.0f);
		//properties.ambient = glm::vec4(material.ambient[0], material.ambient[1], material.ambient[2], 0.0f);
		//properties.specular = glm::vec4(material.specular[0], material.specular[1], material.specular[2], 1.0f);

		mats.push_back(Material(vk_renderer, textures, properties));
	}

	uint64_t vertex_offset = 0;
	uint64_t index_offset = 0;
	for (Mesh& mesh : meshes)
	{
		mesh.vertex_base = vertex_offset;
		vertex_offset += mesh.vertices.size();

		mesh.index_base = index_offset;
		index_offset += mesh.indices.size();
	}

	return Object(vk_renderer, command_pool, meshes, mats);
}
