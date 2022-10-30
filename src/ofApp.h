#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "VisualContent.hpp"
#include "HapMovie.hpp"
#include "Waveform.hpp"
#include "Mesh.hpp"
#include "Turtle.hpp"
#include "Lines.hpp"
#include "ofxCv.h"
#include "ofxOpenCv.h"
#include "ofxPostGlitch.h"
#include "ofxYAML.h"

class ofApp : public ofBaseApp{
    
public:
    
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    void newHapMovie(std::string path, int index, ofVec3f* initPts, int width, int height);
    void displayIncomingData(int width, int height);
    void onsetOccured(int width, int height);
    void drawScreen(int width, int height, int frameNum, bool isNRT);
    void setValsFromCSV(int width, int height, float* csv_data);
    void incrementVecHistoryCounter();
    
    int addVCOptions(int counter, int num){
        for(int i = 0; i < num; i++){
            vc_i_options.push_back(counter);
        };
        
        return counter + 1;
    }
    
    int nVisualContents;
    VisualContent* visual_contents[8];
    vector<int> vc_i_options;
    
    int max_active_vc = 4;
    int* active_vc_i;
    FlowField ff;
    
    // waveform data
    int n_waveforms = 2;
    int waveform_len;
    float** waveforms;
    
    // mags
    int n_magnitudes = 1;
    int magnitude_len = 513;
    float** magnitudes;
    
    // vector data
    int vector_len = 106;
    float vector_data[106];
    std::unordered_map<std::string, float> common_features;
    float** vec_history;
    int vec_history_length;
    bool vec_history_full = false;
    int vec_history_counter = 0;
    
    // other data
    int nPCAs = 4;
    float pcas[4];
    int kClusters = 4;
    int curr_cluster = 0;
    float kmeans[4];
    void setKmeansVec(int cluster,bool check_confidence);
    int previous_cluster = -1;
    int kmeans_confidence = 0;

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
    
    // OSC
    ofxOscReceiver osc_receiver;
    
    bool debug = false;
    bool onset_occured = false;
    float onsetSwitchProb = 1.f;
  
    bool nrtRender = false;
    
    ofFbo main_fbo;
    ofxPostGlitch postGlitch;
    float postGlitchProbs[GLITCH_NUM] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    float postGlitchChangeProb = 0.5;
    
    ofxYAML config;
};
