#include "vulkan_application.h"

#include "graphics/basic_vertex.h"
#include "graphics/basic_uniform.h"
#include "graphics/basic_drawable.h"
#include "graphics/gltf_drawable.h"

VulkanApplication::VulkanApplication() : Application() {}

VulkanApplication::~VulkanApplication() {
    m_renderer.destroy();
    m_vulkan_device->destroy();
    vkDestroySurfaceKHR(m_vulkan_instance.getInstance(), m_surface, nullptr);
    m_vulkan_instance.destroy();
}

void VulkanApplication::VRegisterEvents() {
    Application::VRegisterEvents();
}

void VulkanApplication::RegisterAllDelegates() {
    Application::RegisterAllDelegates();
}

bool VulkanApplication::initGraphics(WindowSurface::WindowPtr window) {
    using namespace std::literals;
        
    m_vulkan_instance.init("Masic"s);
    m_surface = VulkanSwapChain::createSurface(m_vulkan_instance.getInstance(), window);

    m_vulkan_device = std::make_shared<VulkanDevice>();
    m_vulkan_device->init(m_vulkan_instance, m_surface, m_thread_pool);

    m_renderer.init(m_vulkan_device, m_surface, window, m_thread_pool);

    std::shared_ptr<BasicDrawable> drawable = std::make_shared<BasicDrawable>();
    drawable->init(m_vulkan_device, m_renderer.getRenderTarget());
    m_renderer.addDrawable(std::move(drawable));

    std::shared_ptr<GLTFDrawable> gltfdrawable = std::make_shared<GLTFDrawable>();
    gltfdrawable->init(m_vulkan_device, m_renderer.getRenderTarget());
    m_renderer.addDrawable(std::move(gltfdrawable));

    return true;
}

void VulkanApplication::update_frame(uint32_t current_image) {
    m_renderer.update_frame(m_timer, current_image);
}

void VulkanApplication::mainLoop() {
    while(!Application::update()) {
        if(m_window->SkipDraw()) continue;
        update_frame(m_renderer.getSwapchain().getCurrentFrame());
        m_renderer.drawFrame();
    }
}