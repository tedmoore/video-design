//
//  MoviePoint.hpp
//  modular_video_02
//
//  Created by Ted Moore on 6/14/21.
//

#ifndef MoviePoint_hpp
#define MoviePoint_hpp

#include <stdio.h>
#include "ofMain.h"
#include "LagUD.hpp"

class MoviePoint {
public:
    ofVec3f vel, pos, acc;
    float origx, origy;
    LagUD rect_outline_alpha;
    
    void resetPos(){
        cout << "MoviePoint::resetPos " << origx << " " << origy << endl;
        pos.set(origx,origy,0);
    }
    
    void setup(float origx_, float origy_){
        cout << "MoviePoint::setup " << origx_ << " " << origy_ << endl;
        origx = origx_;
        origy = origy_;
        
        rect_outline_alpha.setup(1.f,0.14,0);
        
        resetPos();
        vel.set(0,0,0);
        acc.set(0,0,0);
    }
    
    void applyForce(ofVec3f *force){
        acc.operator+=(*force);
    }
    
    void move(float velLimit){
        vel.operator+=(acc);
        vel.limit(velLimit);
        pos.operator+=(vel);
        acc.operator*=(0);
    }
};

#endif /* MoviePoint_hpp */
