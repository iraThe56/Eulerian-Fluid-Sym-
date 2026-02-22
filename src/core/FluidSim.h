//
// Created by Ira Evetts on 2/13/26.
//

#ifndef FLUIDSIM_H
#define FLUIDSIM_H
#include "glm/ext/scalar_uint_sized.hpp"


class FluidSim {
    public:
// initialization
    FluidSim(int width, int height,int startingConditions=0);
    ~FluidSim();

     float* pressureValuesC;
     float* velocityValuesC;

    float* pressureValuesP;
    float* velocityValuesP;
    uint32_t* cellBehavior;
    // first a way of telling cells how to act and if they should update
    void applyPaddingStyle(int paddingStyle) const;
    void applyStartingConditions(int startingConditions) const;
    void applyCellBehavior();



    // setting up the starting conditions and the reset function


 void reset() const;

    //calculate indexes including padding and without padding
    // int calculateIndexPadded(int x,int y) const;

    [[nodiscard]] int calculatePresureIndex(int x,int y) const;
    [[nodiscard]] int calculateVelocityIndex(int x,int y) const ;

    // returning values

    [[nodiscard]] float getPressureValueC(int x,int y) const;
    [[nodiscard]] float getVelocityValueC(int x,int y,int dimension) const;

    [[nodiscard]] float getPressureValueP(int x,int y) const;
    [[nodiscard]] float getVelocityValueP(int x,int y,int dimension) const;

    // setting values
    void setPressureValueC(int x,int y,float value) const;
    void setVelocityValueC(int x,int y,int dimension, float value) const;

    void setPressureValueP(int x,int y,float value) const;
    void setVelocityValueP(int x,int y,int dimension, float value) const;

    void swapCurrentArrayWithPrevious();



    // basic sim
    void updatePressureValues(float dt) const;
    void defusePressureExplicit(float dt) const;
    void defusePressureImplicit(float dt) const;


    // void updateVelocityValues(int x,int y,int dimension) const;
    // get nearby








// update functions
    void updateFluidSim()const;
    private:


    int padding;
    int sim_width, sim_height;
    int numCells{};

    int array_width;
    int array_height;
    int pressureArrayLength;
    int velocityArrayLength;

    int numDimensions;

    float k;
    int paddingStyle;
    int startingCondition;

    int cellBehaviorState;
    int numSettlingIterations;

};



#endif //FLUIDSIM_H
