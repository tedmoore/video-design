//
//  HapMovie.cpp
//
//  Created by Ted Moore on 12/26/20.
//

#include "HapMovie.hpp"
#include "defines.h"

void HapMovie::setup(std::string path, ofVec3f pt0, ofVec3f pt1, ofVec3f pt2, ofVec3f pt3, float** mags_, int n_mag_, int mag_len_, bool isNRT, FlowField* ff_){
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
        cout << tiffs_dir.getAbsolutePath() << "\n";
        tiffs_dir.sort();
        tiffs = tiffs_dir.getFiles();
    
        ofDirectory bitexact_tiffs_dir(dir.getAbsolutePath() + "/mini-frames");
        cout << bitexact_tiffs_dir.getAbsolutePath() << "\n";
        bitexact_tiffs_dir.sort();
        bitexact_tiffs = bitexact_tiffs_dir.getFiles();
        
        mini_img.allocate(mini_width, mini_height, OF_IMAGE_COLOR);
        mini_pix.allocate(mini_width, mini_height, OF_PIXELS_RGBA);
        
        total_frames = MIN(bitexact_tiffs.size(),tiffs.size());
    }
    
    cluster_freq = TARGET_FRAME_RATE * ofRandom(15,25);
    
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
}

void HapMovie::update(bool isNRT){
    if(!isNRT){
        //cout << "mini vid updated\n";
        mini_vid.update();
    } else {
        nrt_playhead += speed;
        while(nrt_playhead < 0) nrt_playhead += total_frames;
        while(nrt_playhead >= total_frames) nrt_playhead -= total_frames;
    }
}

void HapMovie::display(int width, int height, int frame_num, std::unordered_map<std::string, float>* common_features, bool isNRT){
    
    // show hap
    //cout << "play head: " << nrt_playhead << ", as int: " << int(nrt_playhead) << ", n tiffs: " << tiffs.size() << ", n small tiffs: " << bitexact_tiffs.size() << endl;
    
    if(showHap){
        if(isNRT){
            img.load(tiffs[int(nrt_playhead)].getAbsolutePath());
            texture = img.getTexture();
        } else {
            texture = *player.getTexture();
        }
        //cout << "texture w h: " << texture.getWidth() << " " << texture.getHeight() << "\n";
        ofSetColor(255, alpha);
        texture.draw(points[0],points[1],points[2],points[3]);
    }
    
    // show rects
    // cluster this round
    
    if(showRects){
        
        // just getting the pixels
        if(isNRT){
            string path = bitexact_tiffs[int(nrt_playhead)].getAbsolutePath();
//            cout << path << "\n";
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
//        float rec_w = width * rect_w_mul;
//        float rec_h = height * rect_h_mul;
        int i = 0;
        int x_pos_scaled = 0;
        while(i < mini_width && x_pos_scaled < width){
            int j = 0;
            int y_pos_scaled = 0;
            while(j < mini_height && y_pos_scaled < height){
//                ofColor col = mini_pix.getColor((y * mini_width) + x);
                ofColor col = mini_pix.getColor(i,j);
                
                for(int i = 0; i < N_CLUSTERS; i++){
                    if(center_color_indices[i] == i_counter){
                        //cout << "color at index " << i << " out of 4, " << i_counter << " out of 1024:" << col << "\n";
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
                    int z = 0;
                
                    // get the point at this i, j and apply the force from the ff
                    MoviePoint &mp = moviePoints[(j * mini_width) + i];
                    if(useFF && useFFMaster){
                        ofVec3f force = ff->getOrientationFromPos(mp.pos);
                        force.normalize();
                        force.operator*=(common_features->at("specCentroid") * 0.002);
                        force.z = 0.0005 * common_features->at("specFlatness");
                        mp.applyForce(&force);
                        mp.move(common_features->at("amplitude") * 0.05);
                        x = mp.pos.x * rect_w_mul * width;
                        y = mp.pos.y * rect_h_mul * height;
                        z = mp.pos.z * rect_h_mul * height * zDir;
                    }
                    
                    ofPushMatrix();
                    ofTranslate(x, y, z);
                    
                    // move to the point on the screen that we want to put the rectangle
                    
                    if(ofRandom(1.f) < 0.9999) ofFill();
                    ofFill();
                    ofSetColor(col,local_alpha);
                    ofDrawRectangle(0, 0, rec_w, rec_h);
                    
                    if(local_mag > avg_mag){
                        if(ofRandom(1.f) < 0.9999) ofNoFill();
                        ofSetColor(255,mp.rect_outline_alpha.update(255));
                        ofDrawRectangle(0, 0,rec_w, rec_h);
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
        
        if(showRects){
            avg_mag = summingmag / i_counter;
            
//            cout << "cluster:     " << cluster_this_round << "\n";
//            cout << "mini_width:  " << mini_width << "\n";
//            cout << "mini_height: " << mini_height << "\n";
//            cout << "summing mag: " << summingmag << "\n";
            //cout << "avg mag:     " << avg_mag << "\n\n";
        }
    }
}

void HapMovie::newParams(int width, int height, float** vecHistory, int vector_length, int history_length, bool vecHistoryFull){
    speed = ofMap(pow(ofRandom(1.f),4.f),0.f,1.f,0.9 ,10) * dir_options[int(ofRandom(2.f))];
    player.setSpeed(speed);
    showHap = ofRandom(1.f) < 0.2;
    showRects = ofRandom(1.f) < 0.4;
    rect_w_mul = ofRandom(1.0,3.0);
    rect_h_mul = ofRandom(1.0,3.0);
    
    useFF = ofRandom(1.f) < 0.28;
    
    for(int i = 0; i < nMoviePoints; i ++){
        moviePoints[i].resetPos();
    }
    
    for(int i = 0; i < N_CLUSTERS; i++){
        center_color_indices[i] = ofRandom(mag_len);
    }
    
}

void HapMovie::interact(VisualContent* other){}

void HapMovie::receiveOSC(int width, int height, std::string label, float val){
    if(label == "speed"){
        player.setSpeed(val);
    }
}

void HapMovie::screenResize(int w, int h){
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
