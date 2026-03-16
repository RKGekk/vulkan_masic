#include "vulkan_swapchain.h"

#include "vulkan_device.h"
#include "vulkan_resources_manager.h"
#include "../vulkan_renderer.h"
#include "../../window_surface.h"
#include "vulkan_format_manager.h"
#include "vulkan_semaphores_manager.h"
#include "vulkan_fence_manager.h"
#include "vulkan_image_buffer.h"
#include "../pod/format_config.h"
#include "../../tools/string_tools.h"
#include "../../application.h"

bool VulkanSwapChain::init(std::shared_ptr<VulkanDevice> device, std::shared_ptr<WindowSurface> window, const std::string& rg_file_path) {
    using namespace std::literals;

    m_window = std::move(window);
    m_device = std::move(device);

    pugi::xml_document xml_doc;
	pugi::xml_parse_result parse_res = xml_doc.load_file(rg_file_path.c_str());
	if (!parse_res) { return false;	}

	pugi::xml_node root_node = xml_doc.root();
	if (!root_node) { return false; }
	root_node = root_node.child("RenderGraph");

    pugi::xml_node swapchain_node = root_node.child("Swapchain");
	if (!swapchain_node) return false;

    std::string surface_format_name = swapchain_node.child("SurfaceFormatName").text().as_string();
    m_format_config = Application::GetRenderer().getFormatManager()->getFormat(surface_format_name);
    
    m_swapchain_create_info = VkSwapchainCreateInfoKHR{};
    m_swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    m_swapchain_create_info.surface = m_device->getSurface();

    m_swapchain_support_details = querySwapChainSupport(m_device->getDeviceAbilities().physical_device, m_swapchain_create_info.surface);

    pugi::xml_node min_images_count_node = swapchain_node.child("MinImageCount");
    bool auto_min_images = min_images_count_node.attribute("auto").as_bool();
    uint32_t min_images = min_images_count_node.text().as_uint(0);
    uint32_t image_count = m_swapchain_support_details.capabilities.minImageCount + 1u;
    if(!auto_min_images) {
        image_count = min_images;
    }
    if(m_swapchain_support_details.capabilities.maxImageCount > 0u && image_count > m_swapchain_support_details.capabilities.maxImageCount){
        image_count = m_swapchain_support_details.capabilities.maxImageCount;
    }
    m_swapchain_create_info.minImageCount = image_count;

    m_swapchain_create_info.imageFormat = m_format_config->getVkFormat();
    m_swapchain_create_info.imageColorSpace = m_format_config->getVkColorSpace();
    m_swapchain_create_info.imageExtent = m_format_config->getExtent2D();
    m_swapchain_create_info.imageArrayLayers = m_format_config->getArrayLayers();
    m_swapchain_create_info.imageUsage = m_format_config->getImageUsage();
    m_swapchain_create_info.imageSharingMode = m_format_config->getImagesSharingMode();
    m_swapchain_create_info.queueFamilyIndexCount = m_format_config->getQueueFamilyIndexCount();
    m_swapchain_create_info.pQueueFamilyIndices = m_format_config->getQueueFamilyIndicesPtr();
    m_swapchain_create_info.preTransform = getSurfaceTransformFlag(swapchain_node.child("PreTransform").text().as_string());
    m_swapchain_create_info.compositeAlpha = getCompositeAlphaFlag(swapchain_node.child("CompositeAlpha").text().as_string());

    std::string present_mode_str = swapchain_node.child("RecommendPresentMode").text().as_string();
    if(present_mode_str != "auto"s) {
        m_swapchain_create_info.presentMode = getPresentMode(present_mode_str);
    }
    else {
        m_swapchain_create_info.presentMode = chooseSwapPresentMode(m_swapchain_support_details.present_modes);
    }

    m_swapchain_create_info.clipped = swapchain_node.child("Clipped").text().as_bool();
    m_swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;
    
    VkResult result = vkCreateSwapchainKHR(m_device->getDevice(), &m_swapchain_create_info, nullptr, &m_swapchain);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    result = vkGetSwapchainImagesKHR(m_device->getDevice(), m_swapchain, &m_max_frames, nullptr);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to get swapchain images!");
    }
    
    std::vector<VkImage> swapchain_images(m_max_frames);
    result = vkGetSwapchainImagesKHR(m_device->getDevice(), m_swapchain, &m_max_frames, swapchain_images.data());
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to get swapchain images!");
    }

    std::string resource_type_name = swapchain_node.child("GeneratedResourceType").text().as_string();
    
    size_t sz = swapchain_images.size();
    m_swapchain_images.resize(sz);
    for(size_t i = 0u; i < sz; ++i) {
        m_swapchain_images[i] = Application::GetRenderer().getResourcesManager()->create_image(swapchain_images[i], "swap_chain_"s + std::to_string(i), resource_type_name);
    }

    m_image_available_sem.resize(m_max_frames);
    m_image_available_fen.resize(m_max_frames);

    VkSemaphoreCreateInfo image_available_sema_info{};
    image_available_sema_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo in_flight_fen_info{};
    in_flight_fen_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    in_flight_fen_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for(size_t i = 0u; i < m_max_frames; ++i) {
        m_image_available_sem[i] = Application::GetRenderer().getSemaphoreManager()->getSemaphore();
        m_image_available_fen[i] = Application::GetRenderer().getFenceManager()->getFence();
    }

    return true;
}

void VulkanSwapChain::destroy() {}

void VulkanSwapChain::recreate() {
    destroy();
}

const SwapchainSupportDetails& VulkanSwapChain::getSwapchainSupportDetails() const {
    return m_swapchain_support_details;
}

const VkSwapchainCreateInfoKHR& VulkanSwapChain::getSwapchainParams() const {
    return m_swapchain_create_info;
}

int VulkanSwapChain::getMaxFrames() const {
    return m_max_frames;
}

uint32_t VulkanSwapChain::getCurrentFrame() const {
    return m_current_frame;
}

const uint32_t* VulkanSwapChain::getCurrentFramePtr() const {
    return &m_current_frame;
}

uint32_t VulkanSwapChain::getCurrentSync() const {
    return m_current_sync;
}

uint32_t VulkanSwapChain::fetchNextSync() const {
    return (m_current_sync + 1u) % m_max_frames;
}

bool VulkanSwapChain::setNextFrame(VkFence wait_to) {
    if (wait_to) {
        vkWaitForFences(m_device->getDevice(), 1u, &wait_to, VK_TRUE, UINT64_MAX);
    }
    m_current_sync = fetchNextSync();
    vkWaitForFences(m_device->getDevice(), 1u, getImageAvailableFencePtr(), VK_TRUE, UINT64_MAX);
    vkResetFences(m_device->getDevice(), 1u, getImageAvailableFencePtr());
    VkResult result = vkAcquireNextImageKHR(m_device->getDevice(), m_swapchain, UINT64_MAX, getImageAvailableSemaphore(), getImageAvailableFence(), &m_current_frame);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        return false;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }
    return true;
}

VkSemaphore VulkanSwapChain::getImageAvailableSemaphore(uint32_t image_index) {
    if(uint32_t image_index = CURRENT_SYNC) {
        return m_image_available_sem[m_current_sync];
    }
    else {
        return m_image_available_sem[image_index];
    }
}

VkSemaphore* VulkanSwapChain::getImageAvailableSemaphorePtr(uint32_t image_index) {
    if(uint32_t image_index = CURRENT_SYNC) {
        return &m_image_available_sem[m_current_sync];
    }
    else {
        return &m_image_available_sem[image_index];
    }
}

VkFence VulkanSwapChain::getImageAvailableFence(uint32_t image_index) {
    if(uint32_t image_index = CURRENT_SYNC) {
        return m_image_available_fen[m_current_sync];
    }
    else {
        return m_image_available_fen[image_index];
    }
}

VkFence* VulkanSwapChain::getImageAvailableFencePtr(uint32_t image_index) {
    if(uint32_t image_index = CURRENT_SYNC) {
        return &m_image_available_fen[m_current_sync];
    }
    else {
        return &m_image_available_fen[image_index];
    }
}

const std::shared_ptr<WindowSurface>& VulkanSwapChain::getWindow() const {
    return m_window;
}

VkSwapchainKHR VulkanSwapChain::getSwapchain() const {
    return m_swapchain;
}

const std::vector<std::shared_ptr<VulkanImageBuffer>>& VulkanSwapChain::getSwapchainImages() const {
    return m_swapchain_images;
}

const std::shared_ptr<FormatConfig>& VulkanSwapChain::getFormatConfig() const {
    return m_format_config;
}

VkSurfaceKHR VulkanSwapChain::createSurface(VkInstance vk_instance, GLFWwindow* glfw_window_ptr) {
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkResult result = glfwCreateWindowSurface(vk_instance, glfw_window_ptr, nullptr, &surface);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
    return surface;
}

SwapchainSupportDetails VulkanSwapChain::querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
    SwapchainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
    
    uint32_t format_count = 0u;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);
    if(format_count) {
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, details.formats.data());
    }
    
    uint32_t present_mode_count = 0u;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, nullptr);
    if(present_mode_count) {
        details.present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, details.present_modes.data());
    }

    details.is_native_swapchain_BGR = isNativeSwapChainBGR(details.formats);
    
    return details;
}

VkPresentModeKHR VulkanSwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes){
    for (const auto& available_present_mode : available_present_modes) {
        if(available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return available_present_mode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

bool VulkanSwapChain::isNativeSwapChainBGR(const std::vector<VkSurfaceFormatKHR>& formats) {
    for (const VkSurfaceFormatKHR& fmt : formats) {
        // The preferred format should be the one which is closer to the beginning of the formats
        // container. If BGR is encountered earlier, it should be picked as the format of choice. If RGB
        // happens to be earlier, take it.
        if (fmt.format == VK_FORMAT_R8G8B8A8_UNORM || fmt.format == VK_FORMAT_R8G8B8A8_SRGB || fmt.format == VK_FORMAT_A2R10G10B10_UNORM_PACK32) {
            return false;
        }
        if (fmt.format == VK_FORMAT_B8G8R8A8_UNORM || fmt.format == VK_FORMAT_B8G8R8A8_SRGB || fmt.format == VK_FORMAT_A2B10G10R10_UNORM_PACK32) {
            return true;
        }
    }
    return false;
};

VkExtent2D VulkanSwapChain::chooseSwapExtent(GLFWwindow* m_window, const VkSurfaceCapabilitiesKHR& capabilities) {
    if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }
    else {
        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(m_window, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(m_window, &width, &height);
            glfwWaitEvents();
        }
        
        VkExtent2D actual_extent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };
        
        actual_extent.width = std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actual_extent.height = std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        
        return actual_extent;
    }
}