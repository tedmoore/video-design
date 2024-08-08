//
//  Waveform.hpp
//  fonema video
//
//  Created by Ted Moore on 12/26/20.
//

#ifndef Waveform_hpp
#define Waveform_hpp

#include <stdio.h>

#include <format>

#include "VisualModule.hpp"
#include "ofMain.h"

#define N_BITS 16

struct WaveformParameters {
    int xoff = 0;
    int yoff;
    int zoff = 0;
    float hmul = 1.;
    bool show = true;
};

class Waveform : public VisualModule {
   public:
    enum waveformType {
        NORM = 0,
        LISSAJOUS,  // 1
        IKEDA,      // 2
        GRID,       // 3
        BITS        // 4
    };
    using WaveformStrategy = void (Waveform::*)(SystemState &s);
    WaveformStrategy waveformStrategies[5] = {
        &Waveform::waveforms,
        &Waveform::lissajous,
        &Waveform::ikeda,
        &Waveform::grid,
        &Waveform::bits};

    // SHAPES
    enum Shapes {
        SQUARE = 0,
        CIRCLE,
        TWO_TRIANGLES
    };
    using ShapeStrategy = void (Waveform::*)(SystemState &s, int x, int y, int side, int counter);
    ShapeStrategy shapeStrategies[3] = {
        &Waveform::drawSquare,
        &Waveform::drawCircle,
        &Waveform::drawTwoTriangles};

    // SHAPES TRAVERSAL DIRECTION
    enum ShapeTraversalDirection {
        HEIGHT_WIDTH = 0,
        WIDTH_HEIGHT,
        ANGLE_L,
        ANGLE_R
    };
    using TraverseStrategy = void (Waveform::*)(SystemState &s, const Waveform::Shapes &rs);
    TraverseStrategy traverseStrategies[4] = {
        &Waveform::traverseHeightWidth,
        &Waveform::traverseWidthHeight,
        &Waveform::traverseAngleL,
        &Waveform::traverseAngleR};

    ParamEnumWeighted wfType;
    ShapeTraversalDirection rectsDir = ANGLE_L;
    Shapes rects_shape = SQUARE;

    vector<WaveformParameters> w_params;
    float lissajous_line_width = 1.f;
    float waveform_line_width = 1.f;
    float ikeda_avg = 0.2;

    int maxNWaveforms = 2;

    int rect_side = 0;
    int triangle_side = 0;
    bool trianglesDir = true;
    bool bScaleShapeSize = true;

    // raises the amplitude to this power in order to warp the mapping of the amplitude to the size of the shape displayed
    double scale_size_warp = 0.5;

    string getName() override {
        return "Waveform";
    }

    void update(SystemState &s) override {}
    void loadState(SystemState &s, ofJson &dict) override {}
    void printStatus() override {}

    void setup(SystemState &s, ofJson &config) override {
        lissajous_line_width = checkJsonKey(config,"lissajous-line-width",1.0);
        cout << "lissajous_line_width: " << lissajous_line_width << endl;
        waveform_line_width = checkJsonKey(config,"waveform-line-width",1.0);
        cout << "waveform_line_width: " << waveform_line_width << endl;
        wfType.setup(config["waveform-type-weights"].get<vector<float>>(), checkJsonKey(config,"waveform-type-default",0));
        cout << "wfType: " << wfType.value << endl;
        wfType.setValue(1);
        w_params.resize(N_WAVEFORMS);
        screenResize(s);
        newParams(s);
    }

    void lissajous(SystemState &s) {
        ofSetColor(255);
        ofNoFill();
        ofSetLineWidth(lissajous_line_width);
        float w = s.fbo.getWidth() / 2.f;
        ofBeginShape();
        for (int i = 0; i < WAVEFORM_LEN * 0.1; i++) {
            ofVertex(w_params[1].xoff + w + (s.features.waveforms[0][i] * s.fbo.getHeight()), w_params[1].yoff + (s.features.waveforms[1][i] * s.fbo.getHeight()));
        }
        ofEndShape();
    }

    void waveforms(SystemState &s) {
        for (int i = 0; i < N_WAVEFORMS; i++) {
            if (w_params[i].show) {
                displayWaveform(s, i % maxNWaveforms, w_params[i].xoff, w_params[i].yoff, w_params[i].zoff, w_params[i].hmul, s.fbo.getWidth(), s.fbo.getHeight());
            }
        }
    }

    void ikeda(SystemState &s) {
        int w = s.fbo.getWidth() / N_WAVEFORMS;
        float rect_height = (float)s.fbo.getHeight() / WAVEFORM_LEN;
        ofSetColor(255, pow(s.features.loudness, 2) * 255);  // what should the ikeda alpha be
        ofSetLineWidth(0);
        float runningsum = 0;
        ofSetRectMode(OF_RECTMODE_CORNER);
        for (int i = 0; i < N_WAVEFORMS; i++) {
            for (int y = 0; y < WAVEFORM_LEN; y++) {
                float absval = abs(s.features.waveforms[i][y]);
                runningsum += absval;
                if (absval > ikeda_avg) {
                    ofDrawRectangle(w * i, y * rect_height, w, rect_height);
                }
            }
        }

        ikeda_avg = ofLerp(ikeda_avg, (runningsum / (N_WAVEFORMS * s.fbo.getHeight())), 0.01);
    }

    void grid(SystemState &s) {
        (this->*traverseStrategies[rectsDir])(s, rects_shape);
    }

    void bits(SystemState &s) {
        // TODO have a boolean Param for whether to draw the bits ((L to R) (T to B)) or ((T to B) (L to R))
        int w = s.fbo.getWidth() / 200;
        int sampleCounter = 0;
        int16_t integer;
        bool keepGoing = true;
        ofSetColor(255);
        ofSetLineWidth(0);
        int y = 0;
        while (y < s.fbo.getHeight() and keepGoing) {
            int x = 0;
            while (x < s.fbo.getWidth() and keepGoing) {
                integer = static_cast<int16_t>(s.features.waveforms[0][sampleCounter++] * 32767);
                for (int i = 0; i < N_BITS; i++)
                    if (integer & (1 << i))
                        ofDrawRectangle(x, y + (i * w), w, w);
                keepGoing = sampleCounter < WAVEFORM_LEN;
                x += w;
            }
            y += w * N_BITS;
        }
    }

    void display(SystemState &s) override {
        (this->*waveformStrategies[wfType.value])(s);
    }

    void traverseHeightWidth(SystemState &s, const Shapes &rs) {
        int counter = 0;
        int side = (rect_side * (rs != TWO_TRIANGLES)) + (triangle_side * (rs == TWO_TRIANGLES));
        if (s.verbose) {
            cout << "\treceived width:  " << s.fbo.getWidth() << endl;
            cout << "\trecieved height: " << s.fbo.getHeight() << endl;
            cout << "\tside: " << side << endl;
        };
        int n_down = (s.fbo.getHeight() / side) + 1;
        int n_across = (s.fbo.getWidth() / side) + 1;
        for (int j = 0; j < n_down; j++) {
            for (int i = 0; i < n_across; i++) {
                drawShape(s, i, j, side, rs, counter);
                counter++;
            }
        }
    }

    void traverseWidthHeight(SystemState &s, const Shapes &rs) {
        int counter = 0;
        int side = (rect_side * (rs != TWO_TRIANGLES)) + (triangle_side * (rs == TWO_TRIANGLES));
        if (s.verbose) {
            cout << "\treceived width:  " << s.fbo.getWidth() << endl;
            cout << "\trecieved height: " << s.fbo.getHeight() << endl;
            cout << "\tside: " << side << endl;
        };
        int n_down = (s.fbo.getHeight() / side) + 1;
        int n_across = (s.fbo.getWidth() / side) + 1;
        for (int i = 0; i < n_across; i++) {
            for (int j = 0; j < n_down; j++) {
                drawShape(s, i, j, side, rs, counter);
                counter++;
            }
        }
    }

    void traverseAngleL(SystemState &s, const Shapes &rs) {
        int side = (rect_side * (rs != TWO_TRIANGLES)) + (triangle_side * (rs == TWO_TRIANGLES));
        int i = (s.fbo.getWidth() / side) + 1;
        int j = (s.fbo.getHeight() / side) + 1;
        int counter = 0;
        for (int x = 0; x < i; x++) 
            counter = getNextRect(s, x, 0, i, j, counter, -1, side, rs);
        for (int y = 1; y < j; y++)
            counter = getNextRect(s, i - 1, y, i, j, counter, -1, side, rs);
    }

    void traverseAngleR(SystemState &s, const Shapes &rs) {
        int side = (rect_side * (rs != TWO_TRIANGLES)) + (triangle_side * (rs == TWO_TRIANGLES));
        int i = (s.fbo.getWidth() / side) + 1;
        int j = (s.fbo.getHeight() / side) + 1;
        int counter = 0;
        if (s.verbose) {
            cout << "\t\tside: " << side << endl;
            cout << "\t\ti:    " << i << endl;
            cout << "\t\tj:    " << j << endl;
        }
        for (int x = (i - 1); x >= 0; x--) 
            counter = getNextRect(s, x, 0, i, j, counter, 1, side, rs);
        for (int y = 1; y < j; y++) 
            counter = getNextRect(s, 0, y, i, j, counter, 1, side, rs);
    }

    int getNextRect(SystemState &s, int x, int y, int i, int j, int counter, int xplus, int side, Shapes rs) {
        drawShape(s, x, y, side, rs, counter);
        x += xplus;
        y += 1;
        if ((x >= 0) && (y < j) && (x < i)) 
            return getNextRect(s, x, y, i, j, counter + 1, xplus, side, rs);
        return counter++;
    }

    void drawShape(SystemState &s, int i, int j, int side, Shapes rs, int counter) {
        (this->*shapeStrategies[rs])(s, i * side, j * side, side, counter);
    }

    void drawSquare(SystemState &s, int x, int y, int side, int counter) {
        ofSetRectMode(OF_RECTMODE_CENTER);
        float amp = abs(s.features.waveforms[int(counter / WAVEFORM_LEN)][counter % WAVEFORM_LEN]);
        ofSetColor(255, amp * 255);
        int half_side = side / 2;
        int side_scaled = (side * pow(amp, scale_size_warp) * bScaleShapeSize) + ((1 - bScaleShapeSize) * side);
        ofDrawRectangle(x + half_side, y + half_side, side_scaled, side_scaled);
    }

    void drawTwoTriangles(SystemState &s, int x, int y, int side, int counter) {
        ofPushMatrix();
        int half_side = side / 2;
        ofTranslate(x + half_side, y + half_side);
        ofRotateZDeg(90.f * trianglesDir);
        ofSetColor(255, abs(s.features.waveforms[0][counter % WAVEFORM_LEN]) * 255);
        ofBeginShape();
        ofVertex(-half_side, -half_side);
        ofVertex(half_side, -half_side);
        ofVertex(half_side, half_side);
        ofEndShape();
        ofSetColor(255, abs(s.features.waveforms[1][counter % WAVEFORM_LEN]) * 255);
        ofBeginShape();
        ofVertex(-half_side, -half_side);
        ofVertex(-half_side, half_side);
        ofVertex(half_side, half_side);
        ofEndShape();
        ofPopMatrix();
    }

    void drawCircle(SystemState &s, int x, int y, int side, int counter) {
        float amp = abs(s.features.waveforms[int(counter / WAVEFORM_LEN)][counter % WAVEFORM_LEN]);
        ofSetColor(255, amp * 255);
        int half_side = side / 2;
        int r = (half_side * pow(amp, scale_size_warp) * bScaleShapeSize) + ((1 - bScaleShapeSize) * half_side);
        ofDrawCircle(x + half_side, y + half_side, r);
    }

    void displayWaveform(SystemState &s, int wf_int, int x, int y, int z, float hmul2, int display_width, int display_height) {
        ofSetColor(255, 255, 255, 255);
        ofNoFill();
        ofSetLineWidth(waveform_line_width);
        ofBeginShape();
        float xhop = (float)display_width / (float)WAVEFORM_LEN;
        for (int i = 0; i < WAVEFORM_LEN; i++) {
            float y2 = y + (s.features.waveforms[wf_int][i] * s.fbo.getHeight() * hmul2);
            float x2 = x + (i * xhop);
            ofVertex(x2, y2, -z);
        }
        ofEndShape();
    }

    void newParams(SystemState &s) override {
        wfType.newRandom();
        rectsDir = (ShapeTraversalDirection)ofRandom(4);
        rects_shape = (Shapes)ofRandom(3);
        trianglesDir = ofRandom(1.f) < 0.5;
        bScaleShapeSize = ofRandom(1.f) < 0.4;
        for (int i = 0; i < N_WAVEFORMS; i++) {
            if (i > 0) {
                w_params[i].xoff = ofRandom(-s.fbo.getWidth(), s.fbo.getWidth());
                w_params[i].yoff = ofRandom(0, s.fbo.getHeight());
                w_params[i].zoff = ofRandom(0, s.fbo.getHeight());
                w_params[i].show = ofRandom(1.0) > 0.5;
            }
            w_params[i].hmul = ofRandom(0.3, 1.0);
        }
    }

    ofJson saveState() override {
        ofJson dict;

        dict["wfType"] = wfType.value;
        dict["rectsDir"] = (int)rectsDir;
        dict["rects_shape"] = (int)rects_shape;
        dict["trianglesDir"] = trianglesDir;
        dict["bScaleShapeSize"] = bScaleShapeSize;

        for (int i = 0; i < N_WAVEFORMS; i++) {
            if (i > 0) {
                dict["xoff-" + ofToString(i)] = w_params[i].xoff;
                dict["yoff-" + ofToString(i)] = w_params[i].yoff;
                dict["zoff-" + ofToString(i)] = w_params[i].zoff;
                dict["show-" + ofToString(i)] = w_params[i].show;
            }

            dict["hmul-" + ofToString(i)] = w_params[i].hmul;
        }

        return dict;
    }

    void loadState(ofJson &dict, int width, int height, float **vecHistory, int vector_length, int history_length, bool vecHistoryFull) {
        wfType.value = dict["wfType"].get<int>();
        rectsDir = (ShapeTraversalDirection)dict["rectsDir"].get<int>();
        rects_shape = (Shapes)dict["rects_shape"].get<int>();
        trianglesDir = dict["trianglesDir"].get<bool>();
        bScaleShapeSize = dict["bScaleShapeSize"].get<bool>();

        for (int i = 0; i < N_WAVEFORMS; i++) {
            if (i > 0) {
                w_params[i].xoff = dict["xoff-" + ofToString(i)].get<int>();
                w_params[i].yoff = dict["yoff-" + ofToString(i)].get<int>();
                w_params[i].zoff = dict["zoff-" + ofToString(i)].get<int>();
                w_params[i].show = dict["show-" + ofToString(i)].get<bool>();
            }

            w_params[i].hmul = dict["hmul-" + ofToString(i)].get<float>();
        }
    }

    void interact(SystemState &s, VisualModule *other) override {}

    void receiveOSC(SystemState &s, std::string label, float val) override {
        // if (label == "setMaxNWaveforms") {
        //     maxNWaveforms = int(val);
        // } else if (label == "setWaveformType") {
        //     wfType.setValue(val);
        // } else if (label == "resetLissajousXY") {
        //     w_params[1].xoff = 0;
        //     w_params[1].yoff = s.fbo.getHeight() / 2;
        // }
    }

    void screenResize(SystemState &s) override {
        w_params[0].yoff = s.fbo.getHeight() / 2;
        int bigA = s.fbo.getWidth() * s.fbo.getHeight();
        float littleA = bigA / WAVEFORM_LEN;
        triangle_side = ceil(sqrt(littleA));

        littleA /= N_WAVEFORMS;
        rect_side = ceil(sqrt(littleA));
    }
};

#endif /* Waveform_hpp */
