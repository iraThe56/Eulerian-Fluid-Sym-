//
// Created by Ira Evetts on 1/15/26.
//

#ifndef FPSCOUNTER_H
#define FPSCOUNTER_H



class FPSCounter{

public:
    FPSCounter();
    void calculate_fps();

    int frameRate;


private:
    double lastTime ;
    double currentTime;
    int nbFrames;


};



#endif //FPSCOUNTER_H
