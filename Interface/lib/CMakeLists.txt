add_library(ImGui imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_tables.cpp imgui/imgui_widgets.cpp imgui_coloured.cpp imgui/misc/cpp/imgui_stdlib.cpp)
add_library(ImGuiBackend imgui/backends/imgui_impl_glfw.cpp imgui/backends/imgui_impl_opengl3.cpp)
add_library(ImGuiDemo imgui/imgui_demo.cpp)
add_library(ImPlot implot/implot.cpp implot/implot_demo.cpp implot/implot_items.cpp)
add_library(gl3w imgui/examples/libs/gl3w/GL/gl3w.c)

FILE(GLOB TONIC_SRCS Tonic/src/Tonic/*.cpp)
add_library(Tonic ${TONIC_SRCS})
add_library(RtAudio Tonic/examples/Standalone/RTAudio/RtAudio.cpp)

add_compile_definitions(IMGUI_IMPL_OPENGL_LOADER_GL3W)
add_compile_definitions(__LINUX_PULSE__)

