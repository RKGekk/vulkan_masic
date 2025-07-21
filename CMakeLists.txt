cmake_minimum_required(VERSION 3.25)

project(vktutorial LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

set(SRC_DIR "src")
set(APP_SOURCES "${SRC_DIR}/main.cpp")
set(SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(APP_SHADERS "${SHADER_DIR}/shader.frag" "${SHADER_DIR}/shader.vert")

set(SPV_BIN_DIR "${CMAKE_BINARY_DIR}/Debug/shaders")
file(MAKE_DIRECTORY ${SPV_BIN_DIR})

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

find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_path(TINYGLTF_INCLUDE_DIRS "tiny_gltf.h")
find_package(pugixml CONFIG REQUIRED)

add_executable(vktutorial ${APP_SOURCES})
add_custom_target(Shaders ALL DEPENDS ${SPIRV_SHADERS})

target_include_directories(vktutorial PRIVATE ${TINYGLTF_INCLUDE_DIRS} ${Stb_INCLUDE_DIR})
target_link_libraries(vktutorial PRIVATE Vulkan::Vulkan glfw ${GLFW_LIBRARIES} glm::glm imgui::imgui pugixml::shared pugixml::pugixml)