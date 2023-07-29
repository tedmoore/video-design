//
//  FlowField.cpp
//  fonema video
//
//  Created by Ted Moore on 12/30/20.
//

#include "FlowField.hpp"

void FlowField::setup(int res, float xmin_, float xmax_, float ymin_, float ymax_, float zmin_, float zmax_) {
    
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

int FlowField::ijk2offset(int i, int j, int k){
    return (k * resolution * resolution) + (j * resolution) + i;
}

ofVec3f FlowField::getOrientationFromPos(ofVec3f pos) {
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

void FlowField::display() {
    for (int i = 0; i < resolution; i++) {
        for (int j = 0; j < resolution; j++) {
            for (int k = 0; k < resolution; k++)
            points[ijk2offset(i, j, k)].display();
        }
    }
}
