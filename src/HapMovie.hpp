//
//  HapMovie.hpp
//  fonema video
//
//  Created by Ted Moore on 12/26/20.
//

#ifndef HapMovie_hpp
#define HapMovie_hpp

#include <stdio.h>
#include "ofMain.h"
#include <VisualContent.hpp>
#include "ofxHapPlayer.h"
#include "ofxCv.h"

#define N_MINI_COLOR_POINTS 528

#define N_CLUSTERS 4

class HapMovie: public VisualContent {
public:
    void setup(std::string path, ofVec3f pt0, ofVec3f pt1, ofVec3f pt2, ofVec3f pt3, float** mags_, int n_mag, int mag_len, bool isNRT);
    void update(bool isNRT) override;
    void display(int width, int height, int frame_num, std::unordered_map<std::string, float>* common_features, bool isNRT) override;
    void newParams(int width, int height, float** vecHistory, int vector_length, int history_length, bool vecHistoryFull) override;
    void interact(VisualContent* other) override;
    void receiveOSC(int width, int height, std::string label, float val) override;
    void screenResize(int w, int h) override;
    
    ofxHapPlayer player;
    ofTexture texture;
    ofVec3f points[4];
    ofVideoPlayer mini_vid;
    ofPixels mini_pix;
    //CvPoint3D32f color_points[N_MINI_COLOR_POINTS][3];
    //vector<cv::Point3f> color_points;
    float color_points[N_MINI_COLOR_POINTS][3];
    
    ofColor center_colors[N_CLUSTERS];
    int center_color_indices[N_CLUSTERS];
    
    bool clustered = true;
    int cluster_freq;
    
    float** mags;
    int n_mag;
    int mag_len;
    
    int alpha = 100;
    float speed = 1.f;
    float dir_options[2] = {-1.f,1.f};
    
    bool showHap, showRects;
    
    float avg_mag = 0.5;
    
    int mini_width = 32;
    int mini_height = 32;
    
    float rect_w_mul = 1.f;
    float rect_h_mul = 1.f;
//    ofVideoPlayer player;
//    ofxAVFVideoPlayer* player;
//    vector<ofxAVFVideoPlayer*> players;
//    ofxCv::FlowFarneback fb;
//    ofxCv::FlowPyrLK lk;
//    ofxCv::Flow* curFlow;
    
    vector<ofFile> tiffs;
    vector<ofFile> bitexact_tiffs;
    
    ofImage img;
    ofImage mini_img;
    
    int cluster_counter = 0;
};

#endif /* HapMovie_hpp */
