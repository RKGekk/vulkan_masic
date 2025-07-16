cmake_minimum_required(VERSION 3.25)

project(vktutorial LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

set(SRC_DIR "src")
set(APP_SOURCES "${SRC_DIR}/main.cpp")

find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_path(TINYGLTF_INCLUDE_DIRS "tiny_gltf.h")
find_package(pugixml CONFIG REQUIRED)

add_executable(vktutorial ${APP_SOURCES})

target_include_directories(vktutorial PRIVATE ${TINYGLTF_INCLUDE_DIRS} ${Stb_INCLUDE_DIR})
target_link_libraries(vktutorial PRIVATE Vulkan::Vulkan glfw ${GLFW_LIBRARIES} glm::glm imgui::imgui pugixml::shared pugixml::pugixml)