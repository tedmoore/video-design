//
//  Point.hpp
//  fonema video
//
//  Created by Ted Moore on 1/1/21.
//

#ifndef PointTM_hpp
#define PointTM_hpp

#include <stdio.h>
#include "ofMain.h"

class PointTM {
public:
    void setup(float xsize_, float ysize_, float xmin_, float xmax_, float ymin_, float ymax_, float zmin_, float zmax_, int zDir_);
    void applyForce(ofVec3f* force);
    void move(float jitterMag, float velLimit);
    void add(ofVec3f* other);
    //void setXYZnormed(float x_, float y_, float z_);
    void setXYZ(float x_, float y_, float z_);
    void jitter(float mag);
    void checkEdges();
    void display(int width, int height, float size);
    float x();
    float y();
    float z();
    float distanceTo(PointTM* other);

    ofVec3f pos, vel, acc;
    float xsize, ysize, zmax, xmin, xmax, ymin, ymax, zmin;
    int zDir;
};
#endif /* Point_hpp */
