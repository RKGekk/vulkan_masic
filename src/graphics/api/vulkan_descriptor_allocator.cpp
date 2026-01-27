#include "vulkan_descriptor_allocator.h"

#include "vulkan_descriptor_allocator_page.h"

bool DescriptorAllocator::init(std::shared_ptr<VulkanDevice> device, std::vector<std::shared_ptr<DescSetLayout>> layouts, VkDescriptorPoolCreateFlags flags, uint32_t num_descriptors_per_heap) {
    m_device = device;
    m_num_descriptors_per_heap = num_descriptors_per_heap;

    for(std::shared_ptr<DescSetLayout>& layout : layouts) {
        std::shared_ptr<DescriptorAllocatorPage> new_page = std::make_shared<DescriptorAllocatorPage>();
        new_page->init(m_device, layout, flags, m_num_descriptors_per_heap);
        m_heap_pool.insert({layout->getName(), {new_page}});
    }
}

void DescriptorAllocator::destroy() {
    for(auto&[desc_name, pool] : m_heap_pool) {
        for(std::shared_ptr<DescriptorAllocatorPage>& page : pool) {
            page->destroy();
        }
    }
    m_heap_pool.clear();
}

VkDescriptorSet DescriptorAllocator::Allocate(const std::string& desc_layout_name) {
    HeapPool& pool = m_heap_pool[desc_layout_name];
    bool allocated = false;
    for(std::shared_ptr<DescriptorAllocatorPage> page : pool) {
        if(page->HasSpace(1u)) {
            return page->Allocate();
        }
    }
    
    std::shared_ptr<DescriptorAllocatorPage> new_page = pool[0]->InitNewPage();
    pool.push_back(new_page);

    return new_page->Allocate();
}

std::vector<VkDescriptorSet> DescriptorAllocator::Allocate(const std::string& desc_layout_name, uint32_t num_descriptors) {
    HeapPool& pool = m_heap_pool[desc_layout_name];
    bool allocated = false;
    for(std::shared_ptr<DescriptorAllocatorPage> page : pool) {
        if(page->HasSpace(num_descriptors)) {
            return page->Allocate(num_descriptors);
        }
    }
    
    std::shared_ptr<DescriptorAllocatorPage> new_page = pool[0]->InitNewPage();
    pool.push_back(new_page);

    return new_page->Allocate(num_descriptors);
}

void DescriptorAllocator::ReleaseStaleDescriptors() {
    for(auto&[desc_name, pool] : m_heap_pool) {
        for(std::shared_ptr<DescriptorAllocatorPage>& page : pool) {
            page->ReleaseStaleDescriptors();
        }
    }
}