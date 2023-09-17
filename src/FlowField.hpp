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
    
    void setup(int res, float xmin_, float xmax_, float ymin_, float ymax_, float zmin_, float zmax_) {
        
        resolution = res;
        xmin = xmin_;
        xmax = xmax_;
        ymin = ymin_;
        ymax = ymax_;
        zmin = zmin_;
        zmax = zmax_;
        xrange = xmax - xmin;
        yrange = ymax - ymin;
        zrange = zmax - zmin;
        
        azOff = ofRandom(0, 999);
        elOff = azOff + ofRandom(999,99999);
        
        points = new FlowFieldPoint[resolution * resolution * resolution];
        
        for(int i = 0; i < resolution; i++){
            float x = ((xrange / resolution) * i) + xmin;
            for(int j = 0; j < resolution; j++){
                float y = ((yrange / resolution) * j) + ymin;
                for(int k = 0; k < resolution; k++){
                    float z = ((zrange / resolution) * k) + zmin;
                    FlowFieldPoint* ffp;
                    ffp = new FlowFieldPoint;
                    ffp->setup(x, y, z, i, j, k);
                    points[ijk2offset(i, j, k)] = *ffp;
                }
            }
        }
        
        timeMul.setup(1.0, 0.14, 0);
    }

    int ijk2offset(int i, int j, int k){
        return (k * resolution * resolution) + (j * resolution) + i;
    }

    ofVec3f getOrientationFromPos(ofVec3f pos) {
        //println(width,height);
        //println(pos);
       // cout << pos << "\n";
        int xI = ofClamp(int(resolution * ((pos.x - xmin) / xrange)), 0, resolution-1);
        int yI = ofClamp(int(resolution * ((pos.y - ymin) / yrange)), 0, resolution-1);
        int zI = ofClamp(int(resolution * ((pos.z - zmin) / zrange)), 0, resolution-1);
    //    cout << "FlowField resolution: " << resolution << "\n";
        //cout << xI << " " << yI << " " << zI << "\n\n";
        return points[ijk2offset(xI, yI, zI)].orientation;
    }

    void display() {
        for (int i = 0; i < resolution; i++) {
            for (int j = 0; j < resolution; j++) {
                for (int k = 0; k < resolution; k++)
                points[ijk2offset(i, j, k)].display();
            }
        }
    }

};

#endif /* FlowField_hpp */
