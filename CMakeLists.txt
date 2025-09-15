cmake_minimum_required(VERSION 3.25)

project(vktutorial LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

set(SRC_DIR "src")
set(APP_SOURCES
    "${SRC_DIR}/engine/renderer_enum.h"
    "${SRC_DIR}/application_options.h"
    "${SRC_DIR}/application_options.cpp"
    "${SRC_DIR}/window_surface.h"
    "${SRC_DIR}/window_surface.cpp"
    "${SRC_DIR}/application.h"
    "${SRC_DIR}/application.cpp"
    "${SRC_DIR}/vulkan_application.h"
    "${SRC_DIR}/vulkan_application.cpp"
    "${SRC_DIR}/main.cpp"
    "${SRC_DIR}/events/ievent_data.h"
    "${SRC_DIR}/events/base_event_data.h"
    "${SRC_DIR}/events/base_event_data.cpp"
    "${SRC_DIR}/events/ievent_manager.h"
    "${SRC_DIR}/events/ievent_manager.cpp"
    "${SRC_DIR}/events/event_manager.h"
    "${SRC_DIR}/events/event_manager.cpp"
    "${SRC_DIR}/events/cicadas/window_keycodes.h"
    "${SRC_DIR}/events/cicadas/window_keycodes.cpp"
    "${SRC_DIR}/events/cicadas/key_event_args.h"
    "${SRC_DIR}/events/cicadas/key_event_args.cpp"
    "${SRC_DIR}/events/cicadas/evt_data_key_pressed_event.h"
    "${SRC_DIR}/events/cicadas/evt_data_key_pressed_event.cpp"
    "${SRC_DIR}/events/cicadas/resize_event_args.h"
    "${SRC_DIR}/events/cicadas/resize_event_args.cpp"
    "${SRC_DIR}/events/cicadas/mouse_button_event_args.h"
    "${SRC_DIR}/events/cicadas/mouse_button_event_args.cpp"
    "${SRC_DIR}/events/cicadas/mouse_motion_event_args.h"
    "${SRC_DIR}/events/cicadas/mouse_motion_event_args.cpp"
    "${SRC_DIR}/events/cicadas/mouse_wheel_event_args.h"
    "${SRC_DIR}/events/cicadas/mouse_wheel_event_args.cpp"
    "${SRC_DIR}/events/cicadas/evt_data_resize_window.h"
    "${SRC_DIR}/events/cicadas/evt_data_resize_window.cpp"
    "${SRC_DIR}/events/cicadas/evt_data_window_close.h"
    "${SRC_DIR}/events/cicadas/evt_data_window_close.cpp"
    "${SRC_DIR}/events/cicadas/evt_data_dpi_scale.h"
    "${SRC_DIR}/events/cicadas/evt_data_dpi_scale.cpp"
    "${SRC_DIR}/events/cicadas/evt_data_update_tick.h"
    "${SRC_DIR}/events/cicadas/evt_data_update_tick.cpp"
    "${SRC_DIR}/events/cicadas/evt_data_key_released_event.h"
    "${SRC_DIR}/events/cicadas/evt_data_key_released_event.cpp"
    "${SRC_DIR}/events/cicadas/evt_data_maximize_window.h"
    "${SRC_DIR}/events/cicadas/evt_data_maximize_window.cpp"
    "${SRC_DIR}/events/cicadas/evt_data_minimize_window.h"
    "${SRC_DIR}/events/cicadas/evt_data_minimize_window.cpp"
    "${SRC_DIR}/events/cicadas/evt_data_mouse_button_pressed.h"
    "${SRC_DIR}/events/cicadas/evt_data_mouse_button_pressed.cpp"
    "${SRC_DIR}/events/cicadas/evt_data_mouse_button_released.h"
    "${SRC_DIR}/events/cicadas/evt_data_mouse_button_released.cpp"
    "${SRC_DIR}/events/cicadas/evt_data_mouse_motion.h"
    "${SRC_DIR}/events/cicadas/evt_data_mouse_motion.cpp"
    "${SRC_DIR}/events/cicadas/evt_data_mouse_wheel.h"
    "${SRC_DIR}/events/cicadas/evt_data_mouse_wheel.cpp"
    "${SRC_DIR}/events/cicadas/evt_data_restore_window.h"
    "${SRC_DIR}/events/cicadas/evt_data_restore_window.cpp"
    "${SRC_DIR}/tools/optional.h"
    "${SRC_DIR}/tools/signals.h"
    "${SRC_DIR}/tools/delegate.h"
    "${SRC_DIR}/tools/memory_utility.h"
    "${SRC_DIR}/tools/thread_safe_queue.h"
    "${SRC_DIR}/tools/thread_safe_queue.cpp"
    "${SRC_DIR}/tools/generic_object_factory.h"
    "${SRC_DIR}/tools/generic_object_factory.cpp"
    "${SRC_DIR}/tools/string_tools.h"
    "${SRC_DIR}/tools/string_tools.cpp"
    "${SRC_DIR}/tools/game_timer.h"
    "${SRC_DIR}/tools/game_timer.cpp"
    "${SRC_DIR}/tools/vulkan_tools.h"
    "${SRC_DIR}/tools/cpu_load_balance.h"
    "${SRC_DIR}/tools/cpu_load_balance.cpp"
    "${SRC_DIR}/tools/thread_pool.h"
    "${SRC_DIR}/tools/thread_pool.cpp"
    "${SRC_DIR}/tools/thread_safe_lookup_table.h"
    "${SRC_DIR}/tools/thread_safe_lookup_table.cpp"
    "${SRC_DIR}/graphics/vulkan_instance_layers_and_extensions.h"
    "${SRC_DIR}/graphics/vulkan_instance_layers_and_extensions.cpp"
    "${SRC_DIR}/graphics/vulkan_instance.h"
    "${SRC_DIR}/graphics/vulkan_instance.cpp"
    "${SRC_DIR}/graphics/vulkan_queue_family.h"
    "${SRC_DIR}/graphics/vulkan_queue_family.cpp"
    "${SRC_DIR}/graphics/vulkan_device.h"
    "${SRC_DIR}/graphics/vulkan_device.cpp"
    "${SRC_DIR}/graphics/vulkan_device_extensions.h"
    "${SRC_DIR}/graphics/vulkan_device_extensions.cpp"
    "${SRC_DIR}/graphics/vulkan_swapchain.h"
    "${SRC_DIR}/graphics/vulkan_swapchain.cpp"
    "${SRC_DIR}/graphics/vulkan_command_pool_type.h"
    "${SRC_DIR}/graphics/vulkan_command_pool_type.cpp"
    "${SRC_DIR}/graphics/vulkan_command_buffer.h"
    "${SRC_DIR}/graphics/vulkan_command_buffer.cpp"
    "${SRC_DIR}/graphics/vulkan_command_manager.h"
    "${SRC_DIR}/graphics/vulkan_command_manager.cpp"
    "${SRC_DIR}/graphics/vulkan_drawable.h"
    "${SRC_DIR}/graphics/vulkan_drawable.cpp"
    "${SRC_DIR}/graphics/vulkan_shader.h"
    "${SRC_DIR}/graphics/vulkan_shader.cpp"
    "${SRC_DIR}/graphics/vulkan_pipeline.h"
    "${SRC_DIR}/graphics/vulkan_pipeline.cpp"
    "${SRC_DIR}/graphics/basic_vertex.h"
    "${SRC_DIR}/graphics/basic_uniform.h"
    "${SRC_DIR}/graphics/basic_drawable.h"
    "${SRC_DIR}/graphics/basic_drawable.cpp"
    "${SRC_DIR}/graphics/render_resource.h"
    "${SRC_DIR}/graphics/vulkan_buffer.h"
    "${SRC_DIR}/graphics/vulkan_buffer.cpp"
    "${SRC_DIR}/graphics/vulkan_vertex_buffer.h"
    "${SRC_DIR}/graphics/vulkan_vertex_buffer.cpp"
    "${SRC_DIR}/graphics/vulkan_texture.h"
    "${SRC_DIR}/graphics/vulkan_texture.cpp"
    "${SRC_DIR}/graphics/vulkan_descriptor.h"
    "${SRC_DIR}/graphics/vulkan_descriptor.cpp"
    "${SRC_DIR}/graphics/vulkan_uniform_buffer.h"
    "${SRC_DIR}/graphics/vulkan_uniform_buffer.cpp"
    "${SRC_DIR}/graphics/vulkan_renderer.h"
    "${SRC_DIR}/graphics/vulkan_renderer.cpp"
)
set(SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(TEXTURES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/textures")
set(APP_SHADERS "${SHADER_DIR}/shader.frag" "${SHADER_DIR}/shader.vert")
set(APP_RESOURCES "${TEXTURES_DIR}/texture.jpg")

set(SPV_BIN_DIR "${CMAKE_BINARY_DIR}/Debug/shaders")
set(SH_RESOURCES_DIR "${CMAKE_BINARY_DIR}/Debug/textures")
file(MAKE_DIRECTORY ${SPV_BIN_DIR})
file(MAKE_DIRECTORY ${SH_RESOURCES_DIR})



find_program(GLSLC glslc HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)
if (NOT GLSLC)
    message(FATAL_ERROR "glslc not found at ${GLSLC}")
endif ()

function(compile_shader shader)
    get_filename_component(FILE_NAME ${shader} NAME)
    
    set(SPIRV_OUTPUT "${SPV_BIN_DIR}/${FILE_NAME}.spv")

    add_custom_command(
        OUTPUT ${SPIRV_OUTPUT}
        COMMAND "${GLSLC}" ${shader} -o ${SPIRV_OUTPUT}
        COMMENT "Compiling ${shader} to SPIR-V"
        DEPENDS ${shader} ${APP_SHADERS}
        VERBATIM
    )

    list(APPEND SPIRV_SHADERS ${SPIRV_OUTPUT})
    set(SPIRV_SHADERS ${SPIRV_SHADERS} PARENT_SCOPE)
endfunction()

foreach (SHADER ${APP_SHADERS})
    compile_shader(${SHADER})
endforeach()

function (CopyFile FILE_PATH COPY_OUTPUT_DIRECTORY)
    get_filename_component(FILE_NAME ${FILE_PATH} NAME)
    set(SH_RESOURCES_OUTPUT "${SH_RESOURCES_DIR}/${FILE_NAME}")

    message(STATUS ${FILE_PATH})
    file(COPY ${FILE_PATH} DESTINATION ${SH_RESOURCES_DIR})

    list(APPEND SHADER_RESOURCES ${SH_RESOURCES_OUTPUT})
    set(SHADER_RESOURCES ${SHADER_RESOURCES} PARENT_SCOPE)
endfunction ()

foreach(TEXTURE ${APP_RESOURCES})
    CopyFile(${TEXTURE} ${SH_RESOURCES_DIR})
endforeach()
CopyFile("${SRC_DIR}/application_options.xml" "${CMAKE_BINARY_DIR}/Debug")

find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_path(TINYGLTF_INCLUDE_DIRS "tiny_gltf.h")
find_package(pugixml CONFIG REQUIRED)

add_executable(vktutorial ${APP_SOURCES})
add_custom_target(Shaders ALL DEPENDS ${SPIRV_SHADERS})
add_custom_target(Textures ALL DEPENDS ${SHADER_RESOURCES})
add_dependencies(vktutorial Shaders)
add_dependencies(vktutorial Textures)

target_include_directories(vktutorial PRIVATE ${TINYGLTF_INCLUDE_DIRS} ${Stb_INCLUDE_DIR})
target_link_libraries(vktutorial PRIVATE Vulkan::Vulkan glfw ${GLFW_LIBRARIES} glm::glm imgui::imgui pugixml::shared pugixml::pugixml)