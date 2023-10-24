#include "ofApp.h"
#include <algorithm>
#include "ReaperMarkersFileParser.hpp"

//--------------------------------------------------------------
void ofApp::setup(){
    
    // the config file is loaded here just so that we know whether
    // or not this is a nrt render
    std::ifstream i(ofToDataPath(CONFIG_PATH));
    i >> config;
    
    nrtRender = config["nrt-render"].get<bool>();
        
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
    
    active_module_indices = new int[MAX_ACTIVE_MODULES];
    
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
    waveforms = (float**) malloc(sizeof(float*) * N_WAVEFORMS);
    for(int i = 0; i < N_WAVEFORMS; i++){
        waveforms[i] = (float*) malloc(sizeof(float) * WAVEFORM_LEN);
    }
    // mags
    magnitudes = new float*[N_MAGNITUDES];
    // magnitudes = (float**) malloc(sizeof(float*) * N_MAGNITUDES);
    for(int i = 0; i < N_MAGNITUDES; i++){
        //magnitudes[i] = (float*) malloc(sizeof(float) * MAGNITUDES_LEN);
        magnitudes[i] = new float[MAGNITUDES_LEN];
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
    ff.setup(config["flow-field-resolution"].get<int>(), xmin, xmax, ymin, ymax, zmin, zmax);
    
    // ============ setup modules ===============
    
    vc_i_options.clear();
    
    modules.resize(config["modules"].size());
    
    // 0: waveform
    
    for(nlohmann::json j : config["modules"]){
        if(j["module-type"] == "waveform"){
            Waveform* wf = new Waveform;
            wf->setup(width,height,waveforms, vec_history, DESCRIPTORS_VECTOR_LENGTH, vec_history_length, vec_history_full,j);
            modules[vc_counter] = wf;
            vc_counter = addVCOptions(vc_counter,j["prob"].get<int>());
        } else if (j["module-type"] == "mesh"){
            Mesh* mesh = new Mesh;
            mesh->setup(&ff, xmin, xmax, ymin, ymax, zmin, zmax, xsize, ysize,j);
            modules[vc_counter] = mesh;
            vc_counter = addVCOptions(vc_counter,j["prob"].get<int>());
        } else if (j["module-type"] == "mag-lines"){
            Lines* lines0 = new Lines;
            lines0->setup(magnitudes[0],0,MAGNITUDES_LEN,false,width,height,vec_history, DESCRIPTORS_VECTOR_LENGTH, vec_history_length, vec_history_full);
            modules[vc_counter] = lines0;
            vc_counter = addVCOptions(vc_counter,j["prob"].get<int>());
        } else if (j["module-type"] == "turtle"){
            Turtle* turtle0 = new Turtle;
            turtle0->setup(width,height,vec_history,DESCRIPTORS_VECTOR_LENGTH,vec_history_length,vec_history_full,j);
            modules[vc_counter] = turtle0;
            vc_counter = addVCOptions(vc_counter,j["prob"].get<int>());
        } else if (j["module-type"] == "video"){
            string name = j["name"].get<string>();
            VideoModule* vc = new VideoModule;
            vc->setup(name,initialPoints[0],initialPoints[1],initialPoints[2],initialPoints[3],magnitudes,N_MAGNITUDES,MAGNITUDES_LEN,nrtRender,&ff,j);
            vc->newParams(width, height, vec_history, DESCRIPTORS_VECTOR_LENGTH, vec_history_length, vec_history_full,0);
            modules[vc_counter] = vc;
            vc_counter = addVCOptions(vc_counter,j["prob"].get<int>());
        }
    }
    
    // set how many modules there are total
    n_modules = vc_counter;
    
    // ============ setup vecHistory
    vec_history_length = getVectorHistoryLength();
    
    vec_history = new float*[vec_history_length];
    for(int i = 0; i < vec_history_length; i++){
        vec_history[i] = new float[DESCRIPTORS_VECTOR_LENGTH];
        for(int j = 0; j < DESCRIPTORS_VECTOR_LENGTH; j++){
            vec_history[i][j] = 0;
        }
    }
    
    // ======================= OSC ================
    osc_receiver.setup(11000);
    
    loadConfigFile(CONFIG_PATH);
    
    // =========================== INITIALIZATION =====================
    
    if(config["initial-onset"].get<bool>()){
        onsetOccurred(width, height, 0);
    }
    
    // =========== NRT RENDERING =====================
    if(nrtRender){
        runNrtRender(width,height);
    }
}

void ofApp::runNrtRender(int width, int height){
    string csv_folder = config["csv-folder"].get<string>();
    
    //int max_frames = 600; // 600 frames = 20 seconds
    int max_frames = config["max-frames"].get<int>() == -1 ? INT_MAX : config["max-frames"].get<int>();
    
    ifstream descriptors_file;
    descriptors_file.open(csv_folder + "/descriptors.csv");
    ifstream waveform0_file;
    waveform0_file.open(csv_folder + "/waveform-0.csv");
    ifstream waveform1_file;
    waveform1_file.open(csv_folder + "/waveform-1.csv");
    ifstream mags_file;
    mags_file.open(csv_folder + "/mags.csv");
    
    string reaperMarkerPath = csv_folder + "/reaper-markers.txt";
    bool usingReaperMarkers = ofFile(reaperMarkerPath).exists();
    ReaperMarkersFileParser rmfp;

    if(usingReaperMarkers) rmfp.setup(reaperMarkerPath,config["audio-sample-rate"].get<int>(),config["target-framerate"].get<int>());

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
        
        if(verbose) cout << "reading descriptors..." << endl;
        // descriptors
        line.clear();
        getline(descriptors_file,line);
        csv_line.clear();
        csv_line = ofSplitString(line,",");
        assert(csv_line.size() == (DESCRIPTORS_VECTOR_LENGTH + 1));
        vector<float> csv_line_fl(csv_line.size());
        for(int i = 0; i < csv_line.size(); i++){
            csv_line_fl[i] = ofToFloat(csv_line[i]);
        }
        
        if(verbose){
            cout << "set vals from csv..." << endl;
            for(int i = 0; i < csv_line_fl.size(); i++){
                cout << csv_line_fl[i] << "\t";
            }
            cout << endl;
            cout << "number of floats: " << csv_line_fl.size() << endl;
            cout << "number of zeros:  " << std::count(csv_line_fl.begin(),csv_line_fl.end(),0) << endl;;
        }
        
        setValsFromCSV(width,height,csv_line_fl,frame_num);
        
        if(usingReaperMarkers){
            string rm = rmfp.currentFrame(frame_num);
            cout << "from rmfp: " << rm << endl;
            processReaperMarker(rm,main_fbo.getWidth(), main_fbo.getHeight(),frame_num);
        }
        
        if(verbose) cout << "reading waveforms..." << endl;
        // waveforms
        line.clear();
        getline(waveform0_file,line);
        csv_line.clear();
        csv_line = ofSplitString(line,",");
//            cout << "\twaveform0 line size (strings): " << csv_line.size() << endl;
        assert(csv_line.size() == WAVEFORM_LEN);
        for(int i = 0; i < csv_line.size(); i++){
            waveforms[0][i] = ofToFloat(csv_line[i]);
        }
        
        line.clear();
        getline(waveform1_file,line);
        csv_line.clear();
        csv_line = ofSplitString(line,",");
//            cout << "\twaveform1 line size (strings): " << csv_line.size() << endl;
        assert(csv_line.size() == WAVEFORM_LEN);
        for(int i = 0; i < csv_line.size(); i++){
            waveforms[1][i] = ofToFloat(csv_line[i]);
        }
                 
        if(verbose) cout << "reading mags..." << endl;
        // mags
        line.clear();
        getline(mags_file,line);
        csv_line.clear();
        csv_line = ofSplitString(line,",");
//            cout << "\tmags line size (strings): " << csv_line.size() << endl;
        assert(csv_line.size() == MAGNITUDES_LEN);
        for(int i = 0; i < csv_line.size(); i++){
            magnitudes[0][i] = ofToFloat(csv_line[i]);
        }
        
        // ============     ================
        if(verbose) cout << "prUpdate..." << endl;
        prUpdate(true);
        
        // ============ DRAW ====================
        if(verbose) cout << "rendering frame..." << endl;
        renderFrame(main_fbo.getWidth(), main_fbo.getHeight(), frame_num, true);

        if(verbose) cout << "save to disk..." << endl;
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
    
    while(index < tokens.size()){
        
        if(tokens[index] == "o"){
            onsetOccurred(main_fbo.getWidth(),main_fbo.getHeight(),frame_num);
        } else if(tokens[index] == "sai"){
            int ai[MAX_ACTIVE_MODULES];
            for(int i = 0; i < MAX_ACTIVE_MODULES; i++){
                ai[i] = ofToInt(tokens[++index]);
            }
            setActiveIndices(ai, main_fbo.getWidth(), main_fbo.getHeight(),frame_num);
        } else if(tokens[index] == "loadState"){
            load(saves[ofToInt(tokens[++index])],width,height);
        } else if(tokens[index] == "loadStateFromDisk"){
            ofFile file(ofToDataPath(tokens[++index] + ".json"));
            
            if(file.exists()){
                
                nlohmann::json dict;
                std::ifstream i(ofToDataPath(file.path()));
                i >> dict;
                
                load(dict,width,height);
                
            }else{
                cout << "ofApp::processReaperMarker loadStateFromDisk WARNING: There is no file on disk at that path: " << file.path() << endl;
            }
        } else if(tokens[index] == "sp"){ // set parameter
            int moduleIndex = ofToInt(tokens[++index]);
            string label = tokens[++index];
            float val = ofToFloat(tokens[++index]);
            modules[moduleIndex]->receiveOSC(width,height,label,val);
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
        onsetOccurred(width,height,frame_num); // onsets
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

void ofApp::setActiveIndices(int* ai,int width, int height, unsigned long long frame_num){
    for(int i = 0; i < MAX_ACTIVE_MODULES; i++){
        active_module_indices[i] = ai[i];
        if(active_module_indices[i] >= 0 && modules[active_module_indices[i]]->newParamsProb > ofRandom(1.f)){
            modules[active_module_indices[i]]->newParams(width,height,vec_history, DESCRIPTORS_VECTOR_LENGTH, vec_history_length, vec_history_full,frame_num);
        }
    }
}

void ofApp::onsetOccurred(int width, int height,unsigned long long frame_num){

    // new active vc i
    
    vector<int> chosen_i;
    int ai[MAX_ACTIVE_MODULES];
    for(int i = 0; i < MAX_ACTIVE_MODULES; i++){ // go through the max number that we'll display
        if(moduleIndexUnlocked[i] and (ofRandom(1.f) < onsetSwitchProb)){
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
            ai[i] = active_module_indices[i];
            chosen_i.push_back(ai[i]);
        }
    }

    setActiveIndices(ai, main_fbo.getWidth(), main_fbo.getHeight(),frame_num);

    // blend mode
    if(ofRandom(1.f) < onsetSwitchProb) blendMode = blendModes[blendModePool[int(ofRandom(blendModePool.size()))]];
    
    feedback_amt = (ofRandom(1.f) < feedback_prob) * ofRandom(1, feedback_max);
    
    // ofx post glitch
    postGlitch.newParams();
    
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
            modules[vc_i]->newParamsProb = oscMsg.getArgAsFloat(1);
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
            modules[index]->receiveOSC(ofGetWidth(),ofGetHeight(),str, val);
        } else if (address == "/waveform") {
            int index = oscMsg.getArgAsInt(0);
//            cout << "received waveform: " << index << endl;
            for(int i = 0; i < WAVEFORM_LEN; i++){
                waveforms[index][i] = oscMsg.getArgAsFloat(i+1);
            }
        } else if (address == "/mags") {
            int index = oscMsg.getArgAsInt(0);
//            cout << "mag index: " << index << "\n";
            for(int i = 0; i < MAGNITUDES_LEN; i++){
                magnitudes[index][i] = oscMsg.getArgAsFloat(i+1);
//                cout << magnitudes[index][i] << " ";
            }
//            cout << "\n";
        } else if (address == "/vector") {
            for(int i = 0; i < DESCRIPTORS_VECTOR_LENGTH; i++){
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
            
            onset_occured = use_sc_onsets && (oscMsg.getArgAsFloat(DESCRIPTORS_VECTOR_LENGTH) > 0.5);
        }
    }
    
    if(onset_occured){
        onsetOccurred(main_fbo.getWidth(),main_fbo.getHeight(),ofGetFrameNum());
    }
    
    prUpdate(false);
}

void ofApp::prUpdate(bool isNRT){
        
    for(int i = 0; i < n_modules; i++){
        modules[i]->update(isNRT,&common_features,verbose);
    }
}

void ofApp::renderFrame(int width, int height, unsigned long long frameNum, bool isNRT){
    
    main_fbo.begin();
    
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    
    ofSetColor(0,255 - feedback_amt); // alpha of 255 = no feedback, alpha of 0 = full feedback
    ofDrawRectangle(0, 0, main_fbo.getWidth(), main_fbo.getHeight());
    
    // =============== visualModules ===================
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    
    if(!debug){
        
        if(verbose){
            cout << "Active Module Indices:";
            for(int i = 0; i < MAX_ACTIVE_MODULES; i++){
                int index = active_module_indices[i];
                if(index >= 0) cout << " " << modules[index]->getName();
            }
            cout << endl;
        }
        
        ff.update(frameNum, &common_features);
        
        for(int i = 0; i < MAX_ACTIVE_MODULES; i++){
            int index = active_module_indices[i];
            if(index >= 0){
                for(int j = 0; j < MAX_ACTIVE_MODULES; j++){
                    if((j != i) && (active_module_indices[j] >= 0)){
                        modules[index]->interact(modules[active_module_indices[j]]);
                    }
                }
                modules[index]->display(width,height,frameNum,&common_features, isNRT,verbose);
            }
        }
    } else {
        displayIncomingData(width,height);
    }
    
    main_fbo.end();
    postGlitch.generateFx(&common_features);
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    renderFrame(main_fbo.getWidth(),main_fbo.getHeight(),ofGetFrameNum(),false);
    main_fbo.draw(0,0,ofGetWidth(),ofGetHeight());
    
    if(config["draw-bounds"].get<bool>()){ drawBounds(); };
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
    for(int i = 0; i < N_MAGNITUDES; i++){
        ofSetColor(255,200 - (i * 100));
        for(int x = 0; x < MAGNITUDES_LEN; x++){
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
    for(int i = 0; i < DESCRIPTORS_VECTOR_LENGTH; i++){
        int bar_height = vector_data[i] * vec_height;
        // TODO: i think the colors are wrong
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
    for(int i = 0; i < N_WAVEFORMS; i++){
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
        for(int x = 0; x < WAVEFORM_LEN; x++){
            int xpt = ofMap(x,0,WAVEFORM_LEN-1,xoff,xend);
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
    
    if (key == 'c') loadConfigFile(CONFIG_PATH);
    
    if (key == 's') use_sc_onsets = !use_sc_onsets;
    
    if (key == 'o') onsetOccurred(ofGetWidth(),ofGetHeight(),ofGetFrameNum());
    if (key == 'p'){
        for(int i = 0; i < MAX_ACTIVE_MODULES; i++){
            if(active_module_indices[i] >= 0){
                modules[active_module_indices[i]]->newParams(ofGetWidth(),ofGetHeight(),vec_history, DESCRIPTORS_VECTOR_LENGTH, vec_history_length, vec_history_full,ofGetFrameNum());
            }
        }
    }
    
    for(int i = 0; i < 10; i++){
        if(key == saveKeys[i]){
            saves[i] = save();
            std::ofstream fout(ofToDataPath(ofGetTimestampString() + "_save-" + ofToString(i) + ".json"));
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
    for(int i = 0; i < n_modules; i++){
        modules[i]->screenResize(w, h);
    }
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
    
}
