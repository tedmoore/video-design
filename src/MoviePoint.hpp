//
//  MoviePoint.hpp
//  modular_video_02
//
//  Created by Ted Moore on 6/14/21.
//

#ifndef MoviePoint_hpp
#define MoviePoint_hpp

#include <stdio.h>

#include "LagUD.hpp"
#include "ofMain.h"

class MoviePoint {
   public:
    glm::vec3 vel, pos, acc, original_postiion;
    LagUD rect_outline_alpha;

    void resetPos() {
        pos = original_postiion;
    }

    void setup(float origx_, float origy_) {
        original_postiion = {origx_, origy_, 0};

        rect_outline_alpha.setup(1.f, 0.14, 0);

        resetPos();
        vel = {0, 0, 0};
        acc = {0, 0, 0};
    }

    void applyForce(glm::vec3 &force) {
        acc += force;
    }

    void move(float velLimit) {
        vel += acc;
        vel = limit(vel, velLimit);
        pos += vel;
        acc *= 0;
    }
};

#endif /* MoviePoint_hpp */
