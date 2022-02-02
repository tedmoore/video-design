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
                //                PointTM* pt;
                //                pt = new PointTM;
                //                pt->setup(xsize, ysize, xmin, xmax, ymin, ymax, zmin, zmax, zDir);
                //                points[i] = *pt;
                FlowFieldPoint* ffp;
                ffp = new FlowFieldPoint;
                ffp->setup(x, y, z, i, j, k);
                points[ijk2offset(i, j, k)] = *ffp;
            }
        }
    }
    //    points = new FlowFieldPoint***[resolution];
    //    for(int i = 0; i < resolution; i++){
    //        float x = ((xrange / resolution) * i) + xmin;
    //        points[i] = new FlowFieldPoint**[resolution];
//        for(int j = 0; j < 0; j++){
//            float y = ((yrange / resolution) * j) + ymin;
//            points[i][j] = new FlowFieldPoint*[resolution];
//            for(int k = 0; k < resolution; k++){
//                float z = ((zrange / resolution) * k) + zmin;
//                FlowFieldPoint* ffp;
//                ffp = new FlowFieldPoint;
//                ffp->setup(x,y,z,i,j,k);
//                points[i][j][k] = ffp;
//            }
//        }
//    }
    
//    for (int i = 0; i < resolution; i++) {
//        float x = ((xrange / resolution) * i) + xmin;
//        for (int j = 0; j < resolution; j++) {
//            float y = ((yrange / resolution) * j) + ymin;
//            for (int k = 0; k < resolution; k++) {
//                float z = ((zrange / resolution) * k) + zmin;
//                FlowFieldPoint* ffpt = new FlowFieldPoint;
//                ffpt->setup(x, y, z, i, j, k);
//                points[i][j][k] = ffpt;
//            }
//        }
//    }
    
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

void FlowField::update(int frame_num, std::unordered_map<std::string, float>* common_features) {
    float amp = common_features->at("amplitude");
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
