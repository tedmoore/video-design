//
//  Turtle.hpp
//  modular_video_02
//
//  Created by macprocomputer on 2/2/22.
//

#ifndef Turtle_hpp
#define Turtle_hpp

#include "VisualContent.hpp"
#include "ofMain.h"

class Turtle: public VisualContent {
public:
    
    void setup(float* vec_, int offset_, int vec_len_, bool can_borrow_colors_, int width, int height, float** vecHistory, int vector_length, int history_length, bool vecHistoryFull);
    
    void interact(VisualContent* other){};
    
    void receiveOSC(int width, int height, std::string label, float val){
    }
    
    void display(int width, int height, int frame_num, std::unordered_map<std::string, float>* common_features, bool isNRT){}
    
    void screenResize(int w, int h){}
    
    void newParams(int width, int height, float** vecHistory, int vector_length, int history_length, bool vecHistoryFull){}

    void update(bool isNRT){}
};


#include <stdio.h>

#endif /* Turtle_hpp */
