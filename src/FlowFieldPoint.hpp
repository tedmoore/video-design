//
//  FlowFieldPoint.hpp
//  fonema video
//
//  Created by Ted Moore on 12/30/20.
//

#ifndef FlowFieldPoint_hpp
#define FlowFieldPoint_hpp

#include <stdio.h>
#include "ofMain.h"

class FlowFieldPoint {
public:
    void setup(float x, float y, float z, int i_, int j_, int k_);
    void randomOrientation();
    void update(float time, float thetaRot, float phiRot, float azoff, float eloff);
    void display();
    
    ofVec3f pos;
    ofVec3f orientation;
    int i, j, k;
    float scaler = 0.15;
};

#endif /* FlowFieldPoint_hpp */
