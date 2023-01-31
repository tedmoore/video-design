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
#include <VisualContent.hpp>
#include "ofxHapPlayer.h"
#include "MoviePoint.hpp"
#include "FlowField.hpp"

#include "defines.h"

#define N_CLUSTERS 4

enum UnfoldTilesOrder { LRTB = 0 , LRBT, RLTB , RLBT , TBLR , TBRL , BTLR , BTRL };

class HapMovie: public VisualContent {
public:
    
    ofxHapPlayer player;
    ofTexture texture;
    ofVec3f points[4];
    ofVideoPlayer mini_vid;
    ofPixels mini_pix;
    
    ofColor center_colors[N_CLUSTERS];
    int center_color_indices[N_CLUSTERS];
    
    bool clustered = true;
    int cluster_freq;
    
    float** mags;
    int n_mag;
    int mag_len;
    
    int alpha = 255;
    float speed = 1.f;
    bool reactive_speed = true;
    float dir_options[2] = {-1.f,1.f};
    
    bool showHap, showRects;
    
    float avg_mag = 0.5;
    
    int mini_width = 32;
    int mini_height = 32;
    
    float rect_w_mul = 1.f;
    float rect_h_mul = 1.f;
    
    vector<ofFile> tiffs;
    vector<ofFile> bitexact_tiffs;
    
    ofImage img;
    ofImage mini_img;
        
    MoviePoint* moviePoints;
    int nMoviePoints;
    FlowField *ff;
    
    bool useFF = false;
    bool useFFMaster = true;
    
    int zDir = -1;
    
    float nrt_playhead = 0;
    int total_frames = 0;
    
    float show_hap_prob = 0.2; // 0.2
    float show_rects_prob = 0.4; // 0.4
    float use_ff_prob = 0.28; // 0.28
    
    bool bTile = true;
    float tile_scale = 0.5;
    float tile_offset_scale = 0.5;
    float i_x = 0;
    float i_y = 0;
    int tiles_alpha = 255;
    int n_new_tiles_per_frame = 1;
    int counting_tiles_start_frame = 0;
    UnfoldTilesOrder unfold_tiles_order = LRBT;
    bool dontUnfoldTiles = false;
    
    ofLight light;
    ofVec3f lightPosition = {0,0,0};
    bool bBoxes = false;
    float boxes_prob = 0.2;
    bool zShiftBoxes = true;

    void setup(std::string path, ofVec3f pt0, ofVec3f pt1, ofVec3f pt2, ofVec3f pt3, float** mags_, int n_mag_, int mag_len_, bool isNRT, FlowField* ff_, ofxYAML& config, int videoIndex){
        
        show_hap_prob = config["videos"][videoIndex]["show-hap-prob"].as<float>(); // 0.2
        show_rects_prob = config["videos"][videoIndex]["show-rects-prob"].as<float>(); // 0.4
        use_ff_prob = config["videos"][videoIndex]["use-ff-prob"].as<float>(); // 0.28
        boxes_prob = config["videos"][videoIndex]["boxes-prob"].as<float>(); // 0.28
        
        n_mag = n_mag_;
        mag_len = mag_len_;
        mags = mags_;
        points[0] = pt0;
        points[1] = pt1;
        points[2] = pt2;
        points[3] = pt3;
        
        ff = ff_;
        
        ofDirectory dir(path);
        
        if(!isNRT){ // is real-time
            player.load(dir.getAbsolutePath() + "/hap.mov");
            player.setLoopState(OF_LOOP_NORMAL);
            player.play();
            player.setVolume(0);

            mini_vid.load(dir.getAbsolutePath() + "/mini.mp4");
            mini_vid.setVolume(0);
            mini_vid.setLoopState(OF_LOOP_NORMAL);
            mini_vid.play();
            mini_pix.allocate(mini_vid.getWidth(),mini_vid.getHeight(),mini_vid.getPixelFormat());
        } else { // is non-real-time
            ofDirectory tiffs_dir(dir.getAbsolutePath() + "/frames");
            cout << "\t\t" << tiffs_dir.getAbsolutePath() << "\n";
            tiffs_dir.listDir();
            tiffs_dir.sort();
            tiffs = tiffs_dir.getFiles();
        
            ofDirectory bitexact_tiffs_dir(dir.getAbsolutePath() + "/mini-frames");
            cout << "\t\t" << bitexact_tiffs_dir.getAbsolutePath() << "\n";
            bitexact_tiffs_dir.listDir();
            bitexact_tiffs_dir.sort();
            bitexact_tiffs = bitexact_tiffs_dir.getFiles();
            
            mini_img.allocate(mini_width, mini_height, OF_IMAGE_COLOR);
            mini_pix.allocate(mini_width, mini_height, OF_PIXELS_RGBA);
            
            total_frames = MIN(bitexact_tiffs.size(),tiffs.size());
        }
        
        cluster_freq = config["target-framerate"].as<int>() * ofRandom(15,25);
        
        //cout << "cluster freq: " << cluster_freq << "\n";
        type = HAP;
        
        for(int i = 0; i < N_CLUSTERS; i++){
            center_color_indices[i] = ofRandom(mag_len);
        }
        
        nMoviePoints = mini_width * mini_height;
        
        moviePoints = new MoviePoint[nMoviePoints];
        for(int i = 0; i < mini_width; i++){
            for(int j = 0; j < mini_height; j++){
                MoviePoint *mp = new MoviePoint;
                mp->setup(i / float(mini_width),j / float(mini_height));
                moviePoints[(j * mini_width) + i] = *mp;
            }
        }
        
        light.setPointLight();
        light.setAmbientColor(0);
    }

    void update(bool isNRT, std::unordered_map<std::string, float>* common_features){
        setSpeed((speed * (1-reactive_speed)) + (reactive_speed * ofMap(common_features->at("specFlatness"),0.f,1.f,0.8,3)));
        
        if(!isNRT){
            //cout << "mini vid updated\n";
            mini_vid.update();
        } else {
            nrt_playhead += speed;
            while(nrt_playhead < 0) nrt_playhead += total_frames;
            while(nrt_playhead >= total_frames) nrt_playhead -= total_frames;
        }
    }
    
    void displayHap(int width, int height, int frame_num, std::unordered_map<std::string, float>* common_features, bool isNRT){
                        
        if(isNRT){
            img.load(tiffs[int(nrt_playhead)].getAbsolutePath());
            texture = img.getTexture();
        } else {
            texture = *player.getTexture();
        }
        //cout << "texture w h: " << texture.getWidth() << " " << texture.getHeight() << "\n";
        
        
        if(bTile){
            ofSetColor(255, tiles_alpha);
            int w = width * tile_scale;
            int h = height * tile_scale;
            int x_off = width * tile_offset_scale;
            int y_off = height * tile_offset_scale;
            int x_hop = w + x_off;
            int y_hop = h + y_off;
//            cout << "i_x: " << i_x << "\tw: " << w << "\tinitial x: " << (i_x * w) << endl;
//            cout << "i_y: " << i_y << "\th: " << h << "\tinitial y: " << (i_y * h) << endl;
            int tileCounter = 0;
            
            int make_n_tiles = (frame_num - counting_tiles_start_frame) * n_new_tiles_per_frame;
            
            switch(unfold_tiles_order){
                case LRTB: // 0
                    for(int y = ofMap(i_y,0.f,1.f,-h,y_off); y < height; y += y_hop){
                        for(int x = ofMap(i_x,0.f,1.f,-w,x_off); x < width; x += x_hop){
                            if(tileCounter++ < make_n_tiles) texture.draw(x,y,w,h);
                        }
                    }
                    break;
                case LRBT: // 1
                    for(int y = ofMap(i_y,0.f,1.f,height,height - y_hop); y > -h; y -= y_hop){
                        for(int x = ofMap(i_x,0.f,1.f,-w,x_off); x < width; x += x_hop){
                            if(tileCounter++ < make_n_tiles) texture.draw(x,y,w,h);
                        }
                    }
                    break;
                case RLTB: // 2
                    for(int y = ofMap(i_y,0.f,1.f,-h,y_off); y < height; y += y_hop){
                        for(int x = ofMap(i_x,0.f,1.f,width,width - x_hop); x > -w; x -= x_hop){
                            if(tileCounter++ < make_n_tiles) texture.draw(x,y,w,h);
                        }
                    }
                    break;
                case RLBT: // 3
                    for(int y = ofMap(i_y,0.f,1.f,height,height - y_hop); y > -h; y -= y_hop){
                        for(int x = ofMap(i_x,0.f,1.f,width,width - x_hop); x > -w; x -= x_hop){
                            if(tileCounter++ < make_n_tiles) texture.draw(x,y,w,h);
                        }
                    }
                    break;
                case TBLR: // 4
                    for(int x = ofMap(i_x,0.f,1.f,-w,x_off); x < width; x += x_hop){
                        for(int y = ofMap(i_y,0.f,1.f,-h,y_off); y < height; y += y_hop){
                            if(tileCounter++ < make_n_tiles) texture.draw(x,y,w,h);
                        }
                    }
                    break;
                case BTLR:
                    for(int x = ofMap(i_x,0.f,1.f,-w,x_off); x < width; x += x_hop){
                        for(int y = ofMap(i_y,0.f,1.f,height,height - y_hop); y > -h; y -= y_hop){
                            if(tileCounter++ < make_n_tiles) texture.draw(x,y,w,h);
                        }
                    }
                    break;
                case TBRL:
                    for(int x = ofMap(i_x,0.f,1.f,width,width - x_hop); x > -w; x -= x_hop){
                        for(int y = ofMap(i_y,0.f,1.f,-h,y_off); y < height; y += y_hop){
                            if(tileCounter++ < make_n_tiles) texture.draw(x,y,w,h);
                        }
                    }
                    break;
                case BTRL:
                    for(int x = ofMap(i_x,0.f,1.f,width,width - x_hop); x > -w; x -= x_hop){
                        for(int y = ofMap(i_y,0.f,1.f,height,height - y_hop); y > -h; y -= y_hop){
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
    
    void displayRects(int width, int height, int frame_num, std::unordered_map<std::string, float>* common_features, bool isNRT){
        // just getting the pixels
        if(isNRT){
            string path = bitexact_tiffs[int(nrt_playhead)].getAbsolutePath();
            mini_img.load(path);
            mini_pix = mini_img.getPixels();
        } else {
            if(mini_vid.isFrameNew()) mini_pix = mini_vid.getPixels();
        }

        ofSetLineWidth(1);
        int i_counter = 0;
        float summingmag = 0;
        float rec_w = (width / mini_width) * rect_w_mul;
        float rec_h = (height / mini_height) * rect_h_mul;
        int i = 0;
        int x_pos_scaled = 0;
        
//        ofEnableDepthTest();
        ofEnableLighting();
        light.enable();
        
        while(i < mini_width && x_pos_scaled < width){
            int j = 0;
            int y_pos_scaled = 0;
            while(j < mini_height && y_pos_scaled < height){
                ofColor col = mini_pix.getColor(i,j);
                
                for(int i = 0; i < N_CLUSTERS; i++){
                    if(center_color_indices[i] == i_counter){
                        center_colors[i] = col;
                        break;
                    }
                }
                if(showRects){
                    // showing the rectangles
                    
                    // figure out the alpha
                    float local_mag = mags[0][i_counter];
                    summingmag += local_mag;
                    float local_alpha = ofMap(pow(local_mag,0.5),0.f,1.f,-10.f,255.f);
                    
                    int x = x_pos_scaled;
                    int y = y_pos_scaled;
                    int z = ofMap(local_mag,0,1,height * 0.5,0) * zShiftBoxes;
                
                    // get the point at this i, j and apply the force from the ff
                    MoviePoint &mp = moviePoints[(j * mini_width) + i];
                    if(useFF && useFFMaster){
                        ofVec3f force = ff->getOrientationFromPos(mp.pos);
                        force.normalize();
                        force.operator*=(common_features->at("specCentroid") * 0.002);
                        force.z = 0.0005 * common_features->at("specFlatness");
                        mp.applyForce(&force);
                        mp.move(common_features->at("loudness") * 0.05);
                        x = mp.pos.x * rect_w_mul * width;
                        y = mp.pos.y * rect_h_mul * height;
                        z = mp.pos.z * rect_h_mul * height * zDir;
                    }
                    
                    // move to the point on the screen that we want to put the rectangle
                    ofPushMatrix();
                    ofTranslate(x, y, z);
                    
                    if(ofRandom(1.f) < 0.9999) ofFill();
                    ofFill();
                    ofSetColor(col,local_alpha);
                    
                    float box_depth = ofMap(col.getBrightness(),0,255,rec_w * 1.5, rec_w * 0.1);
                    if(bBoxes){
                        ofDrawBox(rec_w/2,rec_h/2,box_depth/-2,rec_w,rec_h,box_depth);
                    } else {
                        ofDrawRectangle(0, 0, rec_w, rec_h);
                    }
                    
                    if(local_mag > avg_mag){
                        if(ofRandom(1.f) < 0.9999) ofNoFill();
                        ofSetColor(255,mp.rect_outline_alpha.update(255));
                        if(bBoxes){
                            ofDrawBox(rec_w/2,rec_h/2,box_depth/-2,rec_w,rec_h,box_depth);
                        } else {
                            ofDrawRectangle(0, 0, rec_w, rec_h);
                        }
                    }
                    
                    ofPopMatrix();
                }
                i_counter++;
                j++;
                y_pos_scaled += rec_h;
            }
            i++;
            x_pos_scaled += rec_w;
        }
        
        light.disable();
        ofDisableLighting();
        
        avg_mag = summingmag / i_counter;
    }

    void display(int width, int height, int frame_num, std::unordered_map<std::string, float>* common_features, bool isNRT){
        
        if(showHap){
            displayHap(width, height, frame_num, common_features, isNRT);
        }
        
        if(showRects){
            displayRects(width, height, frame_num, common_features, isNRT);
        }
        
    }
    
    ofxYAML::Node saveState(){
        ofxYAML::Node dict;
        
        dict["speed"] = speed;
        dict["reactive_speed"] = reactive_speed;
        dict["showHap"] = showHap;
        dict["showRects"] = showRects;
        dict["rect_w_mul"] = rect_w_mul;
        dict["rect_h_mul"] = rect_h_mul;
        dict["bTile"] = bTile;
        dict["tile_scale"] = tile_scale;
        dict["tile_offset_scale"] = tile_offset_scale;
        dict["i_x"] = i_x;
        dict["i_y"] = i_y;
        dict["tiles_alpha"] = tiles_alpha;
        dict["useFF"] = useFF;
        dict["n_new_tiles_per_frame"] = n_new_tiles_per_frame;
        dict["dontUnfoldTiles"] = dontUnfoldTiles;
        dict["unfold_tiles_order"] = (int)unfold_tiles_order;
        dict["bBoxes"] = bBoxes;
        dict["lightPosition"] = lightPosition;
        dict["zShiftBoxes"] = zShiftBoxes;
        
        for(int i = 0; i < N_CLUSTERS; i++){
            dict["center_color_indices" + ofToString(i)] = center_color_indices[i];
        }
        
        return dict;
    }
    
    void loadState(ofxYAML::Node &dict, int width, int height, float** vecHistory, int vector_length, int history_length, bool vecHistoryFull){
        
        setSpeed(dict["speed"].as<float>());
        reactive_speed = dict["reactive_speed"].as<bool>();
        showHap = dict["showHap"].as<bool>();
        showRects = dict["showRects"].as<bool>();
        rect_w_mul = dict["rect_w_mul"].as<float>();
        rect_h_mul = dict["rect_h_mul"].as<float>();
        bTile = dict["bTile"].as<bool>();
        tile_scale = dict["tile_scale"].as<float>();
        tile_offset_scale = dict["tile_offset_scale"].as<float>();
        i_x = dict["i_x"].as<float>();
        i_y = dict["i_y"].as<float>();
        tiles_alpha = dict["tiles_alpha"].as<int>();
        n_new_tiles_per_frame = dict["n_new_tiles_per_frame"].as<int>();
        dontUnfoldTiles = dict["dontUnfoldTiles"].as<bool>();
        unfold_tiles_order = (UnfoldTilesOrder)dict["unfold_tiles_order"].as<int>();
        useFF = dict["useFF"].as<bool>();
        bBoxes = dict["bBoxes"].as<bool>();
        zShiftBoxes = dict["zShiftBoxes"].as<bool>();
        lightPosition = dict["lightPosition"].as<ofVec3f>();
        
        for(int i = 0; i < N_CLUSTERS; i++){
            center_color_indices[i] = dict["center_color_indices" + ofToString(i)].as<int>();
        }
        
        for(int i = 0; i < nMoviePoints; i ++){
            moviePoints[i].resetPos();
        }
    }

    void newParams(int width, int height, float** vecHistory, int vector_length, int history_length, bool vecHistoryFull, int frame_num){
        
        setSpeed(ofMap(pow(ofRandom(1.f),4.f),0.f,1.f,0.9 ,10) * dir_options[int(ofRandom(2.f))]);
        reactive_speed = ofRandom(1.f) < 0.5;
        
        showHap = ofRandom(1.f) < show_hap_prob; // 0.2
        showRects = ofRandom(1.f) < show_rects_prob; // 0.4
        rect_w_mul = ofRandom(1.0,3.0);
        rect_h_mul = ofRandom(1.0,3.0);
        
        bTile = ofRandom(1.f) < 0.8;
        tile_scale = ofRandom(0.03,0.5);
        tile_offset_scale = ofRandom(0.0,0.4);
        i_x = ofRandom(1.f);
        i_y = ofRandom(1.f);
        tiles_alpha = ofRandom(1,255);
        unfold_tiles_order = (UnfoldTilesOrder)ofRandom(8);
        bBoxes = ofRandom(1.f) < boxes_prob;
        
        useFF = ofRandom(1.f) < use_ff_prob; // 0.28
        
        for(int i = 0; i < N_CLUSTERS; i++){
            center_color_indices[i] = ofRandom(mag_len);
        }
       
        for(int i = 0; i < nMoviePoints; i ++){
            moviePoints[i].resetPos();
        }
        
        dontUnfoldTiles = ofRandom(1.f) < 0.5;
        zShiftBoxes = ofRandom(1.f) < 0.35;
        n_new_tiles_per_frame = ofRandom(1,4) + (999 * dontUnfoldTiles);
        counting_tiles_start_frame = frame_num;
        
        lightPosition.x = ofRandom(width);
        lightPosition.y = ofRandom(height);
        lightPosition.z = ofRandom(height);

        light.setPosition(lightPosition);
    }

    void interact(VisualContent* other){}

    void receiveOSC(int width, int height, std::string label, float val){
        if(label == "speed"){
            setSpeed(val);
        }
    }
    
    void setSpeed(float val){
        speed = val;
        player.setSpeed(speed);
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
