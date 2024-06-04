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

    virtual void interact(VisualModule *other) = 0;
    virtual void receiveOSC(int width, int height, std::string label, float val) = 0;
    virtual void display(int width, int height, unsigned long long frame_num, std::unordered_map<std::string, float> *common_features, bool isNRT, bool verbose) = 0;
    virtual void screenResize(int w, int h) = 0;
    virtual void newParams(int width, int height, VectorHistory &vecHistory, unsigned long long frame_num) = 0;
    virtual void update(bool isNRT, std::unordered_map<std::string, float> *common_features, bool verbose) = 0;
    virtual void processConfigFile(ofJson &dict) = 0;
    virtual void printStatus() = 0;
    virtual ofJson saveState() = 0;
    virtual void loadState(ofJson &dict, int width, int height, VectorHistory &vecHistory) = 0;
    virtual string getName() = 0;
};

#endif /* VisualContent_hpp */
