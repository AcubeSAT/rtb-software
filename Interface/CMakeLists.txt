cmake_minimum_required(VERSION 3.10)
project(RadiationInterface)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

#find_package(glfw3 REQUIRED)
#find_package(glew REQUIRED)
#set(OpenGL_GL_PREFERENCE GLVND)
#find_package(OpenGL REQUIRED)
find_package(Boost 1.50 COMPONENTS system REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(glfw3 glfw3 REQUIRED)
pkg_check_modules(pulse libpulse-simple REQUIRED)

include_directories(
        "${PROJECT_SOURCE_DIR}/lib/imgui"
        "${PROJECT_SOURCE_DIR}/lib/imgui/backends"
        "${PROJECT_SOURCE_DIR}/lib/imgui/examples/libs/gl3w"
        "${PROJECT_SOURCE_DIR}/lib/imgui/misc/cpp"
        "${PROJECT_SOURCE_DIR}/lib/plog/include"
        "${PROJECT_SOURCE_DIR}/lib/magic_enum/include"
        "${PROJECT_SOURCE_DIR}/lib/cereal/include"
        "${PROJECT_SOURCE_DIR}/lib/Tonic/src"
        "${PROJECT_SOURCE_DIR}/lib/rtaudio"
        "${PROJECT_SOURCE_DIR}/lib"
)
add_subdirectory(lib)
add_subdirectory(lib/plog)

file(GLOB SOURCE_FILES *.cpp)
set(VARIOUS)
add_executable(RadiationInterface ${SOURCE_FILES} ${VARIOUS})
target_link_libraries(RadiationInterface ImGui ImGuiBackend ImGuiDemo ImPlot gl3w GL ${glfw3_LIBRARIES} Tonic RtAudio dl rt atomic)
target_link_libraries(RadiationInterface pthread ${Boost_LIBRARIES} ${pulse_LIBRARIES})

add_executable(LCG tests/LCG.cpp)

add_compile_definitions(__LINUX_PULSE__)

set(CMAKE_VERBOSE_MAKEFILE ON)
#set(CMAKE_CXX_FLAGS "-O3")
