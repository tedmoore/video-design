//
//  LagUD.hpp
//  fonema video
//
//  Created by Ted Moore on 12/30/20.
//

#ifndef LagUD_hpp
#define LagUD_hpp

#include <stdio.h>
#include "ofMain.h"

class LagUD {
public:
    
    float value;
    float upLerp;
    float downLerp;
    
    void setup(float upLerp_, float downLerp_, float initValue) {
        value = initValue;
        upLerp = upLerp_;
        downLerp = downLerp_;
    }

    float update(float newValue) {
        const float lerpFactor = (newValue > value) ? upLerp : downLerp;
        value = ofLerp(value, newValue, lerpFactor);
        return value;
    }

};

#endif /* LagUD_hpp */
