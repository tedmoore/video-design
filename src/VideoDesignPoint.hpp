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

class VideoDesignPoint {
public:
    
    ofVec3f pos, vel, acc;
    float xsize, ysize, zmax, xmin, xmax, ymin, ymax, zmin;
    int zDir;
    
    void setup(float xsize_, float ysize_, float xmin_, float xmax_, float ymin_, float ymax_, float zmin_, float zmax_, int zDir_) {
        pos.set(ofRandom(0, xsize), ofRandom(0, ysize), ofRandom(0, zmax));
        xsize = xsize_;
        ysize = ysize_;
        xmin = xmin_;
        xmax = xmax_;
        ymin = ymin_;
        ymax = ymax_;
        zmin = zmin_;
        zmax = zmax_;
        zDir = zDir_;
        vel.set(0, 0, 0);
        acc.set(0, 0, 0);
    }

    void applyForce(ofVec3f* force) {
        acc.operator+=(*force);
    }

    void move(float jitterMag, float velLimit) {
        vel.operator+=(acc);
        vel.limit(velLimit);
        //println(vel);
        //println(velLimit);
        pos.operator+=(vel);
        
        jitter(jitterMag);
        acc.operator*=(0.0);
    }

    void add(ofVec3f* other){
        pos.operator+=(*other);
    }

    //void setXYZnormed(float x_, float y_, float z_) {
    //    setXYZ(x_ * xsize, y_ * ysize, z_ * zmax);
    //}

    void setXYZ(float x_, float y_, float z_) {
        pos.x = x_;
        pos.y = y_;
        pos.z = z_;
        vel.operator*=(0.0);
        acc.operator*=(0.0);
    }

    void jitter(float mag) {
        pos.x += ofRandom(-mag, mag);
        pos.y += ofRandom(-mag, mag);
        pos.z += ofRandom(-mag, mag);
    }

    void checkEdges() {
        if (pos.x >= xmax) pos.x = xmin;
        if (pos.x < xmin) pos.x = xmax;
        if (pos.y >= ymax) pos.y = ymin;
        if (pos.y < ymin) pos.y = ymax;
        if (pos.z >= zmax) pos.z = zmin;
        if (pos.z < zmin) pos.z = zmax;
    }

    void display(int width, int height, float size) {
        ofDrawIcoSphere(pos.x * width, pos.y * height, pos.z * zDir * height, size);
    }

    float x() {
        return pos.x;
    }
    float y() {
        return pos.y;
    }
    float z() {
        return pos.z;
    }

    float distanceTo(VideoDesignPoint* other){
        return pos.distance(other->pos);
    }

};
#endif /* Point_hpp */
