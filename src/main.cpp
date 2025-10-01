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
#include <cstdio>
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

#include "application_options.h"
#include "application.h"
#include "engine/engine.h"
#include "graphics/vulkan_instance_layers_and_extensions.h"
#include "graphics/vulkan_instance.h"
#include "graphics/vulkan_device_extensions.h"
#include "graphics/vulkan_device.h"
#include "graphics/vulkan_command_manager.h"
#include "graphics/vulkan_renderer.h"
#include "graphics/vulkan_drawable.h"
#include "graphics/basic_vertex.h"
#include "graphics/basic_drawable.h"
#include "graphics/vulkan_vertex_buffer.h"
#include "tools/string_tools.h"
#include "tools/game_timer.h"

int main(int, char**){
    using namespace std::literals;

    ApplicationOptions cfg;
    cfg.Init("application_options.xml"s);
    if(!Application::Get().Initialize(cfg)) return 0;
    std::shared_ptr<Engine> pEngine = Engine::GetEngine();
    if(!pEngine || !pEngine->Initialize(cfg)) return 0;
    try {
        Application::Get().run(pEngine);
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    return 0;
}