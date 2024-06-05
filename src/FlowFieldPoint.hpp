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
    
    ofVec3f pos;
    ofVec3f orientation;
    int i, j, k;
    float scaler = 0.15;
    
    void setup(float x, float y, float z, int i_, int j_, int k_) {
        i = i_;
        j = j_;
        k = k_;
        pos.set(x, y, z);
        orientation.set(0, 0, 0);
        
        randomOrientation();
    }

    void randomOrientation() {
        orientation.x = ofRandom(-1.0, 1.0);
        orientation.y = ofRandom(-1.0, 1.0);
        orientation.z = ofRandom(-1.0, 1.0);
        orientation.normalize();
        orientation *= 20;
    }

    void update(float time, float thetaRot, float phiRot, float azoff, float eloff) {
        float theta = ofNoise((i*scaler) + azoff + time, (j*scaler) + azoff + time, (k*scaler) + azoff + time) * TWO_PI;
        float phi = ofNoise((i*scaler) + eloff + time, (j*scaler) + eloff + time, (k*scaler) + eloff + time) * TWO_PI;
        
        //println("time in flow_field point", time);
        theta = theta + thetaRot;
        phi = phi + phiRot;
        
        //if(i==0&&j==0&k==0) println(thetaRot,phiRot);
        
        orientation.z = sin(theta) * cos(phi);
        orientation.y = sin(theta) * sin(phi);
        orientation.x = cos(theta) * -1;
        
        orientation.normalize();
        orientation.operator*=(5);
    }

    void display() {
        ofSetLineWidth(1);
        ofSetColor(255, 0, 0);
        ofDrawLine(pos.x, pos.y, -pos.z, pos.x + orientation.x, pos.y + orientation.y, (-pos.z) + orientation.z);
        ofSetColor(255);
        ofSetLineWidth(2);
        //ofDrawSphere(pos.x, pos.y, -pos.z, 1);
        ofDrawBox(pos,1);
    }

};

#endif /* FlowFieldPoint_hpp */
