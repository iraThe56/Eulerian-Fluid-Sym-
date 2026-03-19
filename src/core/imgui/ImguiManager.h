#ifndef IMGUIMANAGER_H
#define IMGUIMANAGER_H

#include <imgui.h>
#include "GLFW/glfw3.h"

class ImguiManager {
private:
    static ImguiManager* instance;
public:
    static ImguiManager* getInstance();


    bool shouldUpdate = false;
    bool shouldDraw = false;
    bool shouldHaveObstical = false;
    int renderType = 0;
    bool shouldReset = false;
    bool shouldResetDye = false;
    float timestep = 0.3f;
    int acelerationType = 1;
    float overRelaxationValue = 1.9f;
    int numOfSettlingItterations = 60;
    float sidebarWidth;
    int paddingStyle=0;
    int startingCondition=0;

    void init(GLFWwindow *window,float sidebarWidth);
    void render();
    void destroy();
};

#endif