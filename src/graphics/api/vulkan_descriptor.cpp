#include "vulkan_descriptor.h"

bool VulkanDescriptor::init(VkDevice device, std::shared_ptr<DescSetLayout> layout, VkDescriptorSet descriptor_set) {
    m_device = device;
    m_layout = std::move(layout);
    m_descriptor_set = descriptor_set;

    return true;
}

void VulkanDescriptor::destroy() {
    
}

const std::string& VulkanDescriptor::getName() const {
    return m_layout->getName();
}

VkDescriptorSetLayout VulkanDescriptor::getDescLayout() const {
    return m_layout->getDescriptorSetLayout();
}

VkDescriptorSet VulkanDescriptor::getDescriptorSet() const {
    return m_descriptor_set;
}

void VulkanDescriptor::updateDescSampler(VkSampler sampler) {
    VkDescriptorImageInfo image_info{};
    image_info.sampler = sampler;

    VkWriteDescriptorSet desc_write{};
    desc_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    desc_write.dstSet = m_descriptor_set;
    desc_write.dstBinding = m_layout->getBinding(VK_DESCRIPTOR_TYPE_SAMPLER).binding;
    desc_write.dstArrayElement = 0u;
    desc_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    desc_write.descriptorCount = 1u;
    desc_write.pImageInfo = &image_info;
    desc_write.pBufferInfo = nullptr;
    desc_write.pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(m_device, 1u, &desc_write, 0u, nullptr);
}

void VulkanDescriptor::updateDescImage(VkImageView image_view, VkImageLayout image_layout) {
    VkDescriptorImageInfo image_info{};
    image_info.imageView = image_view;
    image_info.imageLayout = image_layout;

    VkWriteDescriptorSet desc_write{};
    desc_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    desc_write.dstSet = m_descriptor_set;
    desc_write.dstArrayElement = 0u;
    desc_write.descriptorCount = 1u;
    desc_write.pImageInfo = &image_info;
    desc_write.pBufferInfo = nullptr;
    desc_write.pTexelBufferView = nullptr;

    if(m_layout->haveBindingType(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)) {
        desc_write.dstBinding = m_layout->getBinding(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE).binding;
        desc_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    }
    else if(m_layout->haveBindingType(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)) {
        desc_write.dstBinding = m_layout->getBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE).binding;
        desc_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    }
    else if(m_layout->haveBindingType(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)) {
        desc_write.dstBinding = m_layout->getBinding(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT).binding;
        desc_write.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    }
    else {
        throw std::runtime_error("decriptor not have images attached!");
    }

    vkUpdateDescriptorSets(m_device, 1u, &desc_write, 0u, nullptr);
}

void VulkanDescriptor::updateDescCombinedImage(VkSampler sampler, VkImageView image_view, VkImageLayout image_layout) {
    VkDescriptorImageInfo image_info{};
    image_info.imageLayout = image_layout;
    image_info.imageView = image_view;
    image_info.sampler = sampler;

    VkWriteDescriptorSet desc_write{};
    desc_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    desc_write.dstSet = m_descriptor_set;
    desc_write.dstBinding = m_layout->getBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER).binding;
    desc_write.dstArrayElement = 0u;
    desc_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    desc_write.descriptorCount = 1u;
    desc_write.pImageInfo = &image_info;
    desc_write.pBufferInfo = nullptr;
    desc_write.pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(m_device, 1u, &desc_write, 0u, nullptr);
}

void VulkanDescriptor::updateDescImageInfo(uint32_t binding, VkSampler sampler, VkImageView image_view, VkImageLayout image_layout) {
    VkDescriptorImageInfo image_info{};
    image_info.imageLayout = image_layout;
    image_info.imageView = image_view;
    image_info.sampler = sampler;

    VkWriteDescriptorSet desc_write{};
    desc_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    desc_write.dstSet = m_descriptor_set;
    desc_write.dstBinding = binding;
    desc_write.dstArrayElement = 0u;
    desc_write.descriptorType = m_layout->getBinding(binding).descriptorType;
    desc_write.descriptorCount = 1u;
    desc_write.pImageInfo = &image_info;
    desc_write.pBufferInfo = nullptr;
    desc_write.pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(m_device, 1u, &desc_write, 0u, nullptr);
}

void VulkanDescriptor::updateDescBuffer(uint32_t binding, VkBuffer buffer, VkDeviceSize size, VkDeviceSize offset) {
    VkDescriptorBufferInfo buffer_info{};
    buffer_info.buffer = buffer;
    buffer_info.offset = offset;
    buffer_info.range = size;

    VkWriteDescriptorSet desc_write{};
    desc_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    desc_write.dstSet = m_descriptor_set;
    desc_write.dstBinding = binding;
    desc_write.dstArrayElement = 0u;
    desc_write.descriptorType = m_layout->getBinding(binding).descriptorType;
    desc_write.descriptorCount = 1u;
    desc_write.pBufferInfo = &buffer_info;
    desc_write.pImageInfo = nullptr;
    desc_write.pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(m_device, 1u, &desc_write, 0u, nullptr);
}

void VulkanDescriptor::updateDescTexel(VkBufferView buffer_view) {
    VkWriteDescriptorSet desc_write{};
    desc_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    desc_write.dstSet = m_descriptor_set;
    desc_write.dstBinding = m_layout->getBinding(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER).binding;
    desc_write.dstArrayElement = 0u;
    desc_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    desc_write.descriptorCount = 1u;
    desc_write.pBufferInfo = nullptr;
    desc_write.pImageInfo = nullptr;
    desc_write.pTexelBufferView = &buffer_view;

    vkUpdateDescriptorSets(m_device, 1u, &desc_write, 0u, nullptr);
}