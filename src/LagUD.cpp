//
//  LagUD.cpp
//  fonema video
//
//  Created by Ted Moore on 12/30/20.
//

#include "LagUD.hpp"

void LagUD::setup(float upLerp_, float downLerp_, float initValue) {
    value = initValue;
    upLerp = upLerp_;
    downLerp = downLerp_;
}

float LagUD::update(float newValue) {
    if (newValue > value) {
        value = ofLerp(value,newValue,upLerp);
    } else {
        value = ofLerp(value,newValue,downLerp);
    }
    //println(value);
    return value;
}
