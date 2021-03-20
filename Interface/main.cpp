#include <iostream>

#include <chrono>
#include <thread>
#include <boost/asio.hpp>
#include <fstream>
#include <map>

#include "lib/imgui/imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"
#include <stdio.h>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <queue>
#include <deque>
#include <algorithm>
#include <plog/Log.h>
#include "plog/Init.h"
#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Formatters/TxtFormatter.h>
#include "Parameters.h"
#include "Experiment.h"
#include "main.h"
#include "SerialHandler.h"
#include "Clock.h"
#include "Latchups.h"

const char* glsl_version = "#version 130";

using namespace std::chrono_literals;

// The number of points to include in the graph
const int GRAPH_SIZE = 300;

static void error_callback(int error, const char *description) {
    fprintf(stderr, "Error %d: %s\n", error, description);
}


bool popupOpen = false;
bool ImguiStarted = false;

std::unique_ptr<SerialHandler> serialHandler;
Latchups latchups;
ImFont * largeFont;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

#pragma clang diagnostic pop


int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "You have not specified the serial interface to use. Usage: ./RadiationInterface [/dev/ttyACM0]"
                  << std::endl;
        return 5;
    }


    static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
    plog::init(plog::verbose, &consoleAppender);
    LOG_INFO << "Hello world!";

    serialHandler = std::make_unique<SerialHandler>();
    serialHandler->port = argv[1];
    std::thread dataThread(&SerialHandler::thread, &*serialHandler);


    // Setup window
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(1280, 720, "Radiation Testing Board Interface", NULL, NULL);
    if (window == NULL) {
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    bool err = gl3wInit() != 0;
    if (err) {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& imguiIo = ImGui::GetIO(); (void)imguiIo;
//    imguiIo.FontGlobalScale = 1.3;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Load Fonts
    // (there is a default font, this is only if you want to change it. see extra_fonts/README.txt for more details)
    ImGui::StyleColorsDark();
//  ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    std::string directory = __FILE__;
    directory.erase(directory.end() - 9, directory.end());

    imguiIo.Fonts->AddFontFromFileTTF((directory + "/lib/imgui/misc/fonts/DroidSans.ttf").c_str(), 18.0f);
    largeFont = imguiIo.Fonts->AddFontFromFileTTF((directory + "/lib/imgui/misc/fonts/DroidSans.ttf").c_str(), 44.0f);
    imguiIo.Fonts->AddFontFromFileTTF((directory + "/ShareTechMono-Regular.ttf").c_str(), 22.0f);
//    io.Fonts->AddFontFromFileTTF("../lib/imgui/misc/fonts/ProggyClean.ttf", 13.0f);
//    io.Fonts->AddFontFromFileTTF("../lib/imgui/misc/fonts/ProggyTiny.ttf", 10.0f);
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());

    bool show_test_window = false;
    ImVec4 clear_color = ImColor(35, 44, 59);

    ImguiStarted = true;

    while (!glfwWindowShouldClose(window)) {
        try {
            glfwPollEvents();

            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            if (show_test_window) {
                ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
                ImGui::ShowDemoWindow();
            }

            if (ImGui::BeginPopupModal("Connection", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("Error in communication with AcubeSAT ground station.");
                ImGui::Separator();
                ImGui::Text("Please connect the receiver (port %s).", "/");

                //static int dummy_i = 0;
                //ImGui::Combo("Combo", &dummy_i, "Delete\0Delete harder\0");

                if (!popupOpen) {
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            } else {
                popupOpen = false;
            }

            ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(400, 70), ImGuiCond_Always);
            ImGui::Begin("SpaceDot CubeSAT");

            ImGui::Checkbox("Test", &show_test_window);
            ImGui::SameLine();
            ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
            ImGui::End();

            ImGui::SetNextWindowPos(ImVec2(20, 90), ImGuiCond_Appearing);
            ImGui::SetNextWindowSize(ImVec2(400, 100), ImGuiCond_Appearing);
            ImGui::Begin("Time");
            clockWindow();
            ImGui::End();

            ImGui::SetNextWindowPos(ImVec2(20, 190), ImGuiCond_Appearing);
            ImGui::SetNextWindowSize(ImVec2(400, 250), ImGuiCond_Appearing);
            ImGui::Begin("Single Event Latchups");
            latchups.window();
            ImGui::End();

            ImGui::SetNextWindowPos(ImVec2(20, 440), ImGuiCond_Appearing);
            ImGui::SetNextWindowSize(ImVec2(400, 295), ImGuiCond_Appearing);
            ImGui::Begin("Parameters");
            parameterWindow();
            ImGui::End();

            ImGui::SetNextWindowPos(ImVec2(450, 20), ImGuiCond_Appearing);
            ImGui::SetNextWindowSize(ImVec2(400, 70), ImGuiCond_Appearing);
            ImGui::Begin("Serial Connection");
            serialHandler->window();
            ImGui::End();

            ImGui::SetNextWindowPos(ImVec2(450, 90), ImGuiCond_Appearing);
            ImGui::SetNextWindowSize(ImVec2(400, 645), ImGuiCond_Appearing);
            ImGui::Begin("Experiments");
            Experiment::window();
            ImGui::End();

            // Rendering
            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(window);
    } catch (...) {
            std::string exception = typeid(std::current_exception()).name();
            LOG_FATAL << "Unhandled exception in main thread: " << exception;

            std::this_thread::sleep_for(50ms);
        }
    }

    // Stop the acquisition thread
    LOG_DEBUG << "Stopping threads...";
    serialHandler->stop();

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    auto future = std::async(std::launch::async, &std::thread::join, &dataThread);
    if (future.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
        LOG_ERROR << "Could not kill data acquisition thread, terminating with force";
        std::terminate();
    }

    return 0;
}
