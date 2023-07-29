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

class VisualModule {
public:
    
    virtual void interact(VisualModule* other){
        //std::cout << "VisualContent::interact\n";
    }
    virtual void receiveOSC(int width, int height, std::string label, float val){
        std::cout << "VisualContent::receiveOSC\n";
    }
    
    virtual void display(int width, int height, unsigned long long frame_num, std::unordered_map<std::string, float>* common_features, bool isNRT, bool verbose){}
    
    virtual void screenResize(int w, int h){}
    
    virtual void newParams(int width, int height, float** vecHistory, int vector_length, int history_length, bool vecHistoryFull, unsigned long long frame_num){}
    
    virtual void update(bool isNRT, std::unordered_map<std::string, float>* common_features, bool verbose){}
    
    virtual void processConfigFile(ofxYAML& config){}
    
    virtual void printStatus(){}
    
    virtual ofxYAML::Node saveState(){
        ofxYAML::Node dict;
        return dict;
    }
    
    virtual void loadState(ofxYAML::Node &dict, int width, int height, float** vecHistory, int vector_length, int history_length, bool vecHistoryFull){}
    
    //bool alwaysUpdate = false;
    VIS_TYPE type = NONE;
    float newParamsProb = 1.f;
    
    virtual string getName(){
        return "VisualModule";
    }
};

#endif /* VisualContent_hpp */
