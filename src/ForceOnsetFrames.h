#ifndef FORCE_ONSET_FRAMES_H
#define FORCE_ONSET_FRAMES_H

#include "ofMain.h"

class ForceOnsetFrames
{
public:
    bool forceOnsets;
    int min;
    int max;
    int nextOnset;

    void setup(ofJson &config)
    {
        
        min = checkJsonKey(config, "force-onset-min-frames", 300);
        max = checkJsonKey(config, "force-onset-max-frames", 600);
        forceOnsets = checkJsonKey(config, "force-onset-frames", false);
        nextOnset = ofRandom(min, max);
    }

    bool checkForOnset(int current_frame){
        if(forceOnsets && current_frame >= nextOnset){
                nextOnset = current_frame + ofRandom(min, max);
                return true;
        } else {
            return false;
        }
    }
};

#endif /* FORCE_ONSET_FRAMES_H */