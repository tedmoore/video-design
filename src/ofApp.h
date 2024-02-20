#pragma once

#include "defines.h"
#include "ofMain.h"
#include "ofxOsc.h"
#include "VisualModule.hpp"
#include "VideoModule.hpp"
#include "Waveform.hpp"
#include "Mesh.hpp"
#include "Turtle.hpp"
#include "Lines.hpp"
#include "ofxPostGlitch.h"
#include "thirdparty/nlohmann/json.hpp"

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
    void displayIncomingData(int width, int height);
    void onsetActions(int width, int height, unsigned long long frame_num, bool isNRT);
    void renderFrame(int width, int height, unsigned long long frameNum, bool isNRT);
    void setValsFromCSV(int width, int height, vector<float>& csv_data, unsigned long long frame_num, bool isNRT);
    void incrementVecHistoryCounter();
    void processReaperMarker(string& cmd, int width, int height, unsigned long long frame_num, bool isNRT);
    void setActiveIndices(int* ai,int width, int height, unsigned long long frame_num);
    void prUpdate(bool isNRT);
    void runNrtRender(int width,int height);
    void drawBounds();
    
    void exit(){
        randomSeedLog.close();
        ofExit();
    }
    
    /* The `onset` method is what should be called if there is no specific seed
     that needs to be set. It will randomly generate a seed, use that seed to
     call `onsetFromSeed`, which then will set the seed with ofSetRandomSeed, and then
     call `onsetActions` to have the proper onset actions unfold*/
    void onset(int width, int height, unsigned long long frame_num, bool isNRT){
        if(verbose) cout << "onset" << endl;
        onsetFromSeed((unsigned long)ofRandom(INT_MAX),width,height,frame_num,isNRT);
    }
        
    void onsetFromSeed(unsigned long seed, int width, int height, unsigned long long frame_num, bool isNRT){
        if(verbose) cout << "onsetFromSeed" << endl;
        currentRandomSeed = seed;
        ofSetRandomSeed(currentRandomSeed);
        onsetActions(width, height, frame_num, isNRT);
    }
    
    string getTimeFromFrameNum(unsigned long long frame_num){
        float sec = frame_num / (float)config["target-framerate"].get<int>();
        int min = int(sec / 60);
        sec = sec - (min * 60);
        int sec_whole = int(sec);
//        int sec_frac = int(round((sec - sec_whole) * 1000));
        int sub_second_frame = frame_num % config["target-framerate"].get<int>();
        return ofToString(min) + ":" + ofToString(sec_whole,2,'0') + "." + ofToString(sub_second_frame,2,'0');
    }
    
    void loadConfigFile(string path){
        
        std::ifstream i(path);
        i >> config;
        
        verbose = config["verbose"].get<bool>();
        debug = config["debug"].get<bool>();
        
        for(int i = 0; i < MAX_ACTIVE_MODULES; i++){
            active_module_indices[i] = config["initial-active-modules"][i].get<int>();
        }
        
        for(int i = 0; i < MAX_ACTIVE_MODULES; i++){
            moduleIndexUnlocked[i] = config["module-indexes-unlocked"].get<vector<int>>()[i];
        }
                
        onsetSwitchProb = config["onset-switch-prob"].get<float>();
        
        feedback_prob = config["feedback-prob"].get<float>();
        feedback_max = config["feedback-max"].get<int>();
        
        ofSetFrameRate(config["target-framerate"].get<int>());
        
        use_sc_onsets = config["use-sc-onsets"].get<bool>();
        
        postGlitchChangeProb = config["post-glitch"]["change-prob"].get<float>();
        
        postGlitchProbs[0] = config["post-glitch"]["convergence-prob"].get<float>();
        postGlitchProbs[1] = config["post-glitch"]["glow-prob"].get<float>();
        postGlitchProbs[2] = config["post-glitch"]["shaker-prob"].get<float>();
        postGlitchProbs[3] = config["post-glitch"]["cutslider-prob"].get<float>();
        postGlitchProbs[4] = config["post-glitch"]["twist-prob"].get<float>();
        postGlitchProbs[5] = config["post-glitch"]["outline-prob"].get<float>();
        postGlitchProbs[6] = config["post-glitch"]["noise-prob"].get<float>();
        postGlitchProbs[7] = config["post-glitch"]["slitscan-prob"].get<float>();
        postGlitchProbs[8] = config["post-glitch"]["swell-prob"].get<float>();
        postGlitchProbs[9] = config["post-glitch"]["invert-prob"].get<float>();
        postGlitchProbs[10] = config["post-glitch"]["highcontrast-prob"].get<float>();
        postGlitchProbs[11] = config["post-glitch"]["blueraise-prob"].get<float>();
        postGlitchProbs[12] = config["post-glitch"]["redraise-prob"].get<float>();
        postGlitchProbs[13] = config["post-glitch"]["greenraise-prob"].get<float>();
        postGlitchProbs[14] = config["post-glitch"]["redinvert-prob"].get<float>();
        postGlitchProbs[15] = config["post-glitch"]["blueinvert-prob"].get<float>();
        postGlitchProbs[16] = config["post-glitch"]["greeninvert-prob"].get<float>();
        
        for(int i = 0; i < config["blend-mode-probs"].size(); i++){
            int n = config["blend-mode-probs"][i].get<int>();
            for(int j = 0; j < n; j++){
                blendModePool.push_back(i);
            }
        }
        
        for(int i = 0; i < n_modules; i++){
            modules[i]->processConfigFile(config["modules"][i]);
        }
  
        // TODO
//        for(int i = 0; i < N_STATE_SAVES; i++){
//            if(config["state-save-" + ofToString(i)]){
//                nlohmann::json state;
//                state.load(config["state-save-" + ofToString(i)].get<string>());
//                saves[i] = state;
//            }
//        }
    }
    
    int addVCOptions(int counter, int num){
        for(int i = 0; i < num; i++){
            vc_i_options.push_back(counter);
        };
        
        return counter + 1;
    }
    
    bool verbose = false;
    
    int n_modules = 0;
    vector<VisualModule*> modules;
    vector<int> vc_i_options;
    bool moduleIndexUnlocked[MAX_ACTIVE_MODULES];
    
    int* active_module_indices;
    FlowField ff;
    
    // waveform data
    float** waveforms;
    
    // mags
    float** magnitudes;
    
    std::ofstream randomSeedLog;
    
    // vector data
    float vector_data[DESCRIPTORS_VECTOR_LENGTH-1]; // 106 not including the onsets at the end
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
    
    unsigned long currentRandomSeed = 0;
    
    ofFbo main_fbo;
    ofxPostGlitch postGlitch;
    float postGlitchProbs[GLITCH_NUM];
    float postGlitchChangeProb = 0.5;
    
    nlohmann::json config;
    
    ofBlendMode blendMode = OF_BLENDMODE_DISABLED;
    
    ofBlendMode blendModes[6] = {OF_BLENDMODE_ADD,OF_BLENDMODE_ALPHA,OF_BLENDMODE_SCREEN,OF_BLENDMODE_DISABLED,OF_BLENDMODE_MULTIPLY,OF_BLENDMODE_SUBTRACT};
    
    vector<int> blendModePool;
    
    int feedback_amt = 0;
    float feedback_prob = 0.f;
    int feedback_max = 255;
    
    bool use_sc_onsets = true;
    
    nlohmann::json saves[N_STATE_SAVES];
    const char saveKeys[N_STATE_SAVES] = {'0','1','2','3','4','5','6','7','8','9'};
    
    nlohmann::json save(){
        nlohmann::json dict;
        
        for(int i = 0; i < n_modules; i++){
            dict["vc-save-" + ofToString(i)] = modules[i]->saveState();
        }
        
        for(int i = 0; i < MAX_ACTIVE_MODULES; i++){
            dict["active_vc" + ofToString(i)] = active_module_indices[i];
        }
        
        dict["postGlitch"] = postGlitch.saveState();
        dict["feedback_amt"] = feedback_amt;
        dict["blendMode"] = (int)blendMode;
        dict["debug"] = debug;
        
        return dict;
    }
    
    void load(nlohmann::json &dict, int width, int height){
        
        for(int i = 0; i < n_modules; i++){
            
            nlohmann::json child = dict["vc-save-" + ofToString(i)];
            
            modules[i]->loadState(child,width,height,vec_history,DESCRIPTORS_VECTOR_LENGTH,vec_history_length,vec_history_full);
        }
        
        for(int i = 0; i < MAX_ACTIVE_MODULES; i++){
            active_module_indices[i] = dict["active_vc" + ofToString(i)].get<int>();
        }
        
        nlohmann::json child = dict["postGlitch"];
        postGlitch.loadState(child);
        
        feedback_amt = dict["feedback_amt"].get<int>();
        blendMode = (ofBlendMode)dict["blendMode"].get<int>();
        debug = dict["debug"].get<bool>();
    }
    
    int getVectorHistoryLength(){
        for(VisualModule* vm : modules){
            if(vm->type == MESH){
                return ((Mesh*)vm)->nPoints;
            }
        }
        return 0;
    }

};
