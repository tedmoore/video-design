#ifndef SYSTEMSTATE_H
#define SYSTEMSTATE_H

struct SystemState {
    bool show_frame_rate = false;
    bool debug = false;
    bool onset_occured = false;
    int target_framerate;
    float onsetSwitchProb = 1.f;
    unsigned long currentRandomSeed = 0;
    bool isNRT = false;
    size_t frame_num = 0;
    int n_modules = 0;
    vector<VisualModule*> modules;
    vector<int> vc_i_options;
    vector<bool> moduleIndexUnlocked;
    vector<int> active_module_indices;
    FlowField *flow_field;
    std::ofstream randomSeedLog;
    VectorHistory vec_history;
    ForceOnsetFrames force_onset_frames;
    bool verbose;
    AudioFeatures features;
    ofxPostGlitch postGlitch;
    int feedback_amt = 0;
    float feedback_prob = 0.f;
    int feedback_max = 255;
    bool use_sc_onsets = true;
    ofJson saveStates[N_STATE_SAVES];
    const char saveStateKeys[N_STATE_SAVES] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    FboRenderer fbo;
    float postGlitchChangeProb = 0.5;
    string config_path;
    ofJson config;
    ofBlendMode blendMode = OF_BLENDMODE_DISABLED;
    ofBlendMode blendModes[6] = {OF_BLENDMODE_ADD, OF_BLENDMODE_ALPHA, OF_BLENDMODE_SCREEN, OF_BLENDMODE_DISABLED, OF_BLENDMODE_MULTIPLY, OF_BLENDMODE_SUBTRACT};
    vector<int> blendModePool;
    MessageParser messageParser;
};

#endif // SYSTEMSTATE_H