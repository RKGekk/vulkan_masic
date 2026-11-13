#include "vulkan_image_buffer.h"

#include "vulkan_device.h"
#include "vulkan_buffer.h"
#include "../pod/format_config.h"
#include "../pod/image_buffer_config.h"
#include "vulkan_resources_manager.h"
#include "../../application.h"
#include "../vulkan_renderer.h"

#include <bit>

// #ifdef STB_IMAGE_IMPLEMENTATION
// #undef STB_IMAGE_IMPLEMENTATION
// #endif

// #ifdef STB_IMAGE_WRITE_IMPLEMENTATION
// #undef STB_IMAGE_WRITE_IMPLEMENTATION
// #endif

//#define STB_IMAGE_IMPLEMENTATION
//#define STBI_MSC_SECURE_CRT 
//#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>

const std::shared_ptr<VulkanImageBuffer> VulkanImageBuffer::IMAGE_BUFFER_NULL_PTR = nullptr;
const std::string VulkanImageBuffer::STAGING_BUFFER_TYPE = "basic_staging_buffer";

VulkanImageBuffer::VulkanImageBuffer(std::shared_ptr<VulkanDevice> device, std::string name) : m_device(std::move(device)), m_name(std::move(name)) {}
VulkanImageBuffer::VulkanImageBuffer(std::shared_ptr<VulkanDevice> device) : m_device(std::move(device)), m_name(std::to_string(rand())) {}

bool VulkanImageBuffer::init(std::shared_ptr<ImageBufferConfig> image_buffer_config) {
    VkImage image = m_image = createImage(m_device, m_image_config, m_name);
    return init(image, std::move(image_buffer_config));
}

bool VulkanImageBuffer::init(VkImage image, std::shared_ptr<ImageBufferConfig> image_buffer_config) {
    using namespace std::literals;

    m_image_config = std::move(image_buffer_config);
    m_image = image;
    VkMemoryAllocateInfo alloc_info = getImageSize(m_device, m_image_config, m_image);
    m_memory = allocateImageMemory(m_device, m_image, alloc_info);

    if(m_image_config->getMemoryProperties() & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
        vkMapMemory(m_device->getDevice(), m_memory, 0, alloc_info.allocationSize, 0u, &m_mapped);
    }

    createViews();

    return true;
}

bool VulkanImageBuffer::init(std::shared_ptr<CommandBatch>& command_buffer, std::shared_ptr<CpuResource> pixels, std::shared_ptr<ImageBufferConfig> image_buffer_config) {
    using namespace std::literals;

    init(std::move(image_buffer_config));
    
    if(!pixels) {
        return true;
    }

    if(m_image_config->getMemoryProperties() & (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
        if(m_image_config->getMemoryProperties() & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
            memcpy(m_mapped, pixels->getDataPtr(), pixels->getSize());
        }
        if(m_image_config->getMemoryProperties() & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
            VkMappedMemoryRange range{};
            range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            range.pNext = nullptr;
            range.memory = m_memory;
            range.offset = 0u;
            range.size = VK_WHOLE_SIZE;

            VkResult result = vkFlushMappedMemoryRanges(m_device->getDevice(), 1u, &range);
            if (result != VK_SUCCESS) {
                throw std::runtime_error("failed to flush buffer to device!");
            }
        }
    }
    else {
        VulkanRenderer& renderer = Application::GetRenderer();
        std::shared_ptr<VulkanResourcesManager>& resources_manager = renderer.getResourcesManager();
        std::shared_ptr<VulkanBuffer> staging_buffer = resources_manager->create_buffer(m_name + "_staging_buffer"s, STAGING_BUFFER_TYPE, pixels, command_buffer);
        command_buffer->addResource(staging_buffer);

        if(m_image_config->getImageInfo().initialLayout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            VulkanCommandManager::transitionImageLayout(
                command_buffer->getCommandBufer(),
                m_image,
                m_image_config->getImageInfo().format,
                m_image_config->getImageInfo().initialLayout,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                m_image_config->getImageInfo().mipLevels
            );
        }
        VulkanCommandManager::copyBufferToImage(
            command_buffer->getCommandBufer(),
            staging_buffer->getBuffer(),
            m_image,
            m_image_config->getImageInfo().extent,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
        );
    }

    if(m_image_config->getImageInfo().mipLevels != 1u) {
        VulkanCommandManager::generateMipmaps(
            command_buffer->getCommandBufer(),
            m_image,
            m_image_config->getImageInfo().format,
            m_image_config->getFormat()->getExtent2D(),
            m_image_config->getImageInfo().mipLevels,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            m_image_config->getAfterInitLayout()
        );
    }
    else if(m_image_config->getImageInfo().initialLayout != m_image_config->getAfterInitLayout()) {
        VulkanCommandManager::transitionImageLayout(
            command_buffer->getCommandBufer(),
            m_image,
            m_image_config->getImageInfo().format,
            m_image_config->getImageInfo().initialLayout,
            m_image_config->getAfterInitLayout(),
            1u
        );
    }

    return true;
}

bool VulkanImageBuffer::init(std::shared_ptr<CommandBatch>& command_buffer, const std::shared_ptr<ImageBufferConfig>& image_buffer_config_template, const std::string& path_to_file) {
    using namespace std::literals;

    int tex_width;
    int tex_height;
    int tex_channels;
    //stbi_set_flip_vertically_on_load(true);
    stbi_uc* pixels = stbi_load(path_to_file.c_str(), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }
    size_t array_size = static_cast<size_t>(tex_width) * tex_height * tex_channels;
    const char* array_start = reinterpret_cast<const char*>(pixels);
    const char* array_end = array_start + array_size;

    std::shared_ptr<ImageBufferConfig> image_buffer_config = image_buffer_config_template->makeInstance(path_to_file + "_cfg"s, {(uint32_t)tex_width, (uint32_t)tex_height});
    std::shared_ptr<CpuResource> img_buff = std::make_shared<CpuResource>(path_to_file + "_image_CPU"s);
	img_buff->init({array_start, array_end});
    bool result = init(command_buffer, img_buff, std::move(image_buffer_config));
    //command_buffer->addResource(std::move(img_buff));

    stbi_image_free(pixels);

    return result;
}

void VulkanImageBuffer::destroy() {
    //m_image_config->destroy();
    for (auto&[view_name, vk_view] : m_image_view_map) {
        vkDestroyImageView(m_device->getDevice(), vk_view, nullptr);
        vk_view = VK_NULL_HANDLE;
    }

    if(!m_image_config->isExternalMemoryControl()) {
        vkDestroyImage(m_device->getDevice(), m_image, nullptr);
        vkFreeMemory(m_device->getDevice(), m_memory, nullptr);
    }
    m_image = VK_NULL_HANDLE;
    m_memory = VK_NULL_HANDLE;
}

VkImage VulkanImageBuffer::getImageBuffer() const {
    return m_image;
}
    
VkImageView VulkanImageBuffer::getImageBufferView() const {
    return (*m_image_view_map.begin()).second;
};

VkImageView VulkanImageBuffer::getImageBufferView(const std::string& view_name) const {
    return m_image_view_map.at(view_name);
}

const std::unordered_map<std::string, VkImageView>& VulkanImageBuffer::getImageViewMap() const {
    return m_image_view_map;
}


VkDeviceMemory VulkanImageBuffer::getMemory() const {
    return m_memory;
}

VkDeviceSize VulkanImageBuffer::getSize() const {
    return m_image_size;
}

VkSubresourceLayout VulkanImageBuffer::getSubresourceSizes(uint32_t mip_level, uint32_t array_layer) const {
    return getSubresourceSizes( (*m_image_config->getViewInfoMap().begin()).second->image_view_info.subresourceRange.aspectMask, mip_level, array_layer);
}

VkSubresourceLayout VulkanImageBuffer::getSubresourceSizes(VkImageAspectFlags aspect, uint32_t mip_level, uint32_t array_layer) const {
    VkSubresourceLayout result{};
    VkImageSubresource subresource{};
    subresource.aspectMask = aspect;
    subresource.mipLevel = mip_level;
    subresource.arrayLayer = array_layer;

    vkGetImageSubresourceLayout(m_device->getDevice(), m_image, &subresource, &result);

    return result;
}

std::shared_ptr<ImageBufferConfig>& VulkanImageBuffer::getImageConfig() {
    return m_image_config;
}

void VulkanImageBuffer::changeLayout(VkCommandBuffer command_buffer, VkImageLayout old_layout, VkImageLayout new_layout) {
    VulkanCommandManager::transitionImageLayout(
        command_buffer,
        m_image,
        m_image_config->getImageInfo().format,
        old_layout,
        new_layout,
        m_image_config->getImageInfo().mipLevels
    );
}

const RenderResource::ResourceName& VulkanImageBuffer::getName() const {
    return m_name;
}

RenderResource::Type VulkanImageBuffer::getType() const {
    return RenderResource::Type::IMAGE;
}

void VulkanImageBuffer::AddUpdateMetadataFn(UpdateMetadataFn fn) {
    m_update_functions.push_back(std::move(fn));
}

void VulkanImageBuffer::ClearUpdateMetadataFn() {
    m_update_functions.clear();
}

VkImage VulkanImageBuffer::createImage(const std::shared_ptr<VulkanDevice>& device, const std::shared_ptr<ImageBufferConfig>& img_cfg, const ResourceName& name) {
    VkImage image;
    VkResult result = vkCreateImage(device->getDevice(), &img_cfg->getImageInfo(), nullptr, &image);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

#ifndef NDEBUG
    std::string buffer_name = "buffer_"s + name;
    auto vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(Application::GetInstance().getInstance(), "vkSetDebugUtilsObjectNameEXT");
    VkDebugUtilsObjectNameInfoEXT name_info = {};
    name_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    name_info.objectType = VK_OBJECT_TYPE_IMAGE;
    name_info.objectHandle = (uint64_t)image;
    name_info.pObjectName = buffer_name.c_str();

    vkSetDebugUtilsObjectNameEXT(device->getDevice(), &name_info);
#endif
}

VkMemoryAllocateInfo VulkanImageBuffer::getImageSize(const std::shared_ptr<VulkanDevice>& device, const std::shared_ptr<ImageBufferConfig>& img_cfg, VkImage image) {
    VkMemoryRequirements mem_req{};
    vkGetImageMemoryRequirements(device->getDevice(), image, &mem_req);
    VkDeviceSize image_size = mem_req.size;
    VkMemoryPropertyFlags props = img_cfg->getMemoryProperties();
    if(img_cfg->isMemoryPropertiesByMemoryRequirements()) {
        int memory_type_idx = std::countr_zero(mem_req.memoryTypeBits);
        if(memory_type_idx < device->getDeviceAbilities().memory_properties.memoryTypeCount) {
            VkMemoryPropertyFlags memory_property_flags = device->getDeviceAbilities().memory_properties.memoryTypes[memory_type_idx].propertyFlags;
            if(memory_property_flags) {
                props = device->getDeviceAbilities().memory_properties.memoryTypes[memory_type_idx].propertyFlags;
            }
            else {
                mem_req.memoryTypeBits = mem_req.memoryTypeBits & ~(1 << memory_type_idx);
                memory_type_idx = std::countr_zero(mem_req.memoryTypeBits);
                if(memory_type_idx < device->getDeviceAbilities().memory_properties.memoryTypeCount) {
                    props = device->getDeviceAbilities().memory_properties.memoryTypes[memory_type_idx].propertyFlags;
                }
            }
        }
    }

    uint32_t mem_type_idx = device->findMemoryType(mem_req.memoryTypeBits, props);
    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_req.size;
    alloc_info.memoryTypeIndex = mem_type_idx;

    return alloc_info;
}

VkDeviceMemory VulkanImageBuffer::allocateImageMemory(const std::shared_ptr<VulkanDevice>& device, VkImage image, const VkMemoryAllocateInfo& alloc_info) {
    VkDeviceMemory memory;

    VkResult result = vkAllocateMemory(device->getDevice(), &alloc_info, nullptr, &memory);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    VkDeviceSize offset = 0u;
    vkBindImageMemory(device->getDevice(), image, memory, offset);

    return memory;
}

void VulkanImageBuffer::createViews() {
    for(auto&[view_type_name, view_cfg_ptr] : m_image_config->getViewInfoMap()) {
        view_cfg_ptr->image_view_info.image = m_image;
        VkResult result = vkCreateImageView(m_device->getDevice(), &view_cfg_ptr->image_view_info, nullptr, &m_image_view_map[view_type_name]);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }

#ifndef NDEBUG
        std::string buffer_view_name = "buffer_view_"s + view_type_name;
        auto vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(Application::GetInstance().getInstance(), "vkSetDebugUtilsObjectNameEXT");
        VkDebugUtilsObjectNameInfoEXT name_info = {};
        name_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        name_info.objectType = VK_OBJECT_TYPE_IMAGE_VIEW;
        name_info.objectHandle = (uint64_t)m_image_view_map[view_type_name];
        name_info.pObjectName = buffer_view_name.c_str();

        vkSetDebugUtilsObjectNameEXT(m_device->getDevice(), &name_info);
#endif
    }
}
