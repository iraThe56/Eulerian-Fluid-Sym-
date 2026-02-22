cmake_minimum_required(VERSION 3.31.6)
project(OpenGLSetup)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")

add_subdirectory(lib)

file(GLOB_RECURSE SOURCES
        "src/*.cpp"
        "src/*.h"
)

add_executable(OpenGLSetup ${SOURCES}
        src/core/FluidSim.cpp
        src/core/FluidSim.h)

# Put the exe in build/<CONFIG> (Debug/Release)
set_target_properties(OpenGLSetup PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build/$<CONFIG>"
)

# Copy assets next to the exe after build
add_custom_command(TARGET OpenGLSetup POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/resources"
        "$<TARGET_FILE_DIR:OpenGLSetup>/resources"
)

target_link_libraries(OpenGLSetup PRIVATE glad glfw glm::glm imgui_lib)