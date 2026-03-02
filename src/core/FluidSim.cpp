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
    paddingStyle=2;
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
    // if (x<0||x>array_width-1||y<0||y>array_height-1) {
    //     throw std::out_of_range("FluidSim::calculatePresureIndex");
    // }
    return (y+padding) * array_width +(x+padding);
}
int FluidSim::calculateVelocityIndex(const int x, const int y) const {
    // if (x<0||x>array_width-1||y<0||y>array_height-1) {
    //     throw std::out_of_range("FluidSim::calculateVelocityIndex");
    // }
    return (y+padding) * (array_width*numDimensions) +(numDimensions*x+padding*numDimensions);
}

int FluidSim::calculateBehaviorIndex(int x, int y) const {
    // if (x<0||x>array_width-1||y<0||y>array_height-1) {
    //     throw std::out_of_range("FluidSim::calculateBehaviorIndex");
    // }
    return (y + padding) * array_width + (x + padding);
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

uint32_t FluidSim::getCellBehavior(const int x, const int y, const int behaviorPart) const {
    if (behaviorPart==0) {
        return cellBehavior[calculateBehaviorIndex(x,y)];
    }
    else if (behaviorPart==1) {
        return cellBehavior[calculateBehaviorIndex(x,y)]&0b00000001;
    }
    throw std::out_of_range("FluidSim::no get cellBehavior provided");
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
void FluidSim::setCellBehavior(int x, int y, uint32_t value) const {
    cellBehavior[calculateBehaviorIndex(x,y)]=value;
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
            if (getCellBehavior(i,j,1)==1) {
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
                if (getCellBehavior(x,y,0)==1) {
                    pressureValuesC[index]=(pressureValuesP[index]+k*dt*((pressureValuesC[index+1]+pressureValuesC[index-1]+pressureValuesC[index-array_width]+pressureValuesC[index+array_width])/4))/(1+k*dt);
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
                //cheeck sorunoung 4 cells
                numOfNearbyActiveCells += getCellBehavior(x-1, y, 1);
                numOfNearbyActiveCells += getCellBehavior(x+1, y, 1);
                numOfNearbyActiveCells += getCellBehavior(x, y-1, 1);
                numOfNearbyActiveCells += getCellBehavior(x, y+1, 1);


                float leftVelocity   = getVelocityValueC(x, y, 0);
                float rightVelocity  = getVelocityValueC(x+1, y, 0);
                float topVelocity    = getVelocityValueC(x, y, 1);
                float bottomVelocity = getVelocityValueC(x, y+1, 1);

                float divergence = overRelxationValue*(rightVelocity - leftVelocity) + (bottomVelocity - topVelocity);

                float divergenceToEachCell=divergence/numOfNearbyActiveCells;

                if (getCellBehavior(x-1, y, 1) == 1) {
                    // Left face needs to push right (positive X direction)
                    setVelocityValueC(x, y, 0, leftVelocity + divergenceToEachCell);
                }
                if (getCellBehavior(x+1, y, 1) == 1) {
                    // Right face needs to push left (negative X direction)
                    setVelocityValueC(x+1, y, 0, rightVelocity - divergenceToEachCell);
                }
                if (getCellBehavior(x, y-1, 1) == 1) {
                    // Top face needs to push down (positive Y direction)
                    setVelocityValueC(x, y, 1, topVelocity + divergenceToEachCell);
                }
                if (getCellBehavior(x, y+1, 1) == 1) {
                    // Bottom face needs to push up (negative Y direction)
                    setVelocityValueC(x, y+1, 1, bottomVelocity - divergenceToEachCell);
                }





            }
        }
    }


}

#include <cmath>     // for std::floor
#include <algorithm> // for std::max, std::min

float FluidSim::interpolateVelocity(float x, float y, int dimension) const {
    // 1. Adjust for staggered grid offsets
    float x_adj = x;
    float y_adj = y;

    if (dimension == 0) {
        // u is at (i, j + 0.5). To make j + 0.5 look like an integer, shift query down.
        y_adj -= 0.5f;
    } else {
        // v is at (i + 0.5, j). To make i + 0.5 look like an integer, shift query left.
        x_adj -= 0.5f;
    }

    // 2. Find the bottom-left integer corner (i0, j0)
    // std::floor is safer than (int) for negative coordinates near padding
    int i0 = static_cast<int>(std::floor(x_adj));
    int j0 = static_cast<int>(std::floor(y_adj));

    // 3. Compute relative fractional distances (weights) tx, ty
    // This matches x/h and y/h in your image (assuming grid spacing h = 1.0)
    float tx = x_adj - i0;
    float ty = y_adj - j0;

    // 4. Neighboring indices
    int i1 = i0 + 1;
    int j1 = j0 + 1;

    // Safety: Clamp indices to valid array bounds (including padding)
    auto clampX = [&](int val) { return std::max(-padding, std::min(val, sim_width + padding)); };
    auto clampY = [&](int val) { return std::max(-padding, std::min(val, sim_height + padding)); };

    i0 = clampX(i0); i1 = clampX(i1);
    j0 = clampY(j0); j1 = clampY(j1);

    // 5. Fetch velocity values at the 4 corners
    float v00 = getVelocityValueP(i0, j0, dimension);
    float v10 = getVelocityValueP(i1, j0, dimension);
    float v01 = getVelocityValueP(i0, j1, dimension);
    float v11 = getVelocityValueP(i1, j1, dimension);

    // 6. Bilinear interpolation formula from your image
    // w00 = 1-tx, w01 = tx, w10 = 1-ty, w11 = ty
    float weight00 = (1.0f - tx) * (1.0f - ty);
    float weight10 = tx * (1.0f - ty);
    float weight01 = (1.0f - tx) * ty;
    float weight11 = tx * ty;

    return weight00 * v00 + weight10 * v10 + weight01 * v01 + weight11 * v11;
}

float FluidSim::interpolateDensity(float x, float y) const {
    // 1. Find the top-left integer corner (i0, j0)
    int i0 = static_cast<int>(std::floor(x));
    int j0 = static_cast<int>(std::floor(y));

    // 2. Compute fractional weights
    float tx = x - i0;
    float ty = y - j0;

    int i1 = i0 + 1;
    int j1 = j0 + 1;

    // Clamp indices to ensure they stay inside the valid array + padding
    auto clampX = [&](int val) { return std::max(-padding, std::min(val, sim_width + padding - 1)); };
    auto clampY = [&](int val) { return std::max(-padding, std::min(val, sim_height + padding - 1)); };

    i0 = clampX(i0); i1 = clampX(i1);
    j0 = clampY(j0); j1 = clampY(j1);

    // 3. Fetch "pressure" (density) values from the 4 corners
    float d00 = getPressureValueP(i0, j0);
    float d10 = getPressureValueP(i1, j0);
    float d01 = getPressureValueP(i0, j1);
    float d11 = getPressureValueP(i1, j1);

    // 4. Bilinear blend
    return (1.0f - tx) * (1.0f - ty) * d00 +
           tx * (1.0f - ty) * d10 +
           (1.0f - tx) * ty * d01 +
           tx * ty * d11;
}
// void FluidSim::advectVelocityAndPressure(const int timestep) const {
//
//     for (int y = 0; y < sim_height; y++) {
//         for (int x = 0; x < sim_width; x++) {
//
//             if (getCellBehavior(x, y, 1) == 1) continue;
//             //stage 1 x velocity
//             float velocityX=getVelocityValueP(x,y,0);
//             float velocityY=(getVelocityValueP(x-1,y,1)+getVelocityValueP(x-1,y+1,1)+getVelocityValueP(x,y,1)+getVelocityValueP(x,y+1,1))/4;
//
//             float preciousXPosition=x-(velocityX*timestep);
//             float preciousYPosition=y-(velocityY*timestep);



void FluidSim::advectVelocityAndPressure(const float timestep) const {
    float dt = static_cast<float>(timestep);

    for (int y = 0; y < sim_height; y++) {
        for (int x = 0; x < sim_width; x++) {
            if (getCellBehavior(x, y, 1) == 0) continue;

            // --- Advect X-velocity (u) ---
            // Current position of u is (x, y + 0.5)
            float u_val = getVelocityValueP(x, y, 0);
            float v_avg = (getVelocityValueP(x - 1, y, 1) + getVelocityValueP(x - 1, y + 1, 1) +
                           getVelocityValueP(x, y, 1)     + getVelocityValueP(x, y + 1, 1)) * 0.25f;

            float prevX = x - u_val * dt;
            float prevY = (y + 0.5f) - v_avg * dt;

            setVelocityValueC(x, y, 0, interpolateVelocity(prevX, prevY, 0));

            // --- Advect Y-velocity (v) ---
            // Current position of v is (x + 0.5, y)
            float v_val = getVelocityValueP(x, y, 1);
            float u_avg = (getVelocityValueP(x, y - 1, 0) + getVelocityValueP(x + 1, y - 1, 0) +
                           getVelocityValueP(x, y, 0)     + getVelocityValueP(x + 1, y, 0)) * 0.25f;

            prevX = (x + 0.5f) - u_avg * dt;
            prevY = y - v_val * dt;

            setVelocityValueC(x, y, 1, interpolateVelocity(prevX, prevY, 1));

            float u_center = (getVelocityValueP(x, y, 0) + getVelocityValueP(x + 1, y, 0)) * 0.5f;
            float v_center = (getVelocityValueP(x, y, 1) + getVelocityValueP(x, y + 1, 1)) * 0.5f;

            // 2. Trace back to find where the density came from
            float prevX_density = (float)x - u_center * dt;
            float prevY_density = (float)y - v_center * dt;

            // 3. Interpolate from previous density field and save to current
            float newDensity = interpolateDensity(prevX_density, prevY_density);
            setPressureValueC(x, y, newDensity);
        }
    }
}









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

void FluidSim::applyAcelerations(const float dt) const {
    if (1==1) {
        for (int y = 0; y < sim_height; y++) {
            for (int x = 0; x < sim_width; x++) {
                setVelocityValueC(x,y,0,(getVelocityValueP(x,y,0)));
                setVelocityValueC(x,y,1,(getVelocityValueP(x,y,1)+(3*dt)));
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
    if (paddingStyle == 2) {
        for (int i = 0; i < pressureArrayLength; i++) {
            pressureValuesP[i] = 255;
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
                setPressureValueP(x,y,0);
                setVelocityValueP(x,y,0,35);
                setVelocityValueP(x,y,1,35);
                setCellBehavior(x,y,0b00000001);

            }
        }
    }
    if (startingCondition==1) {
        for (int y = 0; y < sim_height; y++) {
            for (int x = 0; x < sim_width; x++) {
                setPressureValueP(x,y,(x*y%255));
                setVelocityValueP(x,y,0,(0));
                setVelocityValueP(x,y,1,(x*y%255));
                setCellBehavior(x,y,0b00000001);
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
                    setCellBehavior(x,y,0b00000001);
                }
                else {
                    setPressureValueP(x,y,0);
                    setVelocityValueP(x,y,0,35);
                    setVelocityValueP(x,y,1,35);
                    setCellBehavior(x,y,0b00000001);
                }
            }
        }
    }
    if (startingCondition == 3) { // Lid-Driven Cavity
        float lidSpeed = 20.0f;

        // 1. First, set EVERY velocity to zero (including padding and far edges)
        std::fill_n(velocityValuesP, velocityArrayLength, 0.0f);
        std::fill_n(cellBehavior, pressureArrayLength, 0); // All solid

        for (int y = 0; y < sim_height; y++) {
            for (int x = 0; x < sim_width; x++) {
                // Walls are the very first/last pixels of the sim area
                bool isWall = (x == 0 || x == sim_width - 1 || y == 0 || y == sim_height - 1);

                if (isWall) {
                    setCellBehavior(x, y, 0); // Solid
                    if (y == 0) {
                        // This is the top lid. Give it velocity.
                        setVelocityValueP(x, y, 0, lidSpeed);
                    }
                } else {
                    setCellBehavior(x, y, 1); // Fluid
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


void FluidSim::overrideNumSettlingIterations(int newNumSettlingIterations) {
    numSettlingIterations = newNumSettlingIterations;
}
void FluidSim::overrideOverRelaxationValue(float newOverRelaxationValue) {
    overRelxationValue = newOverRelaxationValue;
}







// setting up basics