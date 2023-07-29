//
//  Lines.hpp
//  fonema video
//
//  Created by Ted Moore on 1/6/21.
//

#ifndef Lines_hpp
#define Lines_hpp

#include <stdio.h>
#include "VisualModule.hpp"
#include "ofMain.h"
#include "HapMovie.hpp"

enum lines_direction { HORZ , VERT };

class Lines: public VisualModule {
public:

    bool inv;
    float* vec;
    float w, h;
    float vec_len;
    int alphaThresh = 10;
    bool can_borrow_colors = false;
    bool is_borrow_colors = false;
    lines_direction dir = HORZ;
    ofColor borrowed_colors[N_CLUSTERS];
    
    void setup(float* vec_, int offset_, int vec_len_, bool can_borrow_colors_, int width, int height, float** vecHistory, int vector_length, int history_length, bool vecHistoryFull){
        can_borrow_colors = can_borrow_colors_;
        vec = vec_;
        vec_len = float(vec_len_);
        
        newParams(width,height,vecHistory,vector_length,history_length,vecHistoryFull,0);
        
        type = LINES;
        
        for(int i = 0; i < N_CLUSTERS; i++){
            ofColor col(255);
            borrowed_colors[i] = col;
        }
    }

    void newParams(int width, int height, float** vecHistory, int vector_length, int history_length, bool vecHistoryFull, unsigned long long frame_num){
        chooseDir();
        chooseInv();
        is_borrow_colors = ofRandom(1.f) < 0.5;
    }
    
    ofxYAML::Node saveState(){
        ofxYAML::Node dict;
        
        dict["dir"] = (int)dir;
        dict["inv"] = inv;
        dict["is_borrow_colors"] = is_borrow_colors;
        
        return dict;
    }
    
    void loadState(ofxYAML::Node &dict, int width, int height, float** vecHistory, int vector_length, int history_length, bool vecHistoryFull){
        
        dir = (lines_direction)dict["dir"].as<int>();
        inv = dict["inv"].as<bool>();
        is_borrow_colors = dict["is_borrow_colors"].as<bool>();
        
    }

    void interact(VisualModule* vc) {
        switch(vc->type){
            case HAP:
                if(can_borrow_colors){
                    HapMovie* hm = (HapMovie*) vc;
                    if(hm->clustered){
                        for(int i = 0; i < N_CLUSTERS; i++){
                            borrowed_colors[i] = hm->center_colors[i];
                        }
                    }
                }
                break;
        }
    };

    void chooseDir() {
        dir = (lines_direction)ofRandom(2);
    }

    void chooseInv() {
        inv = ofRandom(1.0) > 0.5;
    }

    void display(int width, int height, unsigned long long frame_num, std::unordered_map<std::string, float>* common_features, bool isNRT, bool verbose) {
        ofSetLineWidth(0);
        ofFill();
        
        w = width / vec_len;// vec_len is a float
        h = height / vec_len;
        
        for (int i = 0; i < vec_len; i++) {
            float alpha = pow(vec[i], 0.75) * 255.f * (vec[i] > 0);
            if (alpha > alphaThresh) {
                if(is_borrow_colors && can_borrow_colors){
                    ofSetColor(borrowed_colors[i % N_CLUSTERS], alpha);
                } else {
                    ofSetColor(255,255,255,alpha);
                }
                drawLine(width,height,i);
            }
        }
    }

    void drawLine(int width, int height, int i) {
        switch(dir){
            case HORZ:
                    ofDrawRectangle(
                                    0,
                                    ((height - ((i+1) * h)) * inv) + ((i * h) * (1-inv)),
                                    width,
                                    h
                                    );
                break;
            case VERT:
                    ofDrawRectangle(
                                    ((width - ((i+1) * w)) * inv) + ((i * w) * (1-inv)),
                                    0,
                                    w,
                                    height);
                break;
        }
    }
};

#endif /* Lines_hpp */
