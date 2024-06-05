//
//  FlowField.hpp
//  fonema video
//
//  Created by Ted Moore on 12/30/20.
//

#ifndef FlowField_hpp
#define FlowField_hpp

#include <stdio.h>

#include "FlowFieldPoint.hpp"
#include "LagUD.hpp"
#include "ofMain.h"

struct FlowFieldParameters {
    float xsize;
    float ysize;
    float x_size_mul;
    float y_size_mul;
    float xmin;
    float xmax;
    float ymin;
    float ymax;
    float zmin;
    float zmax;
    float xrange;
    float yrange;
    float zrange;
    int zDir;
    int resolution;
};

class FlowField {
   public:
    vector<FlowFieldPoint> points;
    FlowFieldParameters ff_parameters;
    LagUD timeMul;
    float azOff;
    float elOff;

    void update(SystemState& s) {
        float time = (s.frame_num * 0.0065 * timeMul.update(s.features.sensory_dissonance * s.features.amplitude)) + 0.0001;
        float thetaRot = fmod(s.frame_num * 0.007, TWO_PI);
        float phiRot = fmod(s.frame_num * 0.009, TWO_PI);
        for (int i = 0; i < ff_parameters.resolution; i++) {
            for (int j = 0; j < ff_parameters.resolution; j++) {
                for (int k = 0; k < ff_parameters.resolution; k++) {
                    points[ijk2offset(i, j, k)].update(time, thetaRot, phiRot, azOff, elOff);
                }
            }
        }
    }

    void setup(int resolution_) {
        
        ff_parameters.resolution = resolution_;

        ff_parameters.xsize = 1;
        ff_parameters.ysize = 1;
        ff_parameters.x_size_mul = 0.6;
        ff_parameters.y_size_mul = 0.6;
        ff_parameters.xmin = -ff_parameters.xsize * ff_parameters.x_size_mul;
        ff_parameters.xmax = ff_parameters.xsize * (1 + ff_parameters.x_size_mul);
        ff_parameters.ymin = -ff_parameters.ysize * ff_parameters.y_size_mul;
        ff_parameters.ymax = ff_parameters.ysize * (1 + ff_parameters.y_size_mul);
        ff_parameters.zmin = 0;
        ff_parameters.zmax = ff_parameters.ysize;
        ff_parameters.xrange = ff_parameters.xmax - ff_parameters.xmin;
        ff_parameters.yrange = ff_parameters.ymax - ff_parameters.ymin;
        ff_parameters.zrange = ff_parameters.zmax - ff_parameters.zmin;
        ff_parameters.zDir = -1;

        azOff = ofRandom(0, 999);
        elOff = azOff + ofRandom(999, 99999);

        points.resize(ff_parameters.resolution * ff_parameters.resolution * ff_parameters.resolution);

        cout << "FlowField::setup points size: " << points.size() << "\n";

        for (int i = 0; i < ff_parameters.resolution; i++) {
            float x = ((ff_parameters.xrange / ff_parameters.resolution) * i) + ff_parameters.xmin;
            for (int j = 0; j < ff_parameters.resolution; j++) {
                float y = ((ff_parameters.yrange / ff_parameters.resolution) * j) + ff_parameters.ymin;
                for (int k = 0; k < ff_parameters.resolution; k++) {
                    float z = ((ff_parameters.zrange / ff_parameters.resolution) * k) + ff_parameters.zmin;
                    points[ijk2offset(i, j, k)].setup(x, y, z, i, j, k);
                }
            }
        }

        timeMul.setup(1.0, 0.14, 0);
    }

    int ijk2offset(int i, int j, int k) {
        return (k * ff_parameters.resolution * ff_parameters.resolution) + (j * ff_parameters.resolution) + i;
    }

    glm::vec3 getOrientationFromPos(glm::vec3 pos) {
        // println(width,height);
        // println(pos);
        // cout << pos << "\n";
        int xI = ofClamp(int(ff_parameters.resolution * ((pos.x - ff_parameters.xmin) / ff_parameters.xrange)), 0, ff_parameters.resolution - 1);
        int yI = ofClamp(int(ff_parameters.resolution * ((pos.y - ff_parameters.ymin) / ff_parameters.yrange)), 0, ff_parameters.resolution - 1);
        int zI = ofClamp(int(ff_parameters.resolution * ((pos.z - ff_parameters.zmin) / ff_parameters.zrange)), 0, ff_parameters.resolution - 1);
        //    cout << "FlowField resolution: " << resolution << "\n";
        // cout << xI << " " << yI << " " << zI << "\n\n";
        return points[ijk2offset(xI, yI, zI)].orientation;
    }

    void display() {
        for (int i = 0; i < ff_parameters.resolution; i++) {
            for (int j = 0; j < ff_parameters.resolution; j++) {
                for (int k = 0; k < ff_parameters.resolution; k++)
                    points[ijk2offset(i, j, k)].display();
            }
        }
    }
};

#endif /* FlowField_hpp */
