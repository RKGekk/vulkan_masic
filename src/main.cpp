#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <filesystem>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_set>

#include "graphics/vulkan_instance_layers_and_extensions.h"
#include "graphics/vulkan_instance.h"
#include "graphics/vulkan_device_extensions.h"
#include "graphics/vulkan_device.h"
#include "graphics/vulkan_command_manager.h"
#include "graphics/vulkan_renderer.h"
#include "graphics/vulkan_drawable.h"
#include "tools/string_tools.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
const char* WINDOW_TITLE = "Vulkan Test";
const char* APP_NAME = "Hello Triangle";
const char* ENGINE_NAME = "No Engine";

const std::vector<Vertex> g_vertices = {
    {{-0.5f, -0.5f,  0.0f}, { 1.0f,  0.0f,  0.0f}, { 0.0f,  0.0f}},
    {{ 0.5f, -0.5f,  0.0f}, { 0.0f,  1.0f,  0.0f}, { 1.0f,  0.0f}},
    {{ 0.5f,  0.5f,  0.0f}, { 0.0f,  0.0f,  1.0f}, { 1.0f,  1.0f}},
    {{-0.5f,  0.5f,  0.0f}, { 1.0f,  1.0f,  1.0f}, { 0.0f,  1.0f}},

    {{-0.5f, -0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, { 0.0f,  0.0f}},
    {{ 0.5f, -0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}, { 1.0f,  0.0f}},
    {{ 0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f,  1.0f}, { 1.0f,  1.0f}},
    {{-0.5f,  0.5f, -0.5f}, { 1.0f,  1.0f,  1.0f}, { 0.0f,  1.0f}}
};

const std::vector<uint16_t> g_indices = {
    0, 1, 2,
    2, 3, 0,
    4, 5, 6,
    6, 7, 4
};

class VulkanApplication {
public:
    void run() {
        glfwInit();
        
        m_window = initMainWindow();
        initVulkan(m_window);
        mainLoop();
        cleanup();

        glfwTerminate();
    }

private:
    GLFWwindow* m_window = nullptr;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;

    VulkanInstance m_vulkan_instance;
    std::shared_ptr<VulkanDevice> m_vulkan_device;
    
    VulkanRenderer m_renderer;

    GLFWwindow* initMainWindow() {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        GLFWwindow* result = glfwCreateWindow(WIDTH, HEIGHT, WINDOW_TITLE, nullptr, nullptr);

        return result;
    }

    void initVulkan(GLFWwindow* glfw_window_ptr) {
        using namespace std::literals;
        
        m_vulkan_instance.init("Masic"s);
        m_surface = VulkanSwapChain::createSurface(m_vulkan_instance.getInstance(), glfw_window_ptr);

        m_vulkan_device = std::make_shared<VulkanDevice>();
        m_vulkan_device->init(m_vulkan_instance, m_surface);

        m_renderer.init(m_vulkan_device, m_surface, m_window, "textures/texture.jpg");
        std::shared_ptr<VulkanDrawable> drawable = std::make_shared<VulkanDrawable>();
        drawable->init(m_vulkan_device, g_vertices, g_indices);
        m_renderer.addDrawable(std::move(drawable));
    }

    void update_frame(uint32_t current_image) {
        static auto start_time = std::chrono::high_resolution_clock::now();
        
        auto current_time = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();
        float angle = dt * glm::radians(90.f);
        glm::vec3 rotation_axis = glm::vec3(0.0f, 0.0f, 1.0f);
        float aspect = (float)m_renderer.getSwapchain().getSwapchainParams().extent.width / (float)m_renderer.getSwapchain().getSwapchainParams().extent.height;
        
        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), angle, rotation_axis);
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1.0f;
        
        m_renderer.update_frame(ubo);
    }

    void mainLoop() {
        while(!glfwWindowShouldClose(m_window)) {
            glfwPollEvents();
            update_frame(m_renderer.getSwapchain().getCurrentFrame());
            m_renderer.drawFrame();
        }
    }

    void cleanup() {
        m_renderer.destroy();
        m_vulkan_device->destroy();
        vkDestroySurfaceKHR(m_vulkan_instance.getInstance(), m_surface, nullptr);
        m_vulkan_instance.destroy();
        glfwDestroyWindow(m_window);
    }
};

int main(int, char**){
    VulkanApplication app;

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    return 0;
}