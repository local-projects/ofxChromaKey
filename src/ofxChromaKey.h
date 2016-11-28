#pragma once
#include "ofMain.h"

class ofxChromaKey {
public:
    ofxChromaKey();
    void draw();
    void begin();
    void end();
    
    ofShader shader;
    ofColor keyColor;
    float threshold;
	float gamma;
	float hueRange = 0.5;
	float satRange = 0.5;
	float valRange = 0.5;
	bool useHSV;
};