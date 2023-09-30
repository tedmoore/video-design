//
//  HapMovie.hpp
//  fonema video
//
//  Created by Ted Moore on 12/26/20.
//

#ifndef HapMovie_hpp
#define HapMovie_hpp

#include <stdio.h>
#include "ofMain.h"
#include <VisualModule.hpp>
#include "ofxHapPlayer.h"
#include "MoviePoint.hpp"
#include "FlowField.hpp"

#include "defines.h"
#include "Param.hpp"

#define N_CLUSTERS 4
#define VIDEO_MINI_WIDTH 32
#define VIDEO_MINI_HEIGHT 32
#define N_MOVIEPOINTS 1024 // **MUST BE VIDEO_MINI_WIDTH * VIDEO_MINI_HEIGHT

enum UnfoldTilesOrder { LRTB = 0 , LRBT, RLTB , RLBT , TBLR , TBRL , BTLR , BTRL };
enum RectTypes { RECT = 0 , BOX , SPHERE };

class Video{
public:
    ofxHapPlayer hap;
    ofVideoPlayer mini_vid;
    ofColor center_colors[N_CLUSTERS];
    int center_color_indices[N_CLUSTERS];
    
    bool clustered = true;
    vector<ofFile> pngs;
    vector<ofFile> bitexact_pngs;
    string src_path;
    
    int getTotalFrames(){
        return pngs.size();
    }
    
    void setup(string dir, bool isNRT){
        
        src_path = dir;
        
        if(isNRT){ // is non-real-time
            ofDirectory pngs_dir(dir + "/frames");
            cout << "\t\t" << pngs_dir.getAbsolutePath() << "\n";
            pngs_dir.listDir();
            pngs_dir.sort();
            pngs = pngs_dir.getFiles();
        
            ofDirectory bitexact_pngs_dir(dir + "/mini-frames");
            cout << "\t\t" << bitexact_pngs_dir.getAbsolutePath() << "\n";
            bitexact_pngs_dir.listDir();
            bitexact_pngs_dir.sort();
            bitexact_pngs = bitexact_pngs_dir.getFiles();
            
            assert(bitexact_pngs.size() == pngs.size());
            
        } else { // is real-time
            hap.load(dir + "/hap.mov");
            hap.setLoopState(OF_LOOP_NORMAL);
            hap.play();
            hap.setVolume(0);
            
            cout << dir + "/mini.mp4" << endl;

            mini_vid.load(dir + "/mini.mp4");
            cout << "done loading" << endl;
            mini_vid.setVolume(0);
            mini_vid.setLoopState(OF_LOOP_NORMAL);
            mini_vid.play();
        }
        
        for(int i = 0; i < N_CLUSTERS; i++){
            center_color_indices[i] = ofRandom(MAGNITUDES_LEN);
        }
    }
};

class VideoModule: public VisualModule {
public:
    
    vector<Video*> videos;
    
    ofTexture texture;
    ofVec3f points[4];
    ofPixels mini_pix;
    
    float** mags;
    int n_mag;
    int mag_len;
    
    int alpha = 255;
        
    float avg_mag = 0.5;
            
    ofImage img;
    ofImage mini_img;
    
    vector<MoviePoint> moviePoints;
    FlowField *ff;
    
    bool bUseFFMaster = true;
    
    int zDir = -1;
    
    vector<Param*> params;
    
    ParamFloat speed;
    ParamIntList speedDir;
    ParamBool bShowHap;
    ParamBool bUseFF;
    ParamBool bReactiveSpeed;
    ParamBool bTile;
    ParamBool bDontUnfoldTiles;
    ParamFloat rect_w_mul;
    ParamFloat rect_h_mul;
    ParamFloat tile_scale;
    ParamFloat tile_offset_scale;
    ParamFloat i_x;
    ParamFloat i_y;
    ParamInt tiles_alpha;
    ParamBool bZShiftBoxes;
    ParamFloat nrtPlayHead;
    ParamInt currentSubVideoIndex;
    
    unsigned long long n_new_tiles_per_frame = 1;
    unsigned long long counting_tiles_start_frame = 0;
    UnfoldTilesOrder unfold_tiles_order = LRBT;
    
    ParamEnumWeighted rectType;
    
    ofLight light;
    ofVec3f lightPosition = {0,0,0};
    
    string getName(){
        return "HapMovie";
    }
    
    Video* getCurrentVideo(){
        return videos[currentSubVideoIndex.value];
    }
    
    void setup(std::string& name, ofVec3f pt0, ofVec3f pt1, ofVec3f pt2, ofVec3f pt3, float** mags_, int n_mag_, int mag_len_, bool isNRT, FlowField* ff_, nlohmann::json& config){
        
        ofDirectory topDir("videos/" + name);
        vector<ofFile> subDirs = topDir.getFiles();
        videos.resize(subDirs.size());
        
        for(int i = 0; i < subDirs.size(); i++){
            Video* v = new Video();
            videos[i] = v;
            videos[i]->setup(subDirs[i].getAbsolutePath(),isNRT);
        }
        
        if(isNRT){
            
            for (int i = 1; i < videos.size(); i++){
                assert(videos[0]->getTotalFrames() == videos[i]->getTotalFrames());
            }
            
            mini_img.allocate(VIDEO_MINI_WIDTH, VIDEO_MINI_HEIGHT, OF_IMAGE_COLOR);
            mini_pix.allocate(VIDEO_MINI_WIDTH, VIDEO_MINI_HEIGHT, OF_PIXELS_RGBA);
            
        } else {
            // is real-time
            mini_pix.allocate(VIDEO_MINI_WIDTH, VIDEO_MINI_HEIGHT, videos[0]->mini_vid.getPixelFormat());
        }
        
        // speed
        speed.name = "speed";
        speed.min = config["speed-min"].get<float>();
        speed.max = config["speed-max"].get<float>();
        speed.power = config["speed-pow"].get<float>();
        speed.newRandom();
        params.push_back(&speed);
        
        // speed_dir
        speedDir.name = "speedDir";
        speedDir.setup({-1,1},1);
        params.push_back(&speedDir);
        
        // showHap
        bShowHap.name = "bShowHap";
        bShowHap.trueProb = config["bShowHap-prob"].get<float>(); // 0.2
        params.push_back(&bShowHap);
        
        // use_ff
        bUseFF.name = "bUseFF";
        bUseFF.trueProb = config["use-ff-prob"].get<float>(); // 0.28
        params.push_back(&bUseFF);
        
        // reactive_speed;
        bReactiveSpeed.name = "reactiveSpeed";
        params.push_back(&bReactiveSpeed);
        
        // bTile;
        bTile.name = "bTile";
        bTile.trueProb = config["bTile-prob"].get<float>();
        params.push_back(&bTile);
        
        // bDontUnfoldTiles
        bDontUnfoldTiles.name = "bDontUnfoldTiles";
        params.push_back(&bDontUnfoldTiles);
        
        //rect_w_mul
        rect_w_mul.name = "rect_w_mul";
        rect_w_mul.setup(1.f,3.f,1.f,1.f);
        params.push_back(&rect_w_mul);
        
        // rect_h_mul
        rect_h_mul.name = "rect_h_mul";
        rect_h_mul.setup(1.f,3.f,1.f,1.f);
        params.push_back(&rect_h_mul);
        
        // tile_scale
        tile_scale.name = "tile_scale";
        tile_scale.setup(0.03,0.5,1.f,0.5);
        params.push_back(&tile_scale);
        
        // tile_offset_scale
        tile_offset_scale.name = "tile_offset_scale";
        tile_offset_scale.setup(0.0,0.4,1.f,0.0);
        params.push_back(&tile_offset_scale);
        
        // i_x
        i_x.name = "i_x";
        i_x.setup(0.f,1.f,1.f,0.f);
        params.push_back(&i_x);
        
        // i_y
        i_y.name = "i_y";
        i_y.setup(0.f,1.f,1.f,0.f);
        params.push_back(&i_y);
        
        // tiles_alpha
        tiles_alpha.name = "tiles_alpha";
        tiles_alpha.setup(1,255,1);
        params.push_back(&tiles_alpha);
        
        // bZShiftBoxes
        bZShiftBoxes.name = "bZShiftBoxes";
        bZShiftBoxes.trueProb = config["bZShiftBoxes-prob"].get<float>();
        params.push_back(&bZShiftBoxes);
        
        // rectType
        rectType.name = "rectType";
        rectType.setup(config["rectType-weights"].get<vector<int>>(),0);
        params.push_back(&rectType);
        
        // nrtPlayHead
        nrtPlayHead.name = "nrtPlayHead";
        nrtPlayHead.setup(0.f,videos[0]->getTotalFrames(),1.f,0.f);
        params.push_back(&nrtPlayHead);
        
        // currentIndex
        currentSubVideoIndex.name = "currentSubVideoIndex";
        currentSubVideoIndex.setup(0,videos.size(),0); // max argument is just used for randomness, it is [min,max)
        params.push_back(&currentSubVideoIndex);
        
        mags = mags_;
        points[0] = pt0;
        points[1] = pt1;
        points[2] = pt2;
        points[3] = pt3;
        
        ff = ff_;
        
        type = HAP;
        
        moviePoints.resize(VIDEO_MINI_WIDTH * VIDEO_MINI_HEIGHT);
        for(int i = 0; i < VIDEO_MINI_WIDTH; i++){
            for(int j = 0; j < VIDEO_MINI_HEIGHT; j++){
                moviePoints[(j * VIDEO_MINI_WIDTH) + i].setup(i / float(VIDEO_MINI_WIDTH),j / float(VIDEO_MINI_HEIGHT));
            }
        }
        
        light.setPointLight();
        light.setAmbientColor(0);
        
        processConfigFile(config);
    }
    
    void processConfigFile(nlohmann::json& dict){
        for(Param* p : params){
            if(dict.contains(p->name)){
                p->setValue(dict[p->name].get<float>());
            }
        }
    }

    void update(bool isNRT, std::unordered_map<std::string, float>* common_features, bool verbose){
        
        if(bReactiveSpeed.value){
            speed.value = ofMap(pow(common_features->at("specFlatness"),3.f),0.f,1.f,0.8,10);
        }
        
        if(isNRT){
            nrtPlayHead.value += getSpeed();
            int total_frames = videos[0]->getTotalFrames();
            while(nrtPlayHead.value < 0) nrtPlayHead.value += total_frames;
            while(nrtPlayHead.value >= total_frames) nrtPlayHead.value -= total_frames;
        } else {
            for(int i = 0; i < videos.size(); i++){
                videos[i]->mini_vid.update();
                videos[i]->hap.setSpeed(getSpeed());
            }
        }
        
    }
    
    void displayHap(int width, int height, unsigned long long frame_num, std::unordered_map<std::string, float>* common_features, bool isNRT){
                        
        if(isNRT){
            img.load(videos[currentSubVideoIndex.value]->pngs[int(nrtPlayHead.value)].getAbsolutePath());
            texture = img.getTexture();
        } else {
            texture = *videos[currentSubVideoIndex.value]->hap.getTexture();
        }
        
        if(bTile.value){
            ofSetColor(255, tiles_alpha.value);
            int w = width * tile_scale.value;
            int h = height * tile_scale.value;
            int x_off = width * tile_offset_scale.value;
            int y_off = height * tile_offset_scale.value;
            int x_hop = w + x_off;
            int y_hop = h + y_off;
            int tileCounter = 0;
            
            unsigned long long make_n_tiles = (frame_num - counting_tiles_start_frame) * n_new_tiles_per_frame;
            
            switch(unfold_tiles_order){
                case LRTB: // 0
                    for(int y = ofMap(i_y.value,0.f,1.f,-h,y_off); y < height; y += y_hop){
                        for(int x = ofMap(i_x.value,0.f,1.f,-w,x_off); x < width; x += x_hop){
                            if(tileCounter++ < make_n_tiles) texture.draw(x,y,w,h);
                        }
                    }
                    break;
                case LRBT: // 1
                    for(int y = ofMap(i_y.value,0.f,1.f,height,height - y_hop); y > -h; y -= y_hop){
                        for(int x = ofMap(i_x.value,0.f,1.f,-w,x_off); x < width; x += x_hop){
                            if(tileCounter++ < make_n_tiles) texture.draw(x,y,w,h);
                        }
                    }
                    break;
                case RLTB: // 2
                    for(int y = ofMap(i_y.value,0.f,1.f,-h,y_off); y < height; y += y_hop){
                        for(int x = ofMap(i_x.value,0.f,1.f,width,width - x_hop); x > -w; x -= x_hop){
                            if(tileCounter++ < make_n_tiles) texture.draw(x,y,w,h);
                        }
                    }
                    break;
                case RLBT: // 3
                    for(int y = ofMap(i_y.value,0.f,1.f,height,height - y_hop); y > -h; y -= y_hop){
                        for(int x = ofMap(i_x.value,0.f,1.f,width,width - x_hop); x > -w; x -= x_hop){
                            if(tileCounter++ < make_n_tiles) texture.draw(x,y,w,h);
                        }
                    }
                    break;
                case TBLR: // 4
                    for(int x = ofMap(i_x.value,0.f,1.f,-w,x_off); x < width; x += x_hop){
                        for(int y = ofMap(i_y.value,0.f,1.f,-h,y_off); y < height; y += y_hop){
                            if(tileCounter++ < make_n_tiles) texture.draw(x,y,w,h);
                        }
                    }
                    break;
                case BTLR:
                    for(int x = ofMap(i_x.value,0.f,1.f,-w,x_off); x < width; x += x_hop){
                        for(int y = ofMap(i_y.value,0.f,1.f,height,height - y_hop); y > -h; y -= y_hop){
                            if(tileCounter++ < make_n_tiles) texture.draw(x,y,w,h);
                        }
                    }
                    break;
                case TBRL:
                    for(int x = ofMap(i_x.value,0.f,1.f,width,width - x_hop); x > -w; x -= x_hop){
                        for(int y = ofMap(i_y.value,0.f,1.f,-h,y_off); y < height; y += y_hop){
                            if(tileCounter++ < make_n_tiles) texture.draw(x,y,w,h);
                        }
                    }
                    break;
                case BTRL:
                    for(int x = ofMap(i_x.value,0.f,1.f,width,width - x_hop); x > -w; x -= x_hop){
                        for(int y = ofMap(i_y.value,0.f,1.f,height,height - y_hop); y > -h; y -= y_hop){
                            if(tileCounter++ < make_n_tiles) texture.draw(x,y,w,h);
                        }
                    }
                    break;
            }
        } else {
            ofSetColor(255, alpha);
            texture.draw(points[0],points[1],points[2],points[3]);
        }
    }
    
    void displayRects(int width, int height, unsigned long long frame_num, std::unordered_map<std::string, float>* common_features, bool isNRT){
        // just getting the pixels
        if(isNRT){
            string path = videos[currentSubVideoIndex.value]->bitexact_pngs[int(nrtPlayHead.value)].getAbsolutePath();
            mini_img.load(path);
            mini_pix = mini_img.getPixels();
        } else {
            if(videos[currentSubVideoIndex.value]->mini_vid.isFrameNew()) mini_pix = videos[currentSubVideoIndex.value]->mini_vid.getPixels();
        }

        ofSetLineWidth(1);
        int i_counter = 0;
        float summingmag = 0;
        float rec_w = (width / VIDEO_MINI_WIDTH) * rect_w_mul.value;
        float rec_h = (height / VIDEO_MINI_HEIGHT) * rect_h_mul.value;
        int i = 0;
        int x_pos_scaled = 0;
        
        ofEnableDepthTest();
        ofEnableLighting();
        light.enable();
        
        while(i < VIDEO_MINI_WIDTH && x_pos_scaled < width){
            int j = 0;
            int y_pos_scaled = 0;
            while(j < VIDEO_MINI_HEIGHT && y_pos_scaled < height){
                ofColor col = mini_pix.getColor(i,j);
                
                for(int i = 0; i < N_CLUSTERS; i++){
                    if(videos[currentSubVideoIndex.value]->center_color_indices[i] == i_counter){
                        videos[currentSubVideoIndex.value]->center_colors[i] = col;
                        break;
                    }
                }
                
                // figure out the alpha
                float local_mag = mags[0][i_counter];
                summingmag += local_mag;
                float local_alpha = ofMap(pow(local_mag,0.5),0.f,1.f,-10.f,255.f);
                
                int x = x_pos_scaled;
                int y = y_pos_scaled;
                int z = ofMap(local_mag,0.f,1.f,height * 0.5,0) * bZShiftBoxes.value * ((RectTypes)rectType.value != SPHERE);
                
                // get the point at this i, j and apply the force from the ff
                MoviePoint &mp = moviePoints[(j * VIDEO_MINI_WIDTH) + i];
                if(bUseFF.value && bUseFFMaster){
                    ofVec3f force = ff->getOrientationFromPos(mp.pos);
                    force.normalize();
                    force.operator*=(common_features->at("specCentroid") * 0.002);
                    force.z = 0.0005 * common_features->at("specFlatness");
                    mp.applyForce(&force);
                    mp.move(common_features->at("loudness") * 0.05);
                    x = mp.pos.x * rect_w_mul.value * width;
                    y = mp.pos.y * rect_h_mul.value * height;
                    z = mp.pos.z * rect_h_mul.value * height * zDir;
                }
                
                // move to the point on the screen that we want to put the rectangle
                ofPushMatrix();
                ofTranslate(x, y, z);
                
                ofFill();
                ofSetColor(col,local_alpha);
                
                ofSetRectMode(OF_RECTMODE_CENTER);
                float box_depth = ofMap(col.getBrightness(),0,255,rec_w * 1.5, rec_w * 0.1);
                drawRect(rec_w/2,rec_h/2,box_depth/-2,rec_w,rec_h,box_depth,local_mag, j);
                
                if((local_mag > avg_mag) && ((RectTypes)rectType.value != SPHERE)){
                    if(ofRandom(1.f) < 0.9999) ofNoFill();
                    ofSetColor(255,mp.rect_outline_alpha.update(255));
                    drawRect(rec_w * 0.5,rec_h * 0.5,box_depth * -0.5,rec_w,rec_h,box_depth,local_mag,j);
                }
                
                ofPopMatrix();
                
                i_counter++;
                j++;
                y_pos_scaled += rec_h;
            }
            i++;
            x_pos_scaled += rec_w;
        }
        
        light.disable();
        ofDisableLighting();
        ofDisableDepthTest();
        
        avg_mag = summingmag / i_counter;
    }
    
    void drawRect(float x, float y, float z, float w, float h, float depth, float local_mag, int row){
        switch((RectTypes)rectType.value){
            case RECT:
                ofDrawRectangle(x, y, w, h);
                break;
            case BOX:
                ofDrawBox(x,y,z,w,h,depth);
                break;
            case SPHERE:
                ofDrawSphere(x + ((row%2) * w * 0.5),y,z,local_mag * w);
                break;
        }
    }

    void display(int width, int height, unsigned long long frame_num, std::unordered_map<std::string, float>* common_features, bool isNRT, bool verbose){
        
        if(verbose){
            cout << "VideoModule::display\n";
            cout << "\tsrc: " << videos[currentSubVideoIndex.value]->src_path << endl;
            for(Param* p : params){
                cout << "\t" << p->name << ": ";
                p->post();
                cout << endl;
            }
            cout << "\tunfold tiles order: " << unfold_tiles_order << endl;
        }
        
        if(bShowHap.value){
            displayHap(width, height, frame_num, common_features, isNRT);
        } else {
            displayRects(width, height, frame_num, common_features, isNRT);
        }
        
    }
    
    nlohmann::json saveState(){
        nlohmann::json dict;
        
        for(Param* p : params){
            if(p->name != ""){
                dict[p->name] = p->save();
            }
        }

        dict["n_new_tiles_per_frame"] = n_new_tiles_per_frame;
        dict["unfold_tiles_order"] = (int)unfold_tiles_order;
        
        // TODO: because `lightPosition` is a ofVec3 and the nlohmann json package doesn't know what to do with this class
//        dict["lightPosition"] = lightPosition;
        
        for(int i = 0; i < N_CLUSTERS; i++){
            dict["center_color_indices" + ofToString(i)] = videos[currentSubVideoIndex.value]->center_color_indices[i];
        }
        
        return dict;
    }
    
    void loadState(nlohmann::json &dict, int width, int height, float** vecHistory, int vector_length, int history_length, bool vecHistoryFull){

        for(Param* p : params){
            if(dict[p->name]){
                nlohmann::json child = dict[p->name];
                p->load(child);
            }
        }
        
        n_new_tiles_per_frame = dict["n_new_tiles_per_frame"].get<int>();
        unfold_tiles_order = (UnfoldTilesOrder)dict["unfold_tiles_order"].get<int>();
        
        // TODO:
//        lightPosition = dict["lightPosition"].get<ofVec3f>();
        
        for(int i = 0; i < N_CLUSTERS; i++){
            videos[currentSubVideoIndex.value]->center_color_indices[i] = dict["center_color_indices" + ofToString(i)].get<int>();
        }
        
        for(int i = 0; i < N_MOVIEPOINTS; i ++){
            moviePoints[i].resetPos();
        }
    }
    
    float getSpeed(){
        return speed.value * speedDir.value;
    }
    
    void newParams(int width, int height, float** vecHistory, int vector_length, int history_length, bool vecHistoryFull, unsigned long long frame_num){
        
        for(Param* p : params){
            p->newRandom();
        }
    
        unfold_tiles_order = (UnfoldTilesOrder)ofRandom(8);
        
        for(int i = 0; i < N_CLUSTERS; i++){
            videos[currentSubVideoIndex.value]->center_color_indices[i] = ofRandom(mag_len);
        }
       
        for(int i = 0; i < N_MOVIEPOINTS; i ++){
            moviePoints[i].resetPos();
        }
        
        n_new_tiles_per_frame = ofRandom(1,4) + (999 * bDontUnfoldTiles.value);
        
        counting_tiles_start_frame = frame_num;
        
        lightPosition.x = ofRandom(width);
        lightPosition.y = ofRandom(height);
        lightPosition.z = ofRandom(height);

        light.setPosition(lightPosition);
    }

    void interact(VisualModule* other){}

    void receiveOSC(int width, int height, std::string label, float val){
        
        for(Param* p : params){
            if(label == p->name){
                p->setValue(val);
            }
        }
        
        if(label == "speedAndDir"){
            speed.value = abs(val);
            speedDir.value = (val > 0) + ((val < 0) * -1);
            bReactiveSpeed.value = false;
        } else if(label == "position"){ // normalized position
            //            nrt_playHead
            nrtPlayHead.value = val * (videos[0]->getTotalFrames()-1);
            //            mini_video
            videos[currentSubVideoIndex.value]->mini_vid.setPosition(val);
            //            hap
            videos[currentSubVideoIndex.value]->hap.setPosition(val);
        }
    }

    void screenResize(int w, int h){
        int displayX = 0;
        int displayW = 0;
        float displayRatio;
        int displayH = 0;
        int displayY = 0;
        
        if(((float)texture.getWidth() / (float)w) > ((float)texture.getHeight() / (float)h)){
            displayX = 0;
            displayW = w;
            displayRatio = (float)w / (float)texture.getWidth();
            displayH = texture.getHeight() * displayRatio;
            displayY = (ofGetHeight() - displayH) * 0.5;
        } else {
            displayY = 0;
            displayH = h;
            displayRatio = (float)h / (float)texture.getHeight();
            displayW = texture.getWidth() * displayRatio;
            displayX = (w-displayW) * 0.5;
        }
        
        points[0].set(displayX,displayY,0);
        points[1].set(displayX + displayW,displayY,0);
        points[2].set(displayX + displayW,displayY + displayH,0);
        points[3].set(displayX,displayY + displayH,0);
    }

};

#endif /* HapMovie_hpp */
