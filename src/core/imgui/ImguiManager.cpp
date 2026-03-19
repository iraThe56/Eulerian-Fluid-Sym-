#include "ImguiManager.h"
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>

ImguiManager* ImguiManager::instance = nullptr;

ImguiManager* ImguiManager::getInstance() {
    if (instance == nullptr) {
        instance = new ImguiManager();
    }
    return instance;
}

void ImguiManager::init(GLFWwindow *window,float inputed_sidebarWidth) {
    sidebarWidth = inputed_sidebarWidth;
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

    // --- DOCKING TO THE RIGHT LOGIC ---

    ImGuiViewport* viewport = ImGui::GetMainViewport();

    // Set position to the right edge of the viewport
    ImVec2 work_pos = viewport->WorkPos;
    ImVec2 work_size = viewport->WorkSize;
    ImVec2 window_pos = ImVec2(work_pos.x + work_size.x - sidebarWidth, work_pos.y);
    ImVec2 window_size = ImVec2(sidebarWidth, work_size.y);

    ImGui::SetNextWindowPos(window_pos);
    ImGui::SetNextWindowSize(window_size);

    // Window flags to make it feel "docked"
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoCollapse;
    ImGui::SetNextWindowBgAlpha(255);


    if (ImGui::Begin("Simulation Controls", nullptr, window_flags)) {
        ImGui::Text("Settings");
        ImGui::Separator();



        ImGui::Checkbox("Should Update?", &shouldUpdate);
        ImGui::Checkbox("Should Draw?", &shouldDraw);
        ImGui::Checkbox("Add the obstetrical", &shouldHaveObstical);

        ImGui::Separator();
        ImGui::SliderInt("Render type", &renderType, 0, 2);

        if (ImGui::Button("Reset")) {
            shouldReset = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset Dye")) {
            shouldResetDye = true;
        }


        ImGui::Separator();
        ImGui::DragFloat("Timestep", &timestep, 0.006f);
        ImGui::DragFloat("Over relaxation", &overRelaxationValue, 0.006f, 1.0f, 4.0f);
        ImGui::SliderInt("Settling Iterations", &numOfSettlingItterations, 3, 200);
        ImGui::SliderInt("Acceleration Type", &acelerationType, 0, 3);
        ImGui::SliderInt("padding style", &paddingStyle, 0, 2);
        ImGui::SliderInt("Starting Condition", &startingCondition, 0, 5);


 // quick acess to my demos
        if (ImGui::Button("wind tunnle")) {
            shouldReset = true;
            acelerationType=1;
            paddingStyle=0;
            startingCondition=4;
            shouldReset = true;

        }
        ImGui::SameLine();
        if (ImGui::Button("lid driven cavity")) {
            shouldReset = true;

            acelerationType=0;
            paddingStyle=1;
            startingCondition=1;
            shouldReset = true;

        }
        ImGui::SameLine();
        if (ImGui::Button("jet")) {
            shouldReset = true;

            acelerationType=2;
            paddingStyle=0;
            startingCondition=5;
            shouldReset = true;

        }


    }
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImguiManager::destroy() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}