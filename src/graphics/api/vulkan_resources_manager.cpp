#include "vulkan_resources_manager.h"

#include "vulkan_device.h"
#include "vulkan_image_buffer.h"
#include "vulkan_buffer.h"
#include "vulkan_format_manager.h"
#include "vulkan_sampler.h"
#include "../pod/image_buffer_config.h"
#include "../pod/buffer_config.h"
#include "vulkan_buffer.h"
#include "vulkan_image_buffer.h"

#include "../../tools/string_tools.h"

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

void VulkanResourcesManager::destroy() {
	for (auto&[image_cfg_name, image_cfg] : m_image_buffer_config_map) {
		image_cfg->destroy();
	}
	// for (auto&[image_name, image] : m_image_map) {
	// 	image->destroy();
	// }
	for (auto&[buffer_cfg_name, buffer_cfg] : m_buffer_config_map) {
		buffer_cfg->destroy();
	}
	// for (auto&[buffer_name, buffer] : m_buffer_map) {
	// 	buffer->destroy();
	// }
}

std::shared_ptr<VulkanImageBuffer> VulkanResourcesManager::create_image(const std::string& path_to_file) {
	if(m_image_map.contains(path_to_file)) return m_image_map[path_to_file];

	std::shared_ptr<ImageBufferConfig> image_config_template = m_image_buffer_config_map.at("basic_image_resource"s);
	std::shared_ptr<ImageBufferConfig> image_buffer_config = image_config_template->makeInstance("basic_image_resource"s + "_"s + get_uuid());

	std::shared_ptr<VulkanImageBuffer> image = std::make_shared<VulkanImageBuffer>(m_device, path_to_file);
	image->init(image_buffer_config, path_to_file);

	m_image_map[path_to_file] = image;

	return image;
}

std::shared_ptr<VulkanImageBuffer> VulkanResourcesManager::create_image(unsigned char* pixels, VkExtent2D extent, std::string image_name, std::string resource_type_name) {
	if(m_image_map.contains(image_name)) return m_image_map[image_name];

	std::shared_ptr<ImageBufferConfig> image_config_template = m_image_buffer_config_map.at(resource_type_name);
	std::shared_ptr<ImageBufferConfig> image_buffer_config = image_config_template->makeInstance(resource_type_name + "_"s + get_uuid(), extent);

	std::shared_ptr<VulkanImageBuffer> image = std::make_shared<VulkanImageBuffer>(m_device, image_name);
	image->init(pixels, std::move(image_buffer_config));

	m_image_map[image_name] = image;

	return image;
}

std::shared_ptr<VulkanImageBuffer> VulkanResourcesManager::create_image(VkImage vk_image, std::string image_name, std::string resource_type_name) {
	if(m_image_map.contains(image_name)) return m_image_map[image_name];

	std::shared_ptr<ImageBufferConfig> image_config_template = m_image_buffer_config_map.at(resource_type_name);
	std::shared_ptr<ImageBufferConfig> image_buffer_config = image_config_template->makeInstance(resource_type_name + "_"s + get_uuid());

	std::shared_ptr<VulkanImageBuffer> image = std::make_shared<VulkanImageBuffer>(m_device, image_name);
	image->init(vk_image, std::move(image_buffer_config));

	m_image_map[image_name] = image;

	return image;
}

std::shared_ptr<VulkanImageBuffer> VulkanResourcesManager::create_image(VkImage vk_image, VkExtent2D extent, std::string image_name, std::string resource_type_name) {
	using namespace std::literals;

	if(m_image_map.contains(image_name)) return m_image_map[image_name];

	std::shared_ptr<ImageBufferConfig> image_config_template = m_image_buffer_config_map.at(resource_type_name);
	std::shared_ptr<ImageBufferConfig> image_buffer_config = image_config_template->makeInstance(resource_type_name + "_"s + get_uuid());

	std::shared_ptr<VulkanImageBuffer> image = std::make_shared<VulkanImageBuffer>(m_device, image_name);
	image->init(vk_image, std::move(image_buffer_config));

	m_image_map[image_name] = image;

	return image;
}

std::shared_ptr<VulkanImageBuffer> VulkanResourcesManager::create_image(std::string image_name, std::string resource_type_name) {
	if(m_image_map.contains(image_name)) return m_image_map[image_name];

	std::shared_ptr<ImageBufferConfig> image_config_template = m_image_buffer_config_map.at(resource_type_name);
	std::shared_ptr<ImageBufferConfig> image_buffer_config = image_config_template->makeInstance(resource_type_name + "_"s + get_uuid());

	std::shared_ptr<VulkanImageBuffer> image = std::make_shared<VulkanImageBuffer>(m_device, image_name);
	image->init(std::move(image_buffer_config));

	m_image_map[image_name] = image;

	return image;
}

void VulkanResourcesManager::delete_image(const std::string& image_name) {
	m_image_map[image_name]->destroy();
	m_image_map.erase(image_name);
}

void VulkanResourcesManager::delete_image(std::shared_ptr<VulkanImageBuffer> image_ptr) {
	for (auto&[image_name, image] : m_image_map) {
		if(image == image_ptr) {
			image->destroy();
			m_image_map.erase(image_name);
			return;
		}
	}
}

std::shared_ptr<VulkanBuffer> VulkanResourcesManager::create_buffer(const void* data, VkDeviceSize buffer_size, std::string resource_type_name) {
	using namespace std::literals;

	const std::shared_ptr<BufferConfig>& buffer_config_template = m_buffer_config_map.at(resource_type_name);
	std::shared_ptr<BufferConfig> buffer_config;
	if(buffer_config_template->isSizeDeffered()) {
		buffer_config = buffer_config_template->makeInstance(resource_type_name + "_"s + get_uuid(), buffer_size);
	}
	else {
		buffer_config = buffer_config_template->makeInstance(resource_type_name + "_"s + get_uuid(), buffer_config_template->getBufferInfo().size);
	}

	std::shared_ptr<VulkanBuffer> buffer = std::make_shared<VulkanBuffer>(m_device);
	buffer->init(data, std::move(buffer_config));

	return buffer;
}

std::shared_ptr<VulkanBuffer> VulkanResourcesManager::create_buffer(const void* data, VkDeviceSize buffer_size, std::string buffer_name, std::string resource_type_name) {
	using namespace std::literals;

	const std::shared_ptr<BufferConfig>& buffer_config_template = m_buffer_config_map.at(resource_type_name);
	//std::shared_ptr<BufferConfig> buffer_config = buffer_config_template->makeInstance(buffer_name + "_"s + resource_type_name, buffer_size);

	std::shared_ptr<BufferConfig> buffer_config;
	if(buffer_config_template->isSizeDeffered()) {
		buffer_config = buffer_config_template->makeInstance(buffer_name + "_"s + resource_type_name, buffer_size);
	}
	else {
		buffer_config = buffer_config_template->makeInstance(buffer_name + "_"s + resource_type_name, buffer_config_template->getBufferInfo().size);
	}

	std::shared_ptr<VulkanBuffer> buffer = std::make_shared<VulkanBuffer>(m_device, buffer_name);
	buffer->init(data, std::move(buffer_config));

	m_buffer_map[buffer_name] = buffer;

	return buffer;
}

void VulkanResourcesManager::delete_buffer(const std::string& buffer_name) {
	m_buffer_map[buffer_name]->destroy();
	m_buffer_map.erase(buffer_name);
}

void VulkanResourcesManager::delete_buffer(std::shared_ptr<VulkanBuffer> buffer_ptr) {
	for (auto&[buffer_name, buffer] : m_buffer_map) {
		if(buffer == buffer_ptr) {
			buffer->destroy();
			m_buffer_map.erase(buffer_name);
			return;
		}
	}
}

const std::shared_ptr<VulkanImageBuffer>& VulkanResourcesManager::getImageResource(const std::string& resource_global_name) {
	return m_image_map.at(resource_global_name);
}

const std::shared_ptr<VulkanBuffer>& VulkanResourcesManager::getBufferResource(const std::string& resource_global_name) {
	return m_buffer_map.at(resource_global_name);
}

std::shared_ptr<RenderResource> VulkanResourcesManager::getResource(const std::string& resource_global_name) {
	if(m_image_map.contains(resource_global_name)) {
		return m_image_map[resource_global_name];
	}
	else if(m_buffer_map.contains(resource_global_name)) {
		return m_buffer_map[resource_global_name];
	}
	return nullptr;
}

const std::shared_ptr<ImageBufferConfig> VulkanResourcesManager::getImageBufferConfigTemplate(const std::string& template_name) const {
	return m_image_buffer_config_map.at(template_name);
}

const std::shared_ptr<BufferConfig> VulkanResourcesManager::getBufferConfigTemplate(const std::string& template_name) const {
	return m_buffer_config_map.at(template_name);
}