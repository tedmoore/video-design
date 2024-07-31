//
//  Point.hpp
//
//  Created by Ted Moore on 1/1/21.
//

#ifndef PointTM_hpp
#define PointTM_hpp

#include <stdio.h>

#include "ofMain.h"

class VideoDesignPoint {
   public:
    glm::vec3 pos, vel, acc;

    void setup(FlowFieldParameters& ff_parameters) {
        pos = {ofRandom(0, ff_parameters.xsize), ofRandom(0, ff_parameters.ysize), ofRandom(0, ff_parameters.zmax)};
        vel = {0, 0, 0};
        acc = {0, 0, 0};
    }

    void applyForce(glm::vec3 &force) {
        acc += force;
    }

    void move(float jitterMag, float velLimit) {
        vel += acc;
        vel = limit(vel, velLimit);
        pos += vel;
        jitter(jitterMag);
        acc *= 0.0 ;
    }

    void add(glm::vec3 &other) {
        pos += other;
    }

    void setXYZ(float x_, float y_, float z_) {
        pos.x = x_;
        pos.y = y_;
        pos.z = z_;
        vel *= 0.0;
        acc *= 0.0;
    }

    void jitter(float mag) {
        pos.x += ofRandom(-mag, mag);
        pos.y += ofRandom(-mag, mag);
        pos.z += ofRandom(-mag, mag);
    }

    void checkEdges(FlowFieldParameters& ff_parameters) {
        if (pos.x >= ff_parameters.xmax) pos.x = ff_parameters.xmin;
        if (pos.x < ff_parameters.xmin) pos.x = ff_parameters.xmax;
        if (pos.y >= ff_parameters.ymax) pos.y = ff_parameters.ymin;
        if (pos.y < ff_parameters.ymin) pos.y = ff_parameters.ymax;
        if (pos.z >= ff_parameters.zmax) pos.z = ff_parameters.zmin;
        if (pos.z < ff_parameters.zmin) pos.z = ff_parameters.zmax;
    }

    void display(SystemState& s, float size) {
        // ofSetSphereResolution(3);
        ofSetIcoSphereResolution(3);
        ofDrawIcoSphere(pos.x * s.fbo.getWidth(), pos.y * s.fbo.getHeight(), pos.z * s.flow_field->ff_parameters.zDir * s.fbo.getHeight(), size);
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

    float distanceTo(const VideoDesignPoint& other) {
        return glm::distance(pos, other.pos);
    }
};
#endif /* Point_hpp */
