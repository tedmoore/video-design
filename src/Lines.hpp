//
//  Lines.hpp
//  fonema video
//
//  Created by Ted Moore on 1/6/21.
//

#ifndef Lines_hpp
#define Lines_hpp

#include <stdio.h>

#include "VideoModule.hpp"
#include "VisualModule.hpp"
#include "ofMain.h"

enum lines_direction { HORZ = 0,
                       VERT };

class Lines : public VisualModule {
   public:
    bool inv;
    vector<float> *vec;
    int alphaThresh = 10;
    bool can_borrow_colors = false;
    bool is_borrow_colors = false;
    lines_direction dir = HORZ;
    // ofColor borrowed_colors[N_CLUSTERS];

    void receiveOSC(SystemState &s, std::string label, float val) override {}
    void screenResize(SystemState &s) override {}
    void update(SystemState &s) override {}
    void printStatus() override {}

    void setPtr(vector<float> *vec_) {
        vec = vec_;
    }

    void setup(SystemState &s, ofJson &dict) override {
        newParams(s);

        type = LINES;

        // for (int i = 0; i < N_CLUSTERS; i++) {
        //     ofColor col(255);
        //     borrowed_colors[i] = col;
        // }
    }

    void newParams(SystemState &s) override {
        chooseDir();
        chooseInv();
        is_borrow_colors = ofRandom(1.f) < 0.5;
    }

    ofJson saveState() override {
        ofJson dict;

        dict["dir"] = (int)dir;
        dict["inv"] = inv;
        dict["is_borrow_colors"] = is_borrow_colors;

        return dict;
    }

    void loadState(SystemState &s, ofJson &dict) override {
        dir = (lines_direction)dict["dir"].get<int>();
        inv = dict["inv"].get<bool>();
        is_borrow_colors = dict["is_borrow_colors"].get<bool>();
    }

    void interact(SystemState &s, VisualModule *vc) override {
        // switch (vc->type) {
        //     case HAP:
        //         if (can_borrow_colors) {
        //             VideoModule *vm = dynamic_cast<VideoModule *>(vc);
        //             if (vm->getCurrentVideo()->clustered) {
        //                 for (int i = 0; i < N_CLUSTERS; i++) {
        //                     borrowed_colors[i] = vm->getCurrentVideo()->center_colors[i];
        //                 }
        //             }
        //         }
        //         break;
        // }
    };

    void chooseDir() {
        dir = (lines_direction)ofRandom(2);
    }

    void chooseInv() {
        inv = ofRandom(1.0) > 0.5;
    }

    string getName() override {
        return "Lines";
    }

    void display(SystemState &s) override {
        ofSetLineWidth(0);
        ofFill();

        float line_w = s.fbo.getWidth() / (float)DESCRIPTORS_VECTOR_LENGTH;
        float line_h = s.fbo.getHeight() / (float)DESCRIPTORS_VECTOR_LENGTH;

        if (s.verbose) {
            cout << "Lines\n";
            cout << "\tdir:             " << dir << endl;
            cout << "\tinv:             " << inv << endl;
            cout << "\treceived width:  " << s.fbo.getWidth() << endl;
            cout << "\treceived height: " << s.fbo.getHeight() << endl;
            cout << "\tline width:      " << line_w << endl;
            cout << "\tline height:     " << line_h << endl;
        };

        ofSetRectMode(OF_RECTMODE_CORNER);

        for (int i = 0; i < DESCRIPTORS_VECTOR_LENGTH; i++) {
            float alpha = pow(vec->at(i), 0.75) * 255.f * (vec->at(i) > 0);
            if (alpha > alphaThresh) {
                // if (is_borrow_colors && can_borrow_colors) {
                //     ofSetColor(borrowed_colors[i % N_CLUSTERS], alpha);
                // } else {
                ofSetColor(255, 255, 255, alpha);
                // }
                drawLine(s.fbo.getWidth(), s.fbo.getHeight(), line_w, line_h, i);
            }
        }
    }

    void drawLine(int fbo_width, int fbo_height, float line_width, float line_height, int i) {
        switch (dir) {
            case HORZ:
                ofDrawRectangle(
                    0,
                    ((fbo_height - ((i + 1) * line_height)) * inv) + ((i * line_height) * (1 - inv)),
                    fbo_width,
                    line_height);
                break;
            case VERT:
                ofDrawRectangle(
                    ((fbo_width - ((i + 1) * line_width)) * inv) + ((i * line_width) * (1 - inv)),
                    0,
                    line_width,
                    fbo_height);
                break;
        }
    }
};

#endif /* Lines_hpp */
