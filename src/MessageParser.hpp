#ifndef MESSAGEPARSER_HPP
#define MESSAGEPARSER_HPP

#include "ofMain.h"

using MessageAction = std::function<void(const ofxOscMessage &msg)>;

class MessageParser {
    public:
    std::unordered_map<string,MessageAction> actions;

    void registerAction(string name, MessageAction f) {
        actions[name] = f;
    }

    void processReaperMarker(string marker){

        cout << "Reaper Marker: " << marker << endl;

        vector<string> tokens = ofSplitString(marker, " ");
        vector<bool> isAction(tokens.size(), false);

        for (int i = 0; i < tokens.size(); i++){
            if (actions.find(tokens[i]) != actions.end()){
                isAction[i] = true;
            }
        }

        cout << "\ttokens: ";
        for(int i = 0; i < tokens.size(); i++){
            cout << tokens[i] << " ";
        }
        cout << endl;

        int index = 0;
        while (index < isAction.size()){
            if(isAction[index]){
                string action = tokens[index++];
                ofxOscMessage msg;
                while(index < isAction.size() && !isAction[index]){
                    float fl = ofToFloat(tokens[index]);
                    int in = ofToInt(tokens[index]);
                    cout << "float " << fl << " int " << in << endl;
                    index++;
                    msg.addFloatArg(fl);
                }
                performAction(action, msg);
            } else {
                index++;
            }
        }
    }

    void performAction(std::string action, ofxOscMessage &msg) {   
        if(actions.find(action) != actions.end()) {
            actions[action](msg);
        } else if (action == "lastmarker/name"){
            processReaperMarker(msg.getArgAsString(0));
        }
    }
};

#endif /* MESSAGEPARSER_HPP */