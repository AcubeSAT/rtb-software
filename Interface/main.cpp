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
#include <plog/Appenders/RollingFileAppender.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Formatters/TxtFormatter.h>
#include <filesystem>
#include <implot/implot.h>
#include "Parameters.h"
#include "Experiment.h"
#include "main.h"
#include "SerialHandler.h"
#include "Clock.h"
#include "Latchups.h"
#include "Log.h"
#include "Measurement.h"
#include "CAN.h"
#include "Beep.h"
#include "Settings.h"
#include "FontAwesome.h"
#include "CSV.h"
#include "Utilities.h"
#include "MRAM.h"
#include "ConsumptionChecker.h"

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
Measurement measurements;
CAN can;
MRAM mram;
std::optional<Settings> settings;
std::optional<Beep> beep;
std::optional<CSV> csv;
ImFont * largeFont;
ImFont * veryLargeFont;
ImFont * logFont;
ImFont * iconFont;
std::optional<plog::RollingFileAppender<plog::TxtFormatter, plog::NativeEOLConverter<>>> logFileAppender;


const ImWchar* GetGlyphRangesFontAwesome()
{
    static const ImWchar ranges[] =
        {
            0xF000, 0xFFFF,
            0,
        };
    return &ranges[0];
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "You have not specified the serial interface to use. Usage: ./RadiationInterface [/dev/ttyACM0]"
                  << std::endl;
        return 5;
    }

    Log hostLog("Host", {
        Log::LogLevel{"verbose", -6},
        Log::LogLevel{"debug", -5,},
        Log::LogLevel{"info", -4,},
        Log::LogLevel{"warning", -3,},
        Log::LogLevel{"error", -2,},
        Log::LogLevel{"fatal", -1,},
    });
    Log deviceLog("Device", {
        Log::LogLevel{"TRACE", 0},
        Log::LogLevel{"DEBUG", 1},
        Log::LogLevel{"INFO", 2},
        Log::LogLevel{"WARN", 3},
        Log::LogLevel{"ERROR", 4},
        Log::LogLevel{"FATAL", 5},
    });

    static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
    static Log::LogAppender windowAppender(hostLog);
    auto& log = plog::init(plog::verbose, &consoleAppender)
            .addAppender(&windowAppender);
    LOG_INFO << "RadiationInterface started";

    settings.emplace();
    logFileAppender.emplace(LogControl::getLogFileName("host").c_str());
    log.addAppender(&logFileAppender.value());
    LogControl::createLogDirectory();
    LogControl::reset();
    LOG_INFO << "Log output:" << LogControl::getLogFileName("***");

    beep.emplace();
    csv.emplace();
    initialiseParameters();

    serialHandler = std::make_unique<SerialHandler>(deviceLog);
    serialHandler->port = argv[1];
    std::thread dataThread(&SerialHandler::thread, &*serialHandler);

    // Setup window
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHintString(GLFW_X11_CLASS_NAME, "ImGui");
//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(1920, 900, "Radiation Testing Board Interface", NULL, NULL);
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
    ImPlot::CreateContext();
    ImGuiIO& imguiIo = ImGui::GetIO(); (void)imguiIo;
//    imguiIo.FontGlobalScale = 1.3;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // ImPlot configuration
    ImPlot::GetStyle().UseLocalTime = true;
    ImPlot::GetStyle().UseISO8601 = true;
    ImPlot::GetStyle().Use24HourClock = true;

    // Load Fonts
    // (there is a default font, this is only if you want to change it. see extra_fonts/README.txt for more details)
    ImGui::StyleColorsDark();
//  ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    std::string directory = __FILE__;
    directory.erase(directory.end() - 9, directory.end());

    ConsumptionChecker consumption;

    imguiIo.Fonts->AddFontFromFileTTF((directory + "/lib/imgui/misc/fonts/DroidSans.ttf").c_str(), 18.0f);
    largeFont = imguiIo.Fonts->AddFontFromFileTTF((directory + "/lib/imgui/misc/fonts/DroidSans.ttf").c_str(), 44.0f);
    veryLargeFont = imguiIo.Fonts->AddFontFromFileTTF((directory + "/lib/imgui/misc/fonts/DroidSans.ttf").c_str(), 96.0f);
    logFont = imguiIo.Fonts->AddFontFromFileTTF((directory + "/lib/ShareTechMono-Regular.ttf").c_str(), 15.0f);
    iconFont = imguiIo.Fonts->AddFontFromFileTTF((directory + "/lib/Font Awesome 5 Free-Solid-900.otf").c_str(), 16.0f, nullptr, GetGlyphRangesFontAwesome());
//    io.Fonts->AddFontFromFileTTF("../lib/imgui/misc/fonts/ProggyClean.ttf", 13.0f);
//    io.Fonts->AddFontFromFileTTF("../lib/imgui/misc/fonts/ProggyTiny.ttf", 10.0f);
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());



    imguiIo.IniFilename = nullptr;

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

            ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(400, 94), ImGuiCond_FirstUseEver);
            ImGui::Begin("Radiation Configuration");

            ImGui::Checkbox("Test", &show_test_window);
            ImGui::SameLine();
            ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
            ImGui::SameLine();
            ImGui::PushFont(iconFont);
            if (ImGui::Button(FontAwesome::VolumeUp)) {
                beep->beep(Beep::BeepType::Soft);
            }
            ImGui::PopFont();
            HelpTooltip("Sound test beep");
            ImGui::SameLine();
            ImGui::PushFont(iconFont);
            if (ImGui::Button(FontAwesome::Save)) {
                settings->flush();
            }
            ImGui::PopFont();
            HelpTooltip("Save configuration");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(100);
            if (ImGui::SliderFloat("volume", &(settings->volume), Beep::minVolume, 10.0f, settings->volume <= Beep::minVolume + 0.1f ? "Off" : "%.1f dB")) {
                if (std::fabs(settings->volume) < 0.8f) settings->volume = 0.0f; // snap to 0
            }

            ImGui::Spacing();
            ImGui::Text("CPU: ");
            ImGui::SameLine();
            auto cursorPos = ImGui::GetCursorPosY();
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, (ImVec4)ImColor::HSV(0.6f, 0.667f, 0.6f, 0.541f));
            ImGui::ProgressBar(consumption.cpu(), ImVec2(100, 18.0f));
            ImGui::PopStyleColor();
            ImGui::SetCursorPosY(cursorPos);
            ImGui::SameLine(ImGui::GetContentRegionAvail().x / 2.0f);
            ImGui::Text("Memory: %s", consumption.memory().c_str());

            ImGui::End();

            ImGui::SetNextWindowPos(ImVec2(20, 114), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(400, 110), ImGuiCond_FirstUseEver);
            ImGui::Begin("Local Time");
            clockWindow();
            ImGui::End();

            ImGui::SetNextWindowPos(ImVec2(20, 224), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(400, 70), ImGuiCond_FirstUseEver);
            ImGui::Begin("Serial Connection");
            serialHandler->window();
            ImGui::End();

            ImGui::SetNextWindowPos(ImVec2(20, 290), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(400, 441), ImGuiCond_FirstUseEver);
            ImGui::Begin("Parameters");
            parameterWindow();
            ImGui::End();

            ImGui::SetNextWindowPos(ImVec2(450, 20), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(400, 85), ImGuiCond_FirstUseEver);
            ImGui::Begin("Log Title");
            LogControl::logTitleWindow();
            ImGui::End();

            ImGui::SetNextWindowPos(ImVec2(450, 105), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(400, 530), ImGuiCond_FirstUseEver);
            ImGui::Begin("Experiments");
            Experiment::window();
            ImGui::End();

            ImGui::SetNextWindowPos(ImVec2(450, 635), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(400, 100), ImGuiCond_FirstUseEver);
            ImGui::Begin("Custom Log Entry");
            LogControl::customEntryWindow();
            ImGui::End();

            ImGui::SetNextWindowPos(ImVec2(870, 20), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(350, 645), ImGuiCond_FirstUseEver);
            ImGui::Begin("Single Event Latchups");
            latchups.window();
            ImGui::End();

            ImGui::SetNextWindowPos(ImVec2(20, 740), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(1200, imguiIo.DisplaySize.y - 780), ImGuiCond_FirstUseEver);
            ImGui::Begin("Measurements");
            measurements.window();
            ImGui::End();

            ImGui::SetNextWindowPos(ImVec2(1240, 20), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(650, 400), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowCollapsed(true, ImGuiCond_FirstUseEver);
            ImGui::Begin("MRAM");
            mram.window();
            ImGui::End();

            ImGui::SetNextWindowPos(ImVec2(1240, 45), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(650, 400), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowCollapsed(true, ImGuiCond_FirstUseEver);
            ImGui::Begin("CAN Bus");
            can.window();
            ImGui::End();

            ImGui::SetNextWindowPos(ImVec2(1240, 440), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(650, 270), ImGuiCond_FirstUseEver);
            ImGui::Begin("Host Logs");
            hostLog.window();
            ImGui::End();

            ImGui::SetNextWindowPos(ImVec2(1240, 440 + 270 + 31), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(650, 270), ImGuiCond_FirstUseEver);
            ImGui::Begin("Device Logs");
            deviceLog.window();
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
    } catch (const std::exception & e) {
            LOG_FATAL << "Unhandled exception in main thread: " << e.what();
            std::this_thread::sleep_for(50ms);
    } catch (...) {
            auto exception = std::current_exception();
            std::string explanation = typeid(exception).name();
            LOG_FATAL << "Unhandled exception in main thread: " << explanation;

            std::this_thread::sleep_for(50ms);
        }
    }

    // Stop the acquisition thread
    LOG_DEBUG << "Stopping threads...";
    serialHandler->stop();

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
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
