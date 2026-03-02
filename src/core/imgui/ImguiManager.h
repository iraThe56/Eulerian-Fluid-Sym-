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


    int* renderType = new int (0);


    bool shouldReset = new bool(false);

    float* timestep= new float(0.1f);
    int* acelerationType = new int(1) ;

    float* overRelaxationValue = new float(1.9f);
    int* numOfSettlingItterations=new int(5);



    void init(GLFWwindow *window );
    void render();
    void destroy();
};

#endif //IMGUIMANAGER_H
