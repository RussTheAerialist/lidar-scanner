#include "ofApp.h"
#include "ofSerial.h"
#include <math.h>

#define MAX_DISTANCE 8000
#define MIN_DISTANCE 10
#define MAX_THRESHOLD 1000
#define RECV_PORT 4560
#define SEND_PORT 4561
#define SEND_HOST "127.0.0.1"

//--------------------------------------------------------------
void ofApp::setup(){
	debug_view = false;
	ofSetLogLevel(OF_LOG_VERBOSE);
	lidar = ofx::rplidar::ILidar::create("/dev/ttyUSB0");
	if (lidar == nullptr) {
		ofLog(OF_LOG_ERROR) << "Unable to open lidar device, exiting";
		ofExit();
	}

	for(int i=0;i<BINCOUNT;i++) {
		buffer[i][0] = 0;
		buffer[i][1] = 0;
	}

	ofxGuiEnableHiResDisplay();
	gui.setup("Controls");
	gui.add(bottom_distance.set("Bottom", 100, MIN_DISTANCE, MAX_DISTANCE));
	gui.add(top_distance.set("Top", MAX_DISTANCE, MIN_DISTANCE, MAX_DISTANCE));
	gui.add(change_threshold.set("Trigger Threshold", 10.0, 0.0, MAX_THRESHOLD));
	gui.loadFromFile("settings.xml");

	ofAddListener(lidar->onNewFrameAvailable, this, &ofApp::newFrame);
	ofBackground(ofColor::black);

	lidar->startScan();

	osc_recv.setup(RECV_PORT);
	osc_send.setup(SEND_HOST, SEND_PORT); // TODO: Load this from a settings file
}

//--------------------------------------------------------------
void ofApp::update(){
	while(osc_recv.hasWaitingMessages()) {
		ofxOscMessage msg;
		osc_recv.getNextMessage(msg);
		if (msg.getAddress() == "/2/fader1") {
			float value = msg.getArgAsFloat(0);
			bottom_distance = ofMap(value, 0.0, 1.0, MIN_DISTANCE, MAX_DISTANCE);
		} else
		if (msg.getAddress() == "/2/fader2") {
			float value = msg.getArgAsFloat(0);
			top_distance = ofMap(value, 0.0, 1.0, MIN_DISTANCE, MAX_DISTANCE);
		} else if (msg.getAddress() == "/2/fader3")
		{
			float value = msg.getArgAsFloat(0);
			change_threshold = ofMap(value, 0.0, 1.0, 10.0, MAX_THRESHOLD);
		} else if (msg.getAddress() == "/2/2nav3")
		{
			bool released = msg.getArgAsFloat(0) < 0.5;
			if (released) gui.saveToFile("settings.xml");
		} else if (msg.getAddress() == "/6/toggle49")
		{
			debug_view = msg.getArgAsFloat(0) > 0.5;
		}

	}

	for(int i=0; i<BINCOUNT; i++) {
		if (fabs(buffer[i][0] - buffer[i][1]) > change_threshold) {
			change_mask[i] = true;
		} else {
			change_mask[i] = false;
		}
	}
}

//--------------------------------------------------------------
void ofApp::draw(){

	ofFill();
	ofPushMatrix();
	ofTranslate(ofGetWidth() / 2., ofGetHeight() / 2.);
	int radius = 400;
	float angle = ofDegToRad(360 / BINCOUNT);
	float x = radius * cos(angle);
	float y = radius * sin(angle);

	for(int i=0; i<BINCOUNT; i++) {
		if (debug_view) {
			draw_debug(i, radius, angle, x, y);
		} else {
			draw_normal(i, radius, angle, x, y);
		}
		ofBeginShape();
			ofVertex(0, 0); // Start at Center
			ofVertex(radius, 0); // Move up to the outer radius of the circle
			ofBezierVertex(0, 0, radius, 0, x, y);
			ofVertex(0, 0);
		ofEndShape(true);
		ofRotateRad(angle);
	}
	ofPopMatrix();

	gui.draw();
}

void ofApp::draw_normal(int i, int radius, float angle, float x, float y) {
		ofColor fill_color(ofMap(buffer[i][which_buffer ? 1 : 0], bottom_distance, top_distance, 255.0, 0.0));
		ofSetColor(fill_color);

}

void ofApp::draw_debug(int i, int radius, float angle, float x, float y) {
	float value = ofMap(buffer[i][which_buffer ? 1 : 0], bottom_distance, top_distance, 255.0, 0.0);
	ofSetColor(change_mask[i] ? ofColor(value, 0, 0) : ofColor::black);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
	if (lidar->scanning()) {
		lidar->stopScan();
	} else {
		lidar->startScan();
	}
}

void ofApp::newFrame(const ofx::rplidar::Measurement &data) {

	// TODO: Need to lock when a new frame is coming in and lock on the update and draw cycles

	for(size_t i=0; i<data.count; i++) {
		 // TODO: Move this calculation and the other into the library
		int angle = ofWrap(ceil(data.data[i].angle_z_q14 * 90.f / (1 << 14)), 0, 359);
		float radius = data.data[i].dist_mm_q2 / 4.0f;
		int binned_angle = (int)(BINCOUNT*((float)angle / (float)360));
		// TODO: Yikes, this doesn't work the way I want it to. I'm not actually binning here.
		// FIXME: Yup, bad design

		float previous_value = buffer[binned_angle][which_buffer ? 0 : 1]; // TODO: MACRO buffer index calculation

		buffer[binned_angle][which_buffer ? 0 : 1] = ofClamp((previous_value + radius) / 2.0, bottom_distance, top_distance);
		// NOTE: Equal Weight of Current Value and Previous Value might need to be reevaluated at some point in the future

		which_buffer = !which_buffer;
	}
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){

}
