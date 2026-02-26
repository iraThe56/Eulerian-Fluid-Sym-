//
// Created by Ira Evetts on 2/13/26.
//

#include "FluidSim.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>

#include "glm/ext/quaternion_geometric.hpp"

FluidSim::FluidSim(const int width, const int height,int startingConditions) {

    // initlizing values
    sim_width=width;
    sim_height=height;
    k=1.2;
    startingCondition =startingConditions;
    //used for creating and indexing the arrays
    numDimensions=2;
    //calculating the array peramaters
    padding=10;
    paddingStyle=1;
    cellBehaviorState=0;
    acelerationBehavior=1;

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
    applyPaddingStyle(paddingStyle);
    applyStartingConditions(startingCondition);

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
    // if (x<0||x>array_width-1||y<0||y>array_height-1) {
    //     throw std::out_of_range("FluidSim::calculateVelocityIndex");
    // }
    return (y+padding) * (array_width*numDimensions) +(numDimensions*x+padding*numDimensions);
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
    velocityValuesP[calculateVelocityIndex(x,y)+dimension]=value;
}
void FluidSim::setAcelerationBehavior(int newAcelerationBehavior) {
    acelerationBehavior=newAcelerationBehavior;
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
        for (int y = 0; y < sim_height; y++) {
            for (int x = 0; x < sim_width; x++) {
                const int index=calculatePresureIndex(x,y);
                if ((cellBehavior[index]&0b00000001)!=1) {
                    pressureValuesC[index]=(pressureValuesP[index]+k*dt*((pressureValuesC[index+1]+pressureValuesC[index-1]+pressureValuesC[index-array_width]+pressureValuesC[index+array_width])/4))/(1+k*dt);
                }

            }
        }
    }
}

// void FluidSim::applyIncompressibility( int timestep) const {
//     for (int y = 0; y < sim_height; y++) {
//         for (int x = 0; x < sim_width; x++) {
//             sumOfCells=
//
//
//         }
//     }
//
//
// }


// void FluidSim::defuseVelocityImplicit(const float dt) const {
//     for (int iter = 0; iter < numSettlingIterations; iter++){
//         for (int i=0;i<sim_width;i++) {
//             for (int j=0;j<sim_height;j++) {
//                 const int index=calculateVelocityIndex(i,j);
//                 if ((cellBehavior[index]&0b00000001)!=1) {
//                     velocityValuesC[index]=(velocityValuesP[index]+k*dt*((velocityValuesC[index+2]+velocityValuesC[index-2])/2))/(1+k*dt);
//                     velocityValuesC[index+1]=(velocityValuesP[index+1]+k*dt*((velocityValuesC[index-array_width]+velocityValuesC[index+array_width])/2))/(1+k*dt);
//                 }
//
//             }
//         }
//     }
// }

void FluidSim::applyAcelerations(int dt) {
    if (1==1) {
        for (int y = 0; y < sim_height; y++) {
            for (int x = 0; x < sim_width; x++) {
                setVelocityValueP(x,y,0,(getVelocityValueC(x,y,0)));
                setVelocityValueP(x,y,1,(getVelocityValueC(x,y,1)+(3*dt)));
            }
        }
    }


}






void FluidSim::applyPaddingStyle(const int paddingStyle) const {
    if (paddingStyle == 0) {
        for (int i=0;i<pressureArrayLength;i++) {
            pressureValuesP[i]=0;
        }
        for (int i=0;i<velocityArrayLength;i++) {
            velocityValuesP[i]=0;
        }

    }
    if (paddingStyle==1) {
        for (int i=0;i<pressureArrayLength;i++) {
            pressureValuesP[i]=255;
        }
        for (int i=0;i<velocityArrayLength;i++) {
            velocityValuesP[i]=0;
        }

    }

}
void FluidSim::applyStartingConditions( int startingCondition) const {

    if (startingCondition==0) {
        for (int y = 0; y < sim_height; y++) {
            for (int x = 0; x < sim_width; x++) {
                setPressureValueP(x,y,0);
                setVelocityValueP(x,y,0,35);
                setVelocityValueP(x,y,1,35);

            }
        }
    }
    if (startingCondition==1) {
        for (int y = 0; y < sim_height; y++) {
            for (int x = 0; x < sim_width; x++) {
                setPressureValueP(x,y,(x*y%255));
                setVelocityValueP(x,y,0,(0));
                setVelocityValueP(x,y,1,(x*y%255));
            }
        }
    }
    if (startingCondition==2) {
        for (int y = 0; y < sim_height; y++) {
            for (int x = 0; x < sim_width; x++) {
                if (x<10&&y<10) {
                    setPressureValueP(x,y,255);
                    setVelocityValueP(x,y,0,35);
                    setVelocityValueP(x,y,1,35);
                }
                else {
                    setPressureValueP(x,y,0);
                    setVelocityValueP(x,y,0,35);
                    setVelocityValueP(x,y,1,35);
                }
            }
        }
    }

}

// reset the sim
// reset the sim
void FluidSim::reset() const {
    applyPaddingStyle(paddingStyle);
    applyStartingConditions(startingCondition);

    std::copy_n(velocityValuesP, velocityArrayLength, velocityValuesC);
    std::copy_n(pressureValuesP, pressureArrayLength, pressureValuesC);
}





// setting up basics