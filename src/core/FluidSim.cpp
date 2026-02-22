//
// Created by Ira Evetts on 2/13/26.
//

#include "FluidSim.h"

#include <algorithm>
#include <stdexcept>

#include "glm/ext/quaternion_geometric.hpp"

FluidSim::FluidSim(const int width, const int height,int startingConditions) {

    // initlizing values
    sim_width=width;
    sim_height=height;
    k=1.2;
    //used for creating and indexing the arrays
    numDimensions=2;
    //calculating the array peramaters
    padding=10;
    paddingStyle=1;
    cellBehaviorState=0;

    array_width= sim_width+2*padding;
    array_height= sim_height+2*padding;
    numSettlingIterations=10;


    // to acount for using a stagered grid we must add one to both x and y for the velcotie valyes
    // 0,0 is the top right corner
    pressureArrayLength=array_width*array_height;
    velocityArrayLength=(array_width+1)*(array_height+1)*numDimensions;


    pressureValuesC = new float[pressureArrayLength];
    velocityValuesC = new float[velocityArrayLength];
    pressureValuesP = new float[pressureArrayLength];
    velocityValuesP = new float[velocityArrayLength];

    cellBehavior = new uint32_t[pressureArrayLength];


    //setting initial values
    /// setting padding values
    // applyPaddingStyle();
    if (paddingStyle == 0) {
        for (int i=0;i<pressureArrayLength;i++) {
            pressureValuesP[i]=0;
        }
        for (int i=0;i<velocityArrayLength;i++) {
            velocityValuesP[i]=0;
        }

    }
    if (paddingStyle==1) {
        for (int i=0;i<=pressureArrayLength;i++) {
            pressureValuesP[i]=255;
        }
        for (int i=0;i<velocityArrayLength;i++) {
            velocityValuesP[i]=255;
        }

    }
    if (startingConditions==0) {
        for (int y = 0; y < sim_height; y++) {
            for (int x = 0; x < sim_width; x++) {
                setPressureValueP(x,y,0);
                setVelocityValueC(x,y,0,35);
                setVelocityValueC(x,y,1,35);

            }
        }
    }
    if (startingConditions==1) {
        for (int y = 0; y < sim_height; y++) {
            for (int x = 0; x < sim_width; x++) {
                setPressureValueP(x,y,(x*y%255));
                setVelocityValueC(x,y,0,35);
                setVelocityValueC(x,y,1,35);
            }
        }
    }
    if (startingConditions==2) {
        for (int y = 0; y < sim_height; y++) {
            for (int x = 0; x < sim_width; x++) {
                if (x<10&&y<10) {
                    setPressureValueP(x,y,255);
                    setVelocityValueC(x,y,0,35);
                    setVelocityValueC(x,y,1,35);
                }
                else {
                    setPressureValueP(x,y,0);
                    setVelocityValueC(x,y,0,35);
                    setVelocityValueC(x,y,1,35);
                }
            }
        }
    }



    std::copy_n(velocityValuesP, velocityArrayLength, velocityValuesC);
    std::copy_n(pressureValuesP, pressureArrayLength, pressureValuesC);




}
// deleater
FluidSim::~FluidSim() {
    delete [] pressureValuesC;
    delete [] velocityValuesC;
    delete [] pressureValuesP;
    delete [] velocityValuesP;
    delete [] cellBehavior;
}


int FluidSim::calculatePresureIndex(const int x, const int y) const {
    if (x<0||x>array_width-1||y<0||y>array_height-1) {
        throw std::out_of_range("FluidSim::calculatePresureIndex");
    }
    return (y+padding) * array_width +(x+padding);
}
int FluidSim::calculateVelocityIndex(const int x, const int y) const {
    if (x<0||x>array_width-1||y<0||y>array_height-1) {
        throw std::out_of_range("FluidSim::calculateVelocityIndex");
    }
    return x+1 * array_width*numDimensions + y+1;
}


///getting and setting values

// returning values
float FluidSim::getPressureValueC(const int x,const int y) const {
    return pressureValuesC[calculatePresureIndex(x,y)];
}
float FluidSim::getVelocityValueC(const int x,const int y,const int dimension) const {
    return velocityValuesC[calculateVelocityIndex(x,y)+dimension];
}
float FluidSim::getPressureValueP(const int x,const int y) const {
    return pressureValuesP[calculatePresureIndex(x,y)];
}
float FluidSim::getVelocityValueP(const int x,const int y,const int dimension) const {
    return velocityValuesP[calculateVelocityIndex(x,y)+dimension];
}
// setting values
void FluidSim::setPressureValueC(const int x,const int y,const float value) const {
    pressureValuesC[calculatePresureIndex(x,y)]=value;
}
void FluidSim::setVelocityValueC(const int x,const int y,const int dimension,const float value) const {
    velocityValuesC[calculateVelocityIndex(x,y)+dimension]=value;
}
void FluidSim::setPressureValueP(const int x,const int y,const float value) const {
    pressureValuesP[calculatePresureIndex(x,y)]=value;
}
void FluidSim::setVelocityValueP(const int x,const int y,const int dimension,const float value) const {
    velocityValuesP[calculateVelocityIndex(x,y)]=value;
}

// swap buffers (shoudl only be called at the end to an update
void FluidSim::swapCurrentArrayWithPrevious() {
    std::swap(pressureValuesP,pressureValuesC);
    std::swap(velocityValuesP,velocityValuesC);
}


///implementing basic functions
///

void FluidSim::defusePressureExplicit(const float dt) const {
    for (int i=0;i<sim_width;i++) {
        for (int j=0;j<sim_height;j++) {

            const int index=calculatePresureIndex(i,j);
            if ((cellBehavior[index]&0b00000001)!=1) {
                pressureValuesC[index]=pressureValuesP[index]+k*((pressureValuesP[index+1]+pressureValuesP[index-1]+pressureValuesP[index-array_width]+pressureValuesP[index+array_width])/4-pressureValuesP[index])*dt;
            }
        }
    }
}
void FluidSim::defusePressureImplicit(const float dt) const {
    for (int iter = 0; iter < numSettlingIterations; iter++){
        for (int i=0;i<sim_width;i++) {
            for (int j=0;j<sim_height;j++) {
                const int index=calculatePresureIndex(i,j);
                if ((cellBehavior[index]&0b00000001)!=1) {
                    pressureValuesC[index]=(pressureValuesP[index]+k*dt*((pressureValuesC[index+1]+pressureValuesC[index-1]+pressureValuesC[index-array_width]+pressureValuesC[index+array_width])/4))/(1+k*dt);
                }

            }
        }
    }
}





void FluidSim::applyPaddingStyle() {
    if (paddingStyle==0) {
        for (int i=0;i<pressureArrayLength;i++) {
            pressureValuesC[i]=0;
        }
        for (int i=0;i<velocityArrayLength;i++) {
            velocityValuesC[i]=0;
        }

    }
    if (paddingStyle==1) {
        for (int i=0;i<pressureArrayLength;i++) {
            pressureValuesC[i]=255;
        }
        for (int i=0;i<velocityArrayLength;i++) {
            velocityValuesC[i]=0;
        }

    }

}
void FluidSim::applyStartingConditions() {

    if (startingConditions==0) {
        for (int y = 0; y < sim_height; y++) {
            for (int x = 0; x < sim_width; x++) {
                setPressureValueP(x,y,0);
                setVelocityValueC(x,y,0,35);
                setVelocityValueC(x,y,1,35);

            }
        }
    }
    if (startingConditions==1) {
        for (int y = 0; y < sim_height; y++) {
            for (int x = 0; x < sim_width; x++) {
                setPressureValueP(x,y,(x*y%255));
                setVelocityValueC(x,y,0,35);
                setVelocityValueC(x,y,1,35);
            }
        }
    }
    if (startingConditions==2) {
        for (int y = 0; y < sim_height; y++) {
            for (int x = 0; x < sim_width; x++) {
                if (x<10&&y<10) {
                    setPressureValueP(x,y,255);
                    setVelocityValueC(x,y,0,35);
                    setVelocityValueC(x,y,1,35);
                }
                else {
                    setPressureValueP(x,y,0);
                    setVelocityValueC(x,y,0,35);
                    setVelocityValueC(x,y,1,35);
                }
            }
        }
    }

}



// setting up basics