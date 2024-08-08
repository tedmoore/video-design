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

class MessageParser {
    public:
    std::unordered_map<string,MessageAction> actions;

    void registerAction(string name, vector<char> arg_types, MessageActionFunction f) {
        actions[name] = {arg_types,f};
    }

    void registerAction(string name, MessageActionFunction f) {
        actions[name] = {{},f};
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
                string name = tokens[index++];
                ofxOscMessage msg;

                int arg_i = 0;
                while(index < isAction.size() && !isAction[index] && arg_i < actions[name].argument_types.size()){

                    switch(actions[name].argument_types[arg_i]){
                        case 'i':
                            msg.addIntArg(ofToInt(tokens[index++]));
                            break;
                        case 'f':
                            msg.addFloatArg(ofToFloat(tokens[index++]));
                            break;
                        default:
                            cout << "MessageParser::processReaperMarker argument " << actions[name].argument_types[arg_i] << " type not recognized" << endl;
                            assert(false);
                            break;
                    }
                    
                    arg_i++;
                }

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