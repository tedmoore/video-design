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

class Waveform: public VisualContent {
public:
    void setup(int width, int height, float** waveforms_, int n_waveforms_, int length_, float** vecHistory, int vector_length, int history_length, bool vecHistoryFull);
    //void update() override;
    void display(int width, int height, int frame_num, std::unordered_map<std::string, float>* common_features, bool isNRT) override;
    void newParams(int width, int height, float** vecHistory, int vector_length, int history_length, bool vecHistoryFull) override;
    //void newParams();
    void screenResize(int w, int h) override;
    void interact(VisualContent* other) override;
    void receiveOSC(int width, int height, std::string label, float val) override;
    void displayWaveform(int wf_int, int x, int y, int z, float hmul2);
    
    enum waveformType { NORM , LISSAJOUS , IKEDA };
    
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
    
    float ikeda_avg = 0.2;
    
//    bool lissajous = false;
    int maxNWaveforms = 2;
};
#endif /* Waveform_hpp */
