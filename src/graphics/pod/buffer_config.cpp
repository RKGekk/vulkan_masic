#include "buffer_config.h"

#include "../api/vulkan_device.h"
#include "../api/vulkan_format_manager.h"
#include "format_config.h"

#include "../../tools/string_tools.h"

bool BufferConfig::init(const std::shared_ptr<VulkanDevice>& device, const std::string& rg_file_path, const std::shared_ptr<VulkanFormatManager>& format_manager) {
    pugi::xml_document xml_doc;
	pugi::xml_parse_result parse_res = xml_doc.load_file(rg_file_path.c_str());
	if (!parse_res) { return false;	}

	pugi::xml_node root_node = xml_doc.root();
	if (!root_node) { return false; }
	root_node = root_node.child("RenderGraph");

    pugi::xml_node type_of_resources_node = root_node.child("TypeOfResources");
	if (!type_of_resources_node) return false;
    
	for (pugi::xml_node resource_type_node = type_of_resources_node.first_child(); resource_type_node; resource_type_node = resource_type_node.next_sibling()) {
        pugi::xml_node buffer_node = resource_type_node.child("Buffer");
	    if (!buffer_node) continue;
	    return init(device, resource_type_node.attribute("name").as_string(), buffer_node, format_manager);
	}

    return true;
}

bool BufferConfig::init(const std::shared_ptr<VulkanDevice>& device, const std::string& name, const pugi::xml_node& buffer_data, const std::shared_ptr<VulkanFormatManager>& format_manager) {
    using namespace std::literals;

    m_buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

    static std::vector<uint32_t> families = device->getCommandManager()->getQueueFamilyIndices().getIndices();
    m_buffer_info.sharingMode = device->getCommandManager()->getBufferSharingMode();
    m_buffer_info.queueFamilyIndexCount = static_cast<uint32_t>(families.size());
    m_buffer_info.pQueueFamilyIndices = families.data();

    m_name = name;

    pugi::xml_node buffer_flags_node = buffer_data.child("Falgs");
    if(buffer_flags_node) {
        for (pugi::xml_node flag_node = buffer_flags_node.first_child(); flag_node; flag_node = flag_node.next_sibling()) {
		    m_buffer_info.flags |= getBufferCreateFlag(flag_node.text().as_string());
		}
    }

    m_buffer_info.size = buffer_data.child("Size").text().as_uint();
    m_dynamic_size = buffer_data.child("Size").attribute("dynamic").as_bool();
    m_deffered_size = buffer_data.child("Size").attribute("deffered").as_bool();

    for (pugi::xml_node flag_node = buffer_data.child("BufferUsageFlags").first_child(); flag_node; flag_node = flag_node.next_sibling()) {
        m_buffer_info.usage |= getBufferUsageFlag(flag_node.text().as_string());
    }

    for (pugi::xml_node flag_node = buffer_data.child("MemoryProperties").first_child(); flag_node; flag_node = flag_node.next_sibling()) {
        m_memory_properties |= getMemoryPropertyFlag(flag_node.text().as_string());
    }

    pugi::xml_node views_node = buffer_data.child("Views");
    for (pugi::xml_node view_node = views_node.first_child(); view_node; view_node = view_node.next_sibling()) {
        std::shared_ptr<BufferViewConfig> buffer_view_config_ptr = std::make_shared<BufferViewConfig>();

        std::string view_name_str = view_node.attribute("name").as_string();

        buffer_view_config_ptr->view_info = VkBufferViewCreateInfo{};
        buffer_view_config_ptr->view_info.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;

        buffer_view_config_ptr->view_info.flags = 0u;

        buffer_view_config_ptr->view_info.buffer = VK_NULL_HANDLE;

        
        buffer_view_config_ptr->format = format_manager->getFormat(view_node.child("FormatName").text().as_string());
        buffer_view_config_ptr->view_info.format = buffer_view_config_ptr->format->getVkFormat();
        
        buffer_view_config_ptr->view_info.offset = view_node.child("Offset").text().as_uint();
        buffer_view_config_ptr->view_info.range = view_node.child("Range").text().as_uint();

        m_view_info_map[view_name_str] = std::move(buffer_view_config_ptr);
    }

    return true;
}
    
const std::string& BufferConfig::getName() const {
    return m_name;
}

bool BufferConfig::isSizeDynamic() const {
    return m_dynamic_size;
}
    
bool BufferConfig::isSizeDeffered() const {
    return m_deffered_size;
}

void BufferConfig::setSize(VkDeviceSize sz) {
    m_buffer_info.size = sz;
}

const VkBufferCreateInfo& BufferConfig::getBufferInfo() const {
    return m_buffer_info;
}

VkMemoryPropertyFlags BufferConfig::getMemoryProperties() const {
    return m_memory_properties;
}

const std::unordered_map<std::string, std::shared_ptr<BufferViewConfig>>& BufferConfig::getViewMap() const {
    return m_view_info_map;
}

const std::shared_ptr<BufferViewConfig>& BufferConfig::getView() const {
    return (*m_view_info_map.begin()).second;
}

const std::shared_ptr<BufferViewConfig>& BufferConfig::getView(const std::string& view_name) const {
    return m_view_info_map.at(view_name);
}

std::shared_ptr<BufferConfig> BufferConfig::makeInstance(std::string name, VkDeviceSize buffer_size) const {
    using namespace std::literals;

    std::shared_ptr<BufferConfig> instance_ptr = std::make_shared<BufferConfig>();
    instance_ptr->m_name = name;
    instance_ptr->m_buffer_info = m_buffer_info;
    instance_ptr->m_buffer_info.size = buffer_size;
    instance_ptr->m_dynamic_size = m_dynamic_size;
    instance_ptr->m_deffered_size = false;
    instance_ptr->m_memory_properties = m_memory_properties;

    for(const auto&[view_type_name, view_cfg_ptr] : m_view_info_map) {
        instance_ptr->m_view_info_map[view_type_name] = std::make_shared<BufferViewConfig>();
        instance_ptr->m_view_info_map[view_type_name]->view_info = view_cfg_ptr->view_info;
        instance_ptr->m_view_info_map[view_type_name]->format = view_cfg_ptr->format->makeInstance(view_type_name + "_view_format");
    }

    return instance_ptr;
}