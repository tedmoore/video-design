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

    void setup(bool forceOnsets_, int min_, int max_)
    {
        min = min_;
        max = max_;
        forceOnsets = forceOnsets_;
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