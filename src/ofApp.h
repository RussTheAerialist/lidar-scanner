#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxRPlidar.h"
#include "ofxOsc.h"

#define BINCOUNT 36

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
		void newFrame(const ofx::rplidar::Measurement& data);

	private:
		std::unique_ptr<ofx::rplidar::ILidar> lidar;
		bool which_buffer, debug_view;
		float buffer[2][BINCOUNT];
		bool change_mask[BINCOUNT];

		ofxPanel gui;
		ofxOscReceiver osc_recv;
		ofxOscSender osc_send;
		ofParameter<float> bottom_distance;
		ofParameter<float> top_distance;
		ofParameter<float> change_threshold;

		void draw_normal(int i, int radius, float angle, float x, float y);
		void draw_debug(int i, int radius, float angle, float x, float y);
};
