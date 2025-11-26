cmake_minimum_required(VERSION 3.25)

project(vktutorial LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_PARALLEL_LEVEL 18)

set(SRC_DIR "src")
set(APP_SOURCES
    "${SRC_DIR}/procs/process.h"
    "${SRC_DIR}/procs/process.cpp"
    "${SRC_DIR}/procs/process_manager.h"
    "${SRC_DIR}/procs/process_manager.cpp"
    "${SRC_DIR}/procs/exec_process.h"
    "${SRC_DIR}/procs/exec_process.cpp"
    "${SRC_DIR}/procs/delay_process.h"
    "${SRC_DIR}/procs/delay_process.cpp"
    "${SRC_DIR}/procs/count_process.h"
    "${SRC_DIR}/procs/count_process.cpp"
    "${SRC_DIR}/engine/renderer_enum.h"
    "${SRC_DIR}/engine/iengine_logic.h"
    "${SRC_DIR}/engine/base_engine_logic.h"
    "${SRC_DIR}/engine/base_engine_logic.cpp"
    "${SRC_DIR}/engine/iengine_physics.h"
    "${SRC_DIR}/engine/views/human_view.h"
    "${SRC_DIR}/engine/views/human_view.cpp"
    "${SRC_DIR}/engine/screen_elements/iscreen_element.h"
    "${SRC_DIR}/engine/screen_elements/base_ui.h"
    "${SRC_DIR}/engine/screen_elements/base_ui.cpp"
    "${SRC_DIR}/engine/screen_elements/test_menu_ui.h"
    "${SRC_DIR}/engine/screen_elements/test_menu_ui.cpp"
    "${SRC_DIR}/engine/screen_elements/screen_element_scene.h"
    "${SRC_DIR}/engine/screen_elements/screen_element_scene.cpp"
    "${SRC_DIR}/application_options.h"
    "${SRC_DIR}/application_options.cpp"
    "${SRC_DIR}/window_surface.h"
    "${SRC_DIR}/window_surface.cpp"
    "${SRC_DIR}/application.h"
    "${SRC_DIR}/application.cpp"
    "${SRC_DIR}/main.cpp"
    "${SRC_DIR}/physics/triangle_tests.cpp"
    "${SRC_DIR}/physics/triangle_tests.h"
    "${SRC_DIR}/physics/bounding_box.cpp"
    "${SRC_DIR}/physics/bounding_box.h"
    "${SRC_DIR}/physics/bounding_frustum.cpp"
    "${SRC_DIR}/physics/bounding_frustum.h"
    "${SRC_DIR}/physics/bounding_oriented_box.cpp"
    "${SRC_DIR}/physics/bounding_oriented_box.h"
    "${SRC_DIR}/physics/bounding_sphere.cpp"
    "${SRC_DIR}/physics/bounding_sphere.h"
    "${SRC_DIR}/physics/collision_box.h"
    "${SRC_DIR}/physics/collision_data.h"
    "${SRC_DIR}/physics/collision_detector.cpp"
    "${SRC_DIR}/physics/collision_detector.h"
    "${SRC_DIR}/physics/collision_plane.h"
    "${SRC_DIR}/physics/collision_primitive.cpp"
    "${SRC_DIR}/physics/collision_primitive.h"
    "${SRC_DIR}/physics/collision_sphere.h"
    "${SRC_DIR}/physics/contact.cpp"
    "${SRC_DIR}/physics/contact.h"
    "${SRC_DIR}/physics/contact_generator.h"
    "${SRC_DIR}/physics/contact_resolver.cpp"
    "${SRC_DIR}/physics/contact_resolver.h"
    "${SRC_DIR}/physics/ground_contacts.cpp"
    "${SRC_DIR}/physics/ground_contacts.h"
    "${SRC_DIR}/physics/intersection_tests.cpp"
    "${SRC_DIR}/physics/intersection_tests.h"
    "${SRC_DIR}/physics/particle.cpp"
    "${SRC_DIR}/physics/particle.h"
    "${SRC_DIR}/physics/particle_anchored_spring.cpp"
    "${SRC_DIR}/physics/particle_anchored_spring.h"
    "${SRC_DIR}/physics/particle_bungee.cpp"
    "${SRC_DIR}/physics/particle_bungee.h"
    "${SRC_DIR}/physics/particle_cable.cpp"
    "${SRC_DIR}/physics/particle_cable.h"
    "${SRC_DIR}/physics/particle_cable_constraint.cpp"
    "${SRC_DIR}/physics/particle_cable_constraint.h"
    "${SRC_DIR}/physics/particle_constraint.cpp"
    "${SRC_DIR}/physics/particle_constraint.h"
    "${SRC_DIR}/physics/particle_contact.cpp"
    "${SRC_DIR}/physics/particle_contact.h"
    "${SRC_DIR}/physics/particle_contact_generator.h"
    "${SRC_DIR}/physics/particle_contact_resolver.cpp"
    "${SRC_DIR}/physics/particle_contact_resolver.h"
    "${SRC_DIR}/physics/particle_drag.cpp"
    "${SRC_DIR}/physics/particle_drag.h"
    "${SRC_DIR}/physics/particle_fake_spring.cpp"
    "${SRC_DIR}/physics/particle_fake_spring.h"
    "${SRC_DIR}/physics/particle_force_generator.h"
    "${SRC_DIR}/physics/particle_force_registry.cpp"
    "${SRC_DIR}/physics/particle_force_registry.h"
    "${SRC_DIR}/physics/particle_gravity.cpp"
    "${SRC_DIR}/physics/particle_gravity.h"
    "${SRC_DIR}/physics/particle_link.cpp"
    "${SRC_DIR}/physics/particle_link.h"
    "${SRC_DIR}/physics/particle_rod.cpp"
    "${SRC_DIR}/physics/particle_rod.h"
    "${SRC_DIR}/physics/particle_spring.cpp"
    "${SRC_DIR}/physics/particle_spring.h"
    "${SRC_DIR}/physics/particle_world.cpp"
    "${SRC_DIR}/physics/particle_world.h"
    "${SRC_DIR}/physics/rigid_body.cpp"
    "${SRC_DIR}/physics/rigid_body.h"
    "${SRC_DIR}/physics/sphere_contacts.cpp"
    "${SRC_DIR}/physics/sphere_contacts.h"
    "${SRC_DIR}/actors/actor.h"
    "${SRC_DIR}/actors/actor.cpp"
    "${SRC_DIR}/actors/actor_component.h"
    "${SRC_DIR}/actors/actor_component.cpp"
    "${SRC_DIR}/actors/actor_factory.h"
    "${SRC_DIR}/actors/actor_factory.cpp"
    "${SRC_DIR}/actors/transform_component.h"
    "${SRC_DIR}/actors/transform_component.cpp"
    "${SRC_DIR}/actors/camera_component.h"
    "${SRC_DIR}/actors/camera_component.cpp"
    "${SRC_DIR}/actors/base_scene_node_component.h"
    "${SRC_DIR}/actors/base_scene_node_component.cpp"
    "${SRC_DIR}/actors/model_component.h"
    "${SRC_DIR}/actors/model_component.h"
    "${SRC_DIR}/events/ievent_data.h"
    "${SRC_DIR}/events/base_event_data.h"
    "${SRC_DIR}/events/base_event_data.cpp"
    "${SRC_DIR}/events/ievent_manager.h"
    "${SRC_DIR}/events/ievent_manager.cpp"
    "${SRC_DIR}/events/event_manager.h"
    "${SRC_DIR}/events/event_manager.cpp"
    "${SRC_DIR}/events/cicadas/window_keycodes.h"
    "${SRC_DIR}/events/cicadas/window_keycodes.cpp"
    "${SRC_DIR}/events/cicadas/evt_data_destroy_scene_component.h"
    "${SRC_DIR}/events/cicadas/evt_data_destroy_scene_component.cpp"
    "${SRC_DIR}/events/cicadas/key_event_args.h"
    "${SRC_DIR}/events/cicadas/key_event_args.cpp"
    "${SRC_DIR}/events/cicadas/evt_data_environment_loaded.h"
    "${SRC_DIR}/events/cicadas/evt_data_environment_loaded.cpp"
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
    "${SRC_DIR}/events/cicadas/evt_data_new_scene_component.h"
    "${SRC_DIR}/events/cicadas/evt_data_new_scene_component.cpp"
    "${SRC_DIR}/events/cicadas/evt_data_modified_scene_component.h"
    "${SRC_DIR}/events/cicadas/evt_data_modified_scene_component.cpp"
    "${SRC_DIR}/events/cicadas/evt_data_restore_window.h"
    "${SRC_DIR}/events/cicadas/evt_data_restore_window.cpp"
    "${SRC_DIR}/events/cicadas/evt_data_new_actor.h"
    "${SRC_DIR}/events/cicadas/evt_data_new_actor.cpp"
    "${SRC_DIR}/events/cicadas/evt_data_destroy_actor.h"
    "${SRC_DIR}/events/cicadas/evt_data_destroy_actor.cpp"
    "${SRC_DIR}/events/cicadas/evt_data_request_destroy_actor.h"
    "${SRC_DIR}/events/cicadas/evt_data_request_destroy_actor.cpp"
    "${SRC_DIR}/events/cicadas/evt_data_request_new_actor.h"
    "${SRC_DIR}/events/cicadas/evt_data_request_new_actor.cpp"
    "${SRC_DIR}/events/cicadas/evt_data_move_actor.h"
    "${SRC_DIR}/events/cicadas/evt_data_move_actor.cpp"
    "${SRC_DIR}/events/cicadas/evt_data_request_start_game.h"
    "${SRC_DIR}/events/cicadas/evt_data_request_start_game.cpp"
    "${SRC_DIR}/events/cicadas/evt_data_sphere_particle_contact.h"
    "${SRC_DIR}/events/cicadas/evt_data_sphere_particle_contact.cpp"
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
    "${SRC_DIR}/tools/mt_random.h"
    "${SRC_DIR}/tools/mt_random.cpp"
    "${SRC_DIR}/tools/vulkan_tools.h"
    "${SRC_DIR}/tools/cpu_load_balance.h"
    "${SRC_DIR}/tools/cpu_load_balance.cpp"
    "${SRC_DIR}/tools/thread_pool.h"
    "${SRC_DIR}/tools/thread_pool.cpp"
    "${SRC_DIR}/tools/thread_safe_lookup_table.h"
    "${SRC_DIR}/tools/thread_safe_lookup_table.cpp"
    "${SRC_DIR}/scene/material.h"
    "${SRC_DIR}/scene/material.cpp"
    "${SRC_DIR}/scene/scene.h"
    "${SRC_DIR}/scene/scene.cpp"
    "${SRC_DIR}/scene/nodes/scene_node.h"
    "${SRC_DIR}/scene/nodes/scene_node.cpp"
    "${SRC_DIR}/scene/nodes/camera_node.h"
    "${SRC_DIR}/scene/nodes/camera_node.cpp"
    "${SRC_DIR}/scene/nodes/basic_camera_node.h"
    "${SRC_DIR}/scene/nodes/basic_camera_node.cpp"
    "${SRC_DIR}/scene/nodes/scene_node_properties.h"
    "${SRC_DIR}/scene/nodes/scene_node_properties.cpp"
    "${SRC_DIR}/scene/nodes/mesh_node.h"
    "${SRC_DIR}/scene/nodes/mesh_node.cpp"
    "${SRC_DIR}/scene/nodes/aabb_node.h"
    "${SRC_DIR}/scene/nodes/aabb_node.cpp"
    "${SRC_DIR}/graphics/vulkan_layout_tracker.h"
    "${SRC_DIR}/graphics/vulkan_layout_tracker.cpp"
    "${SRC_DIR}/graphics/model_data.h"
    "${SRC_DIR}/graphics/model_data.cpp"
    "${SRC_DIR}/graphics/vulkan_render_target.h"
    "${SRC_DIR}/graphics/vulkan_render_target.cpp"
    "${SRC_DIR}/graphics/vulkan_image_buffer.h"
    "${SRC_DIR}/graphics/vulkan_image_buffer.cpp"
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
    "${SRC_DIR}/graphics/gltf_drawable.h"
    "${SRC_DIR}/graphics/gltf_drawable.cpp"
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
    "${SRC_DIR}/graphics/imgui_drawable.h"
    "${SRC_DIR}/graphics/imgui_drawable.cpp"
)
set(SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data/shaders")
set(TEXTURES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data/textures")
set(OBJECTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data/objects")
set(COMPONENTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data/components")
set(FONT_DIR "${CMAKE_BINARY_DIR}/Debug/fonts")
set(APP_SHADERS "${SHADER_DIR}/shader.frag" "${SHADER_DIR}/shader.vert" "${SHADER_DIR}/imgui.frag" "${SHADER_DIR}/imgui.vert")
set(APP_RESOURCES "${TEXTURES_DIR}/texture.jpg" "${TEXTURES_DIR}/Sketchfab_UV_Checker.png")
set(APP_OBJECTS "${OBJECTS_DIR}/cube.gltf" "${OBJECTS_DIR}/cube.bin")
set(APP_COMPONENTS "${COMPONENTS_DIR}/BasicTransformComponent.xml" "${COMPONENTS_DIR}/LightComponent.xml")
set(APP_FONTS "${FONT_DIR}/OpenSans-Light.ttf")

set(SPV_BIN_DIR "${CMAKE_BINARY_DIR}/Debug/shaders")
set(CONFIG_DIR "${CMAKE_BINARY_DIR}/Debug/")
set(SH_RESOURCES_DIR "${CMAKE_BINARY_DIR}/Debug/textures")
set(SH_OBJ_DIR "${CMAKE_BINARY_DIR}/Debug/objects")
set(COMPONENT_CONFIG_DIR "${CMAKE_BINARY_DIR}/Debug/components")
set(FONTS_OUT_DIR "${CMAKE_BINARY_DIR}/Debug/fonts")
file(MAKE_DIRECTORY ${SPV_BIN_DIR})
file(MAKE_DIRECTORY ${SH_RESOURCES_DIR})
file(MAKE_DIRECTORY ${SH_OBJ_DIR})
file(MAKE_DIRECTORY ${COMPONENT_CONFIG_DIR})
file(MAKE_DIRECTORY ${FONTS_OUT_DIR})

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

function (CopyResourceFile FILE_PATH COPY_OUTPUT_DIRECTORY)
    get_filename_component(FILE_NAME ${FILE_PATH} NAME)
    set(SH_RESOURCES_OUTPUT "${SH_RESOURCES_DIR}/${FILE_NAME}")

    message(STATUS ${FILE_PATH})
    file(COPY ${FILE_PATH} DESTINATION ${SH_RESOURCES_DIR})

    list(APPEND SHADER_RESOURCES ${SH_RESOURCES_OUTPUT})
    set(SHADER_RESOURCES ${SHADER_RESOURCES} PARENT_SCOPE)
endfunction ()

function (CopyObjectFile FILE_PATH COPY_OUTPUT_DIRECTORY)
    get_filename_component(FILE_NAME ${FILE_PATH} NAME)
    set(SH_OBJECTS_OUTPUT "${SH_OBJ_DIR}/${FILE_NAME}")

    message(STATUS ${FILE_PATH})
    file(COPY ${FILE_PATH} DESTINATION ${SH_OBJ_DIR})

    list(APPEND SHADER_OBJECTS ${SH_OBJECTS_OUTPUT})
    set(SHADER_OBJECTS ${OBJECTS} PARENT_SCOPE)
endfunction ()

function (CopyFontFile FILE_PATH COPY_OUTPUT_DIRECTORY)
    get_filename_component(FILE_NAME ${FILE_PATH} NAME)
    set(FONTS_OUTPUT "${FONTS_OUT_DIR}/${FILE_NAME}")

    message(STATUS ${FILE_PATH})
    file(COPY ${FILE_PATH} DESTINATION ${FONTS_OUT_DIR})

    list(APPEND FONT_FILES ${FONTS_OUTPUT})
    set(FONT_FILES ${OBJECTS} PARENT_SCOPE)
endfunction ()

function (CopyComponentFile FILE_PATH COPY_OUTPUT_DIRECTORY)
    get_filename_component(FILE_NAME ${FILE_PATH} NAME)
    set(SH_COMPONENT_OUTPUT "${COMPONENT_CONFIG_DIR}/${FILE_NAME}")

    message(STATUS ${FILE_PATH})
    file(COPY ${FILE_PATH} DESTINATION ${SH_OBJ_DIR})

    list(APPEND COMPONENT_FILES ${COMPONENTS_OUTPUT})
    set(COMPONENT_FILES ${COMPONENTS} PARENT_SCOPE)
endfunction ()

function (CopyConfigFile FILE_PATH COPY_OUTPUT_DIRECTORY)
    get_filename_component(FILE_NAME ${FILE_PATH} NAME)
    set(CONFIG_OUTPUT "${CMAKE_BINARY_DIR}/Debug/${FILE_NAME}")

    message(STATUS ${FILE_PATH})
    file(COPY ${FILE_PATH} DESTINATION "${CMAKE_BINARY_DIR}/Debug")

    list(APPEND RESOURCES_CONFIG ${CONFIG_OUTPUT})
    set(RESOURCES_CONFIG ${RESOURCES_CONFIG} PARENT_SCOPE)
endfunction ()

foreach(TEXTURE ${APP_RESOURCES})
    CopyResourceFile(${TEXTURE} ${SH_RESOURCES_DIR})
endforeach()
foreach(OBJECT ${APP_OBJECTS})
    CopyObjectFile(${OBJECT} ${SH_OBJ_DIR})
endforeach()
foreach(FONT ${APP_FONTS})
    CopyFontFile(${FONT} ${FONT_DIR})
endforeach()

CopyConfigFile("${SRC_DIR}/application_options.xml" "${CMAKE_BINARY_DIR}/Debug")
CopyConfigFile("${SRC_DIR}/main_menu.xml" "${CMAKE_BINARY_DIR}/Debug")

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
add_custom_target(Objects ALL DEPENDS ${SHADER_OBJECTS})
add_custom_target(Components ALL DEPENDS ${COMPONENT_FILES})
add_custom_target(Fonts ALL DEPENDS ${FONT_FILES})
add_dependencies(vktutorial Shaders)
add_dependencies(vktutorial Textures)
add_dependencies(vktutorial Objects)
add_dependencies(vktutorial Components)
add_dependencies(vktutorial Fonts)

target_include_directories(vktutorial PRIVATE ${TINYGLTF_INCLUDE_DIRS} ${Stb_INCLUDE_DIR})
target_link_libraries(vktutorial PRIVATE Vulkan::Vulkan glfw ${GLFW_LIBRARIES} glm::glm imgui::imgui pugixml::shared pugixml::pugixml)