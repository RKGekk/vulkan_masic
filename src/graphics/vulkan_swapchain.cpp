#include "vulkan_swapchain.h"
#include "vulkan_renderer.h"

bool VulkanSwapChain::init(std::shared_ptr<VulkanDevice> device, VkSurfaceKHR surface, GLFWwindow* window) {
    m_window = window;
    m_device = std::move(device);
    
    m_surface = surface;
    m_swapchain_support_details = querySwapChainSupport(m_device->getDeviceAbilities().physical_device, m_surface);

    m_swapchain_params.surface_format = chooseSwapSurfaceFormat(m_swapchain_support_details);
    m_swapchain_params.present_mode = chooseSwapPresentMode(m_swapchain_support_details.present_modes);
    m_swapchain_params.extent = chooseSwapExtent(m_swapchain_support_details.capabilities);

    m_swapchain_params.images_sharing_mode = m_device->getCommandManager().getBufferSharingMode();

    m_swapchain = createSwapchain(m_surface, m_swapchain_params, m_swapchain_support_details, m_device->getCommandManager().getQueueFamilyIndices());
    VkResult result = vkGetSwapchainImagesKHR(m_device->getDevice(), m_swapchain, &m_max_frames, nullptr);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to get swapchain images count!");
    }
    m_swapchain_images = retriveSwapchainBuffers(m_swapchain_params.surface_format.format);

    m_image_available_sem.resize(m_max_frames);
    m_image_available_fen.resize(m_max_frames);

    VkSemaphoreCreateInfo image_available_sema_info{};
    image_available_sema_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo in_flight_fen_info{};
    in_flight_fen_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    in_flight_fen_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for(size_t i = 0u; i < m_max_frames; ++i) {
        VkResult result = vkCreateSemaphore(m_device->getDevice(), &image_available_sema_info, nullptr, &m_image_available_sem[i]);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to create semaphore!");
        }
        
        result = vkCreateFence(m_device->getDevice(), &in_flight_fen_info, nullptr, &m_image_available_fen[i]);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to create fence!");
        }
    }

    return true;
}

void VulkanSwapChain::destroy() {
    size_t sz = m_swapchain_images.size();
    for(size_t i = 0u; i < sz; ++i) {
        vkDestroyImageView(m_device->getDevice(), m_swapchain_images[i].getImageBufferView(), nullptr);
        vkDestroySemaphore(m_device->getDevice(), m_image_available_sem[i], nullptr);
        vkDestroyFence(m_device->getDevice(), m_image_available_fen[i], nullptr);
    }
    vkDestroySwapchainKHR(m_device->getDevice(), m_swapchain, nullptr);

}

void VulkanSwapChain::recreate() {
    destroy();

    m_swapchain_support_details = querySwapChainSupport(m_device->getDeviceAbilities().physical_device, m_surface);

    m_swapchain_params.surface_format = chooseSwapSurfaceFormat(m_swapchain_support_details);
    m_swapchain_params.present_mode = chooseSwapPresentMode(m_swapchain_support_details.present_modes);
    m_swapchain_params.extent = chooseSwapExtent(m_swapchain_support_details.capabilities);

    m_swapchain_params.images_sharing_mode = m_device->getCommandManager().getBufferSharingMode();

    m_swapchain = createSwapchain(m_surface, m_swapchain_params, m_swapchain_support_details, m_device->getCommandManager().getQueueFamilyIndices());
    m_swapchain_images = retriveSwapchainBuffers(m_swapchain_params.surface_format.format);
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
    //VkResult result = vkAcquireNextImageKHR(m_device->getDevice(), m_swapchain, UINT64_MAX, m_image_available_sem[m_current_sync], getImageAvailableFence(), &m_current_frame);
    //VkResult result = vkAcquireNextImageKHR(m_device->getDevice(), m_swapchain, UINT64_MAX, getImageAvailableSemaphore(), VK_NULL_HANDLE, &m_current_frame);
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

VkSurfaceKHR VulkanSwapChain::getSurface() const {
    return m_surface;
}

GLFWwindow* VulkanSwapChain::getWindow() const {
    return m_window;
}

VkSwapchainKHR VulkanSwapChain::getSwapchain() const {
    return m_swapchain;
}

const std::vector<VulkanImageBuffer>& VulkanSwapChain::getSwapchainImages() const {
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

    details.is_native_swapchain_BGR = isNativeSwapChainBGR(details.formats);
    
    return details;
}

VkSurfaceFormatKHR VulkanSwapChain::chooseSwapSurfaceFormat(const SwapchainSupportDetails& available_formats) {
    for(const auto& available_format : available_formats.formats) {
        if(available_format.format == VK_FORMAT_B8G8R8A8_SRGB && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && available_formats.is_native_swapchain_BGR) {
            return available_format;
        }
        if(available_format.format == VK_FORMAT_R8G8B8A8_SRGB && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && !available_formats.is_native_swapchain_BGR) {
            return available_format;
        }
    }
    return available_formats.formats[0];
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

VkSwapchainKHR VulkanSwapChain::createSwapchain(VkSurfaceKHR surface, const SwapchainParams& swapchain_params, const SwapchainSupportDetails& swapchain_support_details, const QueueFamilyIndices& queue_family_indices) const {
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
    VkResult result = vkCreateSwapchainKHR(m_device->getDevice(), &swapchain_create_info, nullptr, &swap_chain);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }
    
    return swap_chain;
}

std::vector<VkImage> VulkanSwapChain::retriveSwapchainImages() const {
    uint32_t swapchain_image_count = 0u;
    VkResult result = vkGetSwapchainImagesKHR(m_device->getDevice(), m_swapchain, &swapchain_image_count, nullptr);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to get swapchain images!");
    }
    std::vector<VkImage> swapchain_images(swapchain_image_count);
    result = vkGetSwapchainImagesKHR(m_device->getDevice(), m_swapchain, &swapchain_image_count, swapchain_images.data());
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to get swapchain images!");
    }
    
    return swapchain_images; 
}

std::vector<VulkanImageBuffer> VulkanSwapChain::retriveSwapchainBuffers(VkFormat format) const {
    std::vector<VkImage> swapchain_images = retriveSwapchainImages();
    size_t sz = swapchain_images.size();
    std::vector<VulkanImageBuffer> swapchain_buffers(sz);
    for(size_t i = 0u; i < sz; ++i) {
        swapchain_buffers[i].init(m_device, swapchain_images[i], m_swapchain_params.extent, format);
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