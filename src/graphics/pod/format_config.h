#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <pugixml.hpp>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class VulkanDevice;
class VulkanSampler;
class WindowSurface;
class SwapchainSupportDetails;

class FormatConfig {
public:
    enum class ExtentSource { AUTO, AS_SWAPCHAIN, EXACT };

    bool init(const std::shared_ptr<VulkanDevice>& device, const std::shared_ptr<WindowSurface>& window, const SwapchainSupportDetails& swapchain_support_details, const std::string& rg_file_path);
    bool init(const std::shared_ptr<VulkanDevice>& device, const std::shared_ptr<WindowSurface>& window, const SwapchainSupportDetails& swapchain_support_details, const pugi::xml_node& format_data);

    const std::string& getName() const;

    VkImageCreateFlags getImageFlags() const;
    bool hasImageFlags(VkImageCreateFlags flags) const;
    void setImageFlags(VkImageCreateFlags flags);
    void addImageFlags(VkImageCreateFlags flags);

    VkImageType getVkImageType() const;
    void setVkImageType(VkImageType type);

    VkExtent2D getExtent2D() const;
    void setExtent2D(VkExtent2D extent);

    VkExtent3D getExtent3D() const;
    void setExtent3D(VkExtent3D extent);

    uint32_t getMipLevels() const;
    void setMipLevels(uint32_t lvl);

    uint32_t getArrayLayers() const;
    void setArrayLayers(uint32_t layers);

    VkSampleCountFlagBits getSamplesCount() const;
    void setSamplesCount(VkSampleCountFlagBits samples);

    VkImageTiling getTiling() const;
    void setTiling(VkImageTiling tiling);

    VkImageUsageFlags getImageUsage() const;
    bool hasImageUsage(VkImageUsageFlags usage) const;
    void setImageUsage(VkImageUsageFlags usage);
    void addImageUsage(VkImageUsageFlags usage);

    VkFormatFeatureFlags getFeatureFlags() const;
    bool hasFeatureFlags(VkFormatFeatureFlags flags) const;
    void setFeatureFlags(VkFormatFeatureFlags flags);
    void addFeatureFlags(VkFormatFeatureFlags flags);

    VkFormat getVkFormat() const;
    void setVkFormat(VkFormat format);

    VkColorSpaceKHR getVkColorSpace() const;
    void setVkColorSpace(VkColorSpaceKHR color_space);

    VkSurfaceFormatKHR getVkSurfaceFormat() const;

    VkSharingMode getImagesSharingMode() const;
    uint32_t getQueueFamilyIndexCount() const;
    const uint32_t* getQueueFamilyIndicesPtr() const;

    std::shared_ptr<FormatConfig> makeInstance(std::string name, VkExtent2D extent = {0, 0}) const;

private:

    std::string m_name;
    VkImageCreateFlags m_image_flags;
    VkImageType m_image_type;
    ExtentSource m_extent_source;
    VkExtent2D m_extent_2D;
    VkExtent3D m_extent_3D;
    uint32_t m_mip_levels;
    uint32_t m_array_layers;
    VkSampleCountFlagBits m_samples;
    VkImageTiling m_tiling;
    VkImageUsageFlags m_usage;
    VkFormatFeatureFlags m_feature_flags;
    VkSharingMode m_images_sharing_mode;
    uint32_t m_queue_family_index_count;
    const uint32_t* m_pQueue_family_indices;

    VkFormat m_format;
    VkColorSpaceKHR m_color_space;
};