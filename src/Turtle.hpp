//
//  Turtle.hpp
//  modular_video_02
//
//  Created by macprocomputer on 2/2/22.
//

#ifndef Turtle_hpp
#define Turtle_hpp

#include <stdio.h>
#include "ofMain.h"
#include "VisualContent.hpp"

class Turtle : public VisualContent {
public:
    
    VIS_TYPE type = TURTLE;
    vector<ofVec3f> path;
    float divisor = 7;
    
    void interact(VisualContent* other){
        
    }
    
    void receiveOSC(int width, int height, std::string label, float val){
        
    }
    
    void display(int width, int height, int frame_num, std::unordered_map<std::string, float>* common_features, bool isNRT){
        
    }
    
    void screenResize(int w, int h){
        
    }
    
    void newParams(int width, int height, float** vecHistory, int vector_length, int history_length, bool vecHistoryFull){
        divisor = int(ofRandom(7));
        path.clear();
        path.push_back(ofVec3f(ofRandom(width),ofRandom(height),ofRandom(height));
    }
    
    void update(bool isNRT){
        
    }
    
};

#endif /* Turtle_hpp */
