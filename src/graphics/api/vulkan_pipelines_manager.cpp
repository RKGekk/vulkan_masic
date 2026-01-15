#include "vulkan_pipelines_manager.h"

#include "../../tools/string_tools.h"

bool VulkanPipelinesManager::init(VkDevice device) {
    m_device = device;

    VkPipelineCacheCreateInfo pipeline_cache_info{};
    pipeline_cache_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    pipeline_cache_info.pNext = nullptr;
    pipeline_cache_info.initialDataSize = 0u;
    pipeline_cache_info.pInitialData = nullptr;
    pipeline_cache_info.flags = 0u;
    VkResult  result = vkCreatePipelineCache(device, &pipeline_cache_info, NULL, &m_pipeline_cache);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline cache!");
    }

    return true;
}

void VulkanPipelinesManager::destroy() {
    vkDestroyPipelineCache(m_device, m_pipeline_cache, NULL);
}

void VulkanPipelinesManager::saveCacheToFile(VkPipelineCache cache, const std::string& file_name) {
    size_t cache_data_size;
    // Determine the size of the cache data.
    VkResult result = vkGetPipelineCacheData(m_device, cache, &cache_data_size, nullptr);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to read pipeline cache size!");
    }

    if (cache_data_size == 0u) return;

    // Allocate a temporary store for the cache data.
    std::vector<char> data(cache_data_size);

    // Retrieve the actual data from the cache.
    result = vkGetPipelineCacheData(m_device, cache, &cache_data_size, data.data());
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to write pipeline cache to file!");
    }
    // Open the file and write the data to it.
    writeFile(file_name, data.size(), data.data());
}