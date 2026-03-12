#include "format_config.h"

#include "../api/vulkan_device.h"
#include "../api/vulkan_swapchain.h"
#include "../../window_surface.h"
#include "../../tools/string_tools.h"

bool FormatConfig::init(const std::shared_ptr<VulkanDevice>& device, const std::shared_ptr<WindowSurface>& window, const SwapchainSupportDetails& swapchain_support_details, const std::string& rg_file_path) {
    pugi::xml_document xml_doc;
	pugi::xml_parse_result parse_res = xml_doc.load_file(rg_file_path.c_str());
	if (!parse_res) { return false;	}

	pugi::xml_node root_node = xml_doc.root();
	if (!root_node) { return false; }
	root_node = root_node.child("RenderGraph");

    pugi::xml_node formats_node = root_node.child("Formats");
	if (!formats_node) return false;
    
	for (pugi::xml_node format_node = formats_node.first_child(); format_node; format_node = format_node.next_sibling()) {
	    return init(device, window, swapchain_support_details, format_node);
	}

    return true;
}

bool FormatConfig::init(const std::shared_ptr<VulkanDevice>& device, const std::shared_ptr<WindowSurface>& window, const SwapchainSupportDetails& swapchain_support_details, const pugi::xml_node& format_data) {
    using namespace std::literals;

    std::string auto_config_str = format_data.attribute("select").as_string();
    bool auto_config = auto_config_str == "auto"s;

    m_name = format_data.attribute("name").as_string();

    pugi::xml_node image_buffer_flags_node = format_data.child("Falgs");
    if(image_buffer_flags_node) {
        for (pugi::xml_node flag_node = image_buffer_flags_node.first_child(); flag_node; flag_node = flag_node.next_sibling()) {
		    m_image_flags |= getVkImageCreateFlag(flag_node.text().as_string());
		}
    }

    m_image_type = getImageType(format_data.child("ImageType").text().as_string());

    pugi::xml_node extent_node = format_data.child("Extent");
    std::string extent_source = extent_node.attribute("source").as_string();
    if(extent_source == "auto") m_extent_source = ExtentSource::AUTO;
    else if(extent_source == "as_swapchain") m_extent_source = ExtentSource::AS_SWAPCHAIN;
    else if(extent_source == "exact") m_extent_source = ExtentSource::EXACT;
    pugi::xml_node width_node = extent_node.child("Width");
    if(width_node && m_extent_source == ExtentSource::EXACT) {
        std::string extent_width_str = width_node.text().as_string();
        m_extent_2D.width = static_cast<uint32_t>(std::stoi(extent_width_str));
        m_extent_3D.width = m_extent_2D.width;
        m_extent_2D.height = static_cast<uint32_t>(std::stoi(extent_node.child("Height").text().as_string()));
        m_extent_3D.height = m_extent_2D.height;

        pugi::xml_node depth_node = extent_node.child("Depth");
        if(depth_node) {
            m_extent_3D.depth = static_cast<uint32_t>(std::stoi(depth_node.text().as_string()));
        }
    }
    else if(m_extent_source == ExtentSource::AS_SWAPCHAIN) {
        VkExtent2D swapchain_extent = VulkanSwapChain::chooseSwapExtent(window->GetWindow(), swapchain_support_details.capabilities);
        m_extent_2D.width = swapchain_extent.width;
        m_extent_3D.width = m_extent_2D.width;

        m_extent_2D.height = swapchain_extent.height;
        m_extent_3D.height = m_extent_2D.height;

        m_extent_3D.depth = 1;
    }
    else {
        m_extent_2D = {1u, 1u};
        m_extent_3D = {1u, 1u, 1u};
    }
    m_aspect = ((float)m_extent_2D.width) / ((float)m_extent_2D.height);
    
    pugi::xml_node mip_node = format_data.child("MipLevels");
    std::string mip_str = mip_node.text().as_string();
    m_mip_levels = 1u;
    if(mip_str == "auto"s) {
        m_mip_levels = static_cast<uint32_t>(std::floor(std::log2(std::max(m_extent_2D.width, m_extent_2D.height))));
    }

    m_array_layers = format_data.child("ArrayLayers").text().as_uint();

    m_samples = getSampleCountFlag(format_data.child("Samples").text().as_string());
    m_tiling = getImageTiling(format_data.child("Tiling").text().as_string());
    
    for (pugi::xml_node flag_node = format_data.child("ImageUsageFlags").first_child(); flag_node; flag_node = flag_node.next_sibling()) {
        m_usage |= getImageUsageFlag(flag_node.text().as_string());
    }

    for (pugi::xml_node flag_node = format_data.child("FormatProperties").first_child(); flag_node; flag_node = flag_node.next_sibling()) {
        m_feature_flags |= getFormatFeatureFlag(flag_node.text().as_string());
    }

    if(auto_config) {
        m_format = device->findSupportedFormat(getFormatCandidates(format_data.child("Candidates")), m_tiling, m_feature_flags, m_usage, m_extent_2D, m_mip_levels, m_samples, m_image_flags, swapchain_support_details.is_native_swapchain_BGR);
    }
    else {
        m_format = getFormat(format_data.child("Candidates").first_child().text().as_string());
    }

    m_color_space = getColorSpace(format_data.child("ColorSpace").text().as_string());

    static std::vector<uint32_t> families = device->getCommandManager()->getQueueFamilyIndices().getIndices();
    m_images_sharing_mode = device->getCommandManager()->getBufferSharingMode();
    m_queue_family_index_count = static_cast<uint32_t>(families.size());
    m_pQueue_family_indices = families.data();
}

const std::string& FormatConfig::getName() const {
    return m_name;
}

VkImageCreateFlags FormatConfig::getImageFlags() const {
    return m_image_flags;
}

bool FormatConfig::hasImageFlags(VkImageCreateFlags flags) const {
    return (m_image_flags && flags) == flags;
}

void FormatConfig::setImageFlags(VkImageCreateFlags flags) {
    m_image_flags = flags;
}

void FormatConfig::addImageFlags(VkImageCreateFlags flags) {
    m_image_flags |= flags;
}

VkImageType FormatConfig::getVkImageType() const {
    return m_image_type;
}

void FormatConfig::setVkImageType(VkImageType type) {
    m_image_type = type;
}

VkExtent2D FormatConfig::getExtent2D() const {
    return m_extent_2D;
}

void FormatConfig::setExtent2D(VkExtent2D extent) {
    m_extent_2D = extent;
    m_extent_3D.width = m_extent_2D.width;
    m_extent_3D.height = m_extent_2D.height;

    m_aspect = ((float)m_extent_2D.width) / ((float)m_extent_2D.height);
}

VkExtent3D FormatConfig::getExtent3D() const {
    return m_extent_3D;
}

void FormatConfig::setExtent3D(VkExtent3D extent) {
    m_extent_3D = extent;
    m_extent_2D.width = m_extent_3D.width;
    m_extent_2D.height = m_extent_3D.height;

    m_aspect = ((float)m_extent_2D.width) / ((float)m_extent_2D.height);
}

uint32_t FormatConfig::getMipLevels() const {
    return m_mip_levels;
}

void FormatConfig::setMipLevels(uint32_t lvl) {
    m_mip_levels = lvl;
}

uint32_t FormatConfig::getArrayLayers() const {
    return m_array_layers;
}

void FormatConfig::setArrayLayers(uint32_t layers) {
    m_array_layers = layers;
}

VkSampleCountFlagBits FormatConfig::getSamplesCount() const {
    return m_samples;
}

void FormatConfig::setSamplesCount(VkSampleCountFlagBits samples) {
    m_samples = samples;
}

VkImageTiling FormatConfig::getTiling() const {
    return m_tiling;
}

void FormatConfig::setTiling(VkImageTiling tiling) {
    m_tiling = tiling;
}

VkImageUsageFlags FormatConfig::getImageUsage() const {
    return m_usage;
}

bool FormatConfig::hasImageUsage(VkImageUsageFlags usage) const {
    return (m_usage && usage) == usage;
}

void FormatConfig::setImageUsage(VkImageUsageFlags usage) {
    m_usage = usage;
}

void FormatConfig::addImageUsage(VkImageUsageFlags usage) {
    m_usage |= usage;
}

VkFormatFeatureFlags FormatConfig::getFeatureFlags() const {
    return m_feature_flags;
}

bool FormatConfig::hasFeatureFlags(VkFormatFeatureFlags flags) const {
    return (m_feature_flags && flags) == flags;
}

void FormatConfig::setFeatureFlags(VkFormatFeatureFlags flags) {
    m_feature_flags = flags;
}

void FormatConfig::addFeatureFlags(VkFormatFeatureFlags flags) {
    m_feature_flags |= flags;
}

VkFormat FormatConfig::getVkFormat() const {
    return m_format;
}

void FormatConfig::setVkFormat(VkFormat format) {
    m_format = format;
}

float FormatConfig::getAspect() const {
    return m_aspect;
}

VkColorSpaceKHR FormatConfig::getVkColorSpace() const {
    return m_color_space;
}

void FormatConfig::setVkColorSpace(VkColorSpaceKHR color_space) {
    m_color_space = color_space;
}

VkSurfaceFormatKHR FormatConfig::getVkSurfaceFormat() const {
    return {m_format, m_color_space};
}

VkSharingMode FormatConfig::getImagesSharingMode() const {
    return m_images_sharing_mode;
}

uint32_t FormatConfig::getQueueFamilyIndexCount() const {
    return m_queue_family_index_count;
}

const uint32_t* FormatConfig::getQueueFamilyIndicesPtr() const {
    return m_pQueue_family_indices;
}

std::shared_ptr<FormatConfig> FormatConfig::makeInstance(std::string name, VkExtent2D extent) const {
    std::shared_ptr<FormatConfig> instance_ptr = std::make_shared<FormatConfig>();
    *instance_ptr = *this;

    instance_ptr->m_name = name;
    if(m_extent_source == ExtentSource::AUTO) {
        instance_ptr->m_extent_2D = extent;
    }

    return instance_ptr;
}