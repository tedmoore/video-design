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
#include "ofxYAML.h"
//#include "ofMain.h"

enum VIS_TYPE { NONE , HAP , WAVEFORM, MESH , LINES , TURTLE };

class VisualContent {
public:
    
    virtual void interact(VisualContent* other){
        //std::cout << "VisualContent::interact\n";
    }
    virtual void receiveOSC(int width, int height, std::string label, float val){
        std::cout << "VisualContent::receiveOSC\n";
    }
    virtual void display(int width, int height, int frame_num, std::unordered_map<std::string, float>* common_features, bool isNRT){}
    virtual void screenResize(int w, int h){}
    
    virtual void newParams(int width, int height, float** vecHistory, int vector_length, int history_length, bool vecHistoryFull){}
    
    virtual void update(bool isNRT){}
    
    virtual void processConfigFile(ofxYAML& config){}
    
    virtual void printStatus(){}
    
    virtual ofxYAML save(){}
    
    virtual void load(ofxYAML &dict){}
    
    //bool alwaysUpdate = false;
    VIS_TYPE type = NONE;
    float newParamsProb = 1.f;
};

#endif /* VisualContent_hpp */
