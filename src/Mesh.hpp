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

class Mesh: public VisualContent {
public:
    void setup(int nPoints_, FlowField* ff_, float xmin_, float xmax_, float ymin_, float ymax_, float zmin_, float zmax_, float xsize_, float ysize_);
    //void update() override;
    void display(int width, int height, int frame_num, std::unordered_map<std::string, float>* common_features, bool isNRT)  override;
    void newParams(int width, int height, float** vecHistory, int vector_length, int history_length, bool vecHistoryFull) override;
    void interact(VisualContent* other) override;
    void receiveOSC(int width, int height, std::string label, float val) override;
    void newPointLocs(float** vecHistory, int vector_length, int history_length, bool vecHistoryFull);
    void drawLine(PointTM* a, PointTM* b, float dist, int width, int height);
    
    int nPoints;
    bool waveformTracking = false;
    FlowField* ff;
    int zDir = -1;
    LagUD velLimit;
    int waveformEffectDim = 0;
    bool useFF = false, useFFmaster = true;
    float xmin, xmax, ymin, ymax, zmin, zmax, xsize, ysize;
    int maxLines = 9000;
    PointTM* points;
};

#endif /* Mesh_hpp */
