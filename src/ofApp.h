#pragma once

#include "ofMain.h"
#include "ofxGui.h"

struct pingPongBuffer {
public:
	void allocate(int _width, int _height, int _internalformat = GL_RGBA) {
		// Allocate
		for (int i = 0; i < 2; i++) {
			FBOs[i].allocate(_width, _height, _internalformat);
			FBOs[i].getTexture().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
		}

		//Assign
		src = &FBOs[0];
		dst = &FBOs[1];

		// Clean
		clear();
	}

	void swap() {
		std::swap(src, dst);
	}

	void clear() {
		for (int i = 0; i < 2; i++) {
			FBOs[i].begin();
			ofClear(0, 255);
			FBOs[i].end();
		}
	}

	ofFbo& operator[](int n) { return FBOs[n]; }
	ofFbo   *src;       // Source       ->  Ping
	ofFbo   *dst;       // Destination  ->  Pong

private:
	ofFbo   FBOs[2];    // Real addresses of ping/pong FBO«s
};

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		// GUI
		ofxPanel guiControl;

		ofxFloatSlider maxSpeed;
		ofxFloatSlider maxForce;
		ofxFloatSlider lifeLength;
		ofxIntSlider trail;

		ofxPanel guiNoises;

		ofxToggle gradientNoise;
		ofxToggle simplexNoise;
		ofxToggle voronoiNoise;

		ofxFloatSlider gradientNoiseScale;
		ofxFloatSlider gradientNoiseAmnt;
		ofxFloatSlider simplexNoiseScale;
		ofxFloatSlider simplexNoiseAmnt;
		ofxFloatSlider voronoiNoiseScale;
		ofxFloatSlider voronoiNoiseAmnt;
		
		ofxFloatSlider fbmHurst;
		ofxFloatSlider fbmFrequency;
		ofxFloatSlider fbmRotation;
		ofxFloatSlider warpRotation;
		
		ofxFloatSlider channelOffsetX;
		ofxFloatSlider channelOffsetY;
		
		ofxFloatSlider contrast;
		ofxFloatSlider noiseSeed;

		ofxIntSlider fbmOctaves;
		ofxIntSlider fbmWarp;
		
		// FBOs
		ofFbo renderFbo;
		ofFbo fbmFbo;

		// PingPong FBOs
		pingPongBuffer posPingPong;
		pingPongBuffer velPingPong;
		pingPongBuffer lifePingPong;

		// Shaders
		ofShader render;
		ofShader positionUpdate;
		ofShader velocityUpdate;
		ofShader lifeUpdate;

		ofShader fbmShader;

		// Meshes
		ofVboMesh mesh;

		// Geometry
		ofPlanePrimitive fbmPlane;

		// Various
		int w, h;
		int numParticles;
		int textureRes;
		int noiseSize;

		ofVec2f size;
		ofVec2f center;
		ofVec2f noiseOffset;
};
