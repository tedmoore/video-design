//
//  Lines.cpp
//  fonema video
//
//  Created by Ted Moore on 1/6/21.
//

#include <stdio.h>
#include "Lines.hpp"

void Lines::setup(float* vec_, int offset_, int vec_len_, bool can_borrow_colors_, int width, int height, float** vecHistory, int vector_length, int history_length, bool vecHistoryFull){
    can_borrow_colors = can_borrow_colors_;
    vec = vec_;
    offset = offset_;
    vec_len = vec_len_;
    
    newParams(width,height,vecHistory,vector_length,history_length,vecHistoryFull);
    
    type = LINES;
    
    for(int i = 0; i < N_CLUSTERS; i++){
        ofColor col(255);
        borrowed_colors[i] = col;
    }
}

void Lines::newParams(int width, int height, float** vecHistory, int vector_length, int history_length, bool vecHistoryFull){
    chooseDir();
    chooseInv();
    is_borrow_colors = ofRandom(2);
}

void Lines::interact(VisualContent* vc) {
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

void Lines::chooseDir() {
    if (ofRandom(1.0) > 0.5) {
        dir = HORZ;
    } else {
        dir = VERT;
    }
    
    //print("dir " + dir);
}

void Lines::chooseInv() {
    if (ofRandom(1.0) > 0.5) {
        inv = true;
    } else {
        inv = false;
    }
    
    //print("inv " + inv);
}

void Lines::display(int width, int height, int frame_num, std::unordered_map<std::string, float>* common_features, bool isNRT) {
    ofSetLineWidth(0);
    ofFill();
    
    w = width / vec_len;
    h = height / vec_len;
    
    for (int i = 0; i < vec_len; i++) {
        float alpha = pow(vec[i], 0.75) * 255.f;
        //cout << "alpha: " << alpha << "\n";
        if (alpha > alphaThresh) {
            if(is_borrow_colors && can_borrow_colors){
                //cout << "borrowed color: " << borrowed_colors[i % N_CLUSTERS] << "\n";
                ofSetColor(borrowed_colors[i % N_CLUSTERS], alpha);
            } else {
                ofSetColor(255, 255,255,alpha);
            }
            drawLine(width,height,i);
        } else {
            //println("alpha below thresh");
            //drawLine(i, alpha, color(0,255,0));
        }
    }
    
    //postVec();
}

void Lines::drawLine(int width, int height, int i) {
    if (dir == HORZ) {
        //cout << "horz\n";
        if (inv) {
            ofDrawRectangle(0, height - ((i+1) * h), width, h);
        } else {
            ofDrawRectangle(0, i * h, width, h);
        }
    } else if (dir == VERT) {
        //cout << "vert\n";
        if (inv) {
            ofDrawRectangle(width - ((i+1) * w), 0, w, height);
        } else {
            ofDrawRectangle(i * w, 0, w, height);
        }
    }
}
