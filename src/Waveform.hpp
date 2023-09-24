//
//  Waveform.hpp
//  fonema video
//
//  Created by Ted Moore on 12/26/20.
//

#ifndef Waveform_hpp
#define Waveform_hpp

#include <stdio.h>
#include "ofMain.h"
#include "VisualModule.hpp"
#include <format>

class Waveform: public VisualModule {
public:
    enum waveformType { NORM = 0, LISSAJOUS , IKEDA, GRID };
    enum rectsDirection { HEIGHT_WIDTH = 0, WIDTH_HEIGHT , ANGLE_L , ANGLE_R };
    enum rectsShape { SQUARE = 0, CIRCLE , TWO_TRIANGLES };
    
    ParamEnumWeighted wfType;
    rectsDirection rectsDir = ANGLE_L;
    rectsShape rects_shape = SQUARE;
    
    int n_waveforms;
    int length;
    float** waveforms;
    
    int h;
    int* xoff;
    int* yoff;
    int* zoff;
    float* hmul;
    bool* show;
    float lissajous_line_width = 1.f;
    float waveform_line_width = 1.f;
    float ikeda_avg = 0.2;
    
    int maxNWaveforms = 2;
    
    int rect_side = 0;
    int triangle_side = 0;
    bool trianglesDir = true;
    bool scale_size = true;
    
    string getName(){
        return "Waveform";
    }
    
    void setup(int width, int height, float** waveforms_, int n_waveforms_, int length_, float** vecHistory, int vector_length, int history_length, bool vecHistoryFull, nlohmann::json config){
        lissajous_line_width = config["lissajous-line-width"].get<float>();
        waveform_line_width = config["waveform-line-width"].get<float>();
        n_waveforms = n_waveforms_;
        length = length_;
        waveforms = waveforms_;
        h = height;
        
        wfType.setup(config["waveform-type-weights"].get<vector<int>>(),config["waveform-type-default"].get<int>());
        wfType.setValue(1);
        
        xoff = new int[n_waveforms];
        yoff = new int[n_waveforms];
        zoff = new int[n_waveforms];
        
        hmul = new float[n_waveforms];
        show = new bool[n_waveforms];
        
        show[0] = true;
        xoff[0] = 0;
        zoff[0] = 0;
        hmul[0] = 1;
        
        screenResize(width,height);
        newParams(width, height, vecHistory,vector_length, history_length, vecHistoryFull,0);
    }

    //void Waveform::update(){}

    void display(int width, int height, unsigned long long frame_num, std::unordered_map<std::string, float>* common_features, bool isNRT, bool verbose){
        
        if(verbose){
            cout << "Waveform::display\n";
            cout << "\twfType:   " << wfType.value << endl;
        };
            
        switch(wfType.value){
            case LISSAJOUS:
            {
                ofSetColor(255);
                ofNoFill();
                ofSetLineWidth(lissajous_line_width);
                float w = width / 2.f;
                ofBeginShape();
                for (int i = 0; i < length * 0.1; i++) {
                    ofVertex(xoff[1] + w + (waveforms[0][i] * h), yoff[1] + (waveforms[1][i] * h));
                }
                ofEndShape();
            }
                break;
            case NORM:
            {
                for (int i = 0; i < n_waveforms; i++) {
                    if (show[i]) {
                        displayWaveform(i % maxNWaveforms, xoff[i], yoff[i], zoff[i], hmul[i],width,height);
                    }
                }
            }
                break;
            case IKEDA:
            {
                int w = width / n_waveforms;
                float rect_height = (float)height / length;
                ofSetColor(255,pow(common_features->at("loudness"),2) * 255); // what should the ikeda alpha be
                ofSetLineWidth(0);
                float runningsum = 0;
                ofSetRectMode(OF_RECTMODE_CORNER);
                for (int i = 0; i < n_waveforms; i++) {
                    for(int y = 0; y < length; y++){
                        float absval = abs(waveforms[i][y]);
                        runningsum += absval;
                        if(absval > ikeda_avg){
                            ofDrawRectangle(w * i, y * rect_height, w, rect_height);
                        }
                    }
                }
                
                ikeda_avg = ofLerp(ikeda_avg, (runningsum / (n_waveforms * height)), 0.01);
            }
                break;
            case GRID:
            {
                if(verbose){
                    cout << "\trectsDir: " << rectsDir << endl;
                };
                switch(rectsDir){
                    case HEIGHT_WIDTH:
                        traverseHeightWidth(width,height,rects_shape,verbose);
                        break;
                    case WIDTH_HEIGHT:
                        traverseWidthHeight(width,height,rects_shape, verbose);
                        break;
                    case ANGLE_L:
                        traverseAngleL(width,height,rects_shape);
                        break;
                    case ANGLE_R:
                        traverseAngleR(width,height,rects_shape, verbose);
                        break;
                }
            }
                break;
        }
    }
    
    void traverseHeightWidth(int width, int height, rectsShape rs, bool verbose){
        int counter = 0;
        int side = (rect_side * (rs != TWO_TRIANGLES)) + (triangle_side * (rs == TWO_TRIANGLES));
        
        if(verbose){
            cout << "\treceived width:  " << width << endl;
            cout << "\trecieved height: " << height << endl;
            cout << "\tside: " << side << endl;
        };
        
        int n_down = (height / side) + 1;
        int n_across = (width / side) + 1;
        
        for(int j = 0; j < n_down; j++){
            for(int i = 0; i < n_across; i++){
                drawShape(i,j,side,rs,counter);
                counter++;
            }
        }
    }
    
    void traverseWidthHeight(int width, int height, rectsShape rs,bool verbose){
        int counter = 0;
        int side = (rect_side * (rs != TWO_TRIANGLES)) + (triangle_side * (rs == TWO_TRIANGLES));
        
        if(verbose){
            cout << "\treceived width:  " << width << endl;
            cout << "\trecieved height: " << height << endl;
            cout << "\tside: " << side << endl;
        };
        
        int n_down = (height / side) + 1;
        int n_across = (width / side) + 1;
        
        
        for(int i = 0; i < n_across; i++){
            for(int j = 0; j < n_down; j++){
                drawShape(i,j,side,rs,counter);
                counter++;
            }
        }
    }
    
    void traverseAngleL(int width, int height, rectsShape rs){
        int side = (rect_side * (rs != TWO_TRIANGLES)) + (triangle_side * (rs == TWO_TRIANGLES));
        
        int i = (width / side) + 1;
        int j = (height / side) + 1;
        int counter = 0;
        
        for(int x = 0; x < i; x++){
            counter = getNextRect(x,0,i,j,counter,-1,side,rs);
        }
        
        for(int y = 1; y < j; y++){
            counter = getNextRect(i-1,y,i,j,counter,-1,side,rs);
        }
    }
    
    void traverseAngleR(int width, int height, rectsShape rs, bool verbose){
        int side = (rect_side * (rs != TWO_TRIANGLES)) + (triangle_side * (rs == TWO_TRIANGLES));
        
        int i = (width / side) + 1;
        int j = (height / side) + 1;
        int counter = 0;
        
        if(verbose){
            cout << "\t\tside: " << side << endl;
            cout << "\t\ti:    " << i << endl;
            cout << "\t\tj:    " << j << endl;
        }
        
        for(int x = (i-1); x >= 0; x--){
            counter = getNextRect(x,0,i,j,counter,1,side,rs);
        }
        
        for(int y = 1; y < j; y++){
            counter = getNextRect(0,y,i,j,counter,1,side,rs);
        }
    }
    
    int getNextRect(int x, int y, int i, int j, int counter, int xplus, int side, rectsShape rs){

        drawShape(x,y,side,rs,counter);
        
        x += xplus;
        y += 1;
        
        if((x >=0) && (y < j) && (x < i)){
            return getNextRect(x,y,i,j,counter + 1,xplus,side,rs);
        }
        
        return counter++;
    }
    
    void drawShape(int i, int j, int side, rectsShape rs, int counter){
        switch(rs){
            case SQUARE:
                ofSetRectMode(OF_RECTMODE_CENTER);
                drawSquare(i*side,j*side,side,abs(waveforms[int(counter / length)][counter % length]));
                break;
                
            case CIRCLE:
                drawCircle(i*side,j*side,side,abs(waveforms[int(counter / length)][counter % length]));
                break;
                
            case TWO_TRIANGLES:
                drawTwoTriangles(i*side,j*side,side,counter);
                break;
                
        }
    }
    
    void drawSquare(int x, int y, int side, float amp){
        ofSetColor(255,amp * 255);
        int half_side = side / 2;
        int side_scaled = (side * amp * scale_size) + ((1-scale_size) * side);
        ofDrawRectangle(x+half_side, y+half_side, side_scaled, side_scaled);
    }
    
    void drawTwoTriangles(int x, int y, int side, int counter){
        ofPushMatrix();
        int half_side = side / 2;
        ofTranslate(x+half_side,y+half_side);
        
        ofRotateZDeg(90.f * trianglesDir);
        
        ofSetColor(255,abs(waveforms[0][counter % length]) * 255);
        ofBeginShape();
        ofVertex(-half_side,-half_side);
        ofVertex(half_side,-half_side);
        ofVertex(half_side,half_side);
        ofEndShape();
        
        ofSetColor(255,abs(waveforms[1][counter % length]) * 255);
        ofBeginShape();
        ofVertex(-half_side,-half_side);
        ofVertex(-half_side,half_side);
        ofVertex(half_side,half_side);
        ofEndShape();
        
        ofPopMatrix();
    }
    
    void drawCircle(int x, int y, int side, float amp){
        ofSetColor(255,amp * 255);
        int half_side = side / 2;
        int r = (half_side * amp * scale_size) + ((1-scale_size) * half_side);
        ofDrawCircle(x+half_side,y+half_side,r);
    }

    void displayWaveform(int wf_int, int x, int y, int z, float hmul2, int display_width, int display_height){
        ofSetColor(255,255,255,255);
        ofNoFill();
        ofSetLineWidth(waveform_line_width);
        ofBeginShape();
        float xhop = (float)display_width / (float)length;
        for (int i = 0; i < length; i++) {
          float y2 = y + (waveforms[wf_int][i] * h * hmul2);
          float x2 = x + (i * xhop);
          ofVertex(x2, y2, -z);
        }
        ofEndShape();
    }

    void newParams(int width, int height, float** vecHistory, int vector_length, int history_length, bool vecHistoryFull, unsigned long long frame_num){

        wfType.newRandom();
        rectsDir = (rectsDirection)ofRandom(5);
        rects_shape = (rectsShape)ofRandom(3);
        trianglesDir = ofRandom(1.f) < 0.5;
        scale_size = ofRandom(1.f) < 0.4;
        
        for (int i = 0; i < n_waveforms; i++) {
            if (i > 0) {
                xoff[i] = ofRandom(-width, width);
                yoff[i] = ofRandom(0, height);
                zoff[i] = ofRandom(0, height);
                show[i] = ofRandom(1.0) > 0.5;
            }
            
            hmul[i] = ofRandom(0.3, 1.0);
        }
    }
    
    nlohmann::json saveState(){
        nlohmann::json dict;
        
        
        
        dict["wfType"] = wfType.value;
        dict["rectsDir"] = (int)rectsDir;
        dict["rects_shape"] = (int)rects_shape;
        dict["trianglesDir"] = trianglesDir;
        dict["scale_size"] = scale_size;
        
        for (int i = 0; i < n_waveforms; i++) {
            if (i > 0) {
                dict["xoff-" + ofToString(i)] = xoff[i];
                dict["yoff-" + ofToString(i)] = yoff[i];
                dict["zoff-" + ofToString(i)] = zoff[i];
                dict["show-" + ofToString(i)] = show[i];
            }
            
            dict["hmul-" + ofToString(i)] = hmul[i];
        }
        
        return dict;
    }
    
    void loadState(nlohmann::json &dict, int width, int height, float** vecHistory, int vector_length, int history_length, bool vecHistoryFull){

        wfType.value = dict["wfType"].get<int>();
        rectsDir = (rectsDirection)dict["rectsDir"].get<int>();
        rects_shape = (rectsShape)dict["rects_shape"].get<int>();
        trianglesDir = dict["trianglesDir"].get<bool>();
        scale_size = dict["scale_size"].get<bool>();
        
        for (int i = 0; i < n_waveforms; i++) {
            if (i > 0) {
                xoff[i] = dict["xoff-" + ofToString(i)].get<int>();
                yoff[i] = dict["yoff-" + ofToString(i)].get<int>();
                zoff[i] = dict["zoff-" + ofToString(i)].get<int>();
                show[i] = dict["show-" + ofToString(i)].get<bool>();
            }
            
            hmul[i] = dict["hmul-" + ofToString(i)].get<float>();
        }
    }

    void interact(VisualModule* other){}

    void receiveOSC(int width, int height, std::string label, float val){
        if(label == "setMaxNWaveforms"){
              maxNWaveforms = int(val);
        } else if (label == "setWaveformType"){
            wfType.setValue(val);
        } else if (label == "resetLissajousXY"){
              xoff[1] = 0;
              yoff[1] = height/2;
        }
    }

    void screenResize(int w, int h) {
        yoff[0] = h / 2;
        int bigA = w * h;
        float littleA = bigA / length;
        triangle_side = ceil(sqrt(littleA));
        
        littleA /= n_waveforms;
        rect_side = ceil(sqrt(littleA));
    
    }
};

#endif /* Waveform_hpp */
