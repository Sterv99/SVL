#include "loader.h"

#include <SVL/common/ErrorHandler.h>
#include <SVL/graphics/renderer.h>
#include <SVL/graphics/window.h>
#include <SVL/graphics/object.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <glm/gtc/type_ptr.hpp>

SVL::Mesh assimp_obj_process_mesh(aiMesh* mesh)
{
	SVL::Mesh obj_mesh;

	for (uint32_t i = 0; i < mesh->mNumVertices; i++)
	{
		SVL::Vertex3D vertex;
		vertex.position.x = mesh->mVertices[i].x;
		vertex.position.y = mesh->mVertices[i].y;
		vertex.position.z = mesh->mVertices[i].z;

		if (mesh->HasNormals())
		{
			vertex.normal.x = mesh->mNormals[i].x;
			vertex.normal.y = mesh->mNormals[i].y;
			vertex.normal.z = mesh->mNormals[i].z;
		}
		else vertex.normal = glm::vec3(0.0f, 0.0f, 0.0f);

		if (mesh->HasTextureCoords(0))
		{
			vertex.tex_coord.x = mesh->mTextureCoords[0][i].x;
			vertex.tex_coord.y = mesh->mTextureCoords[0][i].y;
		}
		else vertex.tex_coord = glm::vec2(0.0f, 0.0f);

		if (mesh->HasVertexColors(0))
		{
			vertex.color.r = mesh->mColors[0][i].r;
			vertex.color.g = mesh->mColors[0][i].g;
			vertex.color.b = mesh->mColors[0][i].b;
		}
		else vertex.color = glm::vec3(1.0f, 1.0f, 1.0f);

		if (mesh->HasTangentsAndBitangents())
		{
			vertex.tangent.x = mesh->mTangents[i].x;
			vertex.tangent.y = mesh->mTangents[i].y;
			vertex.tangent.z = mesh->mTangents[i].z;
		}
		else vertex.tangent = glm::vec3(0.0f, 0.0f, 0.0f);

		obj_mesh.vertices.push_back(vertex);
	}

	for (uint32_t i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (uint32_t j = 0; j < face.mNumIndices; j++)
		{
			obj_mesh.indices.push_back(face.mIndices[j]);
		}
	}

	obj_mesh.material_id = mesh->mMaterialIndex;

	return obj_mesh;
}

SVL::Material assimp_obj_process_material(SVL::Loader* loader, aiMaterial* mat, const SVL::Renderer& renderer, VkCommandPool command_pool, std::string directory)
{
	SVL::Material::MaterialTextures textures{};
	SVL::Material::MaterialProperties properties{};

	aiColor4D color;
	mat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
	properties.diffuse = glm::make_vec4(&color.r);
	mat->Get(AI_MATKEY_COLOR_AMBIENT, color);
	properties.ambient = glm::make_vec4(&color.r);
	mat->Get(AI_MATKEY_COLOR_SPECULAR, color);
	properties.specular = glm::make_vec4(&color.r);
	mat->Get(AI_MATKEY_OPACITY, properties.opacity);
	mat->Get(AI_MATKEY_SHININESS, properties.shininess);

	if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0)
	{
		aiString path;
		mat->GetTexture(aiTextureType_DIFFUSE, 0, &path);

		textures.diffuse = loader->load_img(renderer, VK_FORMAT_R8G8B8A8_UNORM, command_pool, directory + path.C_Str());//SVL::load_img(directory + path.C_Str(), VK_FORMAT_R8G8B8A8_UNORM, window->command_pool(), renderer);
	}
	else
	{
		textures.diffuse = loader->load_ktx(renderer, VK_FORMAT_R8G8B8A8_UNORM, command_pool, "resources/cringe.ktx");
	}

	if (mat->GetTextureCount(aiTextureType_AMBIENT) > 0)
	{
		aiString path;
		mat->GetTexture(aiTextureType_AMBIENT, 0, &path);

		textures.ambient = loader->load_img(renderer, VK_FORMAT_R8G8B8A8_UNORM, command_pool, directory + path.C_Str());

		properties.has_ambient_tex = true;
	}

	if (mat->GetTextureCount(aiTextureType_SPECULAR) > 0)
	{
		aiString path;
		mat->GetTexture(aiTextureType_SPECULAR, 0, &path);

		textures.specular = loader->load_img(renderer, VK_FORMAT_R8G8B8A8_UNORM, command_pool, directory + path.C_Str());//SVL::load_img(directory + path.C_Str(), VK_FORMAT_R8G8B8A8_UNORM, window->command_pool(), renderer);
	
		properties.has_specular_tex = 1.0f;
	}

	if (mat->GetTextureCount(aiTextureType_NORMALS) > 0)
	{
		aiString path;
		mat->GetTexture(aiTextureType_NORMALS, 0, &path);

		textures.normal = loader->load_img(renderer, VK_FORMAT_R8G8B8A8_UNORM, command_pool, directory + path.C_Str());//SVL::load_img(directory + path.C_Str(), VK_FORMAT_R8G8B8A8_UNORM, window->command_pool(), renderer);
	
		properties.has_normal_tex = 1.0f;
	}
	else if (mat->GetTextureCount(aiTextureType_HEIGHT) > 0)
	{
		aiString path;
		mat->GetTexture(aiTextureType_HEIGHT, 0, &path);

		textures.normal = loader->load_img(renderer, VK_FORMAT_R8G8B8A8_UNORM, command_pool, directory + path.C_Str());//SVL::load_img(directory + path.C_Str(), VK_FORMAT_R8G8B8A8_UNORM, window->command_pool(), renderer);
	
		properties.has_normal_tex = 1.0f;
	}

	if (mat->GetTextureCount(aiTextureType_DISPLACEMENT) > 0)
	{
		aiString path;
		mat->GetTexture(aiTextureType_DISPLACEMENT, 0, &path);

		textures.displacement = loader->load_img(renderer, VK_FORMAT_R8G8B8A8_UNORM, command_pool, directory + path.C_Str());

		//properties.has_specular_tex = 1.0f;
	}

	if (mat->GetTextureCount(aiTextureType_SHININESS) > 0)
	{
		aiString path;
		mat->GetTexture(aiTextureType_SHININESS, 0, &path);

		textures.shininess = loader->load_img(renderer, VK_FORMAT_R8G8B8A8_UNORM, command_pool, directory + path.C_Str());

		//properties.has_specular_tex = 1.0f;
	}
	

	
	if (mat->GetTextureCount(aiTextureType_METALNESS) > 0)
	{
		aiString path;
		mat->GetTexture(aiTextureType_METALNESS, 0, &path);

		textures.metalness = loader->load_img(renderer, VK_FORMAT_R8G8B8A8_UNORM, command_pool, directory + path.C_Str());

		//properties.has_specular_tex = 1.0f;
	}

	if (mat->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS) > 0)
	{
		aiString path;
		mat->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &path);

		textures.roughness = loader->load_img(renderer, VK_FORMAT_R8G8B8A8_UNORM, command_pool, directory + path.C_Str());

		//properties.has_specular_tex = 1.0f;
	}
	

	return SVL::Material(renderer, textures, properties);
}

void assimp_obj_process_node(SVL::Loader* loader, const aiScene* scene, aiNode* node, std::vector<SVL::Mesh>& meshes, std::vector<SVL::Material>& materials, const SVL::Renderer& renderer, VkCommandPool command_pool, std::string directory)
{
	for (uint32_t i = 0; i < node->mNumMeshes; i++)
	{
		meshes.push_back(assimp_obj_process_mesh(scene->mMeshes[node->mMeshes[i]]));

		if (meshes[meshes.size() - 1].material_id >= 0)
		{
			materials.push_back(assimp_obj_process_material(loader, scene->mMaterials[meshes[meshes.size() - 1].material_id], renderer, command_pool, directory));
			meshes[meshes.size() - 1].material_id = materials.size() - 1;
		}
	}

	for (uint32_t i = 0; i < node->mNumChildren; i++)
	{
		assimp_obj_process_node(loader, scene, node->mChildren[i], meshes, materials, renderer, command_pool, directory);
	}
}

static void DumpMaterialsToConsole(const aiScene& scene)
{
	const auto dump = [](const aiMaterial& mat, aiTextureType type)
	{
		const unsigned count = mat.GetTextureCount(type);
		if (count > 0)
			std::cout << "  type:" << type << " count:" << count << std::endl;
	};

	for (unsigned i = 0; i < scene.mNumMaterials; i++)
	{
		const aiMaterial& mat = *scene.mMaterials[i];
		std::cout << "mat:" << i << " has..." << std::endl;
		dump(mat, aiTextureType_NONE);
		dump(mat, aiTextureType_DIFFUSE);
		dump(mat, aiTextureType_SPECULAR);
		dump(mat, aiTextureType_AMBIENT);
		dump(mat, aiTextureType_EMISSIVE);
		dump(mat, aiTextureType_HEIGHT);
		dump(mat, aiTextureType_NORMALS);
		dump(mat, aiTextureType_SHININESS);
		dump(mat, aiTextureType_OPACITY);
		dump(mat, aiTextureType_DISPLACEMENT);
		dump(mat, aiTextureType_LIGHTMAP);
		dump(mat, aiTextureType_REFLECTION);
		dump(mat, aiTextureType_BASE_COLOR);
		dump(mat, aiTextureType_NORMAL_CAMERA);
		dump(mat, aiTextureType_EMISSION_COLOR);
		dump(mat, aiTextureType_METALNESS);
		dump(mat, aiTextureType_DIFFUSE_ROUGHNESS);
		dump(mat, aiTextureType_AMBIENT_OCCLUSION);
		dump(mat, aiTextureType_UNKNOWN);
	}
}

SVL::Object SVL::Loader::load_assimp(const Renderer& renderer, const Window& window, std::string filename, uint32_t flags, std::string assets_dir)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filename.c_str(), flags);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		Error("Loader: cant load file: " + filename);
	}

	DumpMaterialsToConsole(*scene);

	std::vector<SVL::Mesh> meshes;
	std::vector<SVL::Material> materials;

	assimp_obj_process_node(this, scene, scene->mRootNode, meshes, materials, renderer, window.command_pool(), assets_dir);

	uint64_t vertex_offset = 0;
	uint64_t index_offset = 0;
	for (SVL::Mesh& obj_mesh : meshes)
	{
		obj_mesh.vertex_base = vertex_offset;
		vertex_offset += obj_mesh.vertices.size();

		obj_mesh.index_base = index_offset;
		index_offset += obj_mesh.indices.size();
	}

	return SVL::Object(renderer, window.command_pool(), meshes, materials);
}