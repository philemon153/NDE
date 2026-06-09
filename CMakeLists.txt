cmake_minimum_required(VERSION 3.22)
project(DemoSceneTestbed LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)

include(FetchContent)
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.4)
FetchContent_Declare(glm  GIT_REPOSITORY https://github.com/g-truc/glm.git GIT_TAG 1.0.1)
FetchContent_Declare(vk_bootstrap GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap.git GIT_TAG v1.4.350)

FetchContent_MakeAvailable(glfw glm vk_bootstrap)

add_executable(demoscene_testbed
        src/main.cpp
        src/VulkanContext.cpp
        src/Renderer.cpp
        src/AudioSystem.cpp
        src/Shader.cpp
        src/Effects/EmptyEffect.cpp
        src/Effects/PlasmaEffect.cpp
        src/Effects/TunnelEffect.cpp
        src/Effects/ParticlesEffect.cpp
        src/Effects/EmptyEffect.cpp
        src/Effects/EmptyEffect.h
)

target_include_directories(demoscene_testbed PRIVATE src)

target_link_libraries(demoscene_testbed PRIVATE
        Vulkan::Vulkan
        glfw
        glm::glm
        vk-bootstrap::vk-bootstrap
)

if (MINGW)
    target_link_libraries(demoscene_testbed PRIVATE mingw32 gdi32 user32 kernel32 dwmapi)
endif()

# Safer asset copying
add_custom_command(TARGET demoscene_testbed POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:demoscene_testbed>/shaders
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:demoscene_testbed>/shaders
        COMMENT "Copying shaders"
)

add_custom_command(TARGET demoscene_testbed POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:demoscene_testbed>/music
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        ${CMAKE_SOURCE_DIR}/music $<TARGET_FILE_DIR:demoscene_testbed>/music
        COMMENT "Copying music"
)