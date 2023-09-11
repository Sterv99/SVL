#include "gui.h"

#include <SVL/common/ErrorHandler.h>
#include <SVL/graphics/renderer.h>
#include <SVL/graphics/tools.h>

#include "external/imgui/imgui.h"
#include "external/imgui/imgui_impl_vulkan.h"
#include "external/imgui-filebrowser/imfilebrowser.h"

#include <glm/gtx/euler_angles.hpp>

#include <unordered_map>

std::vector<SVL::Vertex3D> cube_vertices = {
	//front
	{ { 1.0f,  1.0f, 1.0f },{ 1.0f, 1.0f, 1.0f },{ -1.0f, -1.0f },{ 1.0f, 1.0f, 1.0f } },
	{ { -1.0f,  1.0f, 1.0f },{ 1.0f, 1.0f, 1.0f },{ -1.0f, 0.0f },{ 1.0f, 1.0f, 1.0f } },
	{ { -1.0f, -1.0f, 1.0f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f },{ 1.0f, 1.0f, 1.0f } },
	{ { 1.0f, -1.0f, 1.0f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, -1.0f },{ 1.0f, 1.0f, 1.0f } },

	//back
	{ { 1.0f,  1.0f, -1.0f },{ 1.0f, 1.0f, 1.0f },{ -1.0f, -1.0f },{ 1.0f, 1.0f, 1.0f } },
	{ { -1.0f,  1.0f, -1.0f },{ 1.0f, 1.0f, 1.0f },{ -1.0f, 0.0f },{ 1.0f, 1.0f, 1.0f } },
	{ { -1.0f, -1.0f, -1.0f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f },{ 1.0f, 1.0f, 1.0f } },
	{ { 1.0f, -1.0f, -1.0f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, -1.0f },{ 1.0f, 1.0f, 1.0f } }
};
std::vector<uint32_t> cube_indices = {
	0, 1, 2, 2, 3, 0,
	1, 5, 6, 6, 2, 1,
	7, 6, 5, 5, 4, 7,
	4, 0, 3, 3, 7, 4,
	4, 5, 1, 1, 0, 4,
	3, 2, 6, 6, 7, 3
};

SVL::Mesh cubemesh{ cube_vertices, cube_indices };

static std::unordered_map<std::string, ImGui::FileBrowser> fbs;

MyGUI::MyGUI(const SVL::Window& window, GLFWwindow* glfw)
: SVL::SecCommand(window.renderer()), window(window)
{
	create_descriptor_pool();
	create_command_buffers(window.framebuffers()->size());

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForVulkan(glfw, true);
	ImGui_ImplVulkan_InitInfo init_info{};
	init_info.Instance = vk_renderer.instance();
	init_info.PhysicalDevice = vk_renderer.physical_device();
	init_info.Device = vk_renderer.device();
	init_info.QueueFamily = vk_renderer.graphics_family_index();
	init_info.Queue = vk_renderer.queue();
	init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.DescriptorPool = vk_descriptor_pool;
	init_info.Allocator = nullptr;
	init_info.MinImageCount = window.image_count();
	init_info.ImageCount = window.image_count();
	init_info.CheckVkResultFn = [](VkResult r) {ErrorCheck(r); };
	init_info.MSAASamples = window.get_sample_count();
	ImGui_ImplVulkan_Init(&init_info, window.render_pass());

	VkCommandBuffer command_buffer;
	SVLTools::begin_single_time_commands(vk_renderer.device(), vk_command_pool, command_buffer);
	ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
	SVLTools::end_single_time_commands(vk_renderer.device(), vk_renderer.queue(), vk_command_pool, command_buffer);
	ImGui_ImplVulkan_DestroyFontUploadObjects();
}

MyGUI::~MyGUI()
{
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	destroy_command_buffers();
	destroy_descriptor_pool();
}

void MyGUI::update()
{
	destroy_command_buffers();

	create_command_buffers(window.framebuffers()->size());
}

void MyGUI::update_command_buffers(VkCommandBufferInheritanceInfo inheritance_info, uint32_t index)
{
	VkCommandBufferBeginInfo command_buffer_begin_info = {};
	command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	command_buffer_begin_info.pInheritanceInfo = &inheritance_info;

	ErrorCheck(vkBeginCommandBuffer(vk_command_buffers[index], &command_buffer_begin_info));

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), vk_command_buffers[index]);

	ErrorCheck(vkEndCommandBuffer(vk_command_buffers[index]));
}

void MyGUI::create_descriptor_pool()
{
	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};
	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
	pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;
	ErrorCheck(vkCreateDescriptorPool(vk_renderer.device(), &pool_info, nullptr, &vk_descriptor_pool));
}

void MyGUI::destroy_descriptor_pool()
{
	vkDestroyDescriptorPool(vk_renderer.device(), vk_descriptor_pool, nullptr);
}

inline
static void draw_app(GUIShareData& data)
{
	ImGui::Begin("App");

	ImGui::Text("FPS: %.1f FPS (%.3f ms/frame)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);

	ImGui::Separator();
	ImGui::SliderFloat("Camera speed", &data.cam_sensitivity, 0.5f, 15.0f);
	ImGui::SliderFloat("Mouse sensitivity", &data.mouse_sensitivity, 0.0005f, 0.005f, "%.3f", ImGuiSliderFlags_Logarithmic);

	ImGui::End();
}


inline
static SVL::Texture* draw_load_cubemap(GUIShareData& data)
{
	if (fbs.count("cubemap") == 0)
	{
		ImGui::FileBrowser fb;
		fb.SetTitle("Load cubemap");
		fb.SetTypeFilters({ ".ktx" });
		fbs["cubemap"] = fb;
	}
	ImGui::FileBrowser& fb = fbs["cubemap"];

	SVL::Texture* tex = nullptr;

	if (ImGui::Button("Load##cubemap"))
		fb.Open();

	fb.Display();

	if (fb.HasSelected())
	{
		tex = data.loader.load_cubemap_ktx(VK_FORMAT_R8G8B8A8_UNORM, data.command_pool, fb.GetSelected().string());
		fb.ClearSelected();
	}

	return tex;
}

inline
static void draw_world(GUIShareData& data)
{
	ImGui::Begin("World");
	ImGui::Dummy(ImVec2(0.0f, 5.0f));

	ImGui::Text("Camera position - x: %f y: %f z: %f", data.cam.position().x, data.cam.position().y, data.cam.position().z);

	ImGui::Separator();
	ImGui::Dummy(ImVec2(0.0f, 10.0f));
	ImGui::Text("Skybox");
	
	SVL::Texture* cubemap;
	if (cubemap = draw_load_cubemap(data))
	{
		for (auto obj : data.cubemap_layer.get_objects())
		{
			data.cubemap_layer.del_object(obj);
			delete obj;
		}

		SVL::Model* skybox = new SVL::Model(data.renderer, data.command_pool, cubemesh, *cubemap);

		data.cubemap_layer.add_object(skybox);
		data.object_layer.set_environment_tex(cubemap);
		data.cubemap_layer.update();
		data.object_layer.update();

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(100.0f));
		skybox->set_ubo_model(model);
	}
	ImGui::SameLine();
	if (ImGui::Button("Clear##cubemap"))
	{
		for (auto obj : data.cubemap_layer.get_objects())
		{
			data.cubemap_layer.del_object(obj);
			delete obj;
		}

		data.object_layer.set_environment_tex(nullptr);
		data.cubemap_layer.update();
		data.object_layer.update();
	}

	ImGui::Separator();
	ImGui::Dummy(ImVec2(0.0f, 10.0f));
	ImGui::Text("Lights");

	static int light_selected = 0;
	{
		ImGui::BeginChild("left pane", ImVec2(150, 0), true);
		for (size_t i = 0; i < data.object_layer.point_lights.size(); i++)
		{
			std::string name = "Point Light " + std::to_string(i);
			if (ImGui::Selectable(name.c_str(), light_selected == i))
				light_selected = i;
		}
		ImGui::EndChild();
	}

	ImGui::SameLine();
	{
		SVL::PointLight& light = data.object_layer.point_lights[light_selected];

		ImGui::BeginGroup();
		ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

		bool changed = false;
		bool a = light.color.a == 1.0f;
		changed |= ImGui::Checkbox("enabled", &a);
		light.color.a = a;

		float pos[3] = { light.position.x, light.position.y, light.position.z };
		changed |= ImGui::SliderFloat3("position", pos, -20.0f, 20.0f);
		light.position.x = pos[0];
		light.position.y = pos[1];
		light.position.z = pos[2];

		changed |= ImGui::SliderFloat("attenuation", &light.params.x, 0.0f, 5.0f);

		float rgb[3] = { light.color.r, light.color.g, light.color.b };
		changed |= ImGui::ColorEdit3("color", rgb);
		light.color.r = rgb[0];
		light.color.g = rgb[1];
		light.color.b = rgb[2];

		if (changed)
			data.update();

		ImGui::EndChild();
		ImGui::EndGroup();
	}


	ImGui::End();
}


inline
static SVL::Texture* draw_load_texture(GUIShareData& data, std::string id)
{
	if (fbs.count(id) == 0)
	{
		ImGui::FileBrowser fb;
		fb.SetTitle(std::string("Load texture - " + id));
		fb.SetTypeFilters({ ".png" });
		fbs[id] = fb;
	}
	ImGui::FileBrowser& fb = fbs[id];

	SVL::Texture* tex = nullptr;

	if (ImGui::Button(std::string("Load##" + id).c_str()))
		fb.Open();

	fb.Display();

	if (fb.HasSelected())
	{
		tex = data.loader.load_img(VK_FORMAT_R8G8B8A8_UNORM, data.command_pool, fb.GetSelected().string());
		fb.ClearSelected();
	}

	return tex;
}

void edit_image_channel(SVL::Renderer& renderer, VkCommandPool command_pool, SVL::ImageView& image, uint8_t* dc, unsigned offset)
{
	vkDestroyImageView(renderer.device(), image.view, nullptr);

	VkMemoryRequirements mem_req;
	vkGetImageMemoryRequirements(renderer.device(), image.image, &mem_req);

	VkBuffer buffer;
	VkDeviceMemory buffer_memory;
	SVLTools::create_buffer(renderer.device(), renderer.physical_device(), mem_req.size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &buffer, &buffer_memory);

	image.transition_image_layout(command_pool, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);
	image.copy_image_to_buffer(command_pool, buffer, VK_IMAGE_ASPECT_COLOR_BIT);

	void* da;
	ErrorCheck(vkMapMemory(renderer.device(), buffer_memory, 0, mem_req.size, 0, &da));
	uint8_t* dd = reinterpret_cast<uint8_t*>(da);
	for (size_t i = 0; i < mem_req.size; i += 4)
	{
		dd[i + offset] = dc[i + offset];
	}
	vkUnmapMemory(renderer.device(), buffer_memory);

	image.transition_image_layout(command_pool, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);
	image.copy_buffer_to_image(command_pool, buffer, VK_IMAGE_ASPECT_COLOR_BIT);
	image.transition_image_layout(command_pool, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);

	//destroy buffer
	vkFreeMemory(renderer.device(), buffer_memory, nullptr);
	vkDestroyBuffer(renderer.device(), buffer, nullptr);
	//view
	image.create_2D_image_view(VK_IMAGE_ASPECT_COLOR_BIT);
}

inline
static void draw_objects_materials(GUIShareData& data, SVL::Model* obj, int& mat_selected)
{
	{
		ImGui::BeginChild("left pane", ImVec2(150, 0), true);
		for (size_t i = 0; i < obj->get_materials().size(); i++)
		{
			std::string name = "Material " + std::to_string(i);
			if (ImGui::Selectable(name.c_str(), mat_selected == i))
				mat_selected = i;
		}
		ImGui::EndChild();
	}

	ImGui::SameLine();
	if (obj->get_materials().size() > 0)
	{
		SVL::Material& mat = obj->get_materials()[mat_selected];

		ImGui::BeginGroup();
		ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

		std::string name = "Material " + mat.diffuse_filename;
		ImGui::Text(name.c_str());
		ImGui::Separator();

		SVL::Texture* tex;

		
		if (tex = draw_load_texture(data, "diffuse"))
		{
			mat.textures.diffuse = tex;
			data.object_layer.update();
		}
		ImGui::SameLine();
		name = "Diffuse texture";
		ImGui::Text(name.c_str());
		
		
		if (tex = draw_load_texture(data, "normal"))
		{
			mat.properties.has_normal_tex = true;
			mat.textures.normal = tex;
			mat.update();
			data.object_layer.update();
		}
		ImGui::SameLine();
		if (ImGui::Button("Clear##norm"))
		{
			mat.properties.has_normal_tex = false;
			mat.update();
			data.object_layer.update();
		}
		ImGui::SameLine();
		name = "Normal texture";
		ImGui::Text(name.c_str());
		
		if (tex = draw_load_texture(data, "metal"))
		{
			VkMemoryRequirements mem_req;
			vkGetImageMemoryRequirements(data.renderer.device(), tex->image.image, &mem_req);

			VkBuffer buffer;
			VkDeviceMemory buffer_memory;
			SVLTools::create_buffer(data.renderer.device(), data.renderer.physical_device(), mem_req.size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &buffer, &buffer_memory);

			tex->image.transition_image_layout(data.command_pool, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);
			tex->image.copy_image_to_buffer(data.command_pool, buffer, VK_IMAGE_ASPECT_COLOR_BIT);
			tex->image.transition_image_layout(data.command_pool, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);

			void* da;
			ErrorCheck(vkMapMemory(data.renderer.device(), buffer_memory, 0, mem_req.size, 0, &da));
			uint8_t* dd = reinterpret_cast<uint8_t*>(da);

			edit_image_channel(data.renderer, data.command_pool, mat.textures.metalness_roughness->image, dd, 2);
			mat.textures.metalness_roughness->descriptor.imageView = mat.textures.metalness_roughness->image.view;

			vkUnmapMemory(data.renderer.device(), buffer_memory);
			vkFreeMemory(data.renderer.device(), buffer_memory, nullptr);
			vkDestroyBuffer(data.renderer.device(), buffer, nullptr);

			mat.update();
			data.object_layer.update();
		}
		ImGui::SameLine();
		if (ImGui::Button("Clear##metal"))
		{
			VkMemoryRequirements mem_req;
			vkGetImageMemoryRequirements(data.renderer.device(), mat.textures.metalness_roughness->image.image, &mem_req);

			uint8_t* dd = new uint8_t[mem_req.size]{};
			edit_image_channel(data.renderer, data.command_pool, mat.textures.metalness_roughness->image, dd, 2);
			mat.textures.metalness_roughness->descriptor.imageView = mat.textures.metalness_roughness->image.view;

			delete[] dd;

			mat.update();
			data.object_layer.update();
		}
		ImGui::SameLine();
		name = "Metallic texture";
		ImGui::Text(name.c_str());

		if (tex = draw_load_texture(data, "rough"))
		{
			VkMemoryRequirements mem_req;
			vkGetImageMemoryRequirements(data.renderer.device(), tex->image.image, &mem_req);

			VkBuffer buffer;
			VkDeviceMemory buffer_memory;
			SVLTools::create_buffer(data.renderer.device(), data.renderer.physical_device(), mem_req.size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &buffer, &buffer_memory);

			tex->image.transition_image_layout(data.command_pool, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);
			tex->image.copy_image_to_buffer(data.command_pool, buffer, VK_IMAGE_ASPECT_COLOR_BIT);
			tex->image.transition_image_layout(data.command_pool, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);

			void* da;
			ErrorCheck(vkMapMemory(data.renderer.device(), buffer_memory, 0, mem_req.size, 0, &da));
			uint8_t* dd = reinterpret_cast<uint8_t*>(da);

			edit_image_channel(data.renderer, data.command_pool, mat.textures.metalness_roughness->image, dd, 1);
			mat.textures.metalness_roughness->descriptor.imageView = mat.textures.metalness_roughness->image.view;

			vkUnmapMemory(data.renderer.device(), buffer_memory);
			vkFreeMemory(data.renderer.device(), buffer_memory, nullptr);
			vkDestroyBuffer(data.renderer.device(), buffer, nullptr);

			mat.update();
			data.object_layer.update();
		}
		ImGui::SameLine();
		if (ImGui::Button("Clear##rough"))
		{
			VkMemoryRequirements mem_req;
			vkGetImageMemoryRequirements(data.renderer.device(), mat.textures.metalness_roughness->image.image, &mem_req);

			uint8_t* dd = new uint8_t[mem_req.size]{};
			edit_image_channel(data.renderer, data.command_pool, mat.textures.metalness_roughness->image, dd, 1);
			mat.textures.metalness_roughness->descriptor.imageView = mat.textures.metalness_roughness->image.view;

			delete[] dd;

			mat.update();
			data.object_layer.update();
		}
		ImGui::SameLine();
		name = "Roughness texture";
		ImGui::Text(name.c_str());

		if (tex = draw_load_texture(data, "ao"))
		{
			VkMemoryRequirements mem_req;
			vkGetImageMemoryRequirements(data.renderer.device(), tex->image.image, &mem_req);

			VkBuffer buffer;
			VkDeviceMemory buffer_memory;
			SVLTools::create_buffer(data.renderer.device(), data.renderer.physical_device(), mem_req.size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &buffer, &buffer_memory);

			tex->image.transition_image_layout(data.command_pool, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);
			tex->image.copy_image_to_buffer(data.command_pool, buffer, VK_IMAGE_ASPECT_COLOR_BIT);
			tex->image.transition_image_layout(data.command_pool, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);

			void* da;
			ErrorCheck(vkMapMemory(data.renderer.device(), buffer_memory, 0, mem_req.size, 0, &da));
			uint8_t* dd = reinterpret_cast<uint8_t*>(da);

			edit_image_channel(data.renderer, data.command_pool, mat.textures.metalness_roughness->image, dd, 0);
			mat.textures.metalness_roughness->descriptor.imageView = mat.textures.metalness_roughness->image.view;
			mat.properties.has_ao_tex = 1;

			vkUnmapMemory(data.renderer.device(), buffer_memory);
			vkFreeMemory(data.renderer.device(), buffer_memory, nullptr);
			vkDestroyBuffer(data.renderer.device(), buffer, nullptr);

			mat.update();
			data.object_layer.update();
		}
		ImGui::SameLine();
		if (ImGui::Button("Clear##ao"))
		{
			VkMemoryRequirements mem_req;
			vkGetImageMemoryRequirements(data.renderer.device(), mat.textures.metalness_roughness->image.image, &mem_req);

			uint8_t* dd = new uint8_t[mem_req.size]{255};
			edit_image_channel(data.renderer, data.command_pool, mat.textures.metalness_roughness->image, dd, 0);
			mat.textures.metalness_roughness->descriptor.imageView = mat.textures.metalness_roughness->image.view;
			mat.properties.has_ao_tex = 0;

			delete[] dd;

			mat.update();
			data.object_layer.update();
		}
		ImGui::SameLine();
		name = "Ambient occlusion texture";
		ImGui::Text(name.c_str());

		ImGui::EndChild();
		ImGui::EndGroup();
	}
}

inline
static void draw_add_object(GUIShareData& data)
{
	static ImGui::FileBrowser fb;
	fb.SetTitle("Add Object");
	fb.SetTypeFilters({ ".glb" });

	if (ImGui::Button("Add Object"))
		fb.Open();

	fb.Display();

	if (fb.HasSelected())
	{
		data.object_layer.add_object(new SVL::Model(data.loader.load_gltf(data.command_pool, fb.GetSelected().string())));
		fb.ClearSelected();
	}
}

inline
static void draw_objects(GUIShareData& data)
{
	const std::vector<SVL::Model*>& objects = data.object_layer.get_objects();

	ImGui::Begin("Objects");

	draw_add_object(data);
	ImGui::SameLine();
	if (ImGui::Button("Reload shaders"))
	{
		data.object_layer.update();
		data.cubemap_layer.update();
	}
		

	static int selected = 0;
	static int mat_selected = 0;
	{
		ImGui::BeginChild("left pane", ImVec2(150, 0), true);
		for (size_t i = 0; i < objects.size(); i++)
		{
			std::string name = "Object " + std::to_string(i);
			if (ImGui::Selectable(name.c_str(), selected == i))
			{
				selected = i;
				mat_selected = 0;
			}

		}
		ImGui::EndChild();
	}
	ImGui::SameLine();

	if (objects.size() > 0)
	{
		std::string name = "Object " + std::to_string(selected);
		SVL::Model* obj = objects[selected];
		ImGui::BeginGroup();
		ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

		ImGui::Text(name.c_str());
		ImGui::SameLine(ImGui::GetWindowWidth() - 30);
		if (ImGui::Button("X"))
		{
			data.object_layer.del_object(obj);
			delete obj;
			selected = 0;
			mat_selected = 0;
		}

		ImGui::Separator();

		bool changed{};

		glm::mat4 model = obj->ubo_model();
		glm::vec3 mpos(model[3]);
		model[3][0] = 0;
		model[3][1] = 0;
		model[3][2] = 0;
		
		float pos[3] = { mpos.x, mpos.y, mpos.z };
		changed |= ImGui::SliderFloat3("position", pos, -20.0f, 20.0f);

		float scale = glm::length(model[0]);

		model[0] /= scale;
		model[1] /= glm::length(model[1]);
		model[2] /= glm::length(model[2]);

		glm::vec3 mrot;
		glm::extractEulerAngleXYZ(model, mrot.x, mrot.y, mrot.z);
		float rot[3] = { glm::degrees(mrot.x), glm::degrees(mrot.y), glm::degrees(mrot.z) };
		changed |= ImGui::SliderFloat3("rotation", rot, -89.9f, 89.9f);
		
		changed |= ImGui::SliderFloat("scale", &scale, 0.0f, 100.0f);

		if (changed)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(pos[0], pos[1], pos[2]));
			model = glm::scale(model, glm::vec3(scale));

			glm::vec3 rot_vec = glm::vec3(rot[0], rot[1], rot[2]);
			if (!glm::all(glm::isnan(rot_vec)))
			{
				model = glm::rotate(model, glm::radians(rot[0]), glm::vec3(1.0f, 0.0f, 0.0f));
				model = glm::rotate(model, glm::radians(rot[1]), glm::vec3(0.0f, 1.0f, 0.0f));
				model = glm::rotate(model, glm::radians(rot[2]), glm::vec3(0.0f, 0.0f, 1.0f));
			}
				
			obj->set_ubo_model(model);
		}

		draw_objects_materials(data, obj, mat_selected);

		ImGui::EndChild();
		ImGui::EndGroup();
	}

	ImGui::End();
}

void MyGUI::new_frame()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void MyGUI::draw(GUIShareData& data)
{
	draw_app(data);
	draw_world(data);
	draw_objects(data);

	ImGui::Render();
}

bool MyGUI::want_capture()
{
	ImGuiIO& io = ImGui::GetIO();
	return io.WantCaptureMouse || io.WantCaptureKeyboard;
}
