#pragma once

#include "defines.h"
#include "ofMain.h"
#include "ofxOsc.h"
#include "VisualContent.hpp"
#include "HapMovie.hpp"
#include "Waveform.hpp"
#include "Mesh.hpp"
#include "Turtle.hpp"
#include "Lines.hpp"
#include "ofxCv.h"
#include "ofxOpenCv.h"
#include "ofxPostGlitch.h"
#include "ofxYAML.h"

class ofApp : public ofBaseApp{
    
public:
    
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    void newHapMovie(std::string path, int index, ofVec3f* initPts, int width, int height, ofxYAML& config, int videoIndex);
    void displayIncomingData(int width, int height);
    void onsetOccured(int width, int height);
    void drawScreen(int width, int height, int frameNum, bool isNRT);
    void setValsFromCSV(int width, int height, vector<float>& csv_data);
    void incrementVecHistoryCounter();
    void processReaperMarker(string& cmd);
    void setActiveIndices(int* ai,int width, int height);
    void processConfigFile(string path){
        config.load(path);
        
        for(int i = 0; i < MAX_ACTIVE_MODULES; i++){
            active_vc_i[i] = config["initial-active-modules"][i].as<int>();
        }
        
        onsetSwitchProb = config["onset-switch-prob"].as<float>();
        
        feedback_prob = config["feedback-prob"].as<float>();
        feedback_max = config["feedback-max"].as<int>();
        
        ofSetFrameRate(config["target-framerate"].as<int>());
        
        postGlitchChangeProb = config["post-glitch"]["change-prob"].as<float>();
        
        use_sc_onsets = config["use-sc-onsets"].as<bool>();
        
        postGlitchProbs[0] = config["post-glitch"]["convergence-prob"].as<float>();
        postGlitchProbs[1] = config["post-glitch"]["glow-prob"].as<float>();
        postGlitchProbs[2] = config["post-glitch"]["shaker-prob"].as<float>();
        postGlitchProbs[3] = config["post-glitch"]["cutslider-prob"].as<float>();
        postGlitchProbs[4] = config["post-glitch"]["twist-prob"].as<float>();
        postGlitchProbs[5] = config["post-glitch"]["outline-prob"].as<float>();
        postGlitchProbs[6] = config["post-glitch"]["noise-prob"].as<float>();
        postGlitchProbs[7] = config["post-glitch"]["slitscan-prob"].as<float>();
        postGlitchProbs[8] = config["post-glitch"]["swell-prob"].as<float>();
        postGlitchProbs[9] = config["post-glitch"]["invert-prob"].as<float>();
        postGlitchProbs[10] = config["post-glitch"]["highcontrast-prob"].as<float>();
        postGlitchProbs[11] = config["post-glitch"]["blueraise-prob"].as<float>();
        postGlitchProbs[12] = config["post-glitch"]["redraise-prob"].as<float>();
        postGlitchProbs[13] = config["post-glitch"]["greenraise-prob"].as<float>();
        postGlitchProbs[14] = config["post-glitch"]["redinvert-prob"].as<float>();
        postGlitchProbs[15] = config["post-glitch"]["blueinvert-prob"].as<float>();
        postGlitchProbs[16] = config["post-glitch"]["greeninvert-prob"].as<float>();
        
        for(int i = 0; i < config["blend-mode-probs"].size(); i++){
            int n = config["blend-mode-probs"][i].as<int>();
            for(int j = 0; j < n; j++){
                blendModePool.push_back(i);
            }
        }
        
        for(int i = 0; i < N_VISUAL_CONTENTS; i++){
            visual_contents[i]->processConfigFile(config);
        }
    }
    
    int addVCOptions(int counter, int num){
        for(int i = 0; i < num; i++){
            vc_i_options.push_back(counter);
        };
        
        return counter + 1;
    }
    
    VisualContent* visual_contents[N_VISUAL_CONTENTS];
    vector<int> vc_i_options;
    
    int* active_vc_i;
    FlowField ff;
    
    // waveform data
    int n_waveforms = 2;
    int waveform_len = 1920;
    float** waveforms;
    
    // mags
    int n_magnitudes = 1;
    int magnitude_len = 513;
    float** magnitudes;
    
    // vector data
    int vector_len = 106;
    float vector_data[106]; // 106 not including the onsets at the end
    std::unordered_map<std::string, float> common_features;
    float** vec_history;
    int vec_history_length;
    bool vec_history_full = false;
    int vec_history_counter = 0;

    float xsize;
    float ysize;
    float x_size_mul;
    float y_size_mul;
    float xmin;
    float xmax;
    float ymin;
    float ymax;
    float zmin;
    float zmax;
    
    // OSC
    ofxOscReceiver osc_receiver;
    
    bool debug = false;
    bool onset_occured = false;
    float onsetSwitchProb = 1.f;
  
    bool nrtRender = false;
    
    ofFbo main_fbo;
    ofxPostGlitch postGlitch;
    float postGlitchProbs[GLITCH_NUM];
    float postGlitchChangeProb = 0.5;
    
    ofxYAML config;
    
    ofBlendMode blendMode = OF_BLENDMODE_DISABLED;
    
    ofBlendMode blendModes[6] = {OF_BLENDMODE_ADD,OF_BLENDMODE_ALPHA,OF_BLENDMODE_SCREEN,OF_BLENDMODE_DISABLED,OF_BLENDMODE_MULTIPLY,OF_BLENDMODE_SUBTRACT};
    
    vector<int> blendModePool;
    
    int feedback_amt = 0;
    float feedback_prob = 0.f;
    int feedback_max = 255;
    
    bool use_sc_onsets = true;
    
    ofxYAML saves[10];
    
    ofxYAML save(){
        ofxYAML dict;
        
        for(int i = 0; i < N_VISUAL_CONTENTS; i++){
            dict["vc-save-" + ofToString(i)] = visual_contents[i]->saveState();
        }
        
        for(int i = 0; i < MAX_ACTIVE_MODULES; i++){
            dict["active_vc" + ofToString(i)] = active_vc_i[i];
        }
        
        for(int i = 0; i < GLITCH_NUM; i++){
            dict["postGlitch" + ofToString(i)] = postGlitch.getFx(i);
        }

        dict["feedback_amt"] = feedback_amt;
        dict["blendMode"] = (int)blendMode;
        dict["debug"] = debug;
        
        return dict;
    }
    
    void load(ofxYAML &dict){
        
        for(int i = 0; i < N_VISUAL_CONTENTS; i++){
            
            // ====================================================
            // this line actually is important because i think
            // it is casting YAML::Node that is in the dict into an
            // ofxYAML::Node, which is the type expected by loadState()
            // below. just passing dict["vc-save-" + ofToString(i)]
            // directly into load state doesn't work.
            ofxYAML::Node child = dict["vc-save-" + ofToString(i)];
            // ====================================================
            
            visual_contents[i]->loadState(child);
        }
        
        for(int i = 0; i < MAX_ACTIVE_MODULES; i++){
            active_vc_i[i] = dict["active_vc" + ofToString(i)].as<int>();
        }
        
        for(int i = 0; i < GLITCH_NUM; i++){
            postGlitch.setFx(i,dict["postGlitch" + ofToString(i)].as<bool>());
        }

        feedback_amt = dict["feedback_amt"].as<int>();
        blendMode = (ofBlendMode)dict["blendMode"].as<int>();
        debug = dict["debug"].as<bool>();
    }
};
