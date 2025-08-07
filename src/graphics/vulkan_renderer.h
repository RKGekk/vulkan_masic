#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vector>

#include <stb_image.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "../tools/string_tools.h"
#include "vulkan_device.h"
#include "vulkan_swapchain.h"

struct ImageBuffer {
    VkDeviceMemory memory;
    VkImage image;
    VkImageView image_view;
};

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 tex_coord;
    
    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription binding_desc{};
        binding_desc.binding = 0u;
        binding_desc.stride = sizeof(Vertex);
        binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        
        return binding_desc;
    }
    
    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescritpions() {
        std::array<VkVertexInputAttributeDescription, 3> attribute_desc{};
        attribute_desc[0].binding = 0;
        attribute_desc[0].location = 0;
        attribute_desc[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_desc[0].offset = offsetof(Vertex, pos);
        
        attribute_desc[1].binding = 0;
        attribute_desc[1].location = 1;
        attribute_desc[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_desc[1].offset = offsetof(Vertex, color);
        
        attribute_desc[2].binding = 0;
        attribute_desc[2].location = 2;
        attribute_desc[2].format = VK_FORMAT_R32G32_SFLOAT;
        attribute_desc[2].offset = offsetof(Vertex, tex_coord);
        
        return attribute_desc;
    }
};

class VulkanRenderer {
public:
    bool init(std::shared_ptr<VulkanDevice> device, VkSurfaceKHR surface, GLFWwindow* window, std::shared_ptr<VulkanCommandManager> command_manager, const std::string& texture_path, const std::vector<Vertex>& vertices, const std::vector<uint16_t>& indices);
    void destroy();
    void recreate();

    const VulkanSwapChain& getSwapchain();

    void recordCommandBuffer(VkCommandBuffer command_buffer, uint32_t image_index);
    void drawFrame();
    void setFramebufferResized();
    void update_frame(const UniformBufferObject& ubo);

    static std::vector<VkImageView> getImageViews(VkDevice device, const std::vector<VkImage>& images, VkSurfaceFormatKHR surface_format);
    static VkImageView createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspect_flags, uint32_t mip_levels);
    static void createImage(const std::shared_ptr<VulkanDevice>& device_ptr, VkDeviceMemory& memory, VkImage& image, const VkImageCreateInfo& image_info, VkMemoryPropertyFlags properties);
    static ImageBuffer createImage(const std::shared_ptr<VulkanDevice>& device_ptr, const VkImageCreateInfo& image_info, VkMemoryPropertyFlags properties, VkImageAspectFlags aspect_flags, uint32_t mip_levels);
    static uint32_t findMemoryType(const std::shared_ptr<VulkanDevice>& device_ptr, uint32_t type_filter, VkMemoryPropertyFlags properties);
    static void transitionImageLayout(VkDevice device, VkCommandPool cmd_pool, VkQueue queue, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, uint32_t mip_levels);
    static bool hasStencilComponent(VkFormat format);
    static VkFormat findDepthFormat(const std::shared_ptr<VulkanDevice>& device_ptr);
    static VkShaderModule CreateShaderModule(VkDevice device, const std::string& path);
    static VkShaderModule CreateShaderModule(VkDevice device, const std::vector<char>& buffer);
    static VkRenderPass createRenderPass(const std::shared_ptr<VulkanDevice>& device_ptr, const SwapchainParams& swapchain_params, VkSubpassDependency pass_dependency);
    static void createBuffer(const std::shared_ptr<VulkanDevice>& device_ptr, VkDeviceSize size, QueueFamilyIndices queue_family_indices, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& memory);
    static void copyBufferToImage(VkDevice device, VkCommandPool cmd_pool, VkQueue queue, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    static void generateMipmaps(const std::shared_ptr<VulkanDevice>& device_ptr, VkCommandPool cmd_pool, VkQueue queue, VkImage image, VkFormat image_format, int32_t tex_width, uint32_t tex_height, uint32_t mip_levels);
    static void copyBuffer(VkDevice device, VkCommandPool cmd_pool, VkQueue queue, VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size);

private:

    void createColorResources();
    void createDepthResources();
    void loadShaders();

    VkDescriptorSetLayout createDescSetLayout();
    void createRenderPass();
    void createPipeline(VkDevice device, VkShaderModule vert_shader_modeule, VkShaderModule frag_shader_modeule, VkRenderPass render_pass, SwapchainParams swapchain_params, VkDescriptorSetLayout desc_set_layout, VkSampleCountFlagBits msaa_samples);
    std::vector<VkFramebuffer> createFramebuffers();
    VkImage createImage(VkCommandPool cmd_pool, VkQueue queue, const std::string& path_to_file);
    VkSampler createTextureSampler(uint32_t mip_levels);
    void createAndTransferVertexBuffer(VkCommandPool cmd_pool, VkQueue queue, const std::vector<Vertex>& vertices);
    void createAndTransferIndexBuffer(VkCommandPool cmd_pool, VkQueue queue, const std::vector<uint16_t>& indices);
    void createUniformBuffers(VkDevice device);
    void createDescSets(VkDevice device);
    VkDescriptorPool createDescPool(VkDevice device);
    void createSyncObjects();

    std::shared_ptr<VulkanDevice> m_device;
    std::shared_ptr<VulkanCommandManager> m_command_manager;

    VulkanSwapChain m_swapchain;

    std::vector<VkFramebuffer> m_out_framebuffers;
    std::vector<ImageBuffer> m_out_color_images;
    std::vector<ImageBuffer> m_out_depth_images;

    VkShaderModule m_vert_shader_modeule = VK_NULL_HANDLE;
    VkShaderModule m_frag_shader_modeule = VK_NULL_HANDLE;

    VkRenderPass m_render_pass = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_desc_set_layout = VK_NULL_HANDLE;
    VkPipelineLayout m_pipeline_layout = VK_NULL_HANDLE;
    VkPipeline m_graphics_pipeline = VK_NULL_HANDLE;
    VkSampler m_texture_sampler = VK_NULL_HANDLE;

    uint32_t m_mip_levels = 1u;
    VkImage m_texture_image = VK_NULL_HANDLE;
    VkDeviceMemory m_texture_memory = VK_NULL_HANDLE;
    VkImageView m_texture_view = VK_NULL_HANDLE;

    VkDescriptorPool m_desc_pool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> m_desc_sets;
    std::vector<VkCommandBuffer> m_command_buffers;

    VkBuffer m_vertex_buffer = VK_NULL_HANDLE;
    VkDeviceMemory m_vertex_memory = VK_NULL_HANDLE;
    VkBuffer m_index_buffer = VK_NULL_HANDLE;
    VkDeviceMemory m_index_memory = VK_NULL_HANDLE;
    size_t m_indices_count = 0u;
    std::vector<VkBuffer> m_uniform_buffers;
    std::vector<VkDeviceMemory> m_uniform_memory;
    std::vector<void*> m_uniform_mapped;

    std::vector<VkSemaphore> m_image_available; // signaled when the presentation engine is finished using the image.
    std::vector<VkSemaphore> m_render_finished;
    std::vector<VkFence> m_in_flight_frame; // will be signaled when the command buffers finish

    bool m_framebuffer_resized = false;
};