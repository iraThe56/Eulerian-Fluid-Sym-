#include "ImguiManager.h"

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "GLFW/glfw3.h"

ImguiManager* ImguiManager::instance = nullptr;

ImguiManager* ImguiManager::getInstance() {
    if (instance == nullptr) {
        instance = new ImguiManager();
    }
    return instance;
}

void ImguiManager::init(GLFWwindow *window) {
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = "opengl_imgui.ini";
}

void ImguiManager::render() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Example window
    ImGui::Begin("Hello Ira :3");
    ImGui::Text("It works!");
    ImGui::Checkbox("Should Update?", shouldUpdate);
    ImGui::Checkbox("Should Draw?", shouldDraw);

    ImGui::Checkbox("Should renderPressure?", renderPressure);
    ImGui::Checkbox("Should renderVelocity?", renderVelocity);

    if (ImGui::Button("Reset")) {
        shouldReset=true;
    }



    ImGui::DragFloat("timestep",timestep,.006);
    ImGui::DragInt("acelerationType",acelerationType,1,1,2);
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImguiManager::destroy() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}