#include "ofApp.h"
#include <algorithm>
#include "ReaperMarkersFileParser.hpp"

//--------------------------------------------------------------
void ofApp::setup(){
    
    config.load("config.yaml");

    nrtRender = config["nrt-render"].as<bool>();
        
    int width = 0;
    int height = 0;
    
    if(nrtRender){
        width = 3840; // 4k
        height = 2160;// 4k
    } else {
        width = ofGetWidth();
        height = ofGetHeight();
    }

    main_fbo.allocate(width, height);
    postGlitch.setup(&main_fbo);
    
    active_vc_i = new int[MAX_ACTIVE_MODULES];
    
    ofBackground(0);
    ofEnableSmoothing();
    ofEnableAntiAliasing();
    ofEnableAlphaBlending();

    // ================ DATA STRUCTURES ========================

    common_features["specCentroid"] = 0;
    common_features["specSpread"] = 0;
    common_features["specSkewness"] = 0;
    common_features["specKurtosis"] = 0;
    common_features["specRolloff"] = 0;
    common_features["specFlatness"] = 0;
    common_features["specCrest"] = 0;
    common_features["pitch"] = 0;
    common_features["pitchConfidence"] = 0;
    common_features["loudness"] = 0;
    common_features["truePeak"] = 0;
    common_features["amplitude"] = 0;
    common_features["sensoryDissonance"] = 0;
    common_features["zeroCrossing"] = 0;
    
    // wavforms
    waveforms = (float**) malloc(sizeof(float*) * n_waveforms);
    for(int i = 0; i < n_waveforms; i++){
        waveforms[i] = (float*) malloc(sizeof(float) * waveform_len);
    }
    // mags
    magnitudes = new float*[n_magnitudes];
    // magnitudes = (float**) malloc(sizeof(float*) * n_magnitudes);
    for(int i = 0; i < n_magnitudes; i++){
        //magnitudes[i] = (float*) malloc(sizeof(float) * magnitude_len);
        magnitudes[i] = new float[magnitude_len];
    }

    // ================== VISUAL CONTENTS ==========================
    int vc_counter = 0;
    xsize = 1;
    ysize = 1;
    x_size_mul = 0.6;
    y_size_mul = 0.6;
    xmin = -xsize * x_size_mul;
    xmax = xsize * (1 + x_size_mul);
    ymin = -ysize * y_size_mul;
    ymax = ysize * (1 + y_size_mul);
    zmin = 0;
    zmax = ysize;
    
    // movies points
    ofVec3f initialPoints[4];
    initialPoints[0].set(0,0,-1);
    initialPoints[1].set(width,0,-1);
    initialPoints[2].set(width,height,-1);
    initialPoints[3].set(0,height,-1);
    
    // setup flow field
    ff.setup(config["flow-field-resolution"].as<int>(), xmin, xmax, ymin, ymax, zmin, zmax);
    
    // ============ setup modules ===============
    
    vc_i_options.clear();
    
    // 0: waveform
    Waveform* wf = new Waveform;
    wf->setup(width,height,waveforms,n_waveforms,waveform_len, vec_history, vector_len, vec_history_length, vec_history_full,config);
    visualModules[vc_counter] = wf;
    vc_counter = addVCOptions(vc_counter,config["modules"]["waveform"]["prob"].as<int>());

    cout << "waveform loaded vc_i_options.size(): " << vc_i_options.size() << endl;
    
    // 1: mesh
    Mesh* mesh = new Mesh;
    mesh->setup(config["modules"]["mesh"]["n-points"].as<int>(), &ff, xmin, xmax, ymin, ymax, zmin, zmax, xsize, ysize,config);
    visualModules[vc_counter] = mesh;
    vc_counter = addVCOptions(vc_counter,config["modules"]["mesh"]["prob"].as<int>());
    
    cout << "mesh loaded vc_i_options.size(): " << vc_i_options.size() << endl;
    
    // 2: mag lines
    Lines* lines0 = new Lines;
    lines0->setup(magnitudes[0],0,magnitude_len,false,width,height,vec_history, vector_len, vec_history_length, vec_history_full);
    visualModules[vc_counter] = lines0;
    vc_counter = addVCOptions(vc_counter,config["modules"]["mag-lines"]["prob"].as<int>());
    
    cout << "maglines loaded vc_i_options.size(): " << vc_i_options.size() << endl;
    
    // 3: turtle
    Turtle* turtle0 = new Turtle;
    turtle0->setup(width,height,vec_history,vector_len,vec_history_length,vec_history_full,config);
    visualModules[vc_counter] = turtle0;
    vc_counter = addVCOptions(vc_counter,config["modules"]["turtle"]["prob"].as<int>());
    
    cout << "turtle loaded vc_i_options.size(): " << vc_i_options.size() << endl;
    
    // load videos
    cout << "\n\nconfig['videos'].size(): " << config["videos"].size() << endl;
    for(int i = 0; i < config["videos"].size(); i++){
        string name = config["videos"][i]["name"].as<string>();
        int prob = config["videos"][i]["prob"].as<int>();
        cout << "\tabout to load video " << i << ": " << name << "\n";
        cout << "\t\tvc_counter = " << vc_counter << "\n";
        newHapMovie(name,vc_counter,initialPoints,width,height,config,i);
        vc_counter = addVCOptions(vc_counter,prob);
        cout << "\tvideo " << i << " loaded: " << name << "\t(prob=" << prob << ")\n";
        cout << "\t\tvc_counter = " << vc_counter << "\n\n";
    }
    
    for(int i = 0; i < N_VISUAL_CONTENTS; i++){
        cout << "vc index: " << i << visualModules[i]->type << endl;
    }
    
    // add null options to vc options
//    for(int i = 0; i < 1; i++){
//        vc_i_options.push_back(-1);
//    }
    
    // ============ setup vecHistory
    vec_history_length = mesh->nPoints;
    vec_history = new float*[vec_history_length];
    for(int i = 0; i < vec_history_length; i++){
        vec_history[i] = new float[vector_len];
        for(int j = 0; j < vector_len; j++){
            vec_history[i][j] = 0;
        }
    }
    
    // ======================= OSC ================
    osc_receiver.setup(11000);
    
    processConfigFile("config.yaml");
    
    // =========================== INITIALIZATION =====================
    for(int i = 0; i < MAX_ACTIVE_MODULES; i++){
        active_vc_i[i] = config["initial-active-modules"][i].as<int>();
        cout << "initial active module " << i << ": " << active_vc_i[i] << endl;
    }
    
    if(config["initial-onset"].as<bool>()){
        onsetOccured(width, height, 0);
    }
    
    // ofxFlowTools
    
    int densityWidth = 720;
    int densityHeight = 720;
    int simulationWidth = densityWidth / 2;
    int simulationHeight = densityHeight / 2;
    
    opticalFlow.setup(simulationWidth, simulationHeight);
//    velocityBridgeFlow.setup(simulationWidth, simulationHeight);
//    densityBridgeFlow.setup(simulationWidth, simulationHeight, densityWidth, densityHeight);
//    temperatureBridgeFlow.setup(simulationWidth, simulationHeight);
    combinedBridgeFlow.setup(simulationWidth, simulationHeight, densityWidth, densityHeight);
    fluidFlow.setup(simulationWidth, simulationHeight, densityWidth, densityHeight);
    
    flows.push_back(&opticalFlow);
//    flows.push_back(&velocityBridgeFlow);
//    flows.push_back(&densityBridgeFlow);
//    flows.push_back(&temperatureBridgeFlow);
    flows.push_back(&combinedBridgeFlow);
    flows.push_back(&fluidFlow);
    
    //flowToolsLogo.load("flowtools.png");
    //fluidFlow.addObstacle(flowToolsLogo.getTexture());
    
    // =========== NRT RENDERING
    if(nrtRender){
        runNrtRender(width,height);
    }
}

void ofApp::runNrtRender(int width, int height){
    string csv_folder = config["csv-folder"].as<string>();
    
    //int max_frames = 600; // 600 frames = 20 seconds
    int max_frames = config["max-frames"].as<int>();
    
    ifstream descriptors_file;
    descriptors_file.open(csv_folder + "/descriptors.csv");
    ifstream waveform0_file;
    waveform0_file.open(csv_folder + "/waveform-0.csv");
    ifstream waveform1_file;
    waveform1_file.open(csv_folder + "/waveform-1.csv");
    ifstream mags_file;
    mags_file.open(csv_folder + "/mags.csv");
    
    ReaperMarkersFileParser rmfp;
    rmfp.setup(csv_folder + "/reaper-markers.txt",config["audio-sample-rate"].as<int>(),config["target-framerate"].as<int>());

    // stuff for rendering
    
    string new_dir_path;
    ofFileDialogResult result = ofSystemSaveDialog("", "Choose location to save frames");
    if(result.bSuccess) {
      new_dir_path = result.getPath();
    } else {
        ofExit();
    }
    
    ofDirectory new_dir(new_dir_path);
    new_dir.create();
    
    ofPixels pix;
    
    int frame_num = 0;
    
    string line;
    vector<string> csv_line;
    
    while(!descriptors_file.eof() && frame_num < max_frames){
        
        cout << "frame num: " << frame_num << endl;
        
        // descriptors
        line.clear();
        getline(descriptors_file,line);
        csv_line.clear();
        csv_line = ofSplitString(line,",");
//            cout << "\tdescriptors line size (strings): " << csv_line.size() << endl;
        vector<float> csv_line_fl(csv_line.size());
        for(int i = 0; i < csv_line.size(); i++){
            csv_line_fl[i] = ofToFloat(csv_line[i]);
        }
//            cout << "\tdescriptors line size (floats) : " << csv_line_fl.size() << endl;

        setValsFromCSV(width,height,csv_line_fl,frame_num);
        
        string rm = rmfp.currentFrame(frame_num);
        cout << "from rmfp: " << rm << endl;
        processReaperMarker(rm,main_fbo.getWidth(), main_fbo.getHeight(),frame_num);
        
        // waveforms
        line.clear();
        getline(waveform0_file,line);
        csv_line.clear();
        csv_line = ofSplitString(line,",");
//            cout << "\twaveform0 line size (strings): " << csv_line.size() << endl;
        for(int i = 0; i < csv_line.size(); i++){
            waveforms[0][i] = ofToFloat(csv_line[i]);
        }
        
        line.clear();
        getline(waveform1_file,line);
        csv_line.clear();
        csv_line = ofSplitString(line,",");
//            cout << "\twaveform1 line size (strings): " << csv_line.size() << endl;
        for(int i = 0; i < csv_line.size(); i++){
            waveforms[1][i] = ofToFloat(csv_line[i]);
        }
                    
        // mags
        line.clear();
        getline(mags_file,line);
        csv_line.clear();
        csv_line = ofSplitString(line,",");
//            cout << "\tmags line size (strings): " << csv_line.size() << endl;
        for(int i = 0; i < csv_line.size(); i++){
            magnitudes[0][i] = ofToFloat(csv_line[i]);
        }
        
        // ============     ================
        prUpdate(true);
        
        // ============ DRAW ====================
        drawScreen(main_fbo.getWidth(), main_fbo.getHeight(), frame_num, true);

        // save to disk
        main_fbo.readToPixels(pix);
        ofSaveImage(pix, new_dir_path+"/"+ofToString(frame_num,6,'0')+".tiff",OF_IMAGE_QUALITY_BEST);
        
        frame_num++;
    }
    
    descriptors_file.close();
    waveform0_file.close();
    waveform1_file.close();
    mags_file.close();
    
    ofExit();
}

void ofApp::processReaperMarker(string& cmd, int width, int height, unsigned long long frame_num){
    vector<string> tokens = ofSplitString(cmd," ");
    int index = 0;
    
//    cout << "reaper marker: " << cmd << endl;
    
    while(index < tokens.size()){
        
//        cout << "index: " << index << " " << tokens[index] << endl;
        
        if(tokens[index] == "onset"){
            onsetOccured(main_fbo.getWidth(),main_fbo.getHeight(),frame_num);
            
        } else if(tokens[index] == "setActiveIndices"){
            int ai[MAX_ACTIVE_MODULES];
            
//            cout << "setActiveIndices: ";
            
            for(int i = 0; i < MAX_ACTIVE_MODULES; i++){
                ai[i] = ofToInt(tokens[++index]);
//                cout << ai[i] << " ";
            }
//            cout << endl;
            setActiveIndices(ai, main_fbo.getWidth(), main_fbo.getHeight(),frame_num);
        } else if(tokens[index] == "loadState"){
            load(saves[ofToInt(tokens[++index])],width,height);
        } else if(tokens[index] == "loadStateFromDisk"){
            ofFile file(ofToDataPath(tokens[++index] + ".yaml"));
            
            if(file.exists()){
                ofxYAML dict;
                dict.load(file.path());
                cout << dict << endl;
                load(dict,width,height);
            }else{
                cout << "ofApp::processReaperMarker loadStateFromDisk WARNING: There is no file on disk at that path: " << file.path() << endl;
            }
        } else if(tokens[index] == "setParameter"){
            int moduleIndex = ofToInt(tokens[++index]);
            string label = tokens[++index];
            float val = ofToFloat(tokens[++index]);
            visualModules[moduleIndex]->receiveOSC(width,height,label,val);
        }
        
        index++; // always increment at least one!
    }
}

void ofApp::setValsFromCSV(int width, int height, vector<float>& csv_data, unsigned long long frame_num){
    
    common_features["specCentroid"] = vector_data[0];
    common_features["specSpread"] = vector_data[1];
    common_features["specSkewness"] = vector_data[2];
    common_features["specKurtosis"] = vector_data[3];
    common_features["specRolloff"] = vector_data[4];
    common_features["specFlatness"] = vector_data[5];
    common_features["specCrest"] = vector_data[6];
    common_features["pitch"] = vector_data[7];
    common_features["pitchConfidence"] = vector_data[8];
    common_features["loudness"] = vector_data[9];
    common_features["truePeak"] = vector_data[10];
    common_features["amplitude"] = vector_data[11];
    common_features["sensoryDissonance"] = vector_data[12];
    common_features["zeroCrossing"] = vector_data[13];
    
    onset_occured = false;
    
//    cout << "csv_data.size(): " << csv_data.size() << endl;
    
    float onset_val = csv_data[csv_data.size() - 1];
    if(onset_val > 0.5 && use_sc_onsets){
        onset_occured = true;
        onsetOccured(width,height,frame_num); // onsets
    }
    
//    cout << "\tonset val: " << onset_val << " \tonset occured: " << onset_occured << endl;
    
    for (int i = 0; i < csv_data.size() - 1; i++){
        float val = csv_data[i];
        vector_data[i] = val;
        vec_history[vec_history_counter][i] = val;
    }
    
    incrementVecHistoryCounter();
}

void ofApp::incrementVecHistoryCounter(){
    // check if we just added the last index to the history and if so set true
    if(vec_history_counter == (vec_history_length - 1)) vec_history_full = true;
    
    // increment and modulous
    vec_history_counter = (vec_history_counter + 1) % vec_history_length;
}

void ofApp::newHapMovie(std::string path, int index, ofVec3f* initPts, int width, int height, ofxYAML& config, int videoIndex){
    HapMovie* vc = new HapMovie;
    cout << "\t\tofApp::newHapMovie loading " << path << endl;
    vc->setup(path,initPts[0],initPts[1],initPts[2],initPts[3],magnitudes,n_magnitudes,magnitude_len,nrtRender,&ff,config, videoIndex);
    vc->newParams(width, height, vec_history, vector_len, vec_history_length, vec_history_full,0);
    cout << "\t\tadding " << path << "\t at index " << index << endl;
    visualModules[index] = vc;
}

void ofApp::setActiveIndices(int* ai,int width, int height, unsigned long long frame_num){
    for(int i = 0; i < MAX_ACTIVE_MODULES; i++){
        active_vc_i[i] = ai[i];
        if(active_vc_i[i] >= 0 && visualModules[active_vc_i[i]]->newParamsProb > ofRandom(1.f)){
            visualModules[active_vc_i[i]]->newParams(width,height,vec_history, vector_len, vec_history_length, vec_history_full,frame_num);
        }
    }
}

void ofApp::onsetOccured(int width, int height,unsigned long long frame_num){

    // new active vc i
    
    vector<int> chosen_i;
    int ai[MAX_ACTIVE_MODULES];
    for(int i = 0; i < MAX_ACTIVE_MODULES; i++){ // go through the max number that we'll display
        if(ofRandom(1.f) < onsetSwitchProb){
            bool found = false;
            while(!found){
                // options array is the big pool of options (has duplicates based on probs)
                int rand_int = ofRandom(1.f) * vc_i_options.size(); // random int the size of the options array
                int result = vc_i_options[rand_int]; // the int in the from the options array (which is the index for the modules array)
                if(!std::count(chosen_i.begin(), chosen_i.end(), result)){
                    found = true;
                    chosen_i.push_back(result);
                    ai[i] = result;
                }
            }
        }else{
            ai[i] = active_vc_i[i];
            chosen_i.push_back(ai[i]);
        }
    }

    setActiveIndices(ai, main_fbo.getWidth(), main_fbo.getHeight(),frame_num);

    // blend mode
    if(ofRandom(1.f) < onsetSwitchProb){
        int blendMode_i = int(ofRandom(blendModePool.size()));
        blendMode = blendModes[blendModePool[blendMode_i]];
    }
    
    feedback_amt = (ofRandom(1.f) < feedback_prob) * ofRandom(1, feedback_max);
    show_flow_tools = ofRandom(1.f) < show_flow_prob;
    
    // ofx post glitch
    postGlitch.newParams();
    flow_then_postGlitch = ofRandom(1.f) < 0.95;
    fluid_flow_orientation = (FluidFlowOrientation)ofRandom(4);
    
    for(int i = 0; i < GLITCH_NUM; i++){
        if(ofRandom(1.f) < postGlitchChangeProb){
            if(ofRandom(1.f) < postGlitchProbs[i]){
                postGlitch.setFx((ofxPostGlitchType)i,true);
            } else {
                postGlitch.setFx((ofxPostGlitchType)i,false);
            }
        }
    }
}

//--------------------------------------------------------------
void ofApp::update(){
    onset_occured = false;
    
    // ==================== OSC ================================
    while(osc_receiver.hasWaitingMessages()){
        ofxOscMessage oscMsg;
        osc_receiver.getNextMessage(oscMsg);
        
        string address = oscMsg.getAddress();
        
        // from Reaper:
        if(address == "/lastmarker/name"){
            string cmd = oscMsg.getArgAsString(0);
            processReaperMarker(cmd,ofGetWidth(),ofGetHeight(),ofGetFrameNum());
            
            // from SuperCollider:
        } else if(address == "/setActiveIndices"){
            int ai[MAX_ACTIVE_MODULES];
            for(int i = 0; i < MAX_ACTIVE_MODULES; i++){
                ai[i] = oscMsg.getArgAsInt(i);
            }
            setActiveIndices(ai,main_fbo.getWidth(),main_fbo.getHeight(),ofGetFrameNum());
        } else if (address == "/setOnsetSwitchProb"){
            onsetSwitchProb = oscMsg.getArgAsFloat(0);
        } else if (address == "/setNewParamsProb"){
            int vc_i = oscMsg.getArgAsInt(0);
            visualModules[vc_i]->newParamsProb = oscMsg.getArgAsFloat(1);
        } else if (address == "/setVCOptions"){
            int n_options = oscMsg.getArgAsInt(0);
            vc_i_options.resize(n_options);
            for(int i = 0; i < n_options; i++){
                vc_i_options[i] = oscMsg.getArgAsInt(i+1);
            }
        } else if (address == "/cmd"){
            int index = oscMsg.getArgAsInt(0);
            std::string str = oscMsg.getArgAsString(1);
            float val = oscMsg.getArgAsFloat(2);
            visualModules[index]->receiveOSC(ofGetWidth(),ofGetHeight(),str, val);
        } else if (address == "/waveform") {
            int index = oscMsg.getArgAsInt(0);
//            cout << "received waveform: " << index << endl;
            for(int i = 0; i < waveform_len; i++){
                waveforms[index][i] = oscMsg.getArgAsFloat(i+1);
            }
        } else if (address == "/mags") {
            int index = oscMsg.getArgAsInt(0);
//            cout << "mag index: " << index << "\n";
            for(int i = 0; i < magnitude_len; i++){
                magnitudes[index][i] = oscMsg.getArgAsFloat(i+1);
//                cout << magnitudes[index][i] << " ";
            }
//            cout << "\n";
        } else if (address == "/vector") {
            for(int i = 0; i < vector_len; i++){
                float val = oscMsg.getArgAsFloat(i);
//                cout << val << " ";
                vector_data[i] = val;
                vec_history[vec_history_counter][i] = val;
            }
//            cout << endl;
            
            incrementVecHistoryCounter();
            
            common_features["specCentroid"] = vector_data[0];
            common_features["specSpread"] = vector_data[1];
            common_features["specSkewness"] = vector_data[2];
            common_features["specKurtosis"] = vector_data[3];
            common_features["specRolloff"] = vector_data[4];
            common_features["specFlatness"] = vector_data[5];
            common_features["specCrest"] = vector_data[6];
            common_features["pitch"] = vector_data[7];
            common_features["pitchConfidence"] = vector_data[8];
            common_features["loudness"] = vector_data[9];
            common_features["truePeak"] = vector_data[10];
            common_features["amplitude"] = vector_data[11];
            common_features["sensoryDissonance"] = vector_data[12];
            common_features["zeroCrossing"] = vector_data[13];
            
            onset_occured = use_sc_onsets && (oscMsg.getArgAsFloat(106) > 0.5);
        }
    }
    
    if(onset_occured){
        onsetOccured(main_fbo.getWidth(),main_fbo.getHeight(),ofGetFrameNum());
    }
    
    prUpdate(false);
}

void ofApp::prUpdate(bool isNRT){
        
    for(int i = 0; i < N_VISUAL_CONTENTS; i++){
        visualModules[i]->update(isNRT,&common_features);
    }
    
    // ofxFlowTools
            
    opticalFlow.setInput(main_fbo.getTexture());
    
    opticalFlow.update();
    
    combinedBridgeFlow.setVelocity(opticalFlow.getVelocity());
    combinedBridgeFlow.setDensity(main_fbo.getTexture());
    float dt = 1.0 / max(ofGetFrameRate(), 1.f); // more smooth as 'real' deltaTime.
    combinedBridgeFlow.update(dt);
    
//    velocityBridgeFlow.setVelocity(opticalFlow.getVelocity());
//    velocityBridgeFlow.update(dt);
//    densityBridgeFlow.setDensity(cameraFbo.getTexture());
//    densityBridgeFlow.setVelocity(opticalFlow.getVelocity());
//    densityBridgeFlow.update(dt);
//    temperatureBridgeFlow.setDensity(cameraFbo.getTexture());
//    temperatureBridgeFlow.setVelocity(opticalFlow.getVelocity());
//    temperatureBridgeFlow.update(dt);
    
    fluidFlow.addVelocity(combinedBridgeFlow.getVelocity());
    fluidFlow.addDensity(combinedBridgeFlow.getDensity());
    fluidFlow.addTemperature(combinedBridgeFlow.getTemperature());
    fluidFlow.update(dt);
}

void ofApp::drawScreen(int width, int height, unsigned long long frameNum, bool isNRT){
    
    main_fbo.begin();
    
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    
    ofSetColor(0,255 - feedback_amt); // alpha of 255 = no feedback, alpha of 0 = full feedback
    ofDrawRectangle(0, 0, main_fbo.getWidth(), main_fbo.getHeight());
    
    // =============== visualModules ===================
    // TODO: investigate if it makes sense to just have this _always_ be OF_BLENDMODE_ADD
//    ofEnableBlendMode(OF_BLENDMODE_ADD);
//    ofEnableBlendMode(blendMode);
    
    if(!debug){
        ff.update(frameNum, &common_features);
        
        for(int i = 0; i < MAX_ACTIVE_MODULES; i++){
            int index = active_vc_i[i];
            if(index >= 0){
                for(int j = 0; j < MAX_ACTIVE_MODULES; j++){
                    if((j != i) && (active_vc_i[j] >= 0)){
                        visualModules[index]->interact(visualModules[active_vc_i[j]]);
                    }
                }
                visualModules[index]->display(width,height,frameNum,&common_features, isNRT);
            }
        }
    } else {
        displayIncomingData(width,height);
    }
    
    // ============ ofxPostGlitch & ofxFlowTools ===============
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    if(flow_then_postGlitch){
        if(show_flow_tools){
            //    combinedBridgeFlow.drawInput(0, 0, width, height);
            //    opticalFlow.drawInput(0, 0, width, height);
            //    opticalFlow.draw(0, 0, width, height);
            //    combinedBridgeFlow.drawVelocity(0, 0, width, height);
            //    combinedBridgeFlow.drawDensity(0, 0, width, height);
            //    combinedBridgeFlow.drawTemperature(0, 0, width, height);
            //    fluidFlow.drawObstacle(0, 0, width, height);
            //    fluidFlow.drawObstacleOffset(0, 0, width, height);
            //    fluidFlow.drawBuoyancy(0, 0, width, height);
            //    fluidFlow.drawVorticity(0, 0, width, height);
            //    fluidFlow.drawDivergence(0, 0, width, height);
            //    fluidFlow.drawTemperature(0, 0, width, height);
            //    fluidFlow.drawPressure(0, 0, width, height);
            //    fluidFlow.drawVelocity(0, 0, width, height);
            drawFluidFlow(width,height);
        }
        main_fbo.end();
        postGlitch.generateFx(&common_features);
    } else {
        main_fbo.end();
        postGlitch.generateFx(&common_features);
        if(show_flow_tools){
            main_fbo.begin();
            //    combinedBridgeFlow.drawInput(0, 0, width, height);
            //    opticalFlow.drawInput(0, 0, width, height);
            //    opticalFlow.draw(0, 0, width, height);
            //    combinedBridgeFlow.drawVelocity(0, 0, width, height);
            //    combinedBridgeFlow.drawDensity(0, 0, width, height);
            //    combinedBridgeFlow.drawTemperature(0, 0, width, height);
            //    fluidFlow.drawObstacle(0, 0, width, height);
            //    fluidFlow.drawObstacleOffset(0, 0, width, height);
            //    fluidFlow.drawBuoyancy(0, 0, width, height);
            //    fluidFlow.drawVorticity(0, 0, width, height);
            //    fluidFlow.drawDivergence(0, 0, width, height);
            //    fluidFlow.drawTemperature(0, 0, width, height);
            //    fluidFlow.drawPressure(0, 0, width, height);
            //    fluidFlow.drawVelocity(0, 0, width, height);
            drawFluidFlow(width,height);
            main_fbo.end();
        }
    }
}

void ofApp::drawFluidFlow(int width, int height){
    switch(fluid_flow_orientation){
        case TOP:
            fluidFlow.draw(0, 0, width, height);
            break;
        case RIGHT:
            ofPushMatrix();
//            ofRotateDeg(90, width/2, height/2, 0);
            ofTranslate(width/2,height/2);
            ofRotateZDeg(90);
            fluidFlow.draw(height * -0.5, width * -0.5, height, width);
            ofPopMatrix();
            break;
        case BOTTOM:
            fluidFlow.draw(width, height, -width, -height);
            break;
        case LEFT:
            ofPushMatrix();
            //ofRotateDeg(-90, width/2, height/2, 0);
            ofTranslate(width/2,height/2);
            ofRotateZDeg(-90);
            fluidFlow.draw(height * -0.5, width * -0.5, height, width);
            ofPopMatrix();
            break;
    }
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    drawScreen(main_fbo.getWidth(),main_fbo.getHeight(),ofGetFrameNum(),false);
    main_fbo.draw(0,0,ofGetWidth(),ofGetHeight());
    
//    ofEnableDepthTest(); //this should be off!
//    drawBounds();
//    ofDisableDepthTest();
}

void ofApp::drawBounds(){
    
    ofNoFill();
    ofSetColor(200,100);
    ofDrawBox(ofGetWidth()*0.5, ofGetHeight()*0.5, ofGetHeight() * -0.5, ofGetWidth(), ofGetHeight(), ofGetHeight());
    
    ofFill();
    int offset = 20;
    for(int x = 0; x < 2; x++){
        for(int y = 0; y < 2; y++){
            for(int z = 0; z < 2; z++){
                int x_ = (x * ofGetWidth()) + ((x==0)*offset) + ((x==1) * -offset);
                int y_ = (y * ofGetHeight()) + ((y==0)*offset) + ((y==1) * -offset);
                int z_ = (-z * ofGetHeight()) + ((z==0) * -offset) + ((z==1) * offset);
                ofSetColor(x * 255, y * 255, z * 255);
                ofDrawSphere(x_,y_,z_,100);
            }
        }
    }
    
    int len = 1000;
    ofSetLineWidth(15);
    for(int dir = 0; dir < 3; dir++){
        ofSetColor((dir == 0) * 255, (dir == 1) * 255, (dir == 2) * 255);
        ofDrawLine(0, 0, 0, (dir == 0) * len, (dir == 1) * len, (dir == 2) * -len);
    }
}
    
void ofApp::displayIncomingData(int width, int height){

    // mags
    float xoff = 20;
    int yoff = 20;
    int ystart = height - yoff;
    int mag_height = (height / 2) - (yoff * 2);
    int bar_width = 2;
    int bar_skip = bar_width + 1;
    ofFill();
    ofSetLineWidth(0);
    for(int i = 0; i < n_magnitudes; i++){
        ofSetColor(255,200 - (i * 100));
        for(int x = 0; x < magnitude_len; x++){
            int bar_height = magnitudes[i][x] * mag_height;
            ofDrawRectangle(xoff + (x * bar_skip), ystart - bar_height,bar_width,bar_height);
        }
    }
    
    // vector
    xoff = 20;
    yoff = 20;
    ystart = (height / 2) - yoff;
    int vec_height = (height / 2) - (yoff * 2);
    bar_width = 3;
    bar_skip = bar_width + 2; // gap of 1
    ofFill();
    ofSetLineWidth(0);
    for(int i = 0; i < vector_len; i++){
        int bar_height = vector_data[i] * vec_height;
        if(i > 65){
            ofSetColor(255);
        } else if (i > 53){
            ofSetColor(255,50,50);
        } else if (i > 13){
            ofSetColor(50,50,255);
        } else if (i > 6) {
            ofSetColor(50,255,50);
        } else {
            ofSetColor(0,255,255);
        }
        ofDrawRectangle(xoff + (i * bar_skip), ystart - bar_height, bar_width, bar_height);
    }
    
    // onset
    if(onset_occured){
        ofSetColor(255,255,0);
        ofDrawRectangle((width / 2) - 20,20,40,40);
    }
    
    // waveforms
    xoff = (width * 0.35) + 20;
    int xend = width - 20;
    int wf_height = (height / 2) - 40;
    int middle = (wf_height/2) + 20;
    ofNoFill();
    ofSetLineWidth(1);
    for(int i = 0; i < n_waveforms; i++){
        switch(i){
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
        for(int x = 0; x < waveform_len; x++){
            int xpt = ofMap(x,0,waveform_len-1,xoff,xend);
            float ypt = middle + (waveforms[i][x] * -0.5 * wf_height);
            ofVertex(xpt,ypt);
        }
        ofEndShape();
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if(key == 'd'){
        debug = !debug;
    }
    
//    if(key == 'g'){
//        flow_then_postGlitch = !flow_then_postGlitch;
//    }
    
    // post glitch manual controls
//    if (key == '1') postGlitch.setFx(OFXPOSTGLITCH_CONVERGENCE    , true);
//    if (key == '2') postGlitch.setFx(OFXPOSTGLITCH_GLOW            , true);
//    if (key == '3') postGlitch.setFx(OFXPOSTGLITCH_SHAKER            , true);
//    if (key == '4') postGlitch.setFx(OFXPOSTGLITCH_CUTSLIDER        , true);
//    if (key == '5') postGlitch.setFx(OFXPOSTGLITCH_TWIST            , true);
//    if (key == '6') postGlitch.setFx(OFXPOSTGLITCH_OUTLINE        , true);
//    if (key == '7') postGlitch.setFx(OFXPOSTGLITCH_NOISE            , true);
//    if (key == '8') postGlitch.setFx(OFXPOSTGLITCH_SLITSCAN        , true);
//    if (key == '9') postGlitch.setFx(OFXPOSTGLITCH_SWELL            , true);
//    if (key == '0') postGlitch.setFx(OFXPOSTGLITCH_INVERT            , true);
//
//    if (key == 'q') postGlitch.setFx(OFXPOSTGLITCH_CR_HIGHCONTRAST, true);
//    if (key == 'w') postGlitch.setFx(OFXPOSTGLITCH_CR_BLUERAISE    , true);
//    if (key == 'e') postGlitch.setFx(OFXPOSTGLITCH_CR_REDRAISE    , true);
//    if (key == 'r') postGlitch.setFx(OFXPOSTGLITCH_CR_GREENRAISE    , true);
//    if (key == 't') postGlitch.setFx(OFXPOSTGLITCH_CR_BLUEINVERT    , true);
//    if (key == 'y') postGlitch.setFx(OFXPOSTGLITCH_CR_REDINVERT    , true);
//    if (key == 'u') postGlitch.setFx(OFXPOSTGLITCH_CR_GREENINVERT    , true);
    
    if (key == 'c') processConfigFile("config.yaml");
    
    if (key == 'o') onsetOccured(ofGetWidth(),ofGetHeight(),ofGetFrameNum());
    if (key == 'p'){
        for(int i = 0; i < MAX_ACTIVE_MODULES; i++){
            if(active_vc_i[i] >= 0){
                visualModules[active_vc_i[i]]->newParams(ofGetWidth(),ofGetHeight(),vec_history, vector_len, vec_history_length, vec_history_full,ofGetFrameNum());
            }
        }
    }
    
    char saveKeys[10] = {'0','1','2','3','4','5','6','7','8','9'};
    
    for(int i = 0; i < 10; i++){
        if(key == saveKeys[i]){
            saves[i] = save();
            std::ofstream fout(ofToDataPath(ofGetTimestampString() + "_save-" + ofToString(i) + ".yaml"));
            fout << saves[i];
            break;
        }
    }

    if(key == ')') load(saves[0],ofGetWidth(),ofGetHeight());
    if(key == '!') load(saves[1],ofGetWidth(),ofGetHeight());
    if(key == '@') load(saves[2],ofGetWidth(),ofGetHeight());
    if(key == '#') load(saves[3],ofGetWidth(),ofGetHeight());
    if(key == '$') load(saves[4],ofGetWidth(),ofGetHeight());
    if(key == '%') load(saves[5],ofGetWidth(),ofGetHeight());
    if(key == '^') load(saves[6],ofGetWidth(),ofGetHeight());
    if(key == '&') load(saves[7],ofGetWidth(),ofGetHeight());
    if(key == '*') load(saves[8],ofGetWidth(),ofGetHeight());
    if(key == '(') load(saves[9],ofGetWidth(),ofGetHeight());
    
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
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
void ofApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    main_fbo.allocate(w, h);
    postGlitch.setup(&main_fbo);
    for(int i = 0; i < N_VISUAL_CONTENTS; i++){
        visualModules[i]->screenResize(w, h);
    }
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
    
}
