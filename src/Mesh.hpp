//
//  Mesh.hpp
//  fonema video
//
//  Created by Ted Moore on 1/1/21.
//

#ifndef Mesh_hpp
#define Mesh_hpp

#include <stdio.h>
#include "VisualModule.hpp"
#include "LagUD.hpp"
#include "FlowField.hpp"
#include "PointTM.hpp"
#include "ofMain.h"
#include "VisualModule.hpp"
#include "Waveform.hpp"
#include "ofxYAML.h"

class Mesh: public VisualModule {
public:
    
    int nPoints;
    bool waveformTracking = false;
    FlowField* ff;
    int zDir = -1;
    LagUD velLimit;
    int waveformEffectDim = 0;
    bool useFF = false, useFFmaster = true;
    float xmin, xmax, ymin, ymax, zmin, zmax, xsize, ysize;
    int maxLines = 1500;
    PointTM* points;
    
    float line_width = 1;
    float point_size = 1;
    
    float flow_field_influence = 0.04;
    float speed = 0.025; // 0.0075
    float minSpeed = 0.01;
    LagUD jitterMag;
    float jitter_mul = 0.006;
    float dist_thresh_mul = 0.15;
    
    ofLight light;
    
    string getName(){
        return "Mesh";
    }
    
    void processConfigFile(ofxYAML& config){
        line_width = config["modules"]["mesh"]["line-width"].as<float>();
        point_size = config["modules"]["mesh"]["point-size"].as<float>();
        flow_field_influence = config["modules"]["mesh"]["flow-field-influence"].as<float>();
        speed = config["modules"]["mesh"]["speed"].as<float>();
        minSpeed = config["modules"]["mesh"]["min-speed"].as<float>();
        jitter_mul = config["modules"]["mesh"]["jitter-mul"].as<float>();
        dist_thresh_mul = config["modules"]["mesh"]["dist-thresh-mul"].as<float>();
    }

    void setup(int nPoints_, FlowField* ff_, float xmin_, float xmax_, float ymin_, float ymax_, float zmin_, float zmax_, float xsize_, float ysize_, ofxYAML& config) {

        processConfigFile(config);
        
        xmin = xmin_;
        xmax = xmax_;
        ymin = ymin_;
        ymax = ymax_;
        zmin = zmin_;
        zmax = zmax_;
        xsize = xsize_;
        ysize = ysize_;
        type = MESH;
        
        ff = ff_;
        nPoints = nPoints_;
        points = new PointTM[nPoints];

        
        for(int i = 0; i < nPoints; i++) {
            PointTM* pt;
            pt = new PointTM;
            pt->setup(xsize, ysize, xmin, xmax, ymin, ymax, zmin, zmax, zDir);
            points[i] = *pt;
        }
        
        velLimit.setup(1, 0.14, 0);
        jitterMag.setup(1,0.14,0);
        
        light.setPointLight();
        light.setPosition(0,0,0);
    }

    void newPointLocs(float** vecHistory, int vector_length, int history_length, bool vecHistoryFull) {
        if(vecHistoryFull){
            bool is_used[vector_length];
            for(int i = 0; i < vector_length; i++){
                is_used[i] = false;
            }
            int n_dims = 3;
            int choices[n_dims];
            
            for(int i = 0; i < n_dims; i++) {
                int choice;
                choice = int(ofRandom(vector_length));// there are 14 non-mfcc values
                while (is_used[choice]) {
                    choice = int(ofRandom(vector_length));
                }
                choices[i] = choice;
            }
            
            for(int i = 0; i < nPoints; i++) {
                points[i].setXYZ(vecHistory[i][choices[0]], vecHistory[i][choices[1]], vecHistory[i][choices[2]]);
            }
        } else {
            for (int i = 0; i < nPoints; i++) {
                points[i].setXYZ(ofRandom(0, xsize), ofRandom(0, ysize), ofRandom(0, zmax));
            }
        }
    }

    void newParams(int width, int height, float** vecHistory, int vector_length, int history_length, bool vecHistoryFull, unsigned long long frame_num) {
        newPointLocs(vecHistory, vector_length, history_length, vecHistoryFull);
        
        if (ofRandom(1.0) < 0.8) {
            useFF = true;
        } else {
            useFF = false;
        }
        
        waveformEffectDim = ofRandom(3);
    }
    
    ofxYAML::Node saveState(){
        ofxYAML::Node dict;
        
        dict["useFF"] = useFF;
        dict["waveformEffectDim"] = waveformEffectDim;
        
        return dict;
    }
    
    void loadState(ofxYAML::Node &dict, int width, int height, float** vecHistory, int vector_length, int history_length, bool vecHistoryFull){
        
        useFF = dict["useFF"].as<bool>();
        waveformEffectDim = dict["waveformEffectDim"].as<int>();
        
        newPointLocs(vecHistory, vector_length, history_length, vecHistoryFull);
    }

    void display(int width, int height, unsigned long long frame_num, std::unordered_map<std::string, float>* common_features, bool isNRT, bool verbose) {

//        ofEnableDepthTest();
        ofEnableLighting();
        light.enable();
        
        
        ofFill();
        
        
        
        float amp = common_features->at("loudness");
        float sensDis = common_features->at("sensoryDissonance");
        jitterMag.update(amp * jitter_mul);//amp * 0.01;//MIN(0.01, amp);
        //float jitterMag = 1;
        float distThresh = 0.02 + (sensDis * dist_thresh_mul);
        //println(specFlatness);
        int n_lines = 0;
        
        velLimit.update((amp * speed) + minSpeed);
        
        float scale_factor = height / 1080.f; // 1080 is the native so we'll scale based on that
        
        
        ofFill();
        ofSetColor(255,255);
        
        for (int i = 0; i < nPoints; i++) {
            
            if (useFF && useFFmaster) {
                ofVec3f ori = ff->getOrientationFromPos(points[i].pos);
                ori.normalize();
                ori *= speed * flow_field_influence; // this float multiplier changes the amount that the flow field affects the point's direction
                points[i].applyForce(&ori);
            }
            if (!waveformTracking) {
                points[i].move(jitterMag.value, velLimit.value);
            } else {
                points[i].move(jitterMag.value * 0.1, velLimit.value);
            }
            
            //println(velLimit.value);
            
            points[i].checkEdges();
            points[i].display(width,height,point_size * scale_factor);
            
            if (i < nPoints - 1 && n_lines < maxLines) {
                int i_lines = 0;
                for (int j = i + 1; j < nPoints; j++) {
                    
                    if (i_lines < maxLines * 0.01){
                        float dist = points[i].distanceTo(&points[j]);
                        if(dist < distThresh) {
                            float alpha = ofMap(dist,0.f,distThresh,255.f,0.f);
                            ofSetColor(255,alpha);
                            drawLine(&points[i], &points[j], dist, width, height, scale_factor);
                            n_lines++;
                            i_lines++;
                        }
                    }
                }
            }
        }
        
        waveformTracking = false;
        
        light.disable();
        ofDisableLighting();
//        ofDisableDepthTest();
    }

    void drawLine(PointTM* a, PointTM* b, float dist, int width, int height, float scale_factor) {
        ofSetLineWidth(line_width * scale_factor);
        ofDrawLine(a->x() * width, a->y() * height, a->z() * zDir * height, b->x() * width, b->y() * height, b->z() * zDir * height);
    }

    void interact(VisualModule* vc) {
        switch(vc->type){
            case WAVEFORM:
                Waveform* wf = (Waveform*) vc;
                waveformTracking = true;
                for (int i = 0; i < nPoints; i++) {

                    // first index of waveform is the x pos of this point
                    int wfAI = int(abs(points[i].x())) % wf->length;
                    // second index of waveform is one past
                    int wfBI = (wfAI + 1) % wf->length;
                    // get the values at those points
                    float wfa = wf->waveforms[0][wfAI];
                    float wfb = wf->waveforms[0][wfBI];
                    // make the y direction a result of that
                    float y = (wfb - wfa) * 0.1;
                    //println(y);
                    //if(i == 0) println(y);
                    ofVec3f offset;
                    
                    if (waveformEffectDim == 0) {
                        offset.set(y, 0, 0);
                    } else if (waveformEffectDim == 1) {
                        offset.set(0, y, 0);
                    } else {
                        offset.set(0, 0, y);
                    }
                    //force.mult(3);
                    //println(force);
                    points[i].add(&offset);
                }
                break;
        }
    }

    void receiveOSC(int width, int height, string cmd, float val) {
        if(cmd == "useFFmaster"){
            if (val > 0.5) {
                useFFmaster = true;
            } else {
                useFFmaster = false;
            }
        }
    }

};

#endif /* Mesh_hpp */
