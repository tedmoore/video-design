//
//  Mesh.hpp
//  fonema video
//
//  Created by Ted Moore on 1/1/21.
//

#ifndef Mesh_hpp
#define Mesh_hpp

#include <stdio.h>

#include "LagUD.hpp"
#include "VideoDesignPoint.hpp"
#include "VisualModule.hpp"
#include "Waveform.hpp"
#include "ofMain.h"

class Mesh : public VisualModule {
   public:
    int nPoints;
    bool waveformTracking = false;
    LagUD velLimit;
    int waveformEffectDim = 0;
    bool useFF = false, useFFmaster = true;
    int maxLines = 0;
    vector<VideoDesignPoint> points;

    float line_width = 1;
    float point_size = 1;

    float flow_field_influence = 0.04;
    float speed = 0.025;  // 0.0075
    float minSpeed = 0.01;
    LagUD jitterMag;
    float jitter_mul = 0.006;
    float dist_thresh_mul = 0.15;

    ofLight light;

    string getName() override {
        return "Mesh";
    }

    void screenResize(SystemState &s) override {}
    void update(SystemState &s) override {}
    void printStatus() override {}
    void loadState(SystemState &s, ofJson &dict) override {
        line_width = checkJsonKey(dict, "line-width", 1.0);
        point_size = checkJsonKey(dict, "point-size", 2.0);
        flow_field_influence = checkJsonKey(dict, "flow-field-influence", 0.04);
        speed = checkJsonKey(dict, "speed", 0.025);
        minSpeed = checkJsonKey(dict, "min-speed", 0.01);
        jitter_mul = checkJsonKey(dict, "jitter-mul", 0.006);
        dist_thresh_mul = checkJsonKey(dict, "dist-thresh-mul", 0.15);
        useFF = checkJsonKey(dict, "useFF", true);
        waveformEffectDim = checkJsonKey(dict, "waveformEffectDim", 0);
        maxLines = checkJsonKey(dict, "maxLines", 0);
        newPointLocs(s);
    }

    void setup(SystemState &s, ofJson &config) override {
        loadState(s, config);

        type = MESH;

        nPoints = checkJsonKey(config, "n-points", 500);
        points.resize(nPoints);

        for (int i = 0; i < nPoints; i++) {
            points[i].setup(s.flow_field->ff_parameters);
        }

        velLimit.setup(1, 0.14, 0);
        jitterMag.setup(1, 0.14, 0);

        light.setPointLight();
        light.setPosition(0, 0, 0);
    }

    void newPointLocs(SystemState &s) {
        if (s.vec_history.isFull) {
            bool is_used[DESCRIPTORS_VECTOR_LENGTH];
            for (int i = 0; i < DESCRIPTORS_VECTOR_LENGTH; i++) {
                is_used[i] = false;
            }
            int n_dims = 3;
            int choices[n_dims];

            for (int i = 0; i < n_dims; i++) {
                int choice;
                choice = int(ofRandom(DESCRIPTORS_VECTOR_LENGTH));  // there are 14 non-mfcc values
                while (is_used[choice]) {
                    choice = int(ofRandom(DESCRIPTORS_VECTOR_LENGTH));
                }
                choices[i] = choice;
            }

            for (int i = 0; i < nPoints; i++) {
                points[i].setXYZ(s.vec_history.history[i][choices[0]], s.vec_history.history[i][choices[1]], s.vec_history.history[i][choices[2]]);
            }
        } else {
            for (int i = 0; i < nPoints; i++) {
                points[i].setXYZ(ofRandom(0, s.flow_field->ff_parameters.xsize), ofRandom(0, s.flow_field->ff_parameters.ysize), ofRandom(0, s.flow_field->ff_parameters.zmax));
            }
        }
    }

    void newParams(SystemState &s) override {
        newPointLocs(s);

        if (ofRandom(1.0) < 0.8) {
            useFF = true;
        } else {
            useFF = false;
        }

        waveformEffectDim = ofRandom(3);
    }

    ofJson saveState() override {
        ofJson dict;

        dict["useFF"] = useFF;
        dict["waveformEffectDim"] = waveformEffectDim;

        return dict;
    }

    void display(SystemState &s) override {
        ofEnableLighting();
        // ofEnableDepthTest();
        // ofEnableBlendMode(OF_BLENDMODE_ADD);
        light.enable();

        jitterMag.update(s.features.loudness * jitter_mul);
        float distThresh = 0.03 + (s.features.sensory_dissonance * dist_thresh_mul);
        int n_lines = 0;
        velLimit.update((s.features.loudness * speed) + minSpeed);

        if (s.verbose) {
            cout << "Mesh" << endl;
            cout << "\tamp: " << s.features.loudness << endl;
            cout << "\tsensDis: " << s.features.sensory_dissonance << endl;
            cout << "\tjitter_mul: " << jitter_mul << endl;
            cout << "\tjitterMag: " << jitterMag.value << endl;
            cout << "\tdist_thresh_mul: " << dist_thresh_mul << endl;
            cout << "\tuseFF: " << useFF << endl;
            cout << "\tdistThresh: " << distThresh << endl;
            cout << "\tmin speed: " << minSpeed << endl;
            cout << "\tspeed: " << speed << endl;
            cout << "\tflow field influence: " << flow_field_influence << endl;
        }
        
        ofFill();
        ofSetColor(255, 255);

        // go through all the points...
        for (int i = 0; i < nPoints; i++) {

            // if the flow field is currently being used, get the orientation from the flow field and apply it to the point
            if (useFF && useFFmaster) {
                glm::vec3 ori = s.flow_field->getOrientationFromPos(points[i].pos);
                ori /= ori.length();
                ori *= speed * flow_field_influence;  // this float multiplier changes the amount that the flow field affects the point's direction
                points[i].applyForce(ori);
            }

            /* the actual waveform moving is done in the interact method, so here 
            we're just checking to see if we need to jitter a lot (because we're not
            waveform tracking) or a little, because we are */
            if (!waveformTracking) {
                points[i].move(jitterMag.value, velLimit.value);
            } else {
                points[i].move(jitterMag.value * 0.1, velLimit.value);
            }

            /* make sure this point is not "off screen" (which really means) out of bounds
            of the 3d space that has been defined... */
            points[i].checkEdges(s.flow_field->ff_parameters);
            // display this point
            points[i].display(s, point_size);

            if ((i < (nPoints - 1)) && (n_lines < maxLines)) {
                
                int i_lines = 0;
                for (int j = i + 1; j < nPoints; j++) {
                    // each point can't have more than 1% of the total lines
                    if (i_lines < (maxLines * 0.01)) {

                        float dist = points[i].distanceTo(points[j]);
                        // cout << "dist: " << dist << " / thresh: " << distThresh;
                        
                        if (dist < distThresh) {
                            int alpha = ofMap(dist, 0.f, distThresh, 255.f, 0.f,true);
                            // cout << " alpha: " << alpha << endl;
                            ofSetColor(255, alpha);
                            ofSetLineWidth(line_width);
                            drawLine(s, points[i], points[j], dist);
                            n_lines++;
                            i_lines++;
                        }

                        // cout << endl;
                    }
                }
            }
        }

        waveformTracking = false;

        light.disable();
        // ofEnableBlendMode(s.blendMode);
        // ofDisableDepthTest();
        ofDisableLighting();
    }

    void drawLine(SystemState &s, VideoDesignPoint &a, VideoDesignPoint &b, float dist) {
        ofDrawLine(a.x() * s.fbo.getWidth(), a.y() * s.fbo.getHeight(), a.z() * s.flow_field->ff_parameters.zDir * s.fbo.getHeight(), b.x() * s.fbo.getWidth(), b.y() * s.fbo.getHeight(), b.z() * s.flow_field->ff_parameters.zDir * s.fbo.getHeight());
    }

    void interact(SystemState &s, VisualModule *vc) override {
        switch (vc->type) {
            case WAVEFORM:
                waveformTracking = true;
                for (int i = 0; i < nPoints; i++) {
                    // first index of waveform is the x pos of this point
                    int wfAI = int(abs(points[i].x())) % WAVEFORM_LEN;
                    // second index of waveform is one past
                    int wfBI = (wfAI + 1) % WAVEFORM_LEN;
                    // get the values at those points
                    float wfa = s.features.waveforms[0][wfAI];
                    float wfb = s.features.waveforms[0][wfBI];
                    // make the y direction a result of that
                    float y = (wfb - wfa) * 0.1;
                    glm::vec3 offset(0, 0, 0);
                    offset[waveformEffectDim] = y;
                    points[i].add(offset);
                }
                break;
        }
    }

    void receiveOSC(SystemState &s, string cmd, float val) override {
        if (cmd == "useFFmaster") {
            if (val > 0.5) {
                useFFmaster = true;
            } else {
                useFFmaster = false;
            }
        }
    }
};

#endif /* Mesh_hpp */
