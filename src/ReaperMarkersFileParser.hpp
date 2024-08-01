//
//  ReaperMarkersFileParser.hpp
//  modular_video_02
//
//  Created by macprocomputer on 11/20/22.
//

#ifndef ReaperMarkersFileParser_hpp
#define ReaperMarkersFileParser_hpp

#include <stdio.h>
#include "ofMain.h"

struct ReaperMarker {
    int sample;
    std::string cmd;
};

class ReaperMarkersFileParser {
public:
    double samples_per_frame;
    vector<ReaperMarker> markers;
    int index = 0;
    int start_sample;
    
    void setup(std::string path,int audio_samplerate, int framerate, int start_sample_){

        start_sample = start_sample_;
        samples_per_frame = (double)audio_samplerate / (double)framerate;
        
        ofBuffer buffer = ofBufferFromFile(path);
        for (auto line : buffer.getLines()){
            vector<string> tokens = ofSplitString(line,",");
            ReaperMarker rm;
            rm.sample = ofToInt(tokens[0]);
            rm.cmd = tokens[1];
            markers.push_back(rm);
        }
        
        for(ReaperMarker rm : markers){
            cout << rm.sample << " " << rm.cmd << endl;
        }
    }
  
    std::string currentFrame(int cf){
        double current_sample = start_sample + (cf * samples_per_frame);
        std::string outstring = "";
        while(index < markers.size() && markers[index].sample < current_sample){
            outstring += markers[index].cmd + " ";
            index++;
        }
        return outstring;
    }
};

#endif /* ReaperMarkersFileParser_hpp */
