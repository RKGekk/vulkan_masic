#include "image_buffer_config.h"

#include "../api/vulkan_device.h"
#include "../api/vulkan_sampler.h"

#include "../../tools/string_tools.h"

bool ImageBufferConfig::init(const std::shared_ptr<VulkanDevice>& device, const std::string& rg_file_path) {
    pugi::xml_document xml_doc;
	pugi::xml_parse_result parse_res = xml_doc.load_file(rg_file_path.c_str());
	if (!parse_res) { return false;	}

	pugi::xml_node root_node = xml_doc.root();
	if (!root_node) { return false; }
	root_node = root_node.child("RenderGraph");

    pugi::xml_node type_of_resources_node = root_node.child("TypeOfResources");
	if (!type_of_resources_node) return false;
    
	for (pugi::xml_node resource_type_node = type_of_resources_node.first_child(); resource_type_node; resource_type_node = resource_type_node.next_sibling()) {
        pugi::xml_node image_buffer_node = resource_type_node.child("ImageBuffer");
	    if (!image_buffer_node) continue;
	    return init(device, image_buffer_node);
	}

    return true;
}

bool ImageBufferConfig::init(const std::shared_ptr<VulkanDevice>& device, const pugi::xml_node& image_buffer_data) {
    using namespace std::literals;

    m_image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

    static std::vector<uint32_t> families = device->getCommandManager()->getQueueFamilyIndices().getIndices();
    m_image_info.sharingMode = device->getCommandManager()->getBufferSharingMode();
    m_image_info.queueFamilyIndexCount = static_cast<uint32_t>(families.size());
    m_image_info.pQueueFamilyIndices = families.data();

    m_name = image_buffer_data.attribute("name").as_string();

    pugi::xml_node image_buffer_flags_node = image_buffer_data.child("Falgs");
    if(image_buffer_flags_node) {
        for (pugi::xml_node flag_node = image_buffer_flags_node.first_child(); flag_node; flag_node = flag_node.next_sibling()) {
		    m_image_info.flags |= getVkImageCreateFlag(flag_node.text().as_string());
		}
    }

    m_image_info.imageType = getImageType(image_buffer_data.child("ImageType").text().as_string());
    m_image_info.format = getFormat(image_buffer_data.child("Format").text().as_string());

    pugi::xml_node extent_node = image_buffer_data.child("Extent");
    std::string extent_width_str = extent_node.child("Width").text().as_string();
    if(extent_width_str.size()) {
        m_has_extent = true;
        m_image_info.extent.width = static_cast<uint32_t>(std::stoi(extent_width_str));
        m_image_info.extent.height = static_cast<uint32_t>(std::stoi(extent_node.child("Height").text().as_string()));
        m_image_info.extent.depth = static_cast<uint32_t>(std::stoi(extent_node.child("Depth").text().as_string()));
    }
    else {
        m_has_extent = false;
        m_image_info.extent = {1u, 1u, 1u};
    }
    m_extent_source = extent_node.attribute("source").as_string();
    
    pugi::xml_node mip_node = image_buffer_data.child("MipLevels");
    std::string mip_str = mip_node.text().as_string();
    m_image_info.mipLevels = 1u;
    if(mip_str == "auto"s) {
        m_auto_mip_levels = true;
    }
    else {
        m_auto_mip_levels = false;
    }

    m_image_info.arrayLayers = image_buffer_data.child("ArrayLayers").text().as_uint();
    m_image_info.samples = getSampleCountFlag(image_buffer_data.child("Samples").text().as_string());
    m_image_info.tiling = getImageTiling(image_buffer_data.child("Tiling").text().as_string());
    
    for (pugi::xml_node flag_node = image_buffer_data.child("ImageUsageFlags").first_child(); flag_node; flag_node = flag_node.next_sibling()) {
        m_image_info.usage |= getImageUsageFlag(flag_node.text().as_string());
    }

    m_image_info.initialLayout = getImageLayout(image_buffer_data.child("InitialLayout").text().as_string());

    for (pugi::xml_node flag_node = image_buffer_data.child("MemoryProperties").first_child(); flag_node; flag_node = flag_node.next_sibling()) {
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
        std::string view_name_str = view_node.attribute("name").as_string();

        VkImageViewCreateInfo view_info{};
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

        pugi::xml_node view_flags_node = view_node.child("Flags");
        for (pugi::xml_node flag_node = view_flags_node.first_child(); flag_node; flag_node = flag_node.next_sibling()) {
            view_info.flags |= getImageViewCreateFlag(flag_node.text().as_string());
        }

        view_info.image = VK_NULL_HANDLE;
        view_info.viewType = getImageViewType(view_node.child("ViewType").text().as_string());
        view_info.format = getFormat(view_node.child("Format").text().as_string());
        view_info.components = getComponentMapping(view_node.child("Components"));

        for (pugi::xml_node flag_node = view_node.child("SubresourceRange").child("AspectFlags").first_child(); flag_node; flag_node = flag_node.next_sibling()) {
            view_info.subresourceRange.aspectMask |= getImageAspectFlag(flag_node.text().as_string());
        }

        view_info.subresourceRange.baseMipLevel = view_node.child("SubresourceRange").child("BaseMipLevel").text().as_uint();

        pugi::xml_node level_node = view_node.child("LevelCount");
        std::string level_str = level_node.text().as_string();
        view_info.subresourceRange.levelCount = 1u;
        if(level_str == "auto"s) {
            m_auto_subresource_mip_levels_map[view_name_str] = true;
        }
        else {
            m_auto_subresource_mip_levels_map[view_name_str] = false;
        }

        view_info.subresourceRange.baseArrayLayer = view_node.child("SubresourceRange").child("BaseArrayLayer").text().as_uint();
        view_info.subresourceRange.layerCount = view_node.child("SubresourceRange").child("LayerCount").text().as_uint();

        m_image_view_info_map[view_name_str] = view_info;
    }
}
    
