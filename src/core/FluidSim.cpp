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
    k=10;
    startingCondition =startingConditions;
    //used for creating and indexing the arrays
    numDimensions=2;
    //calculating the array peramaters
    padding=10;
    paddingStyle=3;
    cellBehaviorState=0;
    acelerationBehavior=1;

    array_width= sim_width+2*padding;
    array_height= sim_height+2*padding;
    numSettlingIterations=10;


    // to acount for using a stagered grid we must add one to both x and y for the velcotie valyes
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


int FluidSim::calculatedyeDensityIndex(const int x, const int y) const {
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
float FluidSim::getVelocityValueP(const int x,const int y,const int dimension) const {
    return velocityValuesP[(y + padding) * ((array_width + 1) * numDimensions) + (x + padding) * numDimensions+dimension];
}

uint32_t FluidSim::getCellBehavior(const int x, const int y, const int behaviorPart) const {
    if (behaviorPart==0) {
        return cellBehavior[(y+padding) * array_width +(x+padding)];
    }
    else if (behaviorPart==1) {
        return cellBehavior[(y+padding) * array_width +(x+padding)]&0b00000001;
    }
    throw std::out_of_range("FluidSim::no get cellBehavior provided");
}



// setting values
void FluidSim::setdyeDensityValueC(const int x,const int y,const float value) const {
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



// swap buffers (shoudl only be called at the end to an update
void FluidSim::swapCurrentArrayWithPrevious() {
    std::swap(dyeDensityValuesP,dyeDensityValuesC);
    std::swap(velocityValuesP,velocityValuesC);
}


///implementing basic functions
///

void FluidSim::defuseDyeDensityExplicit(const float dt) const {
    for (int i=0;i<sim_width;i++) {
        for (int j=0;j<sim_height;j++) {

            const int index=calculatedyeDensityIndex(i,j);
            if (getCellBehavior(i,j,1)==1) {
                dyeDensityValuesC[index]=dyeDensityValuesP[index]+k*((dyeDensityValuesP[index+1]+dyeDensityValuesP[index-1]+dyeDensityValuesP[index-array_width]+dyeDensityValuesP[index+array_width])/4-dyeDensityValuesP[index])*dt;
            }
        }
    }
}
void FluidSim::defuseDyeDensityImplicit(const float dt) const {
    for (int iter = 0; iter < numSettlingIterations; iter++){
        for (int y = 0; y < sim_height; y++) {
            for (int x = 0; x < sim_width; x++) {
                const int index=(y+padding) * array_width +(x+padding);
                if (getCellBehavior(x,y,0)==1) {
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
                if (getCellBehavior(x, y, 0) == 1) {

                    // ------------------------------------------
                    // 1. Diffuse X-Velocity (Dimension 0)
                    // ------------------------------------------
                    float uOld = getVelocityValueP(x, y, 0);

                    // Get the 4 neighboring U-velocities
                    // Note: We use the getters to ensure we grab 'Dimension 0' from the neighbors
                    float uLeft   = getVelocityValueC(x - 1, y, 0);
                    float uRight  = getVelocityValueC(x + 1, y, 0);
                    float uTop    = getVelocityValueC(x, y - 1, 0);
                    float uBottom = getVelocityValueC(x, y + 1, 0);

                    float uAverage = (uLeft + uRight + uTop + uBottom) / 4.0f;

                    // Apply the implicit diffusion formula
                    float uNew = (uOld + a * uAverage) / (1.0f + a);
                    setVelocityValueC(x, y, 0, uNew);


                    // ------------------------------------------
                    // 2. Diffuse Y-Velocity (Dimension 1)
                    // ------------------------------------------
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

        // OPTIONAL BUT RECOMMENDED:
        // After every diffusion iteration, you strictly should enforce boundary conditions
        // (set walls to 0) so the diffusion doesn't "leak" non-zero values into the walls.
        // For a simple sim, doing it once at the end of the function is usually enough.
    }
}

void FluidSim::applyIncompressibility( int timestep) const {
    for (int itteration_number=0;itteration_number<numSettlingIterations;itteration_number++) {
        for (int y = 0; y < sim_height; y++) {
            for (int x = 0; x < sim_width; x++) {
                int numOfNearbyActiveCells=0;
                if (getCellBehavior(x,y,0)==1) {


                //cheeck sorunoung 4 cells
                numOfNearbyActiveCells += getCellBehavior(x-1, y, 1);
                numOfNearbyActiveCells += getCellBehavior(x+1, y, 1);
                numOfNearbyActiveCells += getCellBehavior(x, y-1, 1);
                numOfNearbyActiveCells += getCellBehavior(x, y+1, 1);


                float leftVelocity   = getVelocityValueC(x, y, 0);
                float rightVelocity  = getVelocityValueC(x+1, y, 0);
                float topVelocity    = getVelocityValueC(x, y, 1);
                float bottomVelocity = getVelocityValueC(x, y+1, 1);

                float divergence = overRelxationValue*((rightVelocity - leftVelocity) + (bottomVelocity - topVelocity));

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


}

float FluidSim::interpolateDensity(float x, float y) const {
    // 1. Shift coordinates because density lives at cell centers (0.5 offset)
    float shiftedX = x - 0.5f;
    float shiftedY = y - 0.5f;

    // 2. Find the array index of the top-left cell center
    int i0 = std::floor(shiftedX);
    int j0 = std::floor(shiftedY);
    int i1 = i0 + 1;
    int j1 = j0 + 1;

    // 3. Calculate the interpolation weights (fractional part)
    float tx = shiftedX - (float)i0;
    float ty = shiftedY - (float)j0;

    // 4. Clamp indices to prevent reading outside the simulation bounds
    // (Your getters handle padding under the hood, so 0 to sim_width-1 is correct here)
    i0 = std::clamp(i0, 0, sim_width - 1);
    i1 = std::clamp(i1, 0, sim_width - 1);
    j0 = std::clamp(j0, 0, sim_height - 1);
    j1 = std::clamp(j1, 0, sim_height - 1);

    // 5. Fetch the 4 surrounding density (pressure) values
    float d00 = getDyeDensityValueP(i0, j0); // Top-Left
    float d10 = getDyeDensityValueP(i1, j0); // Top-Right
    float d01 = getDyeDensityValueP(i0, j1); // Bottom-Left
    float d11 = getDyeDensityValueP(i1, j1); // Bottom-Right

    // 6. Bilinear Interpolation
    float top    = d00 + tx * (d10 - d00);
    float bottom = d01 + tx * (d11 - d01);
    float finalValue = top + ty * (bottom - top);

    return finalValue;
}


float FluidSim::interpolateVelocity(float x, float y, int dimension) const {
    float shiftedX = x;
    float shiftedY = y;

    // Apply the MAC grid offset based on which velocity face we are interpolating
    if (dimension == 0) {
        // X-Velocity: centered on the Y-axis (offset by 0.5 in Y)
        shiftedY -= 0.5f;
    } else if (dimension == 1) {
        // Y-Velocity: centered on the X-axis (offset by 0.5 in X)
        shiftedX -= 0.5f;
    }

    // Find the array index of the top-left velocity face
    int i0 = std::floor(shiftedX);
    int j0 = std::floor(shiftedY);
    int i1 = i0 + 1;
    int j1 = j0 + 1;

    // Calculate the interpolation weights
    float tx = shiftedX - (float)i0;
    float ty = shiftedY - (float)j0;

    // Clamp indices.
    // Because the MAC grid velocity arrays are (sim_width + 1) and (sim_height + 1),
    // the valid face indices go from 0 up to EXACTLY sim_width and sim_height.
    int maxX = sim_width;
    int maxY = sim_height;

    // (Optional optimization: you can tighten these bounds based on the dimension,
    // e.g., max X-velocity index is sim_width, but max Y-index for X-vel is sim_height - 1.
    // But because of your padding setup, just clamping to sim_width/sim_height is perfectly safe.)
    i0 = std::clamp(i0, 0, maxX);
    i1 = std::clamp(i1, 0, maxX);
    j0 = std::clamp(j0, 0, maxY);
    j1 = std::clamp(j1, 0, maxY);

    // Fetch the 4 surrounding velocity values
    float v00 = getVelocityValueP(i0, j0, dimension);
    float v10 = getVelocityValueP(i1, j0, dimension);
    float v01 = getVelocityValueP(i0, j1, dimension);
    float v11 = getVelocityValueP(i1, j1, dimension);

    // Bilinear Interpolation
    float top    = v00 + tx * (v10 - v00);
    float bottom = v01 + tx * (v11 - v01);
    float finalValue = top + ty * (bottom - top);

    return finalValue;
}



void FluidSim::advectVelocityAndDyeDensity(const float timestep) const {
    // Cast timestep to float for physics math
    float dt = (float)timestep;

    for (int y = 0; y < sim_height; y++) {
        for (int x = 0; x < sim_width; x++) {

            // Skip solid cells
            if (getCellBehavior(x, y, 1) == 0) continue;

            // ---------------------------------------------------------
            // STAGE 1: Advect X-Velocity (The Left Vertical Face)
            // Physical Location: (x, y + 0.5)
            // ---------------------------------------------------------
            {
                // Current X-velocity is right here on this face
                float u = getVelocityValueP(x, y, 0);

                // Y-velocity is the average of the 4 surrounding horizontal faces
                // (This is the average you already figured out!)
                float v_avg = (getVelocityValueP(x-1, y, 1)   + getVelocityValueP(x, y, 1) +
                               getVelocityValueP(x-1, y+1, 1) + getVelocityValueP(x, y+1, 1)) * 0.25f;

                // Trace back from the PHYSICAL location (Note the +0.5 on Y!)
                float prevX = x - (u * dt);
                float prevY = (y + 0.5f) - (v_avg * dt);

                // Interpolate and save to Current state
                float newVelX = interpolateVelocity(prevX, prevY, 0);
                setVelocityValueC(x, y, 0, newVelX);
            }

            // ---------------------------------------------------------
            // STAGE 2: Advect Y-Velocity (The Top Horizontal Face)
            // Physical Location: (x + 0.5, y)
            // ---------------------------------------------------------
            {
                // Current Y-velocity is right here on this face
                float v = getVelocityValueP(x, y, 1);

                // X-velocity is the average of the 4 surrounding vertical faces
                // (We need the two to the left/right, and the two above/below relative to this edge)
                float u_avg = (getVelocityValueP(x, y, 0)   + getVelocityValueP(x+1, y, 0) +
                               getVelocityValueP(x, y-1, 0) + getVelocityValueP(x+1, y-1, 0)) * 0.25f;

                // Trace back from the PHYSICAL location (Note the +0.5 on X!)
                float prevX = (x + 0.5f) - (u_avg * dt);
                float prevY = y - (v * dt);

                // Interpolate and save to Current state
                float newVelY = interpolateVelocity(prevX, prevY, 1);
                setVelocityValueC(x, y, 1, newVelY);
            }

            // ---------------------------------------------------------
            // STAGE 3: Advect Dye/Density (The Cell Center)
            // Physical Location: (x + 0.5, y + 0.5)
            // ---------------------------------------------------------
            {
                // Velocity at the center is the average of the Left/Right faces and Top/Bottom faces
                float u_center = (getVelocityValueP(x, y, 0) + getVelocityValueP(x+1, y, 0)) * 0.5f;
                float v_center = (getVelocityValueP(x, y, 1) + getVelocityValueP(x, y+1, 1)) * 0.5f;

                // Trace back from the PHYSICAL location (Note the +0.5 on BOTH!)
                float prevX = (x + 0.5f) - (u_center * dt);
                float prevY = (y + 0.5f) - (v_center * dt);

                // Interpolate using our special density interpolator
                float newDensity = interpolateDensity(prevX, prevY);
                setdyeDensityValueC(x, y, newDensity);
            }
        }
    }
}



// void FluidSim::applyAcelerations(const float dt) const {
//     if (1==1) {
//         for (int y = 0; y < sim_height; y++) {
//             for (int x = 0; x < sim_width; x++) {
//                 // setVelocityValueC(x,y,0,(getVelocityValueP(x,y,0)));
//                 // setVelocityValueC(x,y,1,(getVelocityValueP(x,y,1)+(3*dt)));
//             }
//         }
//     }
//
//
// }







void FluidSim::applyPaddingStyle(const int paddingStyle) const {
    if (paddingStyle == 0) {
        for (int i=0;i<dyeDensityArrayLength;i++) {
            dyeDensityValuesP[i]=0;
            cellBehavior[i] = 0;
        }
        for (int i=0;i<velocityArrayLength;i++) {
            velocityValuesP[i]=0;

        }

    }
    if (paddingStyle==1) {
        for (int i=0;i<dyeDensityArrayLength;i++) {
            dyeDensityValuesP[i]=255;
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


// setting up basics