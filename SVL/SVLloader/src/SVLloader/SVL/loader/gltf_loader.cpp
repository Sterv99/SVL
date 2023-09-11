#include "loader.h"

#include <SVL/common/ErrorHandler.h>
#include <SVL/graphics/renderer.h>
#include <SVL/graphics/window.h>
#include <SVL/graphics/model.h>
#include <SVL/graphics/texture.h>

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <SVL/external/tiny_gltf.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


SVL::Mesh process_mesh(const tinygltf::Model& model, const tinygltf::Mesh mesh)
{
	SVL::Mesh m;
	for (auto prim : mesh.primitives)
	{
		if (prim == *mesh.primitives.begin() && prim.material >= 0)
			m.material_id = prim.material;

		size_t vertex_size;
		const float* pos_buffer = nullptr;
		int pos_stride = -1;
		if (prim.attributes.find("POSITION") != prim.attributes.end())
		{
			const tinygltf::Accessor& acc = model.accessors[prim.attributes.find("POSITION")->second];
			const tinygltf::BufferView& view = model.bufferViews[acc.bufferView];
			vertex_size = acc.count;

			pos_buffer = reinterpret_cast<const float*>(&(model.buffers[view.buffer].data[acc.byteOffset + view.byteOffset]));
			pos_stride = acc.ByteStride(view) ? (acc.ByteStride(view) / sizeof(float)) : (tinygltf::GetComponentSizeInBytes(TINYGLTF_COMPONENT_TYPE_FLOAT) * tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3));
		}

		const float* nor_buffer = nullptr;
		int nor_stride;
		if (prim.attributes.find("NORMAL") != prim.attributes.end())
		{
			const tinygltf::Accessor& acc = model.accessors[prim.attributes.find("NORMAL")->second];
			const tinygltf::BufferView& view = model.bufferViews[acc.bufferView];

			nor_buffer = reinterpret_cast<const float*>(&(model.buffers[view.buffer].data[acc.byteOffset + view.byteOffset]));
			nor_stride = acc.ByteStride(view) ? (acc.ByteStride(view) / sizeof(float)) : (tinygltf::GetComponentSizeInBytes(TINYGLTF_COMPONENT_TYPE_FLOAT) * tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3));
		}

		const float* uv0_buffer = nullptr;
		int uv0_stride;
		if (prim.attributes.find("TEXCOORD_0") != prim.attributes.end())
		{
			const tinygltf::Accessor& acc = model.accessors[prim.attributes.find("TEXCOORD_0")->second];
			const tinygltf::BufferView& view = model.bufferViews[acc.bufferView];

			uv0_buffer = reinterpret_cast<const float*>(&(model.buffers[view.buffer].data[acc.byteOffset + view.byteOffset]));
			uv0_stride = acc.ByteStride(view) ? (acc.ByteStride(view) / sizeof(float)) : (tinygltf::GetComponentSizeInBytes(TINYGLTF_COMPONENT_TYPE_FLOAT) * tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3));
		}

		const float* tan_buffer = nullptr;
		int tan_stride;
		if (prim.attributes.find("TANGENT") != prim.attributes.end())
		{
			const tinygltf::Accessor& acc = model.accessors[prim.attributes.find("TANGENT")->second];
			const tinygltf::BufferView& view = model.bufferViews[acc.bufferView];

			tan_buffer = reinterpret_cast<const float*>(&(model.buffers[view.buffer].data[acc.byteOffset + view.byteOffset]));
			tan_stride = acc.ByteStride(view) ? (acc.ByteStride(view) / sizeof(float)) : (tinygltf::GetComponentSizeInBytes(TINYGLTF_COMPONENT_TYPE_FLOAT) * tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3));
		}


		for (size_t i = 0; i < vertex_size; i++)
		{
			SVL::Vertex3D v{};
			
			v.position = glm::make_vec3(pos_buffer + (i * pos_stride));

			if (nor_buffer && nor_stride >= 0)
				v.normal = glm::make_vec3(nor_buffer + (i * nor_stride));

			if (uv0_buffer && uv0_stride >= 0)
				v.tex_coord = glm::make_vec3(uv0_buffer + (i * uv0_stride));

			if (tan_buffer && tan_stride >= 0)
				v.tangent = glm::make_vec3(tan_buffer + (i * tan_stride));

			m.vertices.push_back(v);
		}




		if (prim.indices >= 0)
		{
			const tinygltf::Accessor& acc = model.accessors[prim.indices];
			const tinygltf::BufferView& view = model.bufferViews[acc.bufferView];
			size_t index_size = acc.count;

			if (acc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
			{
				const uint32_t* buffer = reinterpret_cast<const uint32_t*>(model.buffers[view.buffer].data.data() + acc.byteOffset + view.byteOffset);
				for (size_t i = 0; i < index_size; i++)
				{
					m.indices.push_back(buffer[i]);
				}
			}
			else if (acc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
			{
				const uint16_t* buffer = reinterpret_cast<const uint16_t*>(model.buffers[view.buffer].data.data() + acc.byteOffset + view.byteOffset);
				for (size_t i = 0; i < index_size; i++)
				{
					m.indices.push_back(buffer[i]);
				}
			}
			else if (acc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
			{
				const uint8_t* buffer = reinterpret_cast<const uint8_t*>(model.buffers[view.buffer].data.data() + acc.byteOffset + view.byteOffset);
				for (size_t i = 0; i < index_size; i++)
				{
					m.indices.push_back(buffer[i]);
				}
			}
			else
			{
				Error("Not supported index component type!");
			}
		}
	}
	return m;
}

SVL::Material process_material(const SVL::Renderer& renderer, const VkCommandPool& command_pool, const tinygltf::Model& model, const tinygltf::Material material)
{
	SVL::Material::MaterialTextures tex{};
	SVL::Material::MaterialProperties prop{};

	if (material.pbrMetallicRoughness.baseColorTexture.index >= 0)
	{
		const tinygltf::Texture t = model.textures[material.pbrMetallicRoughness.baseColorTexture.index];
		const tinygltf::Image img = model.images[t.source];

		VkFormat format{};
		if (img.pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
			format = VK_FORMAT_R8G8B8A8_UNORM;
		else if(img.pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
			format = VK_FORMAT_R16G16B16A16_UNORM;

		tex.diffuse = new SVL::Texture(renderer, command_pool, static_cast<const void*>(img.image.data()), img.image.size(), { static_cast<uint32_t>(img.width), static_cast<uint32_t>(img.height) }, format);
	}

	if (material.normalTexture.index >= 0)
	{
		const tinygltf::Texture t = model.textures[material.normalTexture.index];
		const tinygltf::Image img = model.images[t.source];

		VkFormat format{};
		if (img.pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
			format = VK_FORMAT_R8G8B8A8_UNORM;
		else if (img.pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
			format = VK_FORMAT_R16G16B16A16_UNORM;

		prop.has_normal_tex = true;
		tex.normal = new SVL::Texture(renderer, command_pool, static_cast<const void*>(img.image.data()), img.image.size(), { static_cast<uint32_t>(img.width), static_cast<uint32_t>(img.height) }, format);
	}

	if (material.pbrMetallicRoughness.metallicRoughnessTexture.index >= 0)
	{
		const tinygltf::Texture t = model.textures[material.pbrMetallicRoughness.metallicRoughnessTexture.index];
		const tinygltf::Image img = model.images[t.source];

		VkFormat format{};
		if (img.pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
			format = VK_FORMAT_R8G8B8A8_UNORM;
		else if (img.pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
			format = VK_FORMAT_R16G16B16A16_UNORM;

		prop.has_ao_tex = 1;
		tex.metalness_roughness = new SVL::Texture(renderer, command_pool, static_cast<const void*>(img.image.data()), img.image.size(), { static_cast<uint32_t>(img.width), static_cast<uint32_t>(img.height) }, format);
	}

	if (material.occlusionTexture.index >= 0)
	{
		const tinygltf::Texture t = model.textures[material.occlusionTexture.index];
		const tinygltf::Image img = model.images[t.source];

		VkFormat format{};
		if (img.pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
			format = VK_FORMAT_R8G8B8A8_UNORM;
		else if (img.pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
			format = VK_FORMAT_R16G16B16A16_UNORM;

		prop.has_ao_tex = 2;
		tex.ambient_occulsion = new SVL::Texture(renderer, command_pool, static_cast<const void*>(img.image.data()), img.image.size(), { static_cast<uint32_t>(img.width), static_cast<uint32_t>(img.height) }, format);
	}
	
	return SVL::Material(renderer, tex, prop);
}

void process_node(const tinygltf::Model& model, const tinygltf::Node& node, std::vector<SVL::Mesh>& meshes)
{
	if (node.mesh >= 0)
	{
		meshes.push_back(process_mesh(model, model.meshes[node.mesh]));
	}

	for (int child_i : node.children)
	{
		if(child_i >= 0)
			process_node(model, model.nodes[child_i], meshes);
	}
}

SVL::Model SVL::Loader::load_gltf(VkCommandPool command_pool, std::string filename)
{
	tinygltf::TinyGLTF gltf_loader;
	std::string war, err;

	tinygltf::Model model;

	bool ret = gltf_loader.LoadBinaryFromFile(&model, &err, &war, filename);

	if (!err.empty() || !ret)
	{
		Error("Loader: cant load file: " + filename);
	}
	
	std::vector<SVL::Mesh> meshes;
	std::vector<SVL::Material> materials;

	for (auto mat : model.materials)
	{
		materials.push_back(process_material(vk_renderer, command_pool, model, mat));
	}



	tinygltf::Scene scene = model.scenes[model.defaultScene >= 0 ? model.defaultScene : 0];
	for (auto node_i : scene.nodes)
	{
		process_node(model, model.nodes[node_i], meshes);
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
	
	return Model(vk_renderer, command_pool, meshes, materials);
}