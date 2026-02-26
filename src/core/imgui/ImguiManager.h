#ifndef IMGUIMANAGER_H
#define IMGUIMANAGER_H
#include <imgui.h>

#include "GLFW/glfw3.h"

class ImguiManager {
private:
    static ImguiManager* instance;
public:
    static ImguiManager* getInstance();

    bool* shouldUpdate = new bool(false);
    bool* shouldDraw = new bool(false);

    bool* renderPressure = new bool(false);
    bool* renderVelocity = new bool(true);

    bool shouldReset = new bool(false);

    float* timestep= new float(0.1f);
    int* acelerationType = new int(1) ;



    void init(GLFWwindow *window );
    void render();
    void destroy();
};

#endif //IMGUIMANAGER_H
