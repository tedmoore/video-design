//
//  Waveform.hpp
//  fonema video
//
//  Created by Ted Moore on 12/26/20.
//

#ifndef Waveform_hpp
#define Waveform_hpp

#include <stdio.h>
#include "ofMain.h"
#include <VisualContent.hpp>
#include "ofxYAML.h"

class Waveform: public VisualContent {
public:
    void setup(int width, int height, float** waveforms_, int n_waveforms_, int length_, float** vecHistory, int vector_length, int history_length, bool vecHistoryFull, ofxYAML& config);
    //void update() override;
    void display(int width, int height, int frame_num, std::unordered_map<std::string, float>* common_features, bool isNRT) override;
    void newParams(int width, int height, float** vecHistory, int vector_length, int history_length, bool vecHistoryFull) override;
    //void newParams();
    void screenResize(int w, int h) override;
    void interact(VisualContent* other) override;
    void receiveOSC(int width, int height, std::string label, float val) override;
    void displayWaveform(int wf_int, int x, int y, int z, float hmul2, int display_width, int display_height);
    
    enum waveformType { NORM , LISSAJOUS , IKEDA, RECTS };
    
    waveformType wfType = NORM;
    
    int n_waveforms;
    int length;
    float** waveforms;
    
    int h;
    int* xoff;
    int* yoff;
    int* zoff;
    float* hmul;
    bool* show;
    float lissajous_line_width = 1.f;
    float waveform_line_width = 1.f;
    float ikeda_avg = 0.2;
    
//    bool lissajous = false;
    int maxNWaveforms = 2;
};

void Waveform::setup(int width, int height, float** waveforms_, int n_waveforms_, int length_, float** vecHistory, int vector_length, int history_length, bool vecHistoryFull, ofxYAML& config){
    lissajous_line_width = config["modules"]["waveform"]["lissajous-line-width"].as<float>();
    waveform_line_width = config["modules"]["waveform"]["waveform-line-width"].as<float>();
    n_waveforms = n_waveforms_;
    length = length_;
    waveforms = waveforms_;
    type = WAVEFORM;
    h = height;
    
    xoff = new int[n_waveforms];
    yoff = new int[n_waveforms];
    zoff = new int[n_waveforms];
    
    hmul = new float[n_waveforms];
    show = new bool[n_waveforms];
    
    show[0] = true;
    xoff[0] = 0;
    yoff[0] = height / 2;
    zoff[0] = 0;
    hmul[0] = 1;
    
//    bool lissajous = false;
    //int maxNWaveforms = 2;
    
    newParams(width, height, vecHistory,vector_length, history_length, vecHistoryFull);
}

//void Waveform::update(){}

void Waveform::display(int width, int height, int frame_num, std::unordered_map<std::string, float>* common_features, bool isNRT){
        
    switch(wfType){
        case LISSAJOUS:
        {
            ofSetColor(255);
            ofNoFill();
            ofSetLineWidth(lissajous_line_width);
            float w = width / 2.f;
            ofBeginShape();
            for (int i = 0; i < length * 0.1; i++) {
                ofVertex(xoff[1] + w + (waveforms[0][i] * h), yoff[1] + (waveforms[1][i] * h));
            }
            ofEndShape();
        }
            break;
        case NORM:
        {
            for (int i = 0; i < n_waveforms; i++) {
                if (show[i]) {
                    displayWaveform(i % maxNWaveforms, xoff[i], yoff[i], zoff[i], hmul[i],width,height);
                }
            }
        }
            break;
        case IKEDA:
        {
            int w = width / n_waveforms;
            float rect_height = (float)height / length;
            ofSetColor(255,pow(common_features->at("loudness"),2) * 255); // what should the ikeda alpha be
            ofSetLineWidth(0);
            float runningsum = 0;
            for (int i = 0; i < n_waveforms; i++) {
                for(int y = 0; y < length; y++){
                    float absval = abs(waveforms[i][y]);
                    runningsum += absval;
                    if(absval > ikeda_avg){
                        ofDrawRectangle(w * i, y * rect_height, w, rect_height);
                    }
                }
            }
            
            ikeda_avg = ofLerp(ikeda_avg, (runningsum / (n_waveforms * height)), 0.01);
        }
            break;
        case RECTS:
        {
            // RECTS
            float littleA = (width * height) / float(length);
            int side = ceil(sqrt(littleA));
            
            int counter = 0;
            for(int x = 0; x < width; x+= side){
                for(int y = 0; y < height; y += side){
                    ofSetColor(ofMap(waveforms[0][counter],-1.f,1.f,0,255));
                    ofDrawRectangle(x, y, side, side);
                }
            }
        }
            break;
    }
}

void Waveform::displayWaveform(int wf_int, int x, int y, int z, float hmul2, int display_width, int display_height){
    ofSetColor(255,255,255,255);
    ofNoFill();
    ofSetLineWidth(waveform_line_width);
    ofBeginShape();
    float xhop = (float)display_width / (float)length;
    for (int i = 0; i < length; i++) {
      float y2 = y + (waveforms[wf_int][i] * h * hmul2);
      float x2 = x + (i * xhop);
      ofVertex(x2, y2, -z);
    }
    ofEndShape();
}

void Waveform::newParams(int width, int height, float** vecHistory, int vector_length, int history_length, bool vecHistoryFull){

//    wfType = (waveformType)ofRandom(4.0);
    wfType = RECTS;
    
    for (int i = 0; i < n_waveforms; i++) {
        if (i > 0) {
            xoff[i] = ofRandom(-width, width);
            yoff[i] = ofRandom(0, height);
            zoff[i] = ofRandom(0, height);
            if (ofRandom(1.0) > 0.5) {
                show[i] = true;
            } else {
                show[i] = false;
            }
        }
        
        hmul[i] = ofRandom(0.3, 1.0);
    }
}

void Waveform::interact(VisualContent* other){}

void Waveform::receiveOSC(int width, int height, std::string label, float val){
    if(label == "setMaxNWaveforms"){
          maxNWaveforms = int(val);
    } else if (label == "setWaveformType"){
        wfType = (waveformType)val;
    } else if (label == "resetLissajousXY"){
          xoff[1] = 0;
          yoff[1] = height/2;
    }
}

void Waveform::screenResize(int w, int h) {
    yoff[0] = h / 2;
}

#endif /* Waveform_hpp */
