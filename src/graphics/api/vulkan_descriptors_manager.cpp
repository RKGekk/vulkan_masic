#include "vulkan_descriptors_manager.h"

#include "vulkan_device.h"

bool VulkanDescriptorsManager::init(std::shared_ptr<VulkanDevice> device, const std::string& rg_file_name) {
    m_device = device;

    pugi::xml_document xml_doc;
	pugi::xml_parse_result parse_res = xml_doc.load_file(rg_file_name.c_str());
	if (!parse_res) { return false;	}

	pugi::xml_node root_node = xml_doc.root();
	if (!root_node) { return false; }
	root_node = root_node.child("RenderGraph");

    pugi::xml_node dscriptors_node = root_node.child("Descriptors");
	if (dscriptors_node) {
		for (pugi::xml_node dscriptor_node = dscriptors_node.first_child(); dscriptor_node; dscriptor_node = dscriptor_node.next_sibling()) {
            std::shared_ptr<DescSetLayout> layout;
			layout->init(m_device, dscriptor_node);
			m_name_layout_map.insert({layout->getName(), layout});
		}
	}

	std::unordered_map<VkDescriptorType, size_t> types_map = getTypesCount();
    std::vector<VkDescriptorPoolSize> pool_sizes;
    for (const auto&[desc_type, ct] : types_map) {
        VkDescriptorPoolSize pool_size{};
        pool_size.type = desc_type;
        pool_size.descriptorCount = ct;
        pool_sizes.push_back(pool_size);
    }
    
    m_pool_info = VkDescriptorPoolCreateInfo{};
    m_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    m_pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
    m_pool_info.pPoolSizes = pool_sizes.data();
    m_pool_info.maxSets = static_cast<uint32_t>(m_name_layout_map.size());
    m_pool_info.flags = 0u; // VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT
 
    VkResult result = vkCreateDescriptorPool(m_device->getDevice(), &m_pool_info, nullptr, &m_descriptor_pool);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }

	m_desc_sets.resize(m_name_layout_map.size());

    std::vector<VkDescriptorSetLayout> layouts = getVkDescriptorSetLayouts();
    VkDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = m_descriptor_pool;
    alloc_info.descriptorSetCount = static_cast<uint32_t>(m_name_layout_map.size());
    alloc_info.pSetLayouts = layouts.data();
    
    VkResult result = vkAllocateDescriptorSets(m_device->getDevice(), &alloc_info, m_desc_sets.data());
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

	size_t ct = 0u;
	for(const auto& [desc_name, desc_layout] : m_name_layout_map) {
		m_name_desc_idx_map.insert({desc_name, ct++});
    }

    return true;
}

void VulkanDescriptorsManager::destroy() {

}

std::shared_ptr<DescSetLayout> VulkanDescriptorsManager::getDescSetLayout(const std::string& name) const {
	if(!m_name_layout_map.contains(name)) return nullptr;
	return m_name_layout_map.at(name);
}

const std::unordered_map<std::string, std::shared_ptr<DescSetLayout>>& VulkanDescriptorsManager::getNameLayoutMap() const {
	return m_name_layout_map;
}

VkDescriptorSet VulkanDescriptorsManager::getDescriptorSet(const std::string& desc_set_name) const {
	return m_desc_sets.at(m_name_desc_idx_map.at(desc_set_name));
}

std::unordered_map<VkDescriptorType, size_t> VulkanDescriptorsManager::getTypesCount() {
    std::unordered_map<VkDescriptorType, size_t> result;
    
    for(const auto&[desc_name, desc_layout] : m_name_layout_map) {
        for (VkDescriptorSetLayoutBinding desc_binding : desc_layout->getBindings()) {
            ++result[desc_binding.descriptorType];
        }
    }

    return result;
}

std::vector<VkDescriptorSetLayout> VulkanDescriptorsManager::getVkDescriptorSetLayouts() const {
	std::vector<VkDescriptorSetLayout> res;
	res.reserve(m_name_layout_map.size());
    
    for(const auto& [desc_name, desc_layout] : m_name_layout_map) {
    	res.push_back(desc_layout->getDescriptorSetLayout());
    }

    return res;
}