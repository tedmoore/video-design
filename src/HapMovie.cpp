//
//  HapMovie.cpp
//  fonema video
//
//  Created by Ted Moore on 12/26/20.
//

#include "HapMovie.hpp"
#include "defines.h"

void HapMovie::setup(std::string path, ofVec3f pt0, ofVec3f pt1, ofVec3f pt2, ofVec3f pt3, float** mags_, int n_mag_, int mag_len_, bool isNRT){
    n_mag = n_mag_;
    mag_len = mag_len_;
    mags = mags_;
    points[0] = pt0;
    points[1] = pt1;
    points[2] = pt2;
    points[3] = pt3;
    
    ofDirectory dir(path);
    
    if(!isNRT){
        cout << "\nhap movie:\n" << dir.getAbsolutePath() + "/hap_avf.mov\n\n";
        player.load(dir.getAbsolutePath() + "/hap_avf.mov");
        player.setLoopState(OF_LOOP_NORMAL);
        player.play();
        player.setVolume(0);
        //texture = new ofTexture;
        cout << "player w h: " << player.getWidth() << " " << player.getHeight() << "\n";
        texture.allocate(player.getWidth(),player.getHeight(),GL_RGBA);

        cout << "\nmini movie:\n" << dir.getAbsolutePath() + "/mini_me_scaleToFill.mp4\n\n";
        mini_vid.load(dir.getAbsolutePath() + "/mini_me_scaleToFill.mp4");
        mini_vid.setVolume(0);
        mini_vid.setLoopState(OF_LOOP_NORMAL);
        mini_vid.play();
        mini_pix.allocate(mini_vid.getWidth(),mini_vid.getHeight(),OF_PIXELS_RGBA);
    } else {
        ofDirectory tiffs_dir(dir.getAbsolutePath() + "/tiffs");
        cout << tiffs_dir.getAbsolutePath() << "\n";
        tiffs_dir.sort();
        tiffs = tiffs_dir.getFiles();
    
        ofDirectory bitexact_tiffs_dir(dir.getAbsolutePath() + "/mini_bitexact_tiffs");
        cout << bitexact_tiffs_dir.getAbsolutePath() << "\n";
        bitexact_tiffs_dir.sort();
        bitexact_tiffs = bitexact_tiffs_dir.getFiles();
        
        mini_img.allocate(mini_width, mini_height, OF_IMAGE_COLOR);
        mini_pix.allocate(mini_width, mini_height, OF_PIXELS_RGBA);
    }
    
    cluster_freq = TARGET_FRAME_RATE * ofRandom(15,25);
    
    //cout << "cluster freq: " << cluster_freq << "\n";
    type = HAP;
    
    for(int i = 0; i < N_CLUSTERS; i++){
        center_color_indices[i] = ofRandom(mag_len);
    }
}

void HapMovie::update(bool isNRT){
    if(!isNRT){
        //cout << "mini vid updated\n";
        mini_vid.update();
    }
}

void HapMovie::display(int width, int height, int frame_num, std::unordered_map<std::string, float>* common_features, bool isNRT){
    bool cluster_this_round = false;
    
    //    cout << ofGetFrameNum() + 1 << " " << TARGET_FRAME_RATE * 20 << " " << (ofGetFrameNum() + 1) % (TARGET_FRAME_RATE * 20) << "\n";
    cluster_counter++;
//    if((cluster_counter++ % cluster_freq) == 0){
//        cluster_this_round = true;
//        //cout << "cluster this round\n";
//        center_colors.clear();
//    }
    
    // is nrt
    
    // show hap
    
    if(showHap){
        if(isNRT){
            img.load(tiffs[cluster_counter % tiffs.size()].getAbsolutePath());
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
    
    if(cluster_this_round || showRects){
        
        // just getting the pixels
        if(isNRT){
            string path = bitexact_tiffs[frame_num % bitexact_tiffs.size()].getAbsolutePath();
            cout << path << "\n";
            mini_img.load(path);
            mini_pix = mini_img.getPixels();
        } else {
            if(mini_vid.isFrameNew()) mini_pix = mini_vid.getPixels();
        }

        ofSetLineWidth(1);
        int i_counter = 0;
        float summingmag = 0;
        int rec_w = (width / mini_width) * rect_w_mul;
        int rec_h = (height / mini_height) * rect_h_mul;
        int x = 0;
        int x_pos_scaled = 0;
        while(x < mini_width && x_pos_scaled < width){
            int y = 0;
            int y_pos_scaled = 0;
            while(y < mini_height && y_pos_scaled < height){
//                ofColor col = mini_pix.getColor((y * mini_width) + x);
                ofColor col = mini_pix.getColor(x,y);
                
                for(int i = 0; i < N_CLUSTERS; i++){
                    if(center_color_indices[i] == i_counter){
                        //cout << "color at index " << i << " out of 4, " << i_counter << " out of 1024:" << col << "\n";
                        center_colors[i] = col;
                        break;
                    }
                }
                //cout << col << "\n";
                if(showRects){
                    //cout << x << " " << y << "\n";
                    float local_mag = mags[0][i_counter];
                    summingmag += local_mag;
                    float local_alpha = ofMap(pow(local_mag,0.5),0.f,1.f,-10.f,255.f);
                    
                    if(ofRandom(1.f) < 0.9999) ofFill();
                    ofFill();
                    ofSetColor(col,local_alpha);
                    ofDrawRectangle(x_pos_scaled, y_pos_scaled,rec_w, rec_h);
                    //ofSetColor(255, 0, 0);
                    //ofDrawBitmapString(ofToString(x) + "," + ofToString(y), x * rec_w, (y * rec_h) + 12);
                    
                    if(local_mag > avg_mag){
                        if(ofRandom(1.f) < 0.9999) ofNoFill();
                        ofSetColor(255,255);
                        ofDrawRectangle(x_pos_scaled, y_pos_scaled,rec_w, rec_h);
                    }
                }
                if(cluster_this_round){
                    color_points[i_counter][0] = float(col.r);
                    color_points[i_counter][1] = float(col.g);
                    color_points[i_counter][2] = float(col.b);
                }
                i_counter++;
                y++;
                y_pos_scaled += rec_h;
            }
            x++;
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
        
//        if(cluster_this_round){
//
//
//            cv::Mat points(N_MINI_COLOR_POINTS,3,CV_32F,color_points);
//            //        points.convertTo(points,CV_32F);
//            //        cout << color_points << "\n";
//            //        cout << points << "\n";
//
//            cv::Mat centers(N_CLUSTERS,3,CV_32F);
//            //cv::Mat labels(N_MINI_COLOR_POINTS,1,CV_8U);
//            vector<int> labels;
//            cv::kmeans(points,N_CLUSTERS,labels,cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS,1000,1),10,cv::KmeansFlags::KMEANS_RANDOM_CENTERS,centers);
//            //        cout << centers << "\n";
//
//            for(int i = 0; i < N_CLUSTERS; i++){
//                ofColor col(centers.at<float>(i,0),centers.at<float>(i,1),centers.at<float>(i,2));
//                center_colors.push_back(col);
//                //            cout << center_colors[i] << "\n";
//            }
//
//            ofSort(center_colors,[](ofColor &a, ofColor &b) -> bool {
//                return a.getBrightness() < b.getBrightness();
//            });
//
//            for(int i = 0; i < N_CLUSTERS; i++){
//                center_colors[i].setSaturation(center_colors[i].getSaturation() * 1.4);
//                center_colors[i].setBrightness(center_colors[i].getBrightness() * 1.1);
//                //ofSetColor(center_colors[i]);
//                //ofDrawRectangle(i * wid, 0, wid, wid);
//            }
//
//            clustered = true;
//        }
    }
}

void HapMovie::newParams(int width, int height, float** vecHistory, int vector_length, int history_length, bool vecHistoryFull){
    speed = ofMap(pow(ofRandom(1.f),4),0,1,0.9 ,10) * dir_options[int(ofRandom(2.f))];
    player.setSpeed(speed);
    showHap = ofRandom(1.f) < 0.2;
    showRects = ofRandom(1.f) < 0.4;
    rect_w_mul = ofRandom(1.0,3.0);
    rect_h_mul = ofRandom(1.0,3.0);
    
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
    ofVec3f* pt0 = new ofVec3f(0,0,0);
    ofVec3f* pt1 = new ofVec3f(w,0,0);
    ofVec3f* pt2 = new ofVec3f(w,h,0);
    ofVec3f* pt3 = new ofVec3f(0,h,0);
    
    points[0] = *pt0;
    points[1] = *pt1;
    points[2] = *pt2;
    points[3] = *pt3;
}
