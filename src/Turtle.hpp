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
    vector<int> divisors = {3,4,6,8};
    int angle = 0;
    float stepSize = 30;
    
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
    
    bool onScreen(ofVec3f pt, int width, int height){
        bool a = pt.x >= 0;
        bool b = pt.x < width;
        bool c = pt.y >= 0;
        bool d = pt.y < height;
        
//        cout << "is onscreen: " << (a && b && c && d) << endl;
        
        return a && b && c && d;
    }
    
    void display(int width, int height, int frame_num, std::unordered_map<std::string, float>* common_features, bool isNRT){
        
        float angle = 360.f / divisors[divisor_i];
        
        // make n (=5) steps
        for(int i = 0; i < 5; i++){
            
            // using degrees
            int turns = int(ofRandom(divisors[divisor_i]));// how many turns of "angle" degrees to make;
            
            ofVec3f newvec(stepSize * int(ofRandom(1,4)),0,0);
            
            for(int j = 0; j < turns; j++){
                newvec.rotate(0,0,angle);
            }

            while(!onScreen(newvec + path[path.size() - 1],width,height)){
                newvec.rotate(0,0,angle);
            }

            newvec += path[path.size() - 1];

            path.push_back(newvec);
        }
        
        ofSetColor(255);
        ofNoFill();
        
        ofBeginShape();
        for (int i = 0; i < path.size(); i++) {
            ofVertex(path[i]);
        }
        ofEndShape();
    }
    
    
    void newParams(int width, int height, float** vecHistory, int vector_length, int history_length, bool vecHistoryFull){
        divisor_i = int(ofRandom(divisors.size()));
        stepSize = ofRandom(70) + 30;
        angle = 360 / divisors[divisor_i];
        path.clear();
        path.push_back(ofVec3f(ofRandom(width),ofRandom(height),0));
    }
    
//    void update(bool isNRT){
//
//    }
    
};

#endif /* Turtle_hpp */
