#include "ofApp.h"
#include <algorithm>

//--------------------------------------------------------------
void ofApp::setup(){
    config.load("config.yaml");
    
    nrtRender = config["nrt-render"].as<bool>();
    
    ofSetFrameRate(config["target-framerate"].as<int>());
    
    postGlitchChangeProb = config["post-glitch"]["change-prob"].as<float>();
    
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
        
    int width = 0;
    int height = 0;
    
    float mesh_line_width = config["modules"]["mesh"]["line-width"].as<float>();
    float mesh_point_size = config["modules"]["mesh"]["point-size"].as<float>();
    float lissajous_line_width = config["modules"]["waveform"]["lissajous-line-width"].as<float>();
    
    if(nrtRender){
        width = 3840; // 4k
        height = 2160;// 4k
        
        mesh_line_width *= 2;
        mesh_point_size *= 2;
        lissajous_line_width *= 2;
    } else {
        width = ofGetWidth();
        height = ofGetHeight();
    }

    main_fbo.allocate(width, height);
    postGlitch.setup(&main_fbo);
    
    waveform_len = width;
    
    int max_active_vc = 2;
    active_vc_i = new int[max_active_vc];
    
    ofBackground(0);
    ofEnableAntiAliasing();
    //ofEnableDepthTest();
    //ofEnableAlphaBlending();

    // ================ DATA STRUCTURES ========================

    common_features["amplitude"] = 0;
    common_features["fftCrest"] = 0;
    common_features["fftSlope"] = 0;
    common_features["fftSpread"] = 0;
    common_features["loudness"] = 0;
    common_features["sensoryDissonance"] = 0;
    common_features["specCentroid"] = 0;
    common_features["specFlatness"] = 0;
    common_features["specPcile"] = 0;
    common_features["zeroCrossing"] = 0;
    
    // wavforms
    waveforms = (float**) malloc(sizeof(float*) * n_waveforms);
    for(int i = 0; i < n_waveforms; i++){
        waveforms[i] = (float*) malloc(sizeof(float) * waveform_len);
    }
    // mags
    magnitudes = new float*[n_magnitudes];
//    magnitudes = (float**) malloc(sizeof(float*) * n_magnitudes);
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
    ff.setup(20, xmin, xmax, ymin, ymax, zmin, zmax);
    
    // ============ setup modules ===============
    
    // 0: waveform
    Waveform* wf = new Waveform;
    wf->setup(width,height,waveforms,n_waveforms,waveform_len, vec_history, vector_len, vec_history_length, vec_history_full,lissajous_line_width);
    visual_contents[vc_counter] = wf;
    vc_counter = addVCOptions(vc_counter,config["modules"]["waveform"]["prob"].as<int>());

    // 1: mesh
    Mesh* mesh = new Mesh;
    mesh->setup(config["modules"]["mesh"]["n-points"].as<int>(), &ff, xmin, xmax, ymin, ymax, zmin, zmax, xsize, ysize,mesh_line_width,mesh_point_size);
    visual_contents[vc_counter] = mesh;
    vc_counter = addVCOptions(vc_counter,config["modules"]["mesh"]["prob"].as<int>());
    
    // 2: mag lines
    Lines* lines0 = new Lines;
    lines0->setup(magnitudes[0],0,magnitude_len,false,width,height,vec_history, vector_len, vec_history_length, vec_history_full);
    visual_contents[vc_counter] = lines0;
    vc_counter = addVCOptions(vc_counter,config["modules"]["mag-lines"]["prob"].as<int>());
    
    // 3: turtle
    Turtle* turtle0 = new Turtle;
    turtle0->setup(width,height,vec_history,vector_len,vec_history_length,vec_history_full,config);
    visual_contents[vc_counter] = turtle0;
    vc_counter = addVCOptions(vc_counter,config["modules"]["turtle"]["prob"].as<int>());
    
    // load videos
    for(int i = 0; i < config["videos"].size(); i++){
        
        string name = config["videos"][i]["name"].as<string>();
        int prob = config["videos"][i]["prob"].as<int>();
        
        newHapMovie(name,vc_counter,initialPoints,width,height);
        vc_counter = addVCOptions(vc_counter,prob);
    }
    
    // minus one because we just added one in the last addVCOptions call
    nVisualContents = vc_counter - 1;
    
    // add null options to vc options
    for(int i = 0; i < 1; i++){
        vc_i_options.push_back(-1);
    }
    
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
    
    // =========================== INITIALIZATION =====================
    // initialize to none active
    active_vc_i[0] = -1;
    active_vc_i[1] = -1;
    active_vc_i[2] = -1;
    active_vc_i[3] = -1;
    
    // =========== NRT RENDERING
    if(nrtRender){
        
        
        //*******************************************************************************************************************
        //*******************************************************************************************************************
        //****************************** some optional and useful presets for different renders ***************************
        //*******************************************************************************************************************
        //*******************************************************************************************************************
        
        // this is just for rendering chebyshev
//        onsetSwitchProb = 0;
//        visual_contents[0]->receiveOSC(width,height,"setWaveformType", 1.0); // set to lissajous
//        visual_contents[0]->receiveOSC(width,height,"resetLissajousXY", 1.0); // make sure it's in the middle
        
        
        //*******************************************************************************************************************
        //*******************************************************************************************************************
        //*******************************************************************************************************************
        //*******************************************************************************************************************
        //*******************************************************************************************************************
        
        
        
        //int max_frames = 600; // 600 frames = 20 seconds
        int max_frames = INT_MAX;
        
        string line;
        ifstream data;
        data.open(ofToDataPath(csv_path));
        int line_length = 8303;// 8308
        float* csv_line_fl = new float[line_length];
        
        // stuff for rendering
        
        string new_dir_path;
        ofFileDialogResult result = ofSystemSaveDialog("", "Choose location to save frames");
        if(result.bSuccess) {
          new_dir_path = result.getPath();
        }
        
        ofDirectory new_dir(new_dir_path);
        new_dir.create();
        
        ofPixels pix;
        
        int frame_num = 0;
        
        while(!data.eof() && frame_num < max_frames){
            getline(data,line);
            
            vector<string> csv_line = ofSplitString(line,",");
            
            if(csv_line.size() != line_length){
                cout << "csv line is not expected length. expected: " << line_length << ", found: " << csv_line.size() << endl;
                break;
            }
            
            for(int i = 0; i < line_length; i++){
                csv_line_fl[i] = ofToFloat(csv_line[i]);
            }
            
            int global_frame_num = (int)csv_line_fl[0];
            
            setValsFromCSV(width,height,csv_line_fl);
            
            cout << "frame num: " << frame_num;
            cout << endl;
            
            for(int i = 0; i < nVisualContents; i++){
                visual_contents[i]->update(true);
            }
            
            drawScreen(main_fbo.getWidth(), main_fbo.getHeight(), frame_num, true);

            main_fbo.readToPixels(pix);
            ofSaveImage(pix, new_dir_path+"/"+ofToString(global_frame_num,6,'0')+".tiff",OF_IMAGE_QUALITY_BEST);
            
            frame_num++;
        }
        
        data.close();
        
        ofExit();
    }
}

void ofApp::setValsFromCSV(int width, int height, float* csv_data){
    onset_occured = false;
    
    if(csv_data[3] > 0){
        onset_occured = true;
        onsetOccured(width,height); // onsets
    }
    
    for (int i = 0; i < vector_len; i++){
        float val = csv_data[i + 4];
        vector_data[i] = val;
        vec_history[vec_history_counter][i] = val;
    }
    
    incrementVecHistoryCounter();
    
    common_features["amplitude"] = vector_data[11];
    common_features["fftCrest"] = vector_data[6];
    common_features["fftSlope"] = vector_data[4];
    common_features["fftSpread"] = vector_data[1];
    common_features["loudness"] = vector_data[9];
    common_features["sensoryDissonance"] = vector_data[12];
    common_features["specCentroid"] = vector_data[0];
    common_features["specFlatness"] = vector_data[5];
    common_features["specPcile"] = vector_data[6];
    common_features["zeroCrossing"] = vector_data[13];
    
    for(int i = 0; i < magnitude_len; i++){
        magnitudes[0][i] = csv_data[110 + i];
    }
    
    for(int i = 0; i < waveform_len; i++){
        waveforms[0][i] = csv_data[623 + i];
        waveforms[1][i] = csv_data[4463 + i];
    }
    
    for(int i = 0; i < nPCAs; i++){
        pcas[i] = ofLerp(pcas[i],csv_data[8303 + i],0.14);
    }
    
    setKmeansVec(csv_data[8307],true);
}

void ofApp::setKmeansVec(int cluster,bool check_confidence){
    curr_cluster = cluster;
    
    if(previous_cluster == curr_cluster){
        kmeans_confidence++;
    } else {
        kmeans_confidence = 0;
    }
    
    if(check_confidence){ // increase the 0 to require a higher confidence;
        if(kmeans_confidence > 0){
            for(int i = 0; i < kClusters; i++){
                if(i == curr_cluster){
                    kmeans[i] = 1.f;
                } else {
                    kmeans[i] = 0.f;
                }
            }
        }
    } else {
        for(int i = 0; i < kClusters; i++){
            if(i == curr_cluster){
                kmeans[i] = 1.f;
            } else {
                kmeans[i] = 0.f;
            }
        }
    }
    
    previous_cluster = curr_cluster;
}

void ofApp::incrementVecHistoryCounter(){
    // check if we just added the last index to the history and if so set true
    if(vec_history_counter == vec_history_length - 1) vec_history_full = true;
    
    // increment and modulous
    vec_history_counter = (vec_history_counter + 1) % vec_history_length;
}

void ofApp::newHapMovie(std::string path, int index, ofVec3f* initPts, int width, int height){
    HapMovie* vc = new HapMovie;
    vc->setup(path,initPts[0],initPts[1],initPts[2],initPts[3],magnitudes,n_magnitudes,magnitude_len,nrtRender,&ff);
    vc->newParams(width, height, vec_history, vector_len, vec_history_length, vec_history_full);
    visual_contents[index] = vc;
}

//--------------------------------------------------------------
void ofApp::update(){
    onset_occured = false;
    
    // ==================== OSC ================================
    while(osc_receiver.hasWaitingMessages()){
        ofxOscMessage oscMsg;
        osc_receiver.getNextMessage(oscMsg);

//        cout << oscMsg << "\n";

        string address = oscMsg.getAddress();
        
        if(address == "/setActiveIndices"){
            for(int i = 0; i < max_active_vc; i++){
                active_vc_i[i] = oscMsg.getArgAsInt(i);
            }
        } else if (address == "/setOnsetSwitchProb"){
            onsetSwitchProb = oscMsg.getArgAsFloat(0);
        } else if (address == "/setNewParamsProb"){
            int vc_i = oscMsg.getArgAsInt(0);
            visual_contents[vc_i]->newParamsProb = oscMsg.getArgAsFloat(1);
        } else if (address == "/setVCOptions"){
            int n_options = oscMsg.getArgAsInt(0);
            vc_i_options.resize(n_options);
            for(int i = 0; i < n_options; i++){
                vc_i_options[i] = oscMsg.getArgAsInt(i+1);
            }
            //onsetSwitchProb = oscMsg.getArgAsFloat(0);
        } else if (address == "/cmd"){
            int index = oscMsg.getArgAsInt(0);
            std::string str = oscMsg.getArgAsString(1);
            float val = oscMsg.getArgAsFloat(2);
            visual_contents[index]->receiveOSC(ofGetWidth(),ofGetHeight(),str, val);
        } else if (address == "/kmeans"){
            setKmeansVec(oscMsg.getArgAsInt(0),false);
        } else if (address == "/pca"){
            for(int i = 0; i < nPCAs; i++){
                pcas[i] = oscMsg.getArgAsFloat(i);
            }
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
            
            common_features["amplitude"] = vector_data[11];
            common_features["fftCrest"] = vector_data[6];
            common_features["fftSlope"] = vector_data[4];
            common_features["fftSpread"] = vector_data[1];
            common_features["loudness"] = vector_data[9];
            common_features["sensoryDissonance"] = vector_data[12];
            common_features["specCentroid"] = vector_data[0];
            common_features["specFlatness"] = vector_data[5];
            common_features["specPcile"] = vector_data[6];
            common_features["zeroCrossing"] = vector_data[13];
            
        } else if (address == "/onset"){
            onset_occured = true;
            onsetOccured(main_fbo.getWidth(),main_fbo.getHeight());
        }
    }
    
    for(int i = 0; i < nVisualContents; i++){
        visual_contents[i]->update(false);
    }
}

void ofApp::onsetOccured(int width, int height){

    // new active vc i
    if(onsetSwitchProb > ofRandom(1.f)){
        vector<int> chosen_i;
        for(int i = 0; i < max_active_vc; i++){ // go through the max number that we'll display
            bool found = false;
            while(!found){
                // options array is the big pool of options (has duplicates based on probs)
                int rand_int = ofRandom(1.f) * vc_i_options.size(); // random int the size of the options array
                int result = vc_i_options[rand_int]; // the int in the from the options array (which is the index for the modules array)
                if(!std::count(chosen_i.begin(), chosen_i.end(), result)){
                    found = true;
                    chosen_i.push_back(result);
                    active_vc_i[i] = result; // the module's index
                    
                    if(result >= 0 && visual_contents[result]->newParamsProb > ofRandom(1.f)){
                        visual_contents[result]->newParams(width,height,vec_history, vector_len, vec_history_length, vec_history_full);
                    }
                }
            }
        }
    }
    
    // ofx post glitch
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

void ofApp::drawScreen(int width, int height, int frameNum, bool isNRT){
    
    main_fbo.begin();
    
    ofClear(0,0,0,255);
    
    if(!debug){
        
        ff.update(frameNum, &common_features);
        
        for(int i = 0; i < max_active_vc; i++){
            int index = active_vc_i[i];
            if(index >= 0){
                for(int j = 0; j < max_active_vc; j++){
                    if(j != i && active_vc_i[j] >= 0){
                        visual_contents[index]->interact(visual_contents[active_vc_i[j]]);
                    }
                }
                visual_contents[index]->display(width,height,frameNum,&common_features, isNRT);
            }
        }
    } else {
        displayIncomingData(width,height);
    }
    
    main_fbo.end();
    
    postGlitch.generateFx();
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    drawScreen(main_fbo.getWidth(),main_fbo.getHeight(),ofGetFrameNum(),false);
    main_fbo.draw(0,0);
    
    //ofSetColor(255,0,0);
    //ofDrawBitmapString(ofToString(ofGetFrameRate()), 10, 10);

//    ofSetColor(0, 255, 0);
//    ofDrawSphere(0, 0, 0, 10);
//    ofDrawSphere(xsize, 0, 0, 10);
//    ofDrawSphere(xsize, ysize, 0, 10);
//    ofDrawSphere(0, ysize, 0, 10);
//
//    ofDrawSphere(0, 0, -zmax, 10);
//    ofDrawSphere(xsize, 0, -zmax, 10);
//    ofDrawSphere(xsize, ysize, -zmax, 10);
//    ofDrawSphere(0, ysize, -zmax, 10);
//
//    ofSetColor(255, 0, 0);
//    ofDrawSphere(xmin, ymin, 0 - 10, 10);
//    ofDrawSphere(xmax, ymin, 0 - 10, 10);
//    ofDrawSphere(xmax, ymax, 0 - 10, 10);
//    ofDrawSphere(xmin, ymax, 0, 10);
//
//    ofDrawSphere(xmin, ymin, -zmax + 10, 10);
//    ofDrawSphere(xmax, ymin, -zmax + 10, 10);
//    ofDrawSphere(xmax, ymax, -zmax + 10, 10);
//    ofDrawSphere(xmin, ymax, -zmax, 10);
//
//    ofFill();
//    ofSetColor(255,255,0,100);
//    ofBeginShape();
//    ofVertex(xmin, ymin, zmin);
//    ofVertex(xmin, ymin, -zmax);
//    ofVertex(xmin, ymax, zmax);
//    ofVertex(xmin, ymax, zmin);
//    ofEndShape();

//    ofSetColor(0, 255, 0);
//    ofDrawSphere(0, 0, 0, 10);
//    ofDrawSphere(xsize, 0, 0, 10);
//    ofDrawSphere(xsize, ysize, 0, 10);
//    ofDrawSphere(0, ysize, 0, 10);
//
//    ofDrawSphere(0, 0, zmax, 10);
//    ofDrawSphere(xsize, 0, zmax, 10);
//    ofDrawSphere(xsize, ysize, zmax, 10);
//    ofDrawSphere(0, ysize, zmax, 10);
    
//    cout << "active ints: " << active_vc_i[0] << active_vc_i[1] << active_vc_i[2] << "\n";
//    cout << "frame rate:  " << ofGetFrameRate() << "\n\n";
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
    
    // pca
    xoff = width * 0.35;
    int edgeLen = 70;
    yoff = height / 2;
    for(int i = 0; i < nPCAs; i++){
        ofSetLineWidth(0);
        ofSetColor(0,ofMap(i,0,nPCAs-1,0,255),ofMap(i,0,nPCAs-1,255,0),pow(pcas[i],0.5) * 255);
        ofFill();
        ofDrawRectangle(xoff, yoff, edgeLen, edgeLen);
        
        ofNoFill();
        ofSetLineWidth(2);
        ofSetColor(255);
        ofDrawRectangle(xoff, yoff, edgeLen, edgeLen);
        
        xoff += edgeLen * 1.1;
    }
    
    // kmeans
    xoff = width * 0.35;
    yoff += edgeLen * 1.2;
    for(int i = 0; i < kClusters; i++){
        if(curr_cluster == i){
            ofSetLineWidth(0);
            ofSetColor(ofMap(i,0,nPCAs-1,0,255),ofMap(i,0,nPCAs-1,255,0),0);
            ofFill();
            ofDrawRectangle(xoff, yoff, edgeLen, edgeLen);
        }
        ofNoFill();
        ofSetLineWidth(2);
        ofSetColor(255);
        ofDrawRectangle(xoff, yoff, edgeLen, edgeLen);
        
        xoff += edgeLen * 1.1;
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if(key == 'd'){
        debug = !debug;
    }
    
    // post glitch manual controls
    if (key == '1') postGlitch.setFx(OFXPOSTGLITCH_CONVERGENCE    , true);
    if (key == '2') postGlitch.setFx(OFXPOSTGLITCH_GLOW            , true);
    if (key == '3') postGlitch.setFx(OFXPOSTGLITCH_SHAKER            , true);
    if (key == '4') postGlitch.setFx(OFXPOSTGLITCH_CUTSLIDER        , true);
    if (key == '5') postGlitch.setFx(OFXPOSTGLITCH_TWIST            , true);
    if (key == '6') postGlitch.setFx(OFXPOSTGLITCH_OUTLINE        , true);
    if (key == '7') postGlitch.setFx(OFXPOSTGLITCH_NOISE            , true);
    if (key == '8') postGlitch.setFx(OFXPOSTGLITCH_SLITSCAN        , true);
    if (key == '9') postGlitch.setFx(OFXPOSTGLITCH_SWELL            , true);
    if (key == '0') postGlitch.setFx(OFXPOSTGLITCH_INVERT            , true);

    if (key == 'q') postGlitch.setFx(OFXPOSTGLITCH_CR_HIGHCONTRAST, true);
    if (key == 'w') postGlitch.setFx(OFXPOSTGLITCH_CR_BLUERAISE    , true);
    if (key == 'e') postGlitch.setFx(OFXPOSTGLITCH_CR_REDRAISE    , true);
    if (key == 'r') postGlitch.setFx(OFXPOSTGLITCH_CR_GREENRAISE    , true);
    if (key == 't') postGlitch.setFx(OFXPOSTGLITCH_CR_BLUEINVERT    , true);
    if (key == 'y') postGlitch.setFx(OFXPOSTGLITCH_CR_REDINVERT    , true);
    if (key == 'u') postGlitch.setFx(OFXPOSTGLITCH_CR_GREENINVERT    , true);
    
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
    // post glitch manual controls
    if (key == '1') postGlitch.setFx(OFXPOSTGLITCH_CONVERGENCE    , false);
    if (key == '2') postGlitch.setFx(OFXPOSTGLITCH_GLOW            , false);
    if (key == '3') postGlitch.setFx(OFXPOSTGLITCH_SHAKER            , false);
    if (key == '4') postGlitch.setFx(OFXPOSTGLITCH_CUTSLIDER        , false);
    if (key == '5') postGlitch.setFx(OFXPOSTGLITCH_TWIST            , false);
    if (key == '6') postGlitch.setFx(OFXPOSTGLITCH_OUTLINE        , false);
    if (key == '7') postGlitch.setFx(OFXPOSTGLITCH_NOISE            , false);
    if (key == '8') postGlitch.setFx(OFXPOSTGLITCH_SLITSCAN        , false);
    if (key == '9') postGlitch.setFx(OFXPOSTGLITCH_SWELL            , false);
    if (key == '0') postGlitch.setFx(OFXPOSTGLITCH_INVERT            , false);

    if (key == 'q') postGlitch.setFx(OFXPOSTGLITCH_CR_HIGHCONTRAST, false);
    if (key == 'w') postGlitch.setFx(OFXPOSTGLITCH_CR_BLUERAISE    , false);
    if (key == 'e') postGlitch.setFx(OFXPOSTGLITCH_CR_REDRAISE    , false);
    if (key == 'r') postGlitch.setFx(OFXPOSTGLITCH_CR_GREENRAISE    , false);
    if (key == 't') postGlitch.setFx(OFXPOSTGLITCH_CR_BLUEINVERT    , false);
    if (key == 'y') postGlitch.setFx(OFXPOSTGLITCH_CR_REDINVERT    , false);
    if (key == 'u') postGlitch.setFx(OFXPOSTGLITCH_CR_GREENINVERT    , false);
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
    for(int i = 0; i < nVisualContents; i++){
        visual_contents[i]->screenResize(w, h);
    }
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
    
}
