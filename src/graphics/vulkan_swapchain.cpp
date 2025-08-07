#include "vulkan_swapchain.h"
#include "vulkan_renderer.h"

bool VulkanSwapChain::init(std::shared_ptr<VulkanDevice> device, VkSurfaceKHR surface, GLFWwindow* window) {
    m_window = window;
    m_device = std::move(device);
    
    m_surface = surface;
    m_swapchain_support_details = querySwapChainSupport(m_device->getDeviceAbilities().physical_device, m_surface);

    m_swapchain_params.surface_format = chooseSwapSurfaceFormat(m_swapchain_support_details.formats);
    m_swapchain_params.present_mode = chooseSwapPresentMode(m_swapchain_support_details.present_modes);
    m_swapchain_params.extent = chooseSwapExtent(m_swapchain_support_details.capabilities);

    if(m_device->getQueueFamilyIndices().graphics_family != m_device->getQueueFamilyIndices().present_family) {
        m_swapchain_params.images_sharing_mode = VK_SHARING_MODE_CONCURRENT;
    }
    else {
        m_swapchain_params.images_sharing_mode = VK_SHARING_MODE_EXCLUSIVE;
    }

    m_swapchain = createSwapchain(m_device->getDevice(), m_surface, m_swapchain_params, m_swapchain_support_details, m_device->getQueueFamilyIndices());
    VkResult result = vkGetSwapchainImagesKHR(m_device->getDevice(), m_swapchain, &m_max_frames, nullptr);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to get swapchain images count!");
    }
    m_swapchain_images = getSwapchainBuffers(m_device->getDevice(), m_swapchain, m_swapchain_params.surface_format);

    return true;
}

void VulkanSwapChain::destroy() {
    size_t sz = m_swapchain_images.size();
    for(size_t i = 0u; i < sz; ++i) {
        vkDestroyImageView(m_device->getDevice(), m_swapchain_images[i].view, nullptr);
    }
    vkDestroySwapchainKHR(m_device->getDevice(), m_swapchain, nullptr);
}

void VulkanSwapChain::recreate() {
    destroy();

    m_swapchain_support_details = querySwapChainSupport(m_device->getDeviceAbilities().physical_device, m_surface);

    m_swapchain_params.surface_format = chooseSwapSurfaceFormat(m_swapchain_support_details.formats);
    m_swapchain_params.present_mode = chooseSwapPresentMode(m_swapchain_support_details.present_modes);
    m_swapchain_params.extent = chooseSwapExtent(m_swapchain_support_details.capabilities);

    if(m_device->getQueueFamilyIndices().graphics_family != m_device->getQueueFamilyIndices().present_family) {
        m_swapchain_params.images_sharing_mode = VK_SHARING_MODE_CONCURRENT;
    }
    else {
        m_swapchain_params.images_sharing_mode = VK_SHARING_MODE_EXCLUSIVE;
    }

    m_swapchain = createSwapchain(m_device->getDevice(), m_surface, m_swapchain_params, m_swapchain_support_details, m_device->getQueueFamilyIndices());
    m_swapchain_images = getSwapchainBuffers(m_device->getDevice(), m_swapchain, m_swapchain_params.surface_format);
}

const SwapchainSupportDetails& VulkanSwapChain::getSwapchainSupportDetails() const {
    return m_swapchain_support_details;
}

const SwapchainParams& VulkanSwapChain::getSwapchainParams() const {
    return m_swapchain_params;
}

int VulkanSwapChain::getMaxFrames() const {
    return m_max_frames;
}

uint32_t VulkanSwapChain::getCurrentFrame() const {
    return m_current_frame;
}

void VulkanSwapChain::setNextFrame() {
    m_current_frame = (m_current_frame + 1u) % m_max_frames;
}

VkSurfaceKHR VulkanSwapChain::getSurface() const {
    return m_surface;
}

GLFWwindow* VulkanSwapChain::getWindow() const {
    return m_window;
}

VkSwapchainKHR VulkanSwapChain::getSwapchain() const {
    return m_swapchain;
}

const std::vector<SwapChainBuffer>& VulkanSwapChain::getSwapchainImages() {
    return m_swapchain_images;
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
    
    return details;
}

VkSurfaceFormatKHR VulkanSwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats) {
    for(const auto& available_format : available_formats) {
        if(available_format.format == VK_FORMAT_B8G8R8A8_SRGB && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return available_format;
        }
    }
    return available_formats[0];
}

VkPresentModeKHR VulkanSwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes){
    for (const auto& available_present_mode : available_present_modes) {
        if(available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return available_present_mode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkSwapchainKHR VulkanSwapChain::createSwapchain(VkDevice logical_device, VkSurfaceKHR surface, const SwapchainParams& swapchain_params, const SwapchainSupportDetails& swapchain_support_details, const QueueFamilyIndices& queue_family_indices) {
    uint32_t image_count = swapchain_support_details.capabilities.minImageCount + 1u;
    if(swapchain_support_details.capabilities.maxImageCount > 0u && image_count > swapchain_support_details.capabilities.maxImageCount){
        image_count = swapchain_support_details.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapchain_create_info{};
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.surface = surface;
    swapchain_create_info.minImageCount = image_count;
    swapchain_create_info.imageFormat = swapchain_params.surface_format.format;
    swapchain_create_info.imageColorSpace = swapchain_params.surface_format.colorSpace;
    swapchain_create_info.imageExtent = swapchain_params.extent;
    swapchain_create_info.imageArrayLayers = 1u;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    
    std::vector<uint32_t> family_indices = queue_family_indices.getIndices();
    if(swapchain_params.images_sharing_mode == VK_SHARING_MODE_CONCURRENT) {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_create_info.queueFamilyIndexCount = static_cast<uint32_t>(family_indices.size());
        swapchain_create_info.pQueueFamilyIndices = family_indices.data();
    }
    else {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_create_info.queueFamilyIndexCount = 0u;
        swapchain_create_info.pQueueFamilyIndices = nullptr;
    }
    
    swapchain_create_info.preTransform = swapchain_support_details.capabilities.currentTransform;
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_create_info.presentMode = swapchain_params.present_mode;
    swapchain_create_info.clipped = VK_TRUE;
    swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;
    
    VkSwapchainKHR swap_chain = VK_NULL_HANDLE;
    VkResult result = vkCreateSwapchainKHR(logical_device, &swapchain_create_info, nullptr, &swap_chain);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }
    
    return swap_chain;
}

std::vector<VkImage> VulkanSwapChain::getSwapchainImages(VkDevice device, VkSwapchainKHR swapchain) {
    uint32_t swapchain_image_count = 0u;
    VkResult result = vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, nullptr);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to get swapchain images!");
    }
    std::vector<VkImage> swapchain_images(swapchain_image_count);
    result = vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, swapchain_images.data());
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to get swapchain images!");
    }
    
    return swapchain_images; 
}

std::vector<SwapChainBuffer> VulkanSwapChain::getSwapchainBuffers(VkDevice device, VkSwapchainKHR swapchain, VkSurfaceFormatKHR surface_format) {
    std::vector<VkImage> swapchain_images = getSwapchainImages(device, swapchain);
    std::vector<VkImageView> swapchain_views = VulkanRenderer::getImageViews(device, swapchain_images, surface_format);
    size_t sz = swapchain_images.size();
    std::vector<SwapChainBuffer> swapchain_buffers;
    swapchain_buffers.reserve(sz);
    for(size_t i = 0u; i < sz; ++i) {
        swapchain_buffers.push_back({swapchain_images[i], swapchain_views[i]});
    }

    return swapchain_buffers;
}

VkExtent2D VulkanSwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
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