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
    void setup(float upLerp_, float downLerp_, float initValue);
    float update(float newValue);
    
    float value;
    float upLerp;
    float downLerp;
    
};

#endif /* LagUD_hpp */
