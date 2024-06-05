//
//  Turtle.hpp
//  modular_video_02
//
//  Created by macprocomputer on 2/2/22.
//

#ifndef Turtle_hpp
#define Turtle_hpp

#include <stdio.h>

#include "VisualModule.hpp"
#include "ofMain.h"

class Turtle : public VisualModule {
   public:
    vector<glm::vec3> path;
    int divisor_i = 0;
    vector<int> divisors;
    int angle = 0;
    float stepSize = 30;
    int max_frames = 20;
    int frame_counter = 0;

    void interact(SystemState &s, VisualModule *other) override {}
    void receiveOSC(SystemState &s, std::string label, float val) override {}
    void screenResize(SystemState &s) override {}
    void update(SystemState &s) override {}
    void printStatus() override {}

    string getName() override {
        return "Turtle";
    }

    void setup(SystemState &s, ofJson &config) override {
        type = TURTLE;
        for (int i = 0; i < config["divisors"].size(); i++) {
            divisors.push_back(config["divisors"][i].get<int>());
        }

        max_frames = config["max-frames"].get<int>();

        newParams(s);
    }

    bool onScreen(glm::vec3 pt, int width, int height) {
        bool a = pt.x >= 0;
        bool b = pt.x < width;
        bool c = pt.y >= 0;
        bool d = pt.y < height;

        return a && b && c && d;
    }

    void display(SystemState &s) override {
        if (frame_counter < max_frames) {
            float angle = 360.f / divisors[divisor_i];
            float scale_factor = s.fbo.getHeight() / 1080.f;  // 1080 is native so we scale based off that... *shrug emoji*

            // make n (=5) steps
            for (int i = 0; i < 5; i++) {
                // using degrees
                int turns = int(ofRandom(divisors[divisor_i]));  // how many turns of "angle" degrees to make;

                glm::vec3 newvec(stepSize * int(ofRandom(1, 4) * scale_factor), 0, 0);

                for (int j = 0; j < turns; j++) {
                    newvec = glm::rotateZ(newvec,glm::radians(angle));
                }

                while (!onScreen(newvec + path[path.size() - 1], s.fbo.getWidth(), s.fbo.getHeight())) {
                    newvec = glm::rotateZ(newvec,glm::radians(angle));
                }

                newvec += path[path.size() - 1];

                path.push_back(newvec);
            }
        }

        frame_counter++;

        ofSetColor(255);
        ofNoFill();

        ofBeginShape();
        for (int i = 0; i < path.size(); i++) {
            ofVertex(path[i]);
        }
        ofEndShape();
    }

    void newParams(SystemState &s) override {
        divisor_i = int(ofRandom(divisors.size()));
        stepSize = ofRandom(70) + 30;
        restartPath(s);
    }

    void restartPath(SystemState &s) {
        frame_counter = 0;
        angle = 360 / divisors[divisor_i];
        path.clear();
        path.push_back(glm::vec3(ofRandom(s.fbo.getWidth()), ofRandom(s.fbo.getHeight()), 0));
    }

    ofJson saveState() override {
        ofJson dict;
        dict["divisor_i"] = divisor_i;
        dict["stepSize"] = stepSize;
        return dict;
    }

    void loadState(SystemState &s, ofJson &dict) override {
        divisor_i = dict["divisor_i"].get<int>();
        stepSize = dict["stepSize"].get<float>();
        restartPath(s);
    }
};

#endif /* Turtle_hpp */
