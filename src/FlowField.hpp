//
//  FlowField.hpp
//  fonema video
//
//  Created by Ted Moore on 12/30/20.
//

#ifndef FlowField_hpp
#define FlowField_hpp

#include <stdio.h>
#include "ofMain.h"
#include "FlowFieldPoint.hpp"
#include "LagUD.hpp"

class FlowField {
public:
    void setup(int res, float xmin_, float xmax_, float ymin_, float ymax_, float zmin_, float zmax_);
    ofVec3f getOrientationFromPos(ofVec3f pos);
    void display();
    void update(int frame_num, std::unordered_map<std::string, float>* common_features);
    int ijk2offset(int i, int j, int k);
    
    int resolution;
    FlowFieldPoint* points;
    float xmin, xmax, ymin, ymax, zmin, zmax, xrange, yrange, zrange;
    LagUD timeMul;
    float azOff;
    float elOff;
    
    void update(unsigned long long frame_num, std::unordered_map<std::string, float>* common_features) {
        float amp = common_features->at("loudness");
        float diss = common_features->at("sensoryDissonance");
        float time = (frame_num * 0.0065 * timeMul.update(diss * amp)) + 0.0001;
        float thetaRot = fmod(frame_num * 0.007,TWO_PI);
        float phiRot = fmod(frame_num * 0.009,TWO_PI);
        for (int i = 0; i < resolution; i++) {
            for (int j = 0; j < resolution; j++) {
                for (int k = 0; k < resolution; k++) {
                    points[ijk2offset(i, j, k)].update(time, thetaRot, phiRot, azOff, elOff);
                }
            }
        }
    }
};

#endif /* FlowField_hpp */
