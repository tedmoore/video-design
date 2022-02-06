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
    int divisor_i = 0;
    vector<int> divisors = {3,4,6,8,9,10,12};
    float stepSize = 15;
    
    void setup(int width, int height, float** vecHistory, int vector_length, int history_length, bool vecHistoryFull){
        newParams(width,height,vecHistory,vector_length,history_length,vecHistoryFull);
    }
    
//    void interact(VisualContent* other){
//
//    }
//
//    void receiveOSC(int width, int height, std::string label, float val){
//
//    }
    
    void display(int width, int height, int frame_num, std::unordered_map<std::string, float>* common_features, bool isNRT){
//        float amp = common_features->at("amplitude");
        
        
        for(int i = 0; i < 5; i++){
            
            float angle = (int(ofRandom(1) * divisors[divisor_i]) / float(divisors[divisor_i])) * 360;
            
            cout << "angle: " << angle << endl;
            cout << "divisor: " << divisors[divisor_i] << endl;
            
            ofVec3f newvec(stepSize * int(ofRandom(1,4)),0,0);
//            newvec.normalize();
            newvec.rotate(0,0,angle);
            newvec.operator+=(path[path.size() - 1]);
            path.push_back(newvec);
        }
        
        ofSetColor(255);
        ofNoFill();
        
        ofBeginShape();
        for (int i = 0; i < path.size(); i++) {
//            cout << path[i] << endl;
            ofVertex(path[i]);
        }
        ofEndShape();
        
//        cout << "path size: " << path.size() << endl;
    }
    
//    void screenResize(int w, int h){
//
//    }
    
    void newParams(int width, int height, float** vecHistory, int vector_length, int history_length, bool vecHistoryFull){
        divisor_i = int(ofRandom(divisors.size()));
        stepSize = ofRandom(70) + 30;
        path.clear();
        path.push_back(ofVec3f(ofRandom(width),ofRandom(height),0));
    }
    
//    void update(bool isNRT){
//
//    }
    
};

#endif /* Turtle_hpp */
