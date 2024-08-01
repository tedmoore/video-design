#include "ofApp.h"

#include <algorithm>
#include <unordered_set>

#include "ReaperMarkersFileParser.hpp"

int getVectorHistoryLength(SystemState &s) {
    for (VisualModule *vm : s.modules) {
        if (vm->type == MESH) {
            return ((Mesh *)vm)->nPoints;
        }
    }
    return 0;
}

//--------------------------------------------------------------
void ofApp::setup() {
    
    s.config = ofLoadJson(s.config_path);
    s.isNRT = checkJsonKey(s.config, "nrt-render", false);
    s.target_framerate = checkJsonKey(s.config, "target-framerate-test123", 30);
    cout << "Target framerate: " << s.target_framerate << endl;
    s.features.magnitudes.resize(N_MAGNITUDES);
    for(int i = 0; i < s.features.magnitudes.size(); i++){
        s.features.magnitudes[i].resize(MAGNITUDES_LEN);
    }
    s.features.descriptors_vector.resize(DESCRIPTORS_VECTOR_LENGTH);
    s.features.waveforms.resize(N_WAVEFORMS);
    for(int i = 0; i < s.features.waveforms.size(); i++){
        s.features.waveforms[i].resize(WAVEFORM_LEN);
    }

    if (s.isNRT) {
        s.fbo.allocate(checkJsonKey(s.config, "width", 3840), checkJsonKey(s.config, "height", 2160), GL_RGBA);
    } else {
        s.fbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
    }
    cout << "Fbo allocated" << endl;

    s.postGlitch.setup(&s.fbo.fbo);
    cout << "PostGlitch setup" << endl;

    ofBackground(0);
    ofEnableSmoothing();
    ofEnableAntiAliasing();
    ofEnableAlphaBlending();

    // ================== VISUAL CONTENTS ==========================
    // setup flow field
    s.flow_field = new FlowField;
    s.flow_field->setup(checkJsonKey(s.config, "flow-field-resolution", 10));
    cout << "Flow field setup" << endl;

    // ============ setup modules ===============
    s.n_modules = s.config["modules"].size();
    s.modules.resize(s.n_modules);

    ModuleFactory mf;
    mf.registerFunction("waveform", [&](SystemState &s, ofJson &j) { 
        Waveform *wf = new Waveform;
        wf->setup(s, j); 
        return wf;
    });
    mf.registerFunction("mesh", [&](SystemState &s, ofJson &j) { 
        Mesh *mesh = new Mesh;
        mesh->setup(s, j); 
        return mesh;
    });
    mf.registerFunction("mag-lines", [&](SystemState &s, ofJson &j) { 
        Lines *lines = new Lines;
        lines->setup(s, j);
        lines->setPtr(&s.features.magnitudes[0]);
        return lines;
    });
    mf.registerFunction("turtle", [&](SystemState &s, ofJson &j) { 
        Turtle *turtle = new Turtle;
        turtle->setup(s, j); 
        return turtle;
    });
    mf.registerFunction("video", [&](SystemState &s, ofJson &j) { 
        VideoModule *vc = new VideoModule;
        vc->setup(s, j); 
        return vc;
    });

    for (int i = 0; i < s.n_modules; i++) {
        cout << "Setting up module " << i << endl;
        s.modules[i] = mf.createModule(s, s.config["modules"][i]);
        //addVCOptions(s, i, s.config["modules"][i]["prob"].get<int>());
    }

    cout << "Modules setup" << endl;

    // ============ setup vecHistory ===============
    s.vec_history.setup(getVectorHistoryLength(s));

    cout << "Vector history setup" << endl;

    // ======================= OSC ================
    osc_receiver.setup(11000);

    loadConfigFile(s,s.config_path);
    cout << "Config file loaded" << endl;

    // =========== NRT RENDERING =====================
    if (s.isNRT) {
        runNrtRender(s);
    }

    // =========================== INITIALIZATION =====================
    if (s.config["initial-onset"].get<bool>()) {
        onset(s);
    }
}

void setValsFromCSV(SystemState &s, vector<float> &csv_data) {
    s.features.spectral_centroid = csv_data[0];
    s.features.spectral_spread = csv_data[1];
    s.features.spectral_skewness = csv_data[2];
    s.features.spectral_kurtosis = csv_data[3];
    s.features.spectral_rolloff = csv_data[4];
    s.features.spectral_flatness = csv_data[5];
    s.features.spectral_crest = csv_data[6];
    s.features.pitch = csv_data[7];
    s.features.pitch_confidence = csv_data[8];
    s.features.loudness = csv_data[9];
    s.features.true_peak = csv_data[10];
    s.features.amplitude = csv_data[11];
    s.features.sensory_dissonance = csv_data[12];
    s.features.zero_crossings = csv_data[13];

    s.onset_occured = false;

    s.features.descriptors_vector[csv_data.size() - 1] = csv_data[csv_data.size() - 1];
    if (s.features.descriptors_vector[csv_data.size() - 1] > 0.5 && s.use_sc_onsets) {
        s.onset_occured = true;
    }

    if (s.onset_occured || s.force_onset_frames.checkForOnset(s.frame_num))
        onset(s); 

    for (int i = 0; i < csv_data.size() - 1; i++) {
        s.features.descriptors_vector[i] = csv_data[i];
        s.vec_history.updateCurrentFrameAtIndex(i, csv_data[i]);
    }
    s.vec_history.incrementFrameIndex();
}

void processReaperMarker(SystemState &s, string &cmd) {
    vector<string> tokens = ofSplitString(cmd, " ");
    int index = 0;

    // cout << "tokens: ";
    // for(int i = 0; i < tokens.size(); i++){
    //     cout << tokens[i] << " ";
    // }
    // cout << endl;

    // TODO: strategy pattern
    while (index < tokens.size()) {
        if (tokens[index] == "o") {
            cout << "o found in tokens" << endl;
            onset(s);
        } else if (tokens[index] == "sai") {
            vector<int> ai(s.active_module_indices.size());
            for (int i = 0; i < s.active_module_indices.size(); i++) {
                ai[i] = ofToInt(tokens[++index]);
            }
            setActiveIndices(s,ai);
        } else if (tokens[index] == "loadState") {
            load(s,s.saveStates[ofToInt(tokens[++index])]);
        } else if (tokens[index] == "loadStateFromDisk") {
            ofFile file(ofToDataPath(tokens[++index] + ".json"));

            if (file.exists()) {
                ofJson dict;
                std::ifstream i(ofToDataPath(file.path()));
                i >> dict;

                load(s,dict);
            } else {
                cout << "ofApp::processReaperMarker loadStateFromDisk WARNING: There is no file on disk at that path: " << file.path() << endl;
            }
        } else if (tokens[index] == "onsetSeed") {
            onsetFromSeed(s,ofToInt(tokens[++index]));
        } else if (tokens[index] == "sp") {  // set parameter
            int moduleIndex = ofToInt(tokens[++index]);
            string label = tokens[++index];
            float val = ofToFloat(tokens[++index]);
            s.modules[moduleIndex]->receiveOSC(s, label, val);
        }

        index++;  // always increment at least one!
    }
}

void prUpdate(SystemState &s) {
    for (int i = 0; i < s.n_modules; i++) {
        s.modules[i]->update(s);
    }
}

void displayIncomingData(SystemState &s) {
    // mags
    float xoff = 20;
    int yoff = 20;
    int ystart = s.fbo.getHeight() - yoff;
    int mag_height = (s.fbo.getHeight() / 2) - (yoff * 2);
    int bar_width = 2;
    int bar_skip = bar_width + 1;
    ofFill();
    ofSetLineWidth(0);
    for (int i = 0; i < N_MAGNITUDES; i++) {
        ofSetColor(255, 200 - (i * 100));
        for (int x = 0; x < MAGNITUDES_LEN; x++) {
            int bar_height = s.features.magnitudes[i][x] * mag_height;
            ofDrawRectangle(xoff + (x * bar_skip), ystart - bar_height, bar_width, bar_height);
        }
    }

    // vector
    xoff = 20;
    yoff = 20;
    ystart = (s.fbo.getHeight() / 2) - yoff;
    int vec_height = (s.fbo.getHeight() / 2) - (yoff * 2);
    bar_width = 3;
    bar_skip = bar_width + 2;  // gap of 1
    ofFill();
    ofSetLineWidth(0);
    for (int i = 0; i < DESCRIPTORS_VECTOR_LENGTH; i++) {
        int bar_height = s.features.descriptors_vector[i] * vec_height;
        // TODO: i think the colors are wrong
        if (i > 65) {
            ofSetColor(255);
        } else if (i > 53) {
            ofSetColor(255, 50, 50);
        } else if (i > 13) {
            ofSetColor(50, 50, 255);
        } else if (i > 6) {
            ofSetColor(50, 255, 50);
        } else {
            ofSetColor(0, 255, 255);
        }
        ofDrawRectangle(xoff + (i * bar_skip), ystart - bar_height, bar_width, bar_height);
    }

    // onset
    if (s.onset_occured) {
        ofSetColor(255, 255, 0);
        ofDrawRectangle((s.fbo.getWidth() / 2) - 20, 20, 40, 40);
    }

    // waveforms
    xoff = (s.fbo.getWidth() * 0.35) + 20;
    int xend = s.fbo.getWidth() - 20;
    int wf_height = (s.fbo.getHeight() / 2) - 40;
    int middle = (wf_height / 2) + 20;
    ofNoFill();
    ofSetLineWidth(1);
    for (int i = 0; i < N_WAVEFORMS; i++) {
        switch (i) {
            case 0:
                ofSetColor(255, 50, 255);
                break;
            case 1:
                ofSetColor(255, 50, 50);
                break;
            case 2:
                ofSetColor(50, 50, 255);
                break;
        }
        ofBeginShape();
        for (int x = 0; x < WAVEFORM_LEN; x++) {
            int xpt = ofMap(x, 0, WAVEFORM_LEN - 1, xoff, xend);
            float ypt = middle + (s.features.waveforms[i][x] * -0.5 * wf_height);
            ofVertex(xpt, ypt);
        }
        ofEndShape();
    }
}

void renderFrame(SystemState &s) {
    if (s.verbose)
        cout << "renderFrame " << s.frame_num << endl;

    s.fbo.begin();

    if (s.verbose)
        cout << "fbo.begin()" << endl;

    ofEnableBlendMode(OF_BLENDMODE_ALPHA);

    if (s.verbose)
        cout << "ofEnableBlendMode(OF_BLENDMODE_ALPHA)" << endl;

    ofSetColor(0, 255 - s.feedback_amt);  // alpha of 255 = no feedback, alpha of 0 = full feedback
    ofDrawRectangle(0, 0, s.fbo.getWidth(), s.fbo.getHeight());

    if (s.verbose)
        cout << "ofSetColor(0, 255 - feedback_amt)" << endl;

    // =============== visualModules ===================
    ofEnableBlendMode(OF_BLENDMODE_ADD);

    if (!s.debug) {
        if (s.verbose) {
            cout << "Active Module Indices:";
            for (int i = 0; i < s.active_module_indices.size(); i++) {
                int index = s.active_module_indices[i];
                if (index >= 0)
                    cout << " " << s.modules[index]->getName();
            }
            cout << endl;
        }

        s.flow_field->update(s);

        for (int i = 0; i < s.active_module_indices.size(); i++) {
            int index = s.active_module_indices[i];
            if (index >= 0) {
                for (int j = 0; j < s.active_module_indices.size(); j++) {
                    if ((j != i) && (s.active_module_indices[j] >= 0)) {
                        s.modules[index]->interact(s,s.modules[s.active_module_indices[j]]);
                    }
                }
                s.modules[index]->display(s);
            }
        }
    } else {
        displayIncomingData(s);
    }

    s.fbo.end();

    // TODO: maybe the fx should be generated in between the modules sometimes?
    s.postGlitch.generateFx(s.features);
}

void ofApp::runNrtRender(SystemState& s) {
    if (s.verbose)
        cout << "running nrt render" << endl;

    string csv_folder = s.config["csv-folder"].get<string>();

    // int max_frames = 600; // 600 frames = 20 seconds
    int max_frames = s.config["max-frames"].get<int>() == -1 ? INT_MAX : s.config["max-frames"].get<int>();

    std::ifstream descriptors_file;
    descriptors_file.open(csv_folder + "/descriptors.csv");
    std::ifstream waveform0_file;
    waveform0_file.open(csv_folder + "/waveform-0.csv");
    std::ifstream waveform1_file;
    waveform1_file.open(csv_folder + "/waveform-1.csv");
    std::ifstream mags_file;
    mags_file.open(csv_folder + "/mags.csv");

    string reaperMarkerPath = s.config["reaper-markers"].get<string>();
    bool usingReaperMarkers = ofFile(reaperMarkerPath).exists();
    ReaperMarkersFileParser rmfp;

    cout << "target framerate: " << s.target_framerate << endl;
    if (usingReaperMarkers) rmfp.setup(reaperMarkerPath, s.config["audio-sample-rate"].get<int>(), s.target_framerate, s.config["audio-start-sample"].get<int>());

    // stuff for rendering
    string timestamp = ofGetTimestampString();
    string new_dir_path = s.config["output-folder"].get<string>() + "/" + timestamp;

    ofDirectory new_dir(new_dir_path);
    new_dir.create();

    // TODO: make this path OS agnostic
    s.randomSeedLog.open(new_dir_path + "/_" + timestamp + "-random-seed-log.csv");
    assert(randomSeedLog.is_open());
    s.randomSeedLog << "Frame,Minute:Second.Frame,Seed" << endl;

    if (s.config["initial-onset"].get<bool>()) {
        onset(s);
    }

    string line;
    vector<string> csv_line;

    while (!descriptors_file.eof() && s.frame_num < max_frames) {
        cout << "frame num: " << s.frame_num << endl;

        if (s.verbose)
            cout << "reading descriptors..." << endl;
        // descriptors
        line.clear();
        getline(descriptors_file, line);
        csv_line.clear();
        csv_line = ofSplitString(line, ",");
        assert(csv_line.size() == (DESCRIPTORS_VECTOR_LENGTH + 1));
        vector<float> csv_line_fl(csv_line.size());
        for (int i = 0; i < csv_line.size(); i++) {
            csv_line_fl[i] = ofToFloat(csv_line[i]);
        }

        if (s.verbose) {
            cout << "set vals from csv..." << endl;
            for (int i = 0; i < csv_line_fl.size(); i++) {
                cout << csv_line_fl[i] << "\t";
            }
            cout << endl;
            cout << "number of floats: " << csv_line_fl.size() << endl;
            cout << "number of zeros:  " << std::count(csv_line_fl.begin(), csv_line_fl.end(), 0) << endl;
            ;
        }

        setValsFromCSV(s,csv_line_fl);

        if (usingReaperMarkers) {
            string rm = rmfp.currentFrame(s.frame_num);
            processReaperMarker(s,rm);
        }

        if (s.verbose)
            cout << "reading waveforms..." << endl;
        // waveforms
        line.clear();
        getline(waveform0_file, line);
        csv_line.clear();
        csv_line = ofSplitString(line, ",");
        //            cout << "\twaveform0 line size (strings): " << csv_line.size() << endl;
        assert(csv_line.size() == WAVEFORM_LEN);
        for (int i = 0; i < csv_line.size(); i++) {
            s.features.waveforms[0][i] = ofToFloat(csv_line[i]);
        }

        line.clear();
        getline(waveform1_file, line);
        csv_line.clear();
        csv_line = ofSplitString(line, ",");
        //            cout << "\twaveform1 line size (strings): " << csv_line.size() << endl;
        assert(csv_line.size() == WAVEFORM_LEN);
        for (int i = 0; i < csv_line.size(); i++) {
            s.features.waveforms[1][i] = ofToFloat(csv_line[i]);
        }

        if (s.verbose)
            cout << "reading mags..." << endl;
        // mags
        line.clear();
        getline(mags_file, line);
        csv_line.clear();
        csv_line = ofSplitString(line, ",");
        //            cout << "\tmags line size (strings): " << csv_line.size() << endl;
        assert(csv_line.size() == MAGNITUDES_LEN);
        for (int i = 0; i < csv_line.size(); i++) {
            s.features.magnitudes[0][i] = ofToFloat(csv_line[i]);
        }

        // ============     ================
        if (s.verbose) cout << "prUpdate..." << endl;
        prUpdate(s);

        // ============ DRAW ====================
        if (s.verbose) cout << "rendering frame..." << endl;
        renderFrame(s);

        if (s.verbose) cout << "save to disk..." << endl;
        // save to disk
        s.fbo.write(new_dir_path + "/" + ofToString(s.frame_num, 6, '0') + ".png");

        s.frame_num++;
    }

    descriptors_file.close();
    waveform0_file.close();
    waveform1_file.close();
    mags_file.close();

    exit();
}

//--------------------------------------------------------------
void ofApp::update() {
    if (s.verbose)
        cout << "update " << ofGetFrameNum() << endl;

    s.onset_occured = false;

    // ==================== OSC ================================
    while (osc_receiver.hasWaitingMessages()) {
        ofxOscMessage oscMsg;
        osc_receiver.getNextMessage(oscMsg);

        string address = oscMsg.getAddress();

        // TODO: strategy pattern
        // from Reaper:
        if (address == "/lastmarker/name") {
            string cmd = oscMsg.getArgAsString(0);
            processReaperMarker(s,cmd);

            // from SuperCollider:
        } else if (address == "/setActiveIndices") {
            vector<int> ai(s.active_module_indices.size());
            for (int i = 0; i < s.active_module_indices.size(); i++) {
                ai[i] = oscMsg.getArgAsInt(i);
            }
            setActiveIndices(s,ai);
        } else if (address == "/onset") {
            onset(s);
        } else if (address == "/onsetSeed") {
            unsigned long seed = oscMsg.getArgAsInt(0);
            onsetFromSeed(s,seed);
        } else if (address == "/setOnsetSwitchProb") {
            s.onsetSwitchProb = oscMsg.getArgAsFloat(0);
        } else if (address == "/setNewParamsProb") {
            int vc_i = oscMsg.getArgAsInt(0);
            s.modules[vc_i]->newParamsProb = oscMsg.getArgAsFloat(1);
        } else if (address == "/setVCOptions") {
            int n_options = oscMsg.getArgAsInt(0);
            s.vc_i_options.resize(n_options);
            for (int i = 0; i < n_options; i++) {
                s.vc_i_options[i] = oscMsg.getArgAsInt(i + 1);
            }
        } else if (address == "/cmd") {
            int index = oscMsg.getArgAsInt(0);
            std::string str = oscMsg.getArgAsString(1);
            float val = oscMsg.getArgAsFloat(2);
            s.modules[index]->receiveOSC(s, str, val);
        } else if (address == "/waveform") {
            int index = oscMsg.getArgAsInt(0);
            for (int i = 0; i < WAVEFORM_LEN; i++)
                s.features.waveforms[index][i] = oscMsg.getArgAsFloat(i + 1);
        } else if (address == "/mags") {
            int index = oscMsg.getArgAsInt(0);
            for (int i = 0; i < MAGNITUDES_LEN; i++)
                s.features.magnitudes[index][i] = oscMsg.getArgAsFloat(i + 1);
        } else if (address == "/vector") {
            for (int i = 0; i < DESCRIPTORS_VECTOR_LENGTH; i++) {
                s.features.descriptors_vector[i] = oscMsg.getArgAsFloat(i);
                s.vec_history.updateCurrentFrameAtIndex(i, s.features.descriptors_vector[i]);
            }

            s.vec_history.incrementFrameIndex();

            setValsFromCSV(s, s.features.descriptors_vector);

            s.onset_occured = s.use_sc_onsets && (oscMsg.getArgAsFloat(DESCRIPTORS_VECTOR_LENGTH) > 0.5);
        }
    }

    if (s.onset_occured)
        onset(s);

    prUpdate(s);
}

void drawBounds() {
    ofNoFill();
    ofSetColor(200, 100);
    ofDrawBox(ofGetWidth() * 0.5, ofGetHeight() * 0.5, ofGetHeight() * -0.5, ofGetWidth(), ofGetHeight(), ofGetHeight());

    ofFill();
    int offset = 20;
    for (int x = 0; x < 2; x++) {
        for (int y = 0; y < 2; y++) {
            for (int z = 0; z < 2; z++) {
                int x_ = (x * ofGetWidth()) + ((x == 0) * offset) + ((x == 1) * -offset);
                int y_ = (y * ofGetHeight()) + ((y == 0) * offset) + ((y == 1) * -offset);
                int z_ = (-z * ofGetHeight()) + ((z == 0) * -offset) + ((z == 1) * offset);
                ofSetColor(x * 255, y * 255, z * 255);
                ofDrawSphere(x_, y_, z_, 100);
            }
        }
    }

    int len = 1000;
    ofSetLineWidth(15);
    for (int dir = 0; dir < 3; dir++) {
        ofSetColor((dir == 0) * 255, (dir == 1) * 255, (dir == 2) * 255);
        ofDrawLine(0, 0, 0, (dir == 0) * len, (dir == 1) * len, (dir == 2) * -len);
    }
}

//--------------------------------------------------------------
void ofApp::draw() {
    if (s.verbose)
        cout << "draw " << ofGetFrameNum() << endl;

    renderFrame(s);
    s.fbo.draw(0, 0, ofGetWidth(), ofGetHeight());

    if (s.config["draw-bounds"].get<bool>()) {
        drawBounds();
    };

    if (s.show_frame_rate) {
        ofSetColor(255,0,0);
        ofDrawBitmapString(ofToString(ofGetFrameRate()), 10, 20);
    }

    s.frame_num++;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
    if (key == 'd')
        s.debug = !s.debug;
 
    if (key == 'c')
        loadConfigFile(s,s.config_path);

    if (key == 's')
        s.use_sc_onsets = !s.use_sc_onsets;

    if (key == 'f')
        s.show_frame_rate = !s.show_frame_rate;

    if (key == 'o')
        onset(s);
    if (key == 'p') {
        for (int i = 0; i < s.active_module_indices.size(); i++) {
            if (s.active_module_indices[i] >= 0) {
                s.modules[s.active_module_indices[i]]->newParams(s);
            }
        }
    }

    for (int i = 0; i < 10; i++) {
        if (key == s.saveStateKeys[i]) {
            s.saveStates[i] = save(s);
            std::ofstream fout(ofToDataPath(ofGetTimestampString() + "_save-" + ofToString(i) + ".json"));
            fout << s.saveStates[i];
            break;
        }
    }

    if (key == ')')
        load(s,s.saveStates[0]);
    if (key == '!')
        load(s,s.saveStates[1]);
    if (key == '@')
        load(s,s.saveStates[2]);
    if (key == '#')
        load(s,s.saveStates[3]);
    if (key == '$')
        load(s,s.saveStates[4]);
    if (key == '%')
        load(s,s.saveStates[5]);
    if (key == '^')
        load(s,s.saveStates[6]);
    if (key == '&')
        load(s,s.saveStates[7]);
    if (key == '*')
        load(s,s.saveStates[8]);
    if (key == '(')
        load(s,s.saveStates[9]);
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
    // post glitch manual controls
    //    if (key == '1') postGlitch.setFx(OFXPOSTGLITCH_CONVERGENCE    , false);
    //    if (key == '2') postGlitch.setFx(OFXPOSTGLITCH_GLOW            , false);
    //    if (key == '3') postGlitch.setFx(OFXPOSTGLITCH_SHAKER            , false);
    //    if (key == '4') postGlitch.setFx(OFXPOSTGLITCH_CUTSLIDER        , false);
    //    if (key == '5') postGlitch.setFx(OFXPOSTGLITCH_TWIST            , false);
    //    if (key == '6') postGlitch.setFx(OFXPOSTGLITCH_OUTLINE        , false);
    //    if (key == '7') postGlitch.setFx(OFXPOSTGLITCH_NOISE            , false);
    //    if (key == '8') postGlitch.setFx(OFXPOSTGLITCH_SLITSCAN        , false);
    //    if (key == '9') postGlitch.setFx(OFXPOSTGLITCH_SWELL            , false);
    //    if (key == '0') postGlitch.setFx(OFXPOSTGLITCH_INVERT            , false);
    //
    //    if (key == 'q') postGlitch.setFx(OFXPOSTGLITCH_CR_HIGHCONTRAST, false);
    //    if (key == 'w') postGlitch.setFx(OFXPOSTGLITCH_CR_BLUERAISE    , false);
    //    if (key == 'e') postGlitch.setFx(OFXPOSTGLITCH_CR_REDRAISE    , false);
    //    if (key == 'r') postGlitch.setFx(OFXPOSTGLITCH_CR_GREENRAISE    , false);
    //    if (key == 't') postGlitch.setFx(OFXPOSTGLITCH_CR_BLUEINVERT    , false);
    //    if (key == 'y') postGlitch.setFx(OFXPOSTGLITCH_CR_REDINVERT    , false);
    //    if (key == 'u') postGlitch.setFx(OFXPOSTGLITCH_CR_GREENINVERT    , false);
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {
    s.fbo.allocate(w, h);
    s.postGlitch.setup(&s.fbo.fbo);
    for (int i = 0; i < s.n_modules; i++) {
        s.modules[i]->screenResize(s);
    }
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {
}
