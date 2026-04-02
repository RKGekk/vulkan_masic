#include "descriptor_set_layout.h"

#include "../api/vulkan_device.h"
#include "../api/vulkan_sampler.h"

bool DescSetLayout::init(std::shared_ptr<VulkanDevice> device, const pugi::xml_node& descriptor_sets_node) {
    using namespace std::literals;
    m_device = device;

    m_name = descriptor_sets_node.attribute("name").as_string();
    m_allocator_name = descriptor_sets_node.attribute("allocator").as_string();
    
    pugi::xml_node layout_node = descriptor_sets_node.child("Layout");
	for (pugi::xml_node layout_binding_node = layout_node.first_child(); layout_binding_node; layout_binding_node = layout_binding_node.next_sibling()) {
        std::string layout_binding_name = layout_binding_node.attribute("name").as_string();

        VkDescriptorSetLayoutBinding layout_binding{};
        layout_binding.binding = layout_binding_node.child("Binding").text().as_int();
        layout_binding.descriptorType = getDescriptorType(layout_binding_node.child("DescriptorType").text().as_string());
        layout_binding.descriptorCount = layout_binding_node.child("DescriptorCount").text().as_int();
        
        pugi::xml_node stage_flags_node = layout_binding_node.child("ShaderStageFlags");
        for (pugi::xml_node create_flag = stage_flags_node.first_child(); create_flag; create_flag = create_flag.next_sibling()) {
	        layout_binding.stageFlags |= getShaderStageFlag(create_flag.text().as_string());
	    }
        
        pugi::xml_node immutable_samplers_node = layout_binding_node.child("ImmutableSamplers");
        for (pugi::xml_node sampler_node = immutable_samplers_node.first_child(); sampler_node; sampler_node = sampler_node.next_sibling()) {
            VkSamplerCreateInfo sampler_info = getSamplerCreateInfo(sampler_node);
            std::shared_ptr<VulkanSampler> sampler = std::make_shared<VulkanSampler>(device, m_name + "_immutable_sampler"s);
            sampler->init(sampler_info);
            m_immutable_samplers_ptr.push_back(sampler->getSampler());
            m_immutable_samplers.push_back(std::move(sampler));
        }
        
        if(m_immutable_samplers_ptr.empty()) {
            layout_binding.pImmutableSamplers = nullptr;
        }
        else {
            layout_binding.pImmutableSamplers = m_immutable_samplers_ptr.data();
        }
        
        m_binding_num_to_idx_map[layout_binding.binding] = m_bindings.size();
        m_bindings.push_back(layout_binding);
        m_binding_name_map[layout_binding_name] = layout_binding.binding;
        m_binding_num_to_name_map[layout_binding.binding] = layout_binding_name;
    }

    m_desc_layout_info = VkDescriptorSetLayoutCreateInfo{};
    m_desc_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    
    pugi::xml_node stage_flags_node = descriptor_sets_node.child("LayoutCreateFlags");
    for (pugi::xml_node create_flag = stage_flags_node.first_child(); create_flag; create_flag = create_flag.next_sibling()) {
	    m_desc_layout_info.flags |= getDescriptorSetLayoutCreateFlag(create_flag.text().as_string());
	}
    
    m_desc_layout_info.bindingCount = static_cast<uint32_t>(m_bindings.size());
    m_desc_layout_info.pBindings = m_bindings.data();
    
    VkResult result = vkCreateDescriptorSetLayout(m_device->getDevice(), &m_desc_layout_info, nullptr, &m_desc_layout);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

    return true;
}

void DescSetLayout::destroy() {
    for(std::shared_ptr<VulkanSampler>& sampler_ptr : m_immutable_samplers) {
        sampler_ptr->destroy();
    }
    vkDestroyDescriptorSetLayout(m_device->getDevice(), m_desc_layout, nullptr);
}

const std::string& DescSetLayout::getName() const {
    return m_name;
}

const std::string& DescSetLayout::getAllocatorName() const {
    return m_allocator_name;
}

const DescSetLayout::DescSetBindings& DescSetLayout::getBindings() const {
    return m_bindings;
}

VkDescriptorSetLayoutBinding DescSetLayout::getBinding(VkDescriptorType desc_type) const {
    for(const VkDescriptorSetLayoutBinding& binding : m_bindings) {
        if(binding.descriptorType == desc_type) return binding;
    }
    return {};
}

VkDescriptorSetLayoutBinding DescSetLayout::getBinding(DescSetLayout::BindingNum binding_num) const {
    if(m_binding_num_to_idx_map.contains(binding_num)) {
        return m_bindings.at(m_binding_num_to_idx_map.at(binding_num));
    }
    return {};
}

VkDescriptorSetLayoutBinding DescSetLayout::getBinding(const std::string& binding_name) const {
    if(m_binding_name_map.contains(binding_name)) {
        return m_bindings.at(m_binding_num_to_idx_map.at(m_binding_name_map.at(binding_name)));
    }
    return {};
}

bool DescSetLayout::haveBindingType(VkDescriptorType desc_type) const {
    for(const VkDescriptorSetLayoutBinding& binding : m_bindings) {
        if(binding.descriptorType == desc_type) return true;
    }
    return false;
}

bool DescSetLayout::haveBindingNum(DescSetLayout::BindingNum binding_num) const {
    return m_binding_num_to_idx_map.contains(binding_num);
}

bool DescSetLayout::haveBindingName(const std::string& binding_name) const {
    return m_binding_name_map.contains(binding_name);
}

const std::string& DescSetLayout::getBindingName(VkDescriptorType desc_type) const {
    static const std::string empty = "";
    for(const VkDescriptorSetLayoutBinding& binding : m_bindings) {
        if(binding.descriptorType == desc_type) {
            return m_binding_num_to_name_map.at(binding.binding);
        }
    }
    return empty;
}

const std::string& DescSetLayout::getBindingName(DescSetLayout::BindingNum binding_num) const {
    return m_binding_num_to_name_map.at(binding_num);
}

DescSetLayout::BindingNum DescSetLayout::getBindingNum(const std::string& binding_name) const {
    return m_binding_name_map.at(binding_name);
}

const std::vector<std::shared_ptr<VulkanSampler>>& DescSetLayout::getImmutableSamplers() const {
    return m_immutable_samplers;
}

const std::vector<VkSampler>& DescSetLayout::getImmutableSamplersPtr() const {
    return m_immutable_samplers_ptr;
}

VkDescriptorSetLayoutCreateInfo DescSetLayout::getDescriptorSetLayoutInfo() const {
    return m_desc_layout_info;
}

VkDescriptorSetLayout DescSetLayout::getDescriptorSetLayout() const {
    return m_desc_layout;
}