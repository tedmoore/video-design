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
        vector<string> tokens = ofSplitString(marker, " ");
        vector<bool> isAction(tokens.size(), false);

        for (int i = 0; i < tokens.size(); i++){
            if (actions.find(tokens[i]) != actions.end()){
                isAction[i] = true;
            }
        }

        int index = 0;
        while (index < isAction.size()){
            if(isAction[index]){
                string action = tokens[index++];
                ofxOscMessage msg;
                while(index < isAction.size() && !isAction[index]){
                    msg.addFloatArg(ofToFloat(tokens[index++]));
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