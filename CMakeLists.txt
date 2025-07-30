cmake_minimum_required(VERSION 3.25)

project(vktutorial LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

set(SRC_DIR "src")
set(APP_SOURCES "${SRC_DIR}/main.cpp" "${SRC_DIR}/graphics/vulkan_instance_layers_and_extensions.h" "${SRC_DIR}/graphics/vulkan_instance_layers_and_extensions.cpp" "${SRC_DIR}/tools/string_tools.h" "${SRC_DIR}/tools/string_tools.cpp")
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