//
// Created by Ira Evetts on 1/15/26.
//

#include "FPSCounter.h"

#include <cstdio>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

FPSCounter::FPSCounter() {
    lastTime = 0;
    currentTime = 0;
    nbFrames = 0;
    frameRate=0;
}
void FPSCounter::calculate_fps() {
    double currentTime = glfwGetTime();
    nbFrames++;
    if ( currentTime - lastTime >= 1.0 ){ // If last prinf() was more than 1 sec ago
        // printf and reset timer
        printf("%f frames/sec, %f ms/frame\n", double(nbFrames), 1000.0/double(nbFrames));
        frameRate = nbFrames;
        nbFrames = 0;
        lastTime += 1.0;
    }
}






