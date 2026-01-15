#include "vulkan_descriptor.h"

bool VulkanDescriptor::init(VkDevice device, std::vector<Binding> sets, uint32_t set_copy_ct) {
    m_device = device;
    m_frame_count = set_copy_ct;
    m_bingings_for_sets = std::move(sets);

    m_desc_layout = createDescSetLayout();
    m_descriptor_pool = createDescPool();
    m_descriptor_sets = createDescSets();

    return true;
}

void VulkanDescriptor::destroy() {
    vkDestroyDescriptorPool(m_device, m_descriptor_pool, nullptr);
    vkDestroyDescriptorSetLayout(m_device, m_desc_layout, nullptr);
}

VkDescriptorSetLayout VulkanDescriptor::getDescLayouts() const {
    return m_desc_layout;
}

const std::vector<VkDescriptorSet>& VulkanDescriptor::getDescriptorSets() const {
    return m_descriptor_sets;
}

const std::vector<VulkanDescriptor::Binding>& VulkanDescriptor::getBingingsForSets() const {
    return m_bingings_for_sets;
}

VkDescriptorSetLayout VulkanDescriptor::createDescSetLayout() {
    std::vector<VkDescriptorSetLayoutBinding> bindings = getDescSetLayoutBindings();

    VkDescriptorSetLayoutCreateInfo layout_info{};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
    layout_info.pBindings = bindings.data();
    
    VkDescriptorSetLayout desc_set_layout = VK_NULL_HANDLE;
    VkResult result = vkCreateDescriptorSetLayout(m_device, &layout_info, nullptr, &desc_set_layout);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
    
    return desc_set_layout;
}

VkDescriptorPool VulkanDescriptor::createDescPool() {
    std::unordered_map<VkDescriptorType, size_t> types_map = getTypesCount();
    std::vector<VkDescriptorPoolSize> pool_sizes;
    for (const auto&[desc_type, ct] : types_map) {
        VkDescriptorPoolSize pool_size{};
        pool_size.type = desc_type;
        pool_size.descriptorCount = ct;
        pool_sizes.push_back(pool_size);
    }
    
    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
    pool_info.pPoolSizes = pool_sizes.data();
    pool_info.maxSets = m_frame_count;
    pool_info.flags = 0u;
 
    VkDescriptorPool desc_pool;
    VkResult result = vkCreateDescriptorPool(m_device, &pool_info, nullptr, &desc_pool);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
    return desc_pool;
}

std::vector<VkDescriptorSet> VulkanDescriptor::createDescSets() {
    std::vector<VkDescriptorSet> desc_sets;
    desc_sets.resize(m_frame_count);

    std::vector<VkDescriptorSetLayout> layouts(m_frame_count, m_desc_layout);
    VkDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = m_descriptor_pool;
    alloc_info.descriptorSetCount = static_cast<uint32_t>(m_frame_count);
    alloc_info.pSetLayouts = layouts.data();
    
    VkResult result = vkAllocateDescriptorSets(m_device, &alloc_info, desc_sets.data());
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    size_t bindings_ct = m_bingings_for_sets[0].size();
    std::vector<VkWriteDescriptorSet> desc_writes;
    desc_writes.reserve(bindings_ct * m_frame_count);
    for(size_t i = 0u; i < m_frame_count; ++i) {
        for(size_t binding_id = 0u; binding_id < bindings_ct; ++binding_id) {
            if(m_bingings_for_sets[i][binding_id].layout_binding.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
                VkDescriptorImageInfo image_info{};
                image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                image_info.imageView = m_bingings_for_sets[i][binding_id].image_info->imageView;
                image_info.sampler = m_bingings_for_sets[i][binding_id].sampler;
                
                VkWriteDescriptorSet desc_write{};
                desc_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                desc_write.dstSet = desc_sets[i];
                desc_write.dstBinding = binding_id;
                desc_write.dstArrayElement = 0u;
                desc_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                desc_write.descriptorCount = 1u;
                desc_write.pImageInfo = &image_info;
                desc_write.pBufferInfo = nullptr;
                desc_write.pTexelBufferView = nullptr;

                desc_writes.push_back(desc_write);
            }
            else if(m_bingings_for_sets[i][binding_id].layout_binding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                VkWriteDescriptorSet desc_write{};
                desc_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                desc_write.dstSet = desc_sets[i];
                desc_write.dstBinding = binding_id;
                desc_write.dstArrayElement = 0u;
                desc_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                desc_write.descriptorCount = 1u;
                desc_write.pBufferInfo = m_bingings_for_sets[i][binding_id].buffer_info.get();
                desc_write.pImageInfo = nullptr;
                desc_write.pTexelBufferView = nullptr;

                desc_writes.push_back(desc_write);
            }
        }
    }
    vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(desc_writes.size()), desc_writes.data(), 0u, nullptr);

    return desc_sets;
}

std::unordered_map<VkDescriptorType, size_t> VulkanDescriptor::getTypesCount() {
    std::unordered_map<VkDescriptorType, size_t> result;
    
    for (const Binding& binding: m_bingings_for_sets) {
        for (const VulkanDescriptorBinding& desc_binding: binding) {
            ++result[desc_binding.layout_binding.descriptorType];
        }
    }
    return result;
}

std::vector<VkDescriptorSetLayoutBinding> VulkanDescriptor::getDescSetLayoutBindings() {
    std::vector<VkDescriptorSetLayoutBinding> result;
    result.reserve(m_bingings_for_sets[0u].size());
    for (const VulkanDescriptorBinding& binding: m_bingings_for_sets[0u]) {
        result.push_back(binding.layout_binding);
    }
    
    return result;
}