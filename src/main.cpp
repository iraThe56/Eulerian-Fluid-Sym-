#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include "shader.h"
#define STB_IMAGE_IMPLEMENTATION

#include <imgui.h>

#include "stb_image.h"
#include "core/FPSCounter.h"
#include "core/GameBoard.h"
#include "core/imgui/ImguiManager.h"
#include "core/FluidSim.h"
#include <algorithm>



void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
float sidebarWidth = 350;


unsigned int SCR_HEIGHT = 800;

unsigned int SCR_WIDTH = 800+sidebarWidth;

FPSCounter fpsCounter = FPSCounter();
ImguiManager* imgui = ImguiManager::getInstance();


int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    imgui->init(window,sidebarWidth);

    Shader blitShader("resources/shaders/blit.vsh", "resources/shaders/blit.fsh");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float blitvertices[] = {
        // positions       // texture coords
        -1.0f,  -1.0f, 0.0f, 0.0f, 0.0f, // top right
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, // bottom right
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f, // bottom left
        1.0f,  -1.0f, 0.0f, 1.0f, 0.0f // top left
};
    unsigned int blitindices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
};

    unsigned int blitVBO, blitVAO, blitEBO;
    glGenVertexArrays(1, &blitVAO);
    glGenBuffers(1, &blitVBO);
    glGenBuffers(1, &blitEBO);

    glBindVertexArray(blitVAO);

    glBindBuffer(GL_ARRAY_BUFFER, blitVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(blitvertices), blitvertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, blitEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(blitindices), blitindices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    int bufferWidth = 150;
    int bufferHeight = 150;
    uint8_t *imageData = new uint8_t[bufferWidth * bufferHeight * 4];
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bufferWidth, bufferHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);

    glGenerateMipmap(GL_TEXTURE_2D);

    // render loop
    // -----------

    int timePassed = 0;
    int last_number=0;
    glfwInit();




    GameBoard current_cell_buffer =  GameBoard(bufferWidth,bufferHeight);
    GameBoard last_cell_buffer =  GameBoard(bufferWidth,bufferHeight);

    FluidSim fluidsim = FluidSim(bufferWidth,bufferHeight,4);

    double xpos, ypos;
    int window_width, window_height;
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);

        timePassed=(glfwGetTimerValue()/(glfwGetTimerFrequency()/20));

        fpsCounter.calculate_fps();




        //getting cursor position
        glfwGetCursorPos(window, &xpos, &ypos);


        glfwGetWindowSize(window, &window_width, &window_height);

        float scaled_x =float(current_cell_buffer.return_width())/ float(window_width);
        float scaled_y =float(current_cell_buffer.return_height())/float( window_height);


        int x_val=int(xpos*scaled_x);
        // int y_val=int((1-ypos)*(-1*scaled_y));
        int y_val=int(((window_height)-ypos)*(scaled_y));
        if (imgui->shouldDraw) {


            if (x_val<=window_width && x_val>=1 && y_val<=window_height && y_val>=0) {
                fluidsim.setdyeDensityValueP(x_val,y_val,255*20);
                fluidsim.setdyeDensityValueC(x_val,y_val,255*20);

            }
        }
        if (imgui->shouldReset==true)
        {
            fluidsim.reset();
            imgui->shouldReset=false;
        }
        if (imgui->shouldResetDye==true) {
            fluidsim.setDyeToZero();
            imgui->shouldResetDye=false;

        }

        if (imgui->shouldHaveWall==true) {
            for (int y = 0; y < bufferHeight; y++) {
                for (int x = 0; x < bufferWidth; x++) {
                    if (((abs((y-y_val)*(y-y_val)+(x-x_val)*(x-x_val))))<40){
                        fluidsim.setCellBehavior(x,y,0);
                        // fluidsim.setVelocityValueC(x,y,0,0);
                        // fluidsim.setVelocityValueC(x,y,1,0);
                    }
                    else {
                        fluidsim.setCellBehavior(x,y,0b00000001);
                    }
                }
            }

        }
        else if (imgui->shouldHaveWall==false) {
            // std::cout<<"Have Wall"<<std::endl;
            for (int y = 1; y < bufferHeight; y++) {
                for (int x = 1; x < bufferWidth; x++) {
                    // if (x<bufferHeight/2+10&&x>bufferHeight/2&&y<bufferHeight-10) {
                        fluidsim.setCellBehavior(x,y,0b00000001);
                    // }
                }
            }
        }



        int time=glfwGetTime();

        if (imgui->shouldUpdate) {
            if (last_number<timePassed) {
                //
                // int width=current_cell_buffer.return_width();
                // int height=current_cell_buffer.return_height();
                // last_cell_buffer.set_current_index(0,0);
                // current_cell_buffer.set_current_index(0,0);
                // // for (int y = 0; y < height; y++) {
                // //     for (int x = 0; x < width; x++) {
                // //
                // //
                // //     }
                // // }

                // fluidsim.updateVelocity();

                fluidsim.overrideNumSettlingIterations(imgui->numOfSettlingItterations);
                fluidsim.overrideOverRelaxationValue(imgui->overRelaxationValue);





                float timeStep = imgui->timestep;


                fluidsim.defuseDyeDensityImplicit(-timeStep*30);

                // for(int i=0;i<bufferWidth-10;i++) {
                //         fluidsim.setVelocityValueP(i,bufferHeight-1,1,10);
                // }
                // for(int i=0;i<bufferWidth-10;i++) {
                //     fluidsim.setVelocityValueP(i,1,1,10);
                // }


                // for(int i=0;i<bufferWidth-0;i++) {
                //     fluidsim.setVelocityValueP(i,bufferHeight-1,0,10);
                //     fluidsim.setVelocityValueP(i,0,0,0);
                // }
                // for (int j=0;j <= bufferHeight; j++) {
                //     fluidsim.setVelocityValueP(0,j,0,0);
                //     fluidsim.setVelocityValueP(0,j,1,0);
                //     fluidsim.setVelocityValueP(bufferWidth-1,j,0,0);
                //     fluidsim.setVelocityValueP(bufferWidth-1,j,1,0);
                // }

                for (int x = 0; x < bufferWidth; x++) {
                    fluidsim.setCellBehavior(x, 0, 0);                 // Bottom Wall
                    fluidsim.setCellBehavior(x, bufferHeight - 1, 0); // Top Wall
                }
                float windSpeed = 5.0f;
                for (int y = 1; y < bufferHeight - 1; y++) {
                    // Force X-velocity at the left edge
                    fluidsim.setVelocityValueP(0, y, 0, windSpeed);
                    fluidsim.setVelocityValueC(0, y, 0, windSpeed);

                    // Continuous Smoke/Dye injection
                    if (y % 10 == 0) { // Create streamers
                        fluidsim.setdyeDensityValueP(1, y, 2000.0f);
                    }
                }
                fluidsim.advectVelocityAndDyeDensity(timeStep);
                fluidsim.applyIncompressibility(timeStep);
                fluidsim.swapCurrentArrayWithPrevious();
                last_number=timePassed;
            }
        }




        current_cell_buffer.set_current_index(0,0);
        if (imgui->renderType==0) {
            for (int y = 0; y < bufferHeight; y++) {
                for (int x = 0; x < bufferWidth; x++) {
                    int index = (y * bufferWidth + x) * 4; // 4 channels (RGBA)

                    int value=std::min(fluidsim.getDyeDensityValueC(x,y),255.0f);
                    int rvalue=value;
                    int gvalue=value;
                    int bvalue=value;

                    if (fluidsim.getCellBehavior(x,y)==0) {
                        rvalue=50;
                        gvalue=100;
                        bvalue=120;
                    }

                    imageData[index] = rvalue;     // Red
                    imageData[index + 1] =gvalue ; // Green
                    imageData[index + 2] = bvalue; // Blue
                    imageData[index + 3] = 255; // Alpha
                }
            }
        }
        else if (imgui->renderType==1) {
            for (int y = 0; y < bufferHeight; y++) {
                for (int x = 0; x < bufferWidth; x++) {
                    int index = (y * bufferWidth + x) * 4;


                    float vx = abs(fluidsim.getVelocityValueC(x,y,0)*30);
                    float vy = abs(fluidsim.getVelocityValueC(x,y,1)*30);

                    float rvalue= std::min(vx,255.0f);
                    float gvalue= std::min(vy,255.0f);
                    float bvalue= 0;
                    if (fluidsim.getCellBehavior(x,y)==0) {
                        rvalue=50;
                        gvalue=100;
                        bvalue=120;
                    }



                    imageData[index] = rvalue;     // Red (X velocity)
                    imageData[index + 1] = gvalue; // Green (Y velocity)
                    imageData[index + 2] = bvalue;      // Blue
                    imageData[index + 3] = 255;    // Alpha
                }
            }
        }
        else if (imgui->renderType==2) {

            for (int y = 0; y < bufferHeight; y++) {
                for (int x = 0; x < bufferWidth; x++) {
                    int index = (y * bufferWidth + x) * 4; // 4 channels (RGBA)
                    int result=fluidsim.getCellBehavior(x,y);
                    int gvalue=0;
                    if (result==0b000001) {
                        gvalue=255;
                    }
                    int rvalue=fluidsim.getCellBehavior(x,y);


                    imageData[index] = rvalue;     // Red
                    imageData[index + 1] =gvalue ; // Green
                    imageData[index + 2] = 0; // Blue
                    imageData[index + 3] = 255; // Alpha
                }
            }
        }



        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bufferWidth, bufferHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
        glGenerateMipmap(GL_TEXTURE_2D);

        // render
        // ------
        glClearColor(1.0f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        // render container
        blitShader.use();
        glBindVertexArray(blitVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        imgui->render();



        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &blitVAO);
    glDeleteBuffers(1, &blitVBO);
    glDeleteBuffers(1, &blitEBO);

    glDeleteTextures(1, &texture);

    delete[] imageData;


    imgui->destroy();

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}