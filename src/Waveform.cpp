//
//  Waveform.cpp
//  fonema video
//
//  Created by Ted Moore on 12/26/20.
//

#include "Waveform.hpp"

void Waveform::setup(int width, int height, float** waveforms_, int n_waveforms_, int length_, float** vecHistory, int vector_length, int history_length, bool vecHistoryFull, float lissajous_line_width_){
    lissajous_line_width = lissajous_line_width_;
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
    
    cout << "n waveforms " << n_waveforms << "\n";
    
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
                    displayWaveform(i % maxNWaveforms, xoff[i], yoff[i], zoff[i], hmul[i]);
                }
            }
        }
            break;
        case IKEDA:
        {
            int half_w = width / 2;
            ofSetColor(255,pow(common_features->at("amplitude"),0.5) * 255); // what should the ikeda alpha be
            ofSetLineWidth(1);
            float runningsum = 0;
            for (int i = 0; i < n_waveforms; i++) {
                for(int y = 0; y < height; y++){
                    float absval = abs(waveforms[i][y]);
                    runningsum += absval;
                    if(absval > ikeda_avg){
                        ofDrawLine(half_w * i, y, half_w * (i + 1), y);
                    }
                }
            }
            
            ikeda_avg = ofLerp(ikeda_avg, (runningsum / (n_waveforms * height)), 0.01);
        }
            break;
    }
}

void Waveform::displayWaveform(int wf_int, int x, int y, int z, float hmul2){
    ofSetColor(255,255,255,255);
    ofNoFill();
    ofSetLineWidth(1);
    ofBeginShape();
    //cout << "wf int: " << wf_int << "\t" << x << "\t" << y << "\t" << z << "\t" << hmul2 << "\n";
    for (int i = 0; i < length; i++) {
      float y2 = y + (waveforms[wf_int][i] * h * hmul2);
      float x2 = x + i;
      ofVertex(x2, y2, -z);
    }
    ofEndShape();
}

void Waveform::newParams(int width, int height, float** vecHistory, int vector_length, int history_length, bool vecHistoryFull){

//    if (ofRandom(1.0) < 0.25) {
//        lissajous = true;
//    } else {
//        lissajous = false;
//    }
    
    wfType = (waveformType)ofRandom(3.0);
    
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
        // 0 = norm
        // 1 = lissajous
        // 2 = ikeda
        
//          if (val > 0.5) {
//            lissajous = true;
//          } else {
//            lissajous = false;
//          }
    } else if (label == "resetLissajousXY"){
          xoff[1] = 0;
          yoff[1] = height/2;
    }
}

void Waveform::screenResize(int w, int h) {
    yoff[0] = h / 2;
}
