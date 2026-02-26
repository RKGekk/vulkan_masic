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
    pugi::xml_node width_node = extent_node.child("Width");
    if(width_node && extent_source == "exact") {
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
    else if(extent_source == "as_swapchain") {
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
        m_format = device->findSupportedFormat(getFormatCandidates(format_data.child("Candidates")), m_tiling, m_feature_flags, m_usage, m_extent_2D, m_mip_levels, m_samples, m_image_flags);
    }
    else {
        m_format = getFormat(format_data.child("Candidates").first_child().text().as_string());
    }

    m_color_space = getColorSpace(format_data.child("ColorSpace").text().as_string());
}