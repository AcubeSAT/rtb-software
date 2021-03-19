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
#include "Parameters.h"

const char* glsl_version = "#version 130";

using namespace std::chrono_literals;

// The number of points to include in the graph
const int GRAPH_SIZE = 300;

static void error_callback(int error, const char *description) {
    fprintf(stderr, "Error %d: %s\n", error, description);
}

std::string port;

bool dataSent = false;
bool dataReceived = false;
bool dataError = false;

bool popupOpen = false;
bool ImguiStarted = false;
bool stop = false;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

void dataAcquisition() {
    return;
//    while (!stop) {
//        std::this_thread::sleep_for(500ms);
////        LOG_INFO << "Starting data acquisition thread";
//
//        try {
//            // Serial interface initialisation
//            boost::asio::io_service io;
//            boost::asio::serial_port serial(io, port);
//            serial.set_option(boost::asio::serial_port_base::baud_rate(115200));
//
//            boost::asio::streambuf buf;
//            std::istream is(&buf);
//            std::istringstream iss;
//
//            std::string line;
//            boost::system::error_code ec;
//
////            LOG_INFO << "Connection successful";
//
//            if (popupOpen && ImguiStarted) {
//                // Close the popup
//                popupOpen = false;
//            }
//
//            // Time when the last MySQL data was sent; used to prevent too frequent updates
////        std::chrono::steady_clock::time_point last_update = std::chrono::steady_clock::now();
////        std::chrono::steady_clock::time_point last_zmq_update = std::chrono::steady_clock::now();
//            while (!stop) {
//                try {
////                if (pendingCommand != 0) {
////                    // Send pending command to arduino
////                    std::ostringstream oss;
////                    oss << pendingCommand << '\n';
////                    boost::asio::write(serial, boost::asio::buffer(oss.str()));
////                    dataSent = true;
////                    pendingCommand = 0;
////                }
//
//                    //TODO: Parallelism fixes
//                    if (!txMessages.empty()) {
//                        Message message = txMessages.back();
//                        txMessages.pop();
//
//                        auto data = MessageParser::composeECSS(message);
//
//                        // Now encode the data via COBS
//                        data.insert(data.begin(), static_cast<uint8_t>(MessageType::SpacePacket)); // Append packet type
//
//                        LOG_TRACE << "Will send " << data.size() << " bytes of data. " << data[0];
//
//                        dataSendingDB = true;
//                        uint8_t encoded[258];
//                        auto result = cobs_encode(encoded, 257, data.c_str(), data.size());
//                        encoded[result.out_len] = 0; // The null byte
//                        boost::asio::write(serial, boost::asio::buffer(encoded, result.out_len + 1));
//
//                        dataError = true;
//                    }
//
//                    boost::asio::read_until(serial, buf, '\0', ec);
//                    Logger::format.decimal();
////                LOG_TRACE << "Read " << buf.size() << " bytes of data";
//
//                    std::string receivedAll(reinterpret_cast<const char *>(buf.data().data()), buf.size());
//
//                    // Find the first occurence of a zero
//                    size_t zeroLocation = receivedAll.find('\0');
//                    std::string receivedRaw(reinterpret_cast<const char *>(buf.data().data()), zeroLocation);
//                    buf.consume(zeroLocation + 1);
//
//
//                    // Decode the received data with cobs
//                    uint8_t received[300];
//                    auto result = cobs_decode(received, 300, receivedRaw.c_str(),
//                                              receivedRaw.size()); // strip the last byte
//
//                    Logger::format.hex();
//                    if (result.status != COBS_DECODE_OK) {
//                        LOG_ERROR << "COBS status returned " << (uint8_t) result.status;
//                    }
//
//                    if (result.out_len < 1) {
//                        // Error
//                        LOG_WARNING << "Too small packet received";
//                        continue;
//                    }
//
//                    if (received[0] == Log) {
//                        // Incoming log
//                        LOG_TRACE << "[inc. log] " << std::string(reinterpret_cast<char *>(received + 1),
//                                                                  result.out_len - 2); // strip last newline
//                    } else if (received[0] == SpacePacket) {
//                        dataReceived = true;
//
//                        // Space packet
//                        std::optional<Message> message = MessageParser::parse(received + 1, std::min(static_cast<int>(result.out_len),255) - 1);
//
//                        if (message) {
//                            LOG_TRACE << "Received ECSS[" << message->serviceType << "," << message->messageType << "]";
//
//                            if (message->serviceType == 3 && message->messageType == 25) {
//                                // Housekeeping received
//                                Services.housekeeping.applyHousekeeping(*message);
//                            }
//                        } else {
//                            dataError = true;
//                        }
//                    } else if (received[0] == Ping) {
//                        // Do nothing
//                    } else {
//                        Logger::format.hex();
//                        LOG_WARNING << "Unknown data received: " << received[0] << " "
//                                    << std::string((char *) received, result.out_len);
//                    }
//
//
//
////                    float norm = sqrtf(powf(valMagx, 2) + powf(valMagy, 2) + powf(valMagz, 2));
//
//                    //valMagz *= 100;
//                    //valPressure *= 100;
//                    //valBat *= 100;
//
////        serial.close();
//                } catch (boost::system::system_error &e) {
//                    LOG_ERROR << "UART error: " << e.what();
//                }
//            }
//        } catch (boost::system::system_error &e) {
//            LOG_EMERGENCY << "Unable to open interface " << port << ": " << e.what();
//            if (!popupOpen && ImguiStarted) {
//                popupOpen = true;
//                ImGui::OpenPopup("Connection");
//            }
////        exit(5);
//        } catch (...) {
//            std::string exception = typeid(std::current_exception()).name();
//            LOG_EMERGENCY << "Unhandled exception in data acquisition thread: " << exception;
//        }
//    }
}

#pragma clang diagnostic pop


int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "You have not specified the serial interface to use. Usage: ./RadiationInterface [/dev/ttyACM0]"
                  << std::endl;
        return 5;
    }
    port = argv[1];

    std::thread dataThread(dataAcquisition);

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
                ImGui::Text("Please connect the receiver (port %s).", port.c_str());

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
            ImGui::SetNextWindowSize(ImVec2(400, 100), ImGuiCond_Always);
            ImGui::Begin("SpaceDot CubeSAT");

            ImGui::Checkbox("Test", &show_test_window);
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4({0.1f, 0.9f, 0.05f, 1.0f}));
            ImGui::Checkbox("", &dataReceived);
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4({0.9f, 0.3f, 0.05f, 1.0f}));
            ImGui::Checkbox("Data", &dataSent);
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4({0.9f, 0.4f, 0.05f, 1.0f}));
            ImGui::Checkbox("CRC", &dataError);
            ImGui::PopStyleColor();


            // Reset indicators so that they light up just for one frame
            dataReceived = dataSent = false;
            dataError = false;

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                        ImGui::GetIO().Framerate);
            ImGui::End();

            ImGui::SetNextWindowPos(ImVec2(20, 120), ImGuiCond_Appearing);
            ImGui::SetNextWindowSize(ImVec2(400, 545), ImGuiCond_Appearing);
            ImGui::Begin("Parameters");
            parameterWindow();
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
//            LOG_EMERGENCY << "Unhandled exception in main thread: " << exception;

            std::this_thread::sleep_for(50ms);
        }
    }

    // Stop the acquisition thread
//    LOG_DEBUG << "Stopping threads...";
    stop = true;

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    auto future = std::async(std::launch::async, &std::thread::join, &dataThread);
    if (future.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
//        LOG_EMERGENCY << "Could not kill data acquisition thread, terminating with force";
        std::terminate();
    }

//    LOG_NOTICE << "Threads stopped";

    return 0;
}
