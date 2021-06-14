//
//  Lines.hpp
//  fonema video
//
//  Created by Ted Moore on 1/6/21.
//

#ifndef Lines_hpp
#define Lines_hpp

#include <stdio.h>
#include "VisualContent.hpp"
#include "ofMain.h"
#include "HapMovie.hpp"

enum lines_direction { HORZ , VERT };

class Lines: public VisualContent {
public:

    void setup(float* vec_, int offset_, int vec_len_, bool can_borrow_colors_, int width, int height, float** vecHistory, int vector_length, int history_length, bool vecHistoryFull);
    void newParams(int width, int height, float** vecHistory, int vector_length, int history_length, bool vecHistoryFull) override;
//    void newParams();
    void chooseDir();
    void interact(VisualContent* other) override;
    void chooseInv();
    void display(int width, int height, int frame_num, std::unordered_map<std::string, float>* common_features, bool isNRT) override;
    void drawLine(int width, int height, int i);

    bool inv;
    float* vec;
    float w, h;
    int vec_len;
    int offset;
    int alphaThresh = 10;
    bool can_borrow_colors = false;
    bool is_borrow_colors = false;
    
    lines_direction dir = HORZ;
    
    ofColor borrowed_colors[N_CLUSTERS];
    
};

#endif /* Lines_hpp */
