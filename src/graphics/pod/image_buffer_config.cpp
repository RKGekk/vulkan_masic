#include "image_buffer_config.h"

#include "../api/vulkan_device.h"
#include "../api/vulkan_sampler.h"
#include "../api/vulkan_format_manager.h"
#include "format_config.h"

#include "../../tools/string_tools.h"

bool ImageBufferConfig::init(const std::shared_ptr<VulkanDevice>& device, std::string name, const pugi::xml_node& image_buffer_data, const std::shared_ptr<VulkanFormatManager>& format_manager) {
    using namespace std::literals;

    m_image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

    static std::vector<uint32_t> families = device->getCommandManager()->getQueueFamilyIndices().getIndices();
    m_image_info.sharingMode = device->getCommandManager()->getBufferSharingMode();
    m_image_info.queueFamilyIndexCount = static_cast<uint32_t>(families.size());
    m_image_info.pQueueFamilyIndices = families.data();

    m_name = std::move(name);

    pugi::xml_node image_buffer_flags_node = image_buffer_data.child("Falgs");
    if(image_buffer_flags_node) {
        for (pugi::xml_node flag_node = image_buffer_flags_node.first_child(); flag_node; flag_node = flag_node.next_sibling()) {
		    m_image_info.flags |= getVkImageCreateFlag(flag_node.text().as_string());
		}
    }

    m_format = format_manager->getFormat(image_buffer_data.child("FormatName").text().as_string());
    m_image_info.imageType = m_format->getVkImageType();
    m_image_info.format = m_format->getVkFormat();
    m_image_info.extent = m_format->getExtent3D();
    m_image_info.mipLevels = m_format->getMipLevels();
    m_image_info.arrayLayers = m_format->getArrayLayers();
    m_image_info.samples = m_format->getSamplesCount();
    m_image_info.tiling = m_format->getTiling();
    m_image_info.usage = m_format->getImageUsage();
    
    m_image_info.initialLayout = getImageLayout(image_buffer_data.child("CreationLayout").text().as_string());
    m_after_init_layout = getImageLayout(image_buffer_data.child("AfterInitLayout").text().as_string());

    pugi::xml_node memory_properties_node = image_buffer_data.child("MemoryProperties");
    m_memory_properties_by_memory_requirements = memory_properties_node.attribute("get_by_memory_requirements").as_bool();
    for (pugi::xml_node flag_node = memory_properties_node.first_child(); flag_node; flag_node = flag_node.next_sibling()) {
        m_memory_properties |= getMemoryPropertyFlag(flag_node.text().as_string());
    }

    pugi::xml_node samplers_node = image_buffer_data.child("Samplers");
    for (pugi::xml_node sampler_node = samplers_node.first_child(); sampler_node; sampler_node = sampler_node.next_sibling()) {
        std::string sampler_name_str = sampler_node.attribute("name").as_string();
        VkSamplerCreateInfo sampler_info = getSamplerCreateInfo(sampler_node);
        std::shared_ptr<VulkanSampler> sampler = std::make_shared<VulkanSampler>(device, std::move(sampler_name_str));
        sampler->init(sampler_info);
        m_samplers.push_back(std::move(sampler));
    }

    pugi::xml_node views_node = image_buffer_data.child("Views");
    for (pugi::xml_node view_node = views_node.first_child(); view_node; view_node = view_node.next_sibling()) {
        std::shared_ptr<ImageBufferViewConfig> view_info_ptr = std::make_shared<ImageBufferViewConfig>();

        std::string view_name_str = view_node.attribute("name").as_string();
        
        view_info_ptr->image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

        pugi::xml_node view_flags_node = view_node.child("Flags");
        for (pugi::xml_node flag_node = view_flags_node.first_child(); flag_node; flag_node = flag_node.next_sibling()) {
            view_info_ptr->image_view_info.flags |= getImageViewCreateFlag(flag_node.text().as_string());
        }

        view_info_ptr->image_view_info.image = VK_NULL_HANDLE;
        view_info_ptr->image_view_info.viewType = getImageViewType(view_node.child("ViewType").text().as_string());
        
        view_info_ptr->format = format_manager->getFormat(view_node.child("FormatName").text().as_string());
        view_info_ptr->image_view_info.format = view_info_ptr->format->getVkFormat();

        view_info_ptr->image_view_info.components = getComponentMapping(view_node.child("Components"));

        for (pugi::xml_node flag_node = view_node.child("SubresourceRange").child("AspectFlags").first_child(); flag_node; flag_node = flag_node.next_sibling()) {
            view_info_ptr->image_view_info.subresourceRange.aspectMask |= getImageAspectFlag(flag_node.text().as_string());
        }

        view_info_ptr->image_view_info.subresourceRange.baseMipLevel = view_node.child("SubresourceRange").child("BaseMipLevel").text().as_uint();
        pugi::xml_node level_node = view_node.child("SubresourceRange").child("LevelCount");
        std::string level_str = level_node.text().as_string();
        if(level_str == "auto") {
            view_info_ptr->image_view_info.subresourceRange.levelCount = view_info_ptr->format->getMipLevels();
        }
        else {
            view_info_ptr->image_view_info.subresourceRange.levelCount = 1u;
        }

        view_info_ptr->image_view_info.subresourceRange.baseArrayLayer = view_node.child("SubresourceRange").child("BaseArrayLayer").text().as_uint();
        view_info_ptr->image_view_info.subresourceRange.layerCount = view_info_ptr->format->getArrayLayers();

        m_image_view_info_map[view_name_str] = std::move(view_info_ptr);
    }

    return true;
}
    
const VkImageCreateInfo& ImageBufferConfig::getImageInfo() const {
    return m_image_info;
}

VkImageLayout ImageBufferConfig::getAfterInitLayout() const {
    return m_after_init_layout;
}

const std::vector<std::shared_ptr<VulkanSampler>>& ImageBufferConfig::getSamplers() const {
    return m_samplers;
}

void ImageBufferConfig::setSampler(std::shared_ptr<VulkanSampler> sampler) {
    m_samplers.insert(m_samplers.begin(), std::move(sampler));
}

const std::shared_ptr<VulkanSampler>& ImageBufferConfig::getSampler() const {
    return m_samplers.front();
}

const std::shared_ptr<FormatConfig>& ImageBufferConfig::getFormat() const {
    return m_format;
}

const std::unordered_map<std::string, std::shared_ptr<ImageBufferViewConfig>>& ImageBufferConfig::getViewInfoMap() const {
    return m_image_view_info_map;
}

VkMemoryPropertyFlags ImageBufferConfig::getMemoryProperties() const {
    return m_memory_properties;
}

void ImageBufferConfig::setMemoryProperties(VkMemoryPropertyFlags props) {
    m_memory_properties = props;
}

bool ImageBufferConfig::isMemoryPropertiesByMemoryRequirements() const {
    return m_memory_properties_by_memory_requirements;
}

std::shared_ptr<ImageBufferConfig> ImageBufferConfig::makeInstance(std::string image_instance_name, VkExtent2D extent) const {
    using namespace std::literals;

    std::shared_ptr<ImageBufferConfig> instance_ptr = std::make_shared<ImageBufferConfig>();

    instance_ptr->m_name = std::move(image_instance_name);
    instance_ptr->m_image_info = m_image_info;
    instance_ptr->m_after_init_layout = m_after_init_layout;
    instance_ptr->m_format = m_format->makeInstance(instance_ptr->m_name + "_format", extent);

    for(const auto&[view_type_name, view_cfg_ptr] : m_image_view_info_map) {
        instance_ptr->m_image_view_info_map[view_type_name] = std::make_shared<ImageBufferViewConfig>();
        instance_ptr->m_image_view_info_map[view_type_name]->image_view_info = view_cfg_ptr->image_view_info;
        instance_ptr->m_image_view_info_map[view_type_name]->format = view_cfg_ptr->format->makeInstance(view_type_name + "_view_format", extent);
    }

    instance_ptr->m_memory_properties = m_memory_properties;
    instance_ptr->m_memory_properties_by_memory_requirements = m_memory_properties_by_memory_requirements;
    if(instance_ptr->m_format->getExtentSource() == FormatConfig::ExtentSource::AUTO) {
        instance_ptr->m_image_info.extent = {extent.width, extent.height, 1};
        instance_ptr->m_image_info.mipLevels = instance_ptr->m_format->getMipLevels();
    }
    instance_ptr->m_samplers = m_samplers;

    return instance_ptr;
}