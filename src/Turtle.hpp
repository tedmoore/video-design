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
#include "VisualModule.hpp"

class Turtle : public VisualModule {
public:
    
    vector<ofVec3f> path;
    int divisor_i = 0;
    vector<int> divisors;
    int angle = 0;
    float stepSize = 30;
    int max_frames = 20;
    int frame_counter = 0;
    
    string getName(){
        return "Turtle";
    }
    
    void setup(int width, int height, float** vecHistory, int vector_length, int history_length, bool vecHistoryFull, nlohmann::json config){
        
        type = TURTLE;
        for(int i = 0; i < config["divisors"].size(); i++){
            divisors.push_back(config["divisors"][i].get<int>());
        }
        
        max_frames = config["max-frames"].get<int>();
        
        newParams(width,height,vecHistory,vector_length,history_length,vecHistoryFull,0);
    }
    
    bool onScreen(ofVec3f pt, int width, int height){
        bool a = pt.x >= 0;
        bool b = pt.x < width;
        bool c = pt.y >= 0;
        bool d = pt.y < height;
        
        return a && b && c && d;
    }
    
    void display(int width, int height, unsigned long long frame_num, std::unordered_map<std::string, float>* common_features, bool isNRT, bool verbose){
        
        if(frame_counter < max_frames){
            float angle = 360.f / divisors[divisor_i];
            float scale_factor = height / 1080.f; // 1080 is native so we scale based off that... *shrug emoji*
            
            // make n (=5) steps
            for(int i = 0; i < 5; i++){
                
                // using degrees
                int turns = int(ofRandom(divisors[divisor_i]));// how many turns of "angle" degrees to make;
                
                ofVec3f newvec(stepSize * int(ofRandom(1,4) * scale_factor),0,0);
                
                for(int j = 0; j < turns; j++){
                    newvec.rotate(0,0,angle);
                }
                
                while(!onScreen(newvec + path[path.size() - 1],width,height)){
                    newvec.rotate(0,0,angle);
                }
                
                newvec += path[path.size() - 1];
                
                path.push_back(newvec);
            }
        }
        
        frame_counter++;
        
        ofSetColor(255);
        ofNoFill();
        
        ofBeginShape();
        for (int i = 0; i < path.size(); i++) {
            ofVertex(path[i]);
        }
        ofEndShape();
    }
    
    
    void newParams(int width, int height, float** vecHistory, int vector_length, int history_length, bool vecHistoryFull, unsigned long long frame_num){
        divisor_i = int(ofRandom(divisors.size()));
        stepSize = ofRandom(70) + 30;
        restartPath(width,height);
    }
    
    void restartPath(int width, int height){
        frame_counter = 0;
        angle = 360 / divisors[divisor_i];
        path.clear();
        path.push_back(ofVec3f(ofRandom(width),ofRandom(height),0));
    }
    
    nlohmann::json saveState(){
        nlohmann::json dict;
        dict["divisor_i"] = divisor_i;
        dict["stepSize"] = stepSize;
        return dict;
    }
    
    void loadState(nlohmann::json &dict, int width, int height, float** vecHistory, int vector_length, int history_length, bool vecHistoryFull){
        divisor_i = dict["divisor_i"].get<int>();
        stepSize = dict["stepSize"].get<float>();
        restartPath(width,height);
    }
};

#endif /* Turtle_hpp */
