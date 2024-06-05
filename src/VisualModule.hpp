//
//  VisualContent.hpp
//  fonema video
//
//  Created by Ted Moore on 12/26/20.
//

#ifndef VisualContent_hpp
#define VisualContent_hpp

#include <stdio.h>

#include <iostream>
#include <unordered_map>
// #include "ofMain.h"

enum VIS_TYPE { NONE = 0,
                HAP,
                WAVEFORM,
                MESH,
                LINES,
                TURTLE };

class VisualModule {
   public:
    VIS_TYPE type = NONE;
    float newParamsProb = 1.f;

    virtual void setup(SystemState &s, ofJson &config) = 0;
    virtual void interact(SystemState &s, VisualModule *other) = 0;
    virtual void receiveOSC(SystemState &s, std::string label, float val) = 0;
    virtual void display(SystemState &s) = 0;
    virtual void screenResize(SystemState &s) = 0;
    virtual void newParams(SystemState &s) = 0;
    virtual void update(SystemState &s) = 0;
    virtual void printStatus() = 0;
    virtual ofJson saveState() = 0;
    virtual void loadState(SystemState &s, ofJson &dict) = 0;
    virtual string getName() = 0;
};

#endif /* VisualContent_hpp */
