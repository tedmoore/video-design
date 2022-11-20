//
//  Mesh.hpp
//  fonema video
//
//  Created by Ted Moore on 1/1/21.
//

#ifndef Mesh_hpp
#define Mesh_hpp

#include <stdio.h>
#include "VisualContent.hpp"
#include "LagUD.hpp"
#include "FlowField.hpp"
#include "PointTM.hpp"
#include "ofMain.h"
#include "VisualContent.hpp"
#include "Waveform.hpp"
#include "ofxYAML.h"

class Mesh: public VisualContent {
public:
    void setup(int nPoints_, FlowField* ff_, float xmin_, float xmax_, float ymin_, float ymax_, float zmin_, float zmax_, float xsize_, float ysize_, ofxYAML& config);
    //void update() override;
    void display(int width, int height, int frame_num, std::unordered_map<std::string, float>* common_features, bool isNRT)  override;
    void newParams(int width, int height, float** vecHistory, int vector_length, int history_length, bool vecHistoryFull) override;
    void processConfigFile(ofxYAML& config) override;
    void interact(VisualContent* other) override;
    void receiveOSC(int width, int height, std::string label, float val) override;
    void newPointLocs(float** vecHistory, int vector_length, int history_length, bool vecHistoryFull);
    void drawLine(PointTM* a, PointTM* b, float dist, int width, int height, float scale_factor);
    
    int nPoints;
    bool waveformTracking = false;
    FlowField* ff;
    int zDir = -1;
    LagUD velLimit;
    int waveformEffectDim = 0;
    bool useFF = false, useFFmaster = true;
    float xmin, xmax, ymin, ymax, zmin, zmax, xsize, ysize;
    int maxLines = 15000;
    PointTM* points;
    
    float line_width = 1;
    float point_size = 1;
    
    float flow_field_influence = 0.04;
    float speed = 0.025; // 0.0075
    float minSpeed = 0.01;
    LagUD jitterMag;
    float jitter_mul = 0.006;
    float dist_thresh_mul = 0.15;
};

#endif /* Mesh_hpp */
