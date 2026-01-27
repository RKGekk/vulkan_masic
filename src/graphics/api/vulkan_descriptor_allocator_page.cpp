#include "vulkan_descriptor_allocator_page.h"

#include "vulkan_device.h"

bool DescriptorAllocatorPage::init(std::shared_ptr<VulkanDevice> device, std::shared_ptr<DescSetLayout> layout, VkDescriptorPoolCreateFlags flags, uint32_t num_descriptors_per_heap) {
    m_device = device;
    m_layout = layout;
    m_num_descriptors_per_heap = num_descriptors_per_heap;

    std::unordered_map<VkDescriptorType, size_t> types_map = getTypesCount();
    for (const auto&[desc_type, ct] : types_map) {
        VkDescriptorPoolSize pool_size{};
        pool_size.type = desc_type;
        pool_size.descriptorCount = static_cast<uint32_t>(ct) * num_descriptors_per_heap;
        m_pool_sizes.push_back(pool_size);
    }
    
    m_pool_info = VkDescriptorPoolCreateInfo{};
    m_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    m_pool_info.poolSizeCount = static_cast<uint32_t>(m_pool_sizes.size());
    m_pool_info.pPoolSizes = m_pool_sizes.data();
    m_pool_info.maxSets = num_descriptors_per_heap;
    m_pool_info.flags = flags;
 
    VkResult result = vkCreateDescriptorPool(m_device->getDevice(), &m_pool_info, nullptr, &m_descriptor_pool);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }

    m_desc_sets.resize(num_descriptors_per_heap);

    std::vector<VkDescriptorSetLayout> layouts(num_descriptors_per_heap, m_layout->getDescriptorSetLayout());
    VkDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = m_descriptor_pool;
    alloc_info.descriptorSetCount = num_descriptors_per_heap;
    alloc_info.pSetLayouts = layouts.data();
    
    VkResult result = vkAllocateDescriptorSets(m_device->getDevice(), &alloc_info, m_desc_sets.data());
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    return true;
}

void DescriptorAllocatorPage::destroy() {
    vkFreeDescriptorSets(m_device->getDevice(), m_descriptor_pool, static_cast<uint32_t>(m_desc_sets.size()), m_desc_sets.data());
    vkDestroyDescriptorPool(m_device->getDevice(), m_descriptor_pool, nullptr);
}

void DescriptorAllocatorPage::reset() {
    vkFreeDescriptorSets(m_device->getDevice(), m_descriptor_pool, static_cast<uint32_t>(m_desc_sets.size()), m_desc_sets.data());
    vkResetDescriptorPool(m_device->getDevice(), m_descriptor_pool, 0u);
}

std::shared_ptr<DescSetLayout> DescriptorAllocatorPage::GetDescSetLayout() const {
    return m_layout;
}

bool DescriptorAllocatorPage::HasSpace(uint32_t num_descriptors) const {
    return m_desc_sets.size() > num_descriptors;
}

uint32_t DescriptorAllocatorPage::NumFreeHandles() const {
    return m_desc_sets.size();
}

std::vector<VkDescriptorSet> DescriptorAllocatorPage::Allocate(uint32_t num_descriptors) {
    std::vector<VkDescriptorSet>::iterator start_it = m_desc_sets.end() - num_descriptors;
    std::vector<VkDescriptorSet> res;
    res.reserve(num_descriptors);
    std::move(start_it, m_desc_sets.end(), std::back_inserter(res));
    m_desc_sets.erase(start_it, m_desc_sets.end());

    return res;
}

VkDescriptorSet DescriptorAllocatorPage::Allocate() {
    VkDescriptorSet res;
    res = m_desc_sets.back();
    m_desc_sets.pop_back();

    return res;
}

void DescriptorAllocatorPage::Free(std::vector<VkDescriptorSet> desc) {
    m_stale_desc_sets.insert(m_stale_desc_sets.end(), desc.begin(), desc.end());
}

void DescriptorAllocatorPage::Free(VkDescriptorSet desc) {
    m_stale_desc_sets.push_back(desc);
}

void DescriptorAllocatorPage::ReleaseStaleDescriptors() {
    m_desc_sets.insert(m_desc_sets.end(), m_stale_desc_sets.begin(), m_stale_desc_sets.end());
    m_stale_desc_sets.clear();
}

std::shared_ptr<DescriptorAllocatorPage> DescriptorAllocatorPage::InitNewPage() const {
    std::shared_ptr<DescriptorAllocatorPage> new_page = std::make_shared<DescriptorAllocatorPage>();
    new_page->init(m_device, m_layout, m_pool_info.flags, m_num_descriptors_per_heap);

    return new_page;
}

std::unordered_map<VkDescriptorType, size_t> DescriptorAllocatorPage::getTypesCount() const {
    std::unordered_map<VkDescriptorType, size_t> result;
    
    for (VkDescriptorSetLayoutBinding desc_binding : m_layout->getBindings()) {
        result[desc_binding.descriptorType] += desc_binding.descriptorCount;
    }

    return result;
}