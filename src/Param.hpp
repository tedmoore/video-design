//
//  Param.hpp
//  modular_video_02
//
//  Created by macprocomputer on 2/6/23.
//

#ifndef Param_hpp
#define Param_hpp

#include <stdio.h>
#include "ofMain.h"

class Param {
public:
    string name = "";
    void virtual load(ofxYAML::Node &y){
        reportError("load");
    }
    
    void reportError(string method){
        cout << "Param::" << method <<" ERROR: " << method << " being called in Param parent class." << endl;
    }
    
    void virtual newRandom(){
        reportError("newRandom");
    }
    
    ofxYAML::Node virtual save(){
        reportError("save");
        ofxYAML::Node dict;
        return dict;
    }
    
    void virtual post(){
        cout << "post being called in Param parent class\n";
    }
};

class ParamBool : public Param {
public:
    
    bool value = false;
    float trueProb = 0.5;
    
    void post(){
        cout << value;
    }
    
    ofxYAML::Node save(){
        ofxYAML::Node dict;
        dict["value"] = value;
        return dict;
    }
    
    void load(ofxYAML::Node &y){
        value = y["value"].as<bool>();
    }
    
    void newRandom(){
        value = ofRandom(1.f) < trueProb;
    }
};

class ParamFloat : public Param {
public:
    float value = 0.f;
    float min = 0.f;
    float max = 1.f;
    float power = 1.f;
    
    void post(){
        cout << value;
    }
    
    void setup(float min_, float max_, float power_, float val){
        min = min_;
        max = max_;
        power = power_;
        value = val;
    }
    
    ofxYAML::Node save(){
        ofxYAML::Node dict;
        dict["value"] = value;
        return dict;
    }
    
    void load(ofxYAML::Node &y){
        value = y["value"].as<float>();
    }
    
    void newRandom(){
        value = ofMap(pow(ofRandom(1.f),power),0.f,1.f,min,max);
    }
};

class ParamInt : public Param {
public:
    int value = 0.f;
    int min = 0.f;
    int max = 1.f;
    
    void post(){
        cout << value;
    }
    
    void setup(int min_, int max_, int val){
        min = min_;
        max = max_;
        value = val;
    }
    
    ofxYAML::Node save(){
        ofxYAML::Node dict;
        dict["value"] = value;
        return dict;
    }
    
    void load(ofxYAML::Node &y){
        value = y["value"].as<int>();
    }
    
    void newRandom(){
        value = ofRandom(min,max);
    }
};

class ParamIntList : public Param {
public:
    vector<int> listOptions;
    int value = 0;
    
    void post(){
        cout << value << " (whole list:";
        for(int op : listOptions){
            cout << " " << op;
        }
        cout << ")";
    }
    
    void setup(vector<int> list, int val){
        listOptions = list;
        value = val;
    }
    
    ofxYAML::Node save(){
        ofxYAML::Node dict;
        dict["value"] = value;
        return dict;
    }
    
    void load(ofxYAML::Node &y){
        value = y["value"].as<int>();
    }
    
    void newRandom(){
        value = listOptions[ofRandom(listOptions.size())];
    }
};

class ParamEnum : public Param {
public:
    int nEntries = 0;
    int value = 0;
    
    void post(){
        cout << value;
    }
    
    void setup(int nEntries_, int val){
        nEntries_ = nEntries;
        value = val;
    }
    
    ofxYAML::Node save(){
        ofxYAML::Node dict;
        dict["value"] = value;
        return dict;
    }
    
    void load(ofxYAML::Node &y){
        value = y["value"].as<int>();
    }
    
    void newRandom(){
        value = (int)ofRandom(nEntries);
    }
};

class ParamEnumWeighted : public Param {
public:
    vector<int> options;
    int value = 0;
    
    void post(){
        cout << value;
    }
    
    void setup(vector<int> options_, int val){
        options = options_;
        value = val;
    }
    
    ofxYAML::Node save(){
        ofxYAML::Node dict;
        dict["value"] = value;
        return dict;
    }
    
    void load(ofxYAML::Node &y){
        value = y["value"].as<int>();
    }
    
    void newRandom(){
        value = options[ofRandom(options.size())];
    }
};

#endif /* Param_hpp */
