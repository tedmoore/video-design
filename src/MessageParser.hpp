#ifndef MESSAGEPARSER_HPP
#define MESSAGEPARSER_HPP

#include "ofMain.h"

using MessageActionFunction = std::function<void(const ofxOscMessage &msg)>;

struct MessageAction {
    vector<char> argument_types;
    MessageActionFunction function;

    // Default constructor
    MessageAction() = default;

    // constructor with parameters
    MessageAction(vector<char> at, MessageActionFunction f) : argument_types(at), function(f) {}
};

using StringTransformer = std::function<void(const std::string &, ofxOscMessage &msg)>;

inline void stringToFloat(const std::string &s, ofxOscMessage &msg) {
    msg.addFloatArg(ofToFloat(s));
}

inline void stringToInt(const std::string &s, ofxOscMessage &msg) {
    msg.addIntArg(ofToInt(s));
}

class MessageParser {
    public:
    std::unordered_map<string,MessageAction> actions;
    std::unordered_map<char,StringTransformer> string_transformers = {
        {'i', stringToInt},
        {'f', stringToFloat}
    };

    void registerAction(string name, vector<char> arg_types, MessageActionFunction f) {
        actions[name] = {arg_types,f};
    }

    void registerAction(string name, MessageActionFunction f) {
        actions[name] = {{},f};
    }

    void processReaperMarker(string marker){

        vector<string> tokens = ofSplitString(marker, " ");
        vector<bool> isAction(tokens.size(), false);

        for (int i = 0; i < isAction.size(); i++){
            if (actions.find(tokens[i]) != actions.end()){
                isAction[i] = true;
            }
        }

        int index = 0;
        while (index < isAction.size()){
            if(isAction[index]){
                string name = tokens[index++];
                ofxOscMessage msg;

                int arg_i = 0;
                while(index < isAction.size() && !isAction[index] && arg_i < actions[name].argument_types.size())
                    string_transformers[actions[name].argument_types[arg_i++]](tokens[index], msg);                    
                
                performAction(name, msg);
            } else {
                index++;
            }
        }
    }

    void performAction(std::string action, ofxOscMessage &msg) {   
        if(actions.find(action) != actions.end()) {
            actions[action].function(msg);
        } else if (action == "lastmarker/name"){
            processReaperMarker(msg.getArgAsString(0));
        }
    }
};

#endif /* MESSAGEPARSER_HPP */