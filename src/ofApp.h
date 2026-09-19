#pragma once

#include <unordered_set>

#include "ofMain.h"
#include "defines.h"

class VisualModule;
class FlowField;

#include "ofxOsc.h"
#include "ForceOnsetFrames.h"
#include "VectorHistory.h"
#include "FboRenderer.h"
#include "AudioFeatures.h"
#include "ofxPostGlitch.h"
#include "MessageParser.hpp"
#include "SystemState.h"
#include "FlowField.hpp"

// Modules
#include "VisualModule.hpp"
#include "Lines.hpp"
#include "Mesh.hpp"
#include "Turtle.hpp"
#include "VideoModule.hpp"
#include "Waveform.hpp"
#include "ModuleFactory.hpp"

inline void addVCOptions(SystemState &s, int module_index, int num_times_to_insert) {
    for (int i = 0; i < num_times_to_insert; i++) {
        s.vc_i_options.push_back(module_index);
    };
}

inline string getTimeFromFrameNum(SystemState &s) {
    float sec = s.frame_num / (float)s.target_framerate;
    int min = int(sec / 60);
    sec = sec - (min * 60);
    int sec_whole = int(sec);
    //        int sec_frac = int(round((sec - sec_whole) * 1000));
    int sub_second_frame = s.frame_num % s.target_framerate;
    return ofToString(min) + ":" + ofToString(sec_whole, 2, '0') + "." + ofToString(sub_second_frame, 2, '0');
}

inline void setActiveIndices(SystemState &s, vector<int> &ai) {
    for (int i = 0; i < s.active_module_indices.size(); i++) {
        s.active_module_indices[i] = ai[i];
        if (s.active_module_indices[i] >= 0 && s.modules[s.active_module_indices[i]]->newParamsProb > ofRandom(1.f)) {
            s.modules[s.active_module_indices[i]]->newParams(s);
        }
    }
}

inline void onsetActions(SystemState &s) {
    if (s.verbose)
        cout << "Onset Actions." << endl;

    // Write the `currentRandomSeed` to the file, indicating the frame number so that it can be referenced later
    if (s.isNRT) {
        s.randomSeedLog << s.frame_num << "," << getTimeFromFrameNum(s) << "," << s.currentRandomSeed << endl;
    }

    // new active vc i

    vector<int> chosen_i;
    vector<int> ai(s.active_module_indices.size());
    for (int i = 0; i < s.active_module_indices.size(); i++) {  // go through the max number that we'll display

        if (s.moduleIndexUnlocked[i] && (ofRandom(1.f) < s.onsetSwitchProb)) {
            std::unordered_set<int> chosen_set(chosen_i.begin(), chosen_i.end());
            bool found = false;
            while (!found) {
                int rand_int = ofRandom(0,s.vc_i_options.size());  // random int the size of the options array
                int result = s.vc_i_options[rand_int];          // the int from the options array (which is the index for the modules array)

                if (chosen_set.find(result) == chosen_set.end()) {
                    found = true;
                    chosen_i.push_back(result);
                    ai[i] = result;
                    break;
                }
            }
        } else {
            ai[i] = s.active_module_indices[i];
            chosen_i.push_back(ai[i]);
        }
    }

    if (s.verbose) {
        cout << "Chosen active module indices: ";
        for (int i = 0; i < ai.size(); i++) {
            cout << ai[i] << " ";
        }
        cout << endl;
    }

    setActiveIndices(s, ai);

    if (s.verbose) {
        cout << "Active module indices set." << endl;
    }

    // blend mode
    if (ofRandom(1.f) < s.onsetSwitchProb)
        s.blendMode = s.blendModes[s.blendModePool[int(ofRandom(s.blendModePool.size()))]];

    if (s.verbose) {
        cout << "Blend mode set to: " << s.blendMode << endl;
    }

    s.feedback_amt = (ofRandom(1.f) < s.feedback_prob) * ofRandom(1, s.feedback_max);

    if (s.verbose) {
        cout << "Feedback amount set to: " << s.feedback_amt << endl;
    }

    s.postGlitch.onset();

    if (s.verbose) {
        cout << "Post-glitch onset triggered." << endl;
    }
}

inline void onsetFromSeed(SystemState &s, unsigned long seed) {
    if (s.verbose)
        cout << "onsetFromSeed" << endl;
    s.currentRandomSeed = seed;
    cout << "onset from seed: " << s.currentRandomSeed << endl;
    ofSetRandomSeed(s.currentRandomSeed);
    cout << "random seed set to: " << s.currentRandomSeed << endl;
    onsetActions(s);
}

/* The `onset` method is what should be called if there is no specific seed
 that needs to be set. It will randomly generate a seed, use that seed to
 call `onsetFromSeed`, which then will set the seed with ofSetRandomSeed, and then
 call `onsetActions` to have the proper onset actions unfold*/
inline void onset(SystemState &s) {
    if (s.verbose)
        cout << "onset" << endl;
    onsetFromSeed(s, (unsigned long)ofRandom(INT_MAX));
}

inline void loadConfigFile(SystemState &s, string path) {
    cout << "Loading config file at: " << ofToDataPath(path) << endl;

    ofFile file(path);
    if (!file.exists()) {
        cout << "Config file not found at: " << path << endl;
        ofExit();
    }

    s.config = ofLoadJson(path);

    cout << "File loaded" << endl;

    s.force_onset_frames.setup(s.config);

    s.verbose = checkJsonKey(s.config, "verbose", false);
    s.debug = checkJsonKey(s.config, "debug", false);
    s.n_modules = s.config["modules"].size();

    // assert(s.config["initial-active-modules"].size() == s.config["module-indexes-unlocked"].size() && "In config file 'initial-active-modules' and 'module-indexes-unlocked' are not the same length");

    s.active_module_indices.resize(s.config["initial-active-modules"].size());
    for (int i = 0; i < s.active_module_indices.size(); i++) {
        s.active_module_indices[i] = s.config["initial-active-modules"][i].get<int>();
        assert(active_module_indices[i] < s.n_modules);
    }

    s.moduleIndexUnlocked.resize(s.active_module_indices.size());
    for (int i = 0; i < s.active_module_indices.size(); i++) {
        s.moduleIndexUnlocked[i] = s.config["module-indexes-unlocked"].get<vector<int>>()[i];
    }

    s.vc_i_options.clear();
    for (int i = 0; i < s.n_modules; i++) {
        addVCOptions(s, i, s.config["modules"][i]["prob"].get<int>());
    }

    s.onsetSwitchProb = checkJsonKey(s.config, "onset-switch-prob", 1.0);

    s.feedback_prob = checkJsonKey(s.config, "feedback-prob", 0.25);
    s.feedback_max = checkJsonKey(s.config, "feedback-max", 254);

    ofSetFrameRate(s.config["target-framerate"].get<int>());

    s.use_sc_onsets = checkJsonKey(s.config, "use-sc-onsets", true);

    s.postGlitch.loadState(s.config["post-glitch"]);

    s.blendModePool.clear();
    for (int i = 0; i < s.config["blend-mode-probs"].size(); i++) {
        int n = s.config["blend-mode-probs"][i].get<int>();
        for (int j = 0; j < n; j++) {
            s.blendModePool.push_back(i);
        }
    }
    cout << "Blend Mode Pool loaded" << endl;

    for (int i = 0; i < s.n_modules; i++) {
        cout << "Loading Module " << i << endl;
        s.modules[i]->loadState(s, s.config["modules"][i]);
    }
}

inline ofJson save(SystemState &s) {
    ofJson dict;

    for (int i = 0; i < s.n_modules; i++) {
        dict["vc-save-" + ofToString(i)] = s.modules[i]->saveState();
    }

    for (int i = 0; i < s.active_module_indices.size(); i++) {
        dict["active_vc" + ofToString(i)] = s.active_module_indices[i];
    }

    dict["post-glitch"] = s.postGlitch.saveState();
    dict["feedback_amt"] = s.feedback_amt;
    dict["blendMode"] = (int)s.blendMode;
    dict["debug"] = s.debug;

    return dict;
}

inline void load(SystemState &s, ofJson &dict) {
    for (int i = 0; i < s.n_modules; i++) {
        ofJson child = dict["vc-save-" + ofToString(i)];

        s.modules[i]->loadState(s, child);
    }

    for (int i = 0; i < s.active_module_indices.size(); i++) {
        s.active_module_indices[i] = dict["active_vc" + ofToString(i)].get<int>();
    }

    s.postGlitch.loadState(dict["post-glitch"]);
    s.feedback_amt = checkJsonKey(dict,"feedback_amt",254);
    s.blendMode = (ofBlendMode)checkJsonKey(dict,"blendMode",0);
    s.debug = checkJsonKey(dict,"debug",false);
}

class ofApp : public ofBaseApp {
   public:
    SystemState s;
    ofxOscReceiver osc_receiver;

    void setup();
    void update();
    void draw();
    void runNrtRender(SystemState &s);

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y);
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);

    ofApp(string config_path_) {
        s.config_path = config_path_;
    }

    void exit() {
        s.randomSeedLog.close();
        ofExit();
    }
};
