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
    
};

#endif /* FlowField_hpp */
