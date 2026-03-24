//
// Created by Ira Evetts on 2/13/26.
//

#include "FluidSim.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>

#include "glm/ext/quaternion_geometric.hpp"

FluidSim::FluidSim(const int width, const int height,int startingConditions) {

    // initializing values
    sim_width=width;
    sim_height=height;
    k=10;
    startingCondition =startingConditions;
    //used for creating and indexing the arrays
    numDimensions=2;
    //calculating the array parameters
    padding=10;
    paddingStyle=0;
    cellBehaviorState=0;
    acelerationBehavior=1;

    array_width= sim_width+2*padding;
    array_height= sim_height+2*padding;
    numSettlingIterations=10;


    // to account for using a staggered grid we must add one to both x and y for the velocities values
    // 0,0 is the top right corner
    dyeDensityArrayLength=array_width*array_height;
    velocityArrayLength=(array_width+1)*(array_height+1)*numDimensions;


    dyeDensityValuesC = new float[dyeDensityArrayLength];
    velocityValuesC = new float[velocityArrayLength];
    dyeDensityValuesP = new float[dyeDensityArrayLength];
    velocityValuesP = new float[velocityArrayLength];

    cellBehavior = new uint32_t[dyeDensityArrayLength];



    //setting initial values
    /// setting padding values
    applyPaddingStyle(paddingStyle);
    applyStartingConditions(startingCondition);
    acelerationBehavior=0;

    std::copy_n(velocityValuesP, velocityArrayLength, velocityValuesC);
    std::copy_n(dyeDensityValuesP, dyeDensityArrayLength, dyeDensityValuesC);




}
// deleater
FluidSim::~FluidSim() {
    delete [] dyeDensityValuesC;
    delete [] velocityValuesC;
    delete [] dyeDensityValuesP;
    delete [] velocityValuesP;
    delete [] cellBehavior;
}


int FluidSim::calculateDyeDensityIndex(const int x, const int y) const {
    // if (x<0||x>array_width-1||y<0||y>array_height-1) {
    //     throw std::out_of_range("FluidSim::calculatedyeDensityIndex");
    // }
    return (y+padding) * array_width +(x+padding);
}
int FluidSim::calculateVelocityIndex(const int x, const int y) const {
    // if (x<0||x>array_width-1||y<0||y>array_height-1) {
    //     throw std::out_of_range("FluidSim::calculateVelocityIndex");
    // }
    return (y + padding) * ((array_width + 1) * numDimensions) + (x + padding) * numDimensions;
}

int FluidSim::calculateBehaviorIndex(int x, int y) const {
    // if (x<0||x>array_width-1||y<0||y>array_height-1) {
    //     throw std::out_of_range("FluidSim::calculateBehaviorIndex");
    // }
    return (y+padding) * array_width +(x+padding);
}



///getting and setting values

// returning values
 float FluidSim::getDyeDensityValueC(const int x,const int y) const {
    return dyeDensityValuesC[(y+padding) * array_width +(x+padding)];
}
 float FluidSim::getVelocityValueC(const int x,const int y,const int dimension) const {
    return velocityValuesC[(y + padding) * ((array_width + 1) * numDimensions) + (x + padding) * numDimensions+dimension];
}
 float FluidSim::getDyeDensityValueP(const int x,const int y) const {
    return dyeDensityValuesP[(y+padding) * array_width +(x+padding)];
}
inline float FluidSim::getVelocityValueP(const int x,const int y,const int dimension) const {
    return velocityValuesP[(y + padding) * ((array_width + 1) * numDimensions) + (x + padding) * numDimensions+dimension];
}

uint32_t FluidSim::getCellBehavior(const int x, const int y) const {
    // if (behaviorPart==0) {
        return cellBehavior[(y+padding) * array_width +(x+padding)];
    // }
    // else if (behaviorPart==1) {
    //     return cellBehavior[(y+padding) * array_width +(x+padding)]&0b00000001;
    // }
    // throw std::out_of_range("FluidSim::no get cellBehavior provided");
}



// setting values
inline void FluidSim::setdyeDensityValueC(const int x,const int y,const float value) const {
    dyeDensityValuesC[(y+padding) * array_width +(x+padding)]=value;
}
void FluidSim::setVelocityValueC(const int x,const int y,const int dimension,const float value) const {
    velocityValuesC[(y + padding) * ((array_width + 1) * numDimensions) + (x + padding) * numDimensions+dimension]=value;
}
void FluidSim::setdyeDensityValueP(const int x,const int y,const float value) const {
    dyeDensityValuesP[(y+padding) * array_width +(x+padding)]=value;
}
void FluidSim::setVelocityValueP(const int x,const int y,const int dimension,const float value) const {
    velocityValuesP[(y + padding) * ((array_width + 1) * numDimensions) + (x + padding) * numDimensions+dimension]=value;
}
void FluidSim::setAcelerationBehavior(int newAcelerationBehavior) {
    acelerationBehavior=newAcelerationBehavior;
}
void FluidSim::setCellBehavior(int x, int y, uint32_t value) const {
    cellBehavior[(y+padding) * array_width +(x+padding)]=value;
}



// swap buffers (should only be called at the end to an update
void FluidSim::swapCurrentArrayWithPrevious() {

    std::swap(dyeDensityValuesP,dyeDensityValuesC);
    std::swap(velocityValuesP,velocityValuesC);
}


///implementing basic functions


void FluidSim::defuseDyeDensityExplicit(const float dt) const {
    for (int i=0;i<sim_width;i++) {
        for (int j=0;j<sim_height;j++) {

            const int index=calculateDyeDensityIndex(i,j);
            if (getCellBehavior(i,j)==1) {
                dyeDensityValuesC[index]=dyeDensityValuesP[index]+k*((dyeDensityValuesP[index+1]+dyeDensityValuesP[index-1]+dyeDensityValuesP[index-array_width]+dyeDensityValuesP[index+array_width])/4-dyeDensityValuesP[index])*dt;
            }
        }
    }
}
void FluidSim::defuseDyeDensityImplicit(const float dt) const {
    for (int iter = 0; iter < numSettlingIterations; iter++){
        for (int y = 0; y < sim_height; y++) {
            for (int x = 0; x < sim_width; x++) {
                const int index=getCellBehavior(x,y);
                if (getCellBehavior(x,y)==1) {
                    dyeDensityValuesC[index]=(dyeDensityValuesP[index]+k*dt*((dyeDensityValuesC[index+1]+dyeDensityValuesC[index-1]+dyeDensityValuesC[index-array_width]+dyeDensityValuesC[index+array_width])/4))/(1+k*dt);
                }

            }
        }
    }
}

void FluidSim::defuseVelocityImplicit(const float dt) const {
    float a = k * dt; // Pre-calculate the coefficient factor

    for (int iter = 0; iter < numSettlingIterations; iter++) {
        for (int y = 0; y < sim_height; y++) {
            for (int x = 0; x < sim_width; x++) {

                // Only diffuse if this is a fluid cell
                if (getCellBehavior(x, y) == 1) {

                    // 1. Diffuse X-Velocity
                    float uOld = getVelocityValueP(x, y, 0);

                    // Get the 4 neighboring U-velocities
                    float uLeft   = getVelocityValueC(x - 1, y, 0);
                    float uRight  = getVelocityValueC(x + 1, y, 0);
                    float uTop    = getVelocityValueC(x, y - 1, 0);
                    float uBottom = getVelocityValueC(x, y + 1, 0);

                    float uAverage = (uLeft + uRight + uTop + uBottom) / 4.0f;

                    // Apply the implicit diffusion formula
                    float uNew = (uOld + a * uAverage) / (1.0f + a);
                    setVelocityValueC(x, y, 0, uNew);



                    // 2. Diffuse Y-Velocity
                    float vOld = getVelocityValueP(x, y, 1);

                    // Get the 4 neighboring V-velocities
                    float vLeft   = getVelocityValueC(x - 1, y, 1);
                    float vRight  = getVelocityValueC(x + 1, y, 1);
                    float vTop    = getVelocityValueC(x, y - 1, 1);
                    float vBottom = getVelocityValueC(x, y + 1, 1);

                    float vAverage = (vLeft + vRight + vTop + vBottom) / 4.0f;

                    // Apply the implicit diffusion formula
                    float vNew = (vOld + a * vAverage) / (1.0f + a);
                    setVelocityValueC(x, y, 1, vNew);
                }
            }
        }
    }
}

void FluidSim::applyIncompressibility( int timestep) const {
    for (int itteration_number=0;itteration_number<numSettlingIterations;itteration_number++) {
        for (int y = 0; y < sim_height; y++) {
            for (int x = 0; x < sim_width; x++) {
                int numOfNearbyActiveCells=0;

                //check if it's a cell that should be updated
                if (getCellBehavior(x,y)==1) {

                //cheeck sorunoung 4 cells
                int leftCellBehavior=getCellBehavior(x-1, y);
                int rightCellBehavior=getCellBehavior(x+1, y);
                int topCellBehavior=getCellBehavior(x, y-1);
                int bottomCellBehavior=getCellBehavior(x, y+1);

                numOfNearbyActiveCells= leftCellBehavior+rightCellBehavior+topCellBehavior+bottomCellBehavior;

                if (numOfNearbyActiveCells == 0) continue;


                const float leftVelocity   = getVelocityValueC(x, y, 0) * leftCellBehavior;
                const float rightVelocity  = getVelocityValueC(x+1, y, 0) * rightCellBehavior;
                const float topVelocity    = getVelocityValueC(x, y, 1) * topCellBehavior;      // y, not y-1
                const float bottomVelocity = getVelocityValueC(x, y+1, 1) * bottomCellBehavior; // y+1



                float divergence = overRelxationValue*((rightVelocity - leftVelocity) + (bottomVelocity - topVelocity));
                float divergenceToEachCell=divergence/numOfNearbyActiveCells;
                if (leftCellBehavior== 1) {

                    setVelocityValueC(x, y, 0, leftVelocity + divergenceToEachCell);
                } else {
                    setVelocityValueC(x, y, 0, 0.0f);
                }

                if (rightCellBehavior == 1) {
                    setVelocityValueC(x+1, y, 0, rightVelocity - divergenceToEachCell);
                } else {
                    setVelocityValueC(x+1, y, 0, 0.0f);
                }

                if (topCellBehavior == 1) {
                    setVelocityValueC(x, y, 1, topVelocity + divergenceToEachCell);
                } else {
                    setVelocityValueC(x, y, 1, 0.0f);
                }

                if (bottomCellBehavior== 1) {
                    setVelocityValueC(x, y+1, 1, bottomVelocity - divergenceToEachCell);
                } else {
                    setVelocityValueC(x, y+1, 1, 0.0f);
                }

                }
            }
        }
    }
}

float FluidSim::interpolateDensity(float x, float y) const {

    //shift coordinates because density lives at cell centers (0.5 offset)
    //(am not fully sure why the half works but my reserch said it would)
    float shiftedX = x - 0.5f;
    float shiftedY = y - 0.5f;

    //find the array index of the top-left cell center
    int leftCellX = std::floor(shiftedX);
    int topCellY = std::floor(shiftedY);
    int rightCellX = leftCellX + 1;
    int bottomCellY = topCellY + 1;

    //calculate the interpolation weights
    const float horizontalWeight = shiftedX - (float)leftCellX;
    const float verticalWeight = shiftedY - (float)topCellY;

    //clamp
    leftCellX = std::clamp(leftCellX, 0, sim_width - 1);
    rightCellX = std::clamp(rightCellX, 0, sim_width - 1);
    topCellY = std::clamp(topCellY, 0, sim_height - 1);
    bottomCellY = std::clamp(bottomCellY, 0, sim_height - 1);

    //fetch the 4 surrounding density values
    const float topLeftDensity     = getDyeDensityValueP(leftCellX, topCellY);
    const float topRightDensity    = getDyeDensityValueP(rightCellX, topCellY);
    const float bottomLeftDensity  = getDyeDensityValueP(leftCellX, bottomCellY);
    const float bottomRightDensity = getDyeDensityValueP(rightCellX, bottomCellY);

    //bilinear interpolation woop woop
    const float topInterpolatedDensity    = topLeftDensity + horizontalWeight * (topRightDensity - topLeftDensity);
    const float bottomInterpolatedDensity = bottomLeftDensity + horizontalWeight * (bottomRightDensity - bottomLeftDensity);

    const float finalInterpolatedDensity  = topInterpolatedDensity + verticalWeight * (bottomInterpolatedDensity - topInterpolatedDensity);

    return finalInterpolatedDensity;
}


float FluidSim::interpolateVelocity(float x, float y, int dimension) const {
    float shiftedX = x;
    float shiftedY = y;

    //shift coordinates to handle the MAC grid
    if (dimension == 0) {
        shiftedY -= 0.5f;
    } else if (dimension == 1) {
        shiftedX -= 0.5f;
    }

    //find the array index of the top-left velocity face
    int leftCellX = std::floor(shiftedX);
    int topCellY = std::floor(shiftedY);
    int rightCellX = leftCellX + 1;
    int bottomCellY = topCellY + 1;


    const float horizontalWeight = shiftedX - (float)leftCellX;
    const float verticalWeight = shiftedY - (float)topCellY;

    //clamp all of the cell coordinates to stay inside the grid
    const int maxX = sim_width;
    const int maxY = sim_height;

    leftCellX = std::clamp(leftCellX, 0, maxX);
    rightCellX = std::clamp(rightCellX, 0, maxX);
    topCellY = std::clamp(topCellY, 0, maxY);
    bottomCellY = std::clamp(bottomCellY, 0, maxY);

    //get the 4 surrounding velocity values
    const float topLeftVelocity     = getVelocityValueP(leftCellX, topCellY, dimension);
    const float topRightVelocity    = getVelocityValueP(rightCellX, topCellY, dimension);
    const float bottomLeftVelocity  = getVelocityValueP(leftCellX, bottomCellY, dimension);
    const float bottomRightVelocity = getVelocityValueP(rightCellX, bottomCellY, dimension);

    //bilinear interpolation
    const float topInterpolatedVelocity = topLeftVelocity + horizontalWeight * (topRightVelocity - topLeftVelocity);
    const float bottomInterpolatedVelocity = bottomLeftVelocity + horizontalWeight * (bottomRightVelocity - bottomLeftVelocity);

    const float finalInterpolatedVelocity = topInterpolatedVelocity + verticalWeight * (bottomInterpolatedVelocity - topInterpolatedVelocity);

    return finalInterpolatedVelocity;
}


void FluidSim::advectVelocityAndDyeDensity(const float timestep) const {
    // Cast timestep to float for physics math
    float dt = (float)timestep;

    for (int y = 0; y < sim_height; y++) {
        for (int x = 0; x < sim_width; x++) {
            if (getCellBehavior(x, y) == 0) continue;
            {
                float u = getVelocityValueP(x, y, 0);
                float v_avg = (getVelocityValueP(x-1, y, 1)   + getVelocityValueP(x, y, 1) +
                               getVelocityValueP(x-1, y+1, 1) + getVelocityValueP(x, y+1, 1)) * 0.25f;

                float prevX = x - (u * dt);
                float prevY = (y + 0.5f) - (v_avg * dt);

                float newVelX = interpolateVelocity(prevX, prevY, 0);
                setVelocityValueC(x, y, 0, newVelX);
            }
            {

                float v = getVelocityValueP(x, y, 1);


                float u_avg = (getVelocityValueP(x, y, 0)   + getVelocityValueP(x+1, y, 0) +
                               getVelocityValueP(x, y-1, 0) + getVelocityValueP(x+1, y-1, 0)) * 0.25f;

                float prevX = (x + 0.5f) - (u_avg * dt);
                float prevY = y - (v * dt);

                // Interpolate and save to Current state
                float newVelY = interpolateVelocity(prevX, prevY, 1);
                setVelocityValueC(x, y, 1, newVelY);
            }

            {
                // Velocity at the center is the average of the Left/Right faces and Top/Bottom faces
                float u_center = (getVelocityValueP(x, y, 0) + getVelocityValueP(x+1, y, 0)) * 0.5f;
                float v_center = (getVelocityValueP(x, y, 1) + getVelocityValueP(x, y+1, 1)) * 0.5f;

                // Trace back from the location
                float prevX = (x + 0.5f) - (u_center * dt);
                float prevY = (y + 0.5f) - (v_center * dt);

                float newDensity = interpolateDensity(prevX, prevY);
                setdyeDensityValueC(x, y, newDensity);
            }
        }
    }
}



void FluidSim::applyAcelerations(const float dt) const {
    if (acelerationBehavior==0){
        for(int i=0;i<sim_width-1;i++) {
            setVelocityValueP(i,sim_height-1,0,10*dt);
        }
    }
    if (acelerationBehavior==1){
        for (int x = 0; x < sim_width; x++) {
            setCellBehavior(x, 0, 0);                 // Bottom Wall
            setCellBehavior(x, sim_height - 1, 0); // Top Wall
        }
        float windSpeed = 5.0f;
        for (int y = 1; y < sim_height - 1; y++) {
            setVelocityValueP(0, y, 0, windSpeed*dt);
            setVelocityValueC(0, y, 0, windSpeed*dt);

        // adding dye
        if (y % 10 == 0) {
            setdyeDensityValueP(1, y, 1000.0f);
        }
        }
    }

}




void FluidSim::setDyeToZero() const {
    for (int y = 0; y < sim_height; y++) {
        for (int x = 0; x < sim_width; x++) {
            setdyeDensityValueP(x,y,0);
            setdyeDensityValueC(x,y,0);

        }
    }
}

void FluidSim::applyPaddingStyle(const int paddingStyle) const {
    if (paddingStyle == 0) {
        for (int i=0;i<dyeDensityArrayLength;i++) {
            dyeDensityValuesP[i]=0;
            cellBehavior[i] = 1;
        }
        for (int i=0;i<velocityArrayLength;i++) {
            velocityValuesP[i]=0;

        }

    }
    if (paddingStyle==1) {
        for (int i=0;i<dyeDensityArrayLength;i++) {
            dyeDensityValuesP[i]=0;
            cellBehavior[i] = 0;
        }
        for (int i=0;i<velocityArrayLength;i++) {
            velocityValuesP[i]=0;

        }

    }
    if (paddingStyle == 2) {
        for (int i = 0; i < dyeDensityArrayLength; i++) {
            dyeDensityValuesP[i] = 255;
            cellBehavior[i] = 0;
        }
        for (int i = 0; i < velocityArrayLength; i++) {
            velocityValuesP[i] = 0;
        }
    }

}
void FluidSim::applyStartingConditions( int startingCondition) const {
    if (startingCondition==0) {
        for (int y = 0; y < sim_height; y++) {
            for (int x = 0; x < sim_width; x++) {
                setdyeDensityValueP(x,y,0);
                setVelocityValueP(x,y,0,10);
                setVelocityValueP(x,y,1,10);
                setCellBehavior(x,y,0b00000001);

            }
        }
    }
    if (startingCondition==1) {
        for (int y = 0; y < sim_height; y++) {
            for (int x = 0; x < sim_width; x++) {
                setdyeDensityValueP(x,y,(x*y%255));
                setVelocityValueP(x,y,0,(0));
                setVelocityValueP(x,y,1,(x*y%255)/100);
                setCellBehavior(x,y,0b00000001);


            }
        }
    }
    if (startingCondition==2) {
        for (int y = 0; y < sim_height; y++) {
            for (int x = 0; x < sim_width; x++) {
                if (x<12&&y<12) {
                    setdyeDensityValueP(x,y,1000);
                    setVelocityValueP(x,y,0,30);
                    setVelocityValueP(x,y,1,30);
                    setCellBehavior(x,y,0b00000001);
                }
                else {
                    setdyeDensityValueP(x,y,0);
                    setVelocityValueP(x,y,0,2);
                    setVelocityValueP(x,y,1,3);
                    setCellBehavior(x,y,0b00000001);
                }
            }
        }
    }
    if (startingCondition==3) {
        for (int y = 0; y < sim_height; y++) {
            for (int x = 0; x < sim_width; x++) {
                int shiftedx=x-50;
                int shiftedy=y-50;
                if (shiftedx*shiftedx+shiftedy*shiftedy<100) {
                    setCellBehavior(x,y,0);
                }
                else {
                    setdyeDensityValueP(x,y,0);
                    setVelocityValueP(x,y,0,2);
                    setVelocityValueP(x,y,1,3);
                    setCellBehavior(x,y,0b00000001);
                }
            }
        }
    }
    if (startingCondition == 4) {


        for (int y = 0; y < sim_height; y++) {
            for (int x = 0; x < sim_width; x++) {
                // Set everything to Fluid (1) by default
                setCellBehavior(x, y, 1);

                // Set Top and Bottom rows to Wall (0) to contain the wind
                if (y == 0 || y == sim_height - 1) {
                    setCellBehavior(x, y, 0);
                    setVelocityValueP(x, y, 0, 0);
                    setVelocityValueP(x, y, 1, 0);
                } else {
                    // Initial wind speed inside the tunnel
                    setVelocityValueP(x, y, 0, 5.0f);
                    setVelocityValueP(x, y, 1, 0);
                }
                setdyeDensityValueP(x, y, 0);
            }
        }
    }

}




// reset the sim
void FluidSim::reset() const {
    applyPaddingStyle(paddingStyle);
    applyStartingConditions(startingCondition);

    std::copy_n(velocityValuesP, velocityArrayLength, velocityValuesC);
    std::copy_n(dyeDensityValuesP, dyeDensityArrayLength, dyeDensityValuesC);
}


void FluidSim::overrideNumSettlingIterations(int newNumSettlingIterations) {
    numSettlingIterations = newNumSettlingIterations;
}
void FluidSim::overrideOverRelaxationValue(float newOverRelaxationValue) {
    overRelxationValue = newOverRelaxationValue;
}


