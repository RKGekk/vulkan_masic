#include "vulkan_resources_manager.h"

#include "vulkan_device.h"
#include "vulkan_image_buffer.h"
#include "vulkan_buffer.h"
#include "../pod/image_buffer_config.h"
#include "../pod/buffer_config.h"

#include "../../tools/string_tools.h"

#include <stb_image.h>

#include <utility>

VulkanResourcesManager::VulkanResourcesManager(std::shared_ptr<VulkanDevice> device, std::shared_ptr<VulkanFormatManager> format_manager) : m_device(std::move(device)), m_format_manager(std::move(format_manager)) {}

bool VulkanResourcesManager::init(const std::string& rg_file_path) {
    pugi::xml_document xml_doc;
	pugi::xml_parse_result parse_res = xml_doc.load_file(rg_file_path.c_str());
	if (!parse_res) { return false;	}

	pugi::xml_node root_node = xml_doc.root();
	if (!root_node) { return false; }
	root_node = root_node.child("RenderGraph");

    pugi::xml_node type_of_resources_node = root_node.child("TypeOfResources");
	if (!type_of_resources_node) return false;
    
	for (pugi::xml_node resource_type_node = type_of_resources_node.first_child(); resource_type_node; resource_type_node = resource_type_node.next_sibling()) {
		std::string name = resource_type_node.attribute("name").as_string();

	    if (pugi::xml_node image_buffer_node = resource_type_node.child("ImageBuffer")) {
			std::shared_ptr<ImageBufferConfig> image_buffer_config_ptr = std::make_shared<ImageBufferConfig>();
			image_buffer_config_ptr->init(m_device, name, image_buffer_node, m_format_manager);
			m_image_buffer_config_map[name] = std::move(image_buffer_config_ptr);
		}
		else if(pugi::xml_node buffer_node = resource_type_node.child("Buffer")) {
			std::shared_ptr<BufferConfig> buffer_config_ptr = std::make_shared<BufferConfig>();
			buffer_config_ptr->init(m_device, name, buffer_node, m_format_manager);
			m_buffer_config_map[name] = std::move(buffer_config_ptr);
		}
	}

    return true;
}

std::shared_ptr<VulkanImageBuffer> VulkanResourcesManager::create_image(const std::string& path_to_file, std::shared_ptr<VulkanSampler> sampler) {
	if(m_image_map.contains(path_to_file)) return m_image_map[path_to_file];

	std::shared_ptr<ImageBufferConfig> basic_image_config = m_image_buffer_config_map.at("basic_image_resource");

	int tex_width;
    int tex_height;
    int tex_channels;
    stbi_uc* pixels = stbi_load(path_to_file.c_str(), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

	std::shared_ptr<VulkanImageBuffer> image = std::make_shared<VulkanImageBuffer>(m_device, path_to_file);
	image->init(pixels, VkExtent2D{static_cast<uint32_t>(tex_width), static_cast<uint32_t>(tex_height)}, std::move(basic_image_config));

	stbi_image_free(pixels);

	return image;
}