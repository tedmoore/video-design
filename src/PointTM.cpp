//
//  Point.cpp
//  fonema video
//
//  Created by Ted Moore on 1/1/21.
//

#include "PointTM.hpp"

void PointTM::setup(float xsize_, float ysize_, float xmin_, float xmax_, float ymin_, float ymax_, float zmin_, float zmax_, int zDir_) {
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

void PointTM::applyForce(ofVec3f* force) {
    acc.operator+=(*force);
}

void PointTM::move(float jitterMag, float velLimit) {
    vel.operator+=(acc);
    vel.limit(velLimit);
    //println(vel);
    //println(velLimit);
    pos.operator+=(vel);
    
    jitter(jitterMag);
    acc.operator*=(0.0);
}

void PointTM::add(ofVec3f* other){
    pos.operator+=(*other);
}

//void PointTM::setXYZnormed(float x_, float y_, float z_) {
//    setXYZ(x_ * xsize, y_ * ysize, z_ * zmax);
//}

void PointTM::setXYZ(float x_, float y_, float z_) {
    pos.x = x_;
    pos.y = y_;
    pos.z = z_;
    vel.operator*=(0.0);
    acc.operator*=(0.0);
}

void PointTM::jitter(float mag) {
    pos.x += ofRandom(-mag, mag);
    pos.y += ofRandom(-mag, mag);
    pos.z += ofRandom(-mag, mag);
}

void PointTM::checkEdges() {
    if (pos.x >= xmax) pos.x = xmin;
    if (pos.x < xmin) pos.x = xmax;
    if (pos.y >= ymax) pos.y = ymin;
    if (pos.y < ymin) pos.y = ymax;
    if (pos.z >= zmax) pos.z = zmin;
    if (pos.z < zmin) pos.z = zmax;
}

void PointTM::display(int width, int height, float size) {
    ofDrawIcoSphere(pos.x * width, pos.y * height, pos.z * zDir * height, size);
}

float PointTM::x() {
    return pos.x;
}
float PointTM::y() {
    return pos.y;
}
float PointTM::z() {
    return pos.z;
}

float PointTM::distanceTo(PointTM* other){
    return pos.distance(other->pos);
}
