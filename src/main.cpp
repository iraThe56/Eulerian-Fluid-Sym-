#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include "shader.h"
#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"
#include "core/FPSCounter.h"
#include "core/GameBoard.h"
#include "core/imgui/ImguiManager.h"
#include "core/FluidSim.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 800;
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

    imgui->init(window);

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

    int bufferWidth = 300;
    int bufferHeight = 300;
    uint8_t *imageData = new uint8_t[bufferWidth * bufferHeight * 4];
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bufferWidth, bufferHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);

    glGenerateMipmap(GL_TEXTURE_2D);

    // render loop
    // -----------

    int timePassed = 0;
    int last_number=0;
    glfwInit();



 // cell state buffer
    GameBoard current_cell_buffer =  GameBoard(bufferWidth,bufferHeight);
    GameBoard last_cell_buffer =  GameBoard(bufferWidth,bufferHeight);

    FluidSim fluidsim = FluidSim(bufferWidth,bufferHeight,1);





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


        if (imgui->shouldDraw[0]) {
            int x_val=int(xpos*scaled_x);
            int y_val=int((1-ypos)*(-1*scaled_y));

            if (x_val<=window_width && x_val>=1 && y_val<=window_height && y_val>=0) {
                fluidsim.setPressureValueP(x_val,y_val,255);
                fluidsim.setPressureValueC(x_val,y_val,255);

            }


        }
        // float Kernel[9]=[1,1,1, 1,1,1, 1,1,1];



        int time=glfwGetTime();

        if (imgui->shouldUpdate[0]) {
            if (last_number<timePassed) {

                int width=current_cell_buffer.return_width();
                int height=current_cell_buffer.return_height();
                last_cell_buffer.set_current_index(0,0);
                current_cell_buffer.set_current_index(0,0);
                for (int y = 0; y < height; y++) {
                    for (int x = 0; x < width; x++) {





                    }
                }
                fluidsim.defusePressureImplicit(imgui->timestep[0]);
                fluidsim.swapCurrentArrayWithPrevious();


                last_number=timePassed;
            }
        }




        current_cell_buffer.set_current_index(0,0);
        for (int y = 0; y < bufferHeight; y++) {
            for (int x = 0; x < bufferWidth; x++) {
                int index = (y * bufferWidth + x) * 4; // 4 channels (RGBA)
                int value=fluidsim.getPressureValueC(x,y);
                // if (value>100) {
                //     value=255;
                // }

                imageData[index] = value;     // Red
                imageData[index + 1] =value ; // Green
                imageData[index + 2] = value; // Blue
                imageData[index + 3] = 255; // Alpha
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