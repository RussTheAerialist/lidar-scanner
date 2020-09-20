#include "ofApp.h"
#include "ofSerial.h"
#include <math.h>

#define MAX_DISTANCE 8000
#define MIN_DISTANCE 10
#define MAX_THRESHOLD 1000
#define RECV_PORT 4560
#define SEND_PORT 4561
#define SEND_HOST "localhost"

//--------------------------------------------------------------
void ofApp::setup(){
	debug_view = false;
	ofSetLogLevel(OF_LOG_VERBOSE);
	lidar = ofx::rplidar::ILidar::create("/dev/ttyUSB0");
	if (lidar == nullptr) {
		ofLog(OF_LOG_ERROR) << "Unable to open lidar device, exiting";
		ofExit();
	}

	memset(buffer, 0, sizeof(float) * BINCOUNT * 2);
	memset(change_mask, 0, sizeof(bool) * BINCOUNT);
	memset(note_state, 0, sizeof(bool) * BINCOUNT);

	ofxGuiEnableHiResDisplay();
	gui.setup("Controls");
	gui.add(bottom_distance.set("Bottom", 100, MIN_DISTANCE, MAX_DISTANCE));
	gui.add(top_distance.set("Top", MAX_DISTANCE, MIN_DISTANCE, MAX_DISTANCE));
	gui.add(change_threshold.set("Change Threshold", 10.0, 0.0, MAX_THRESHOLD));
	gui.add(note_threshold.set("Note Threshold", 0.5, 0.0, 1.0));
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
		if (fabs(buffer[0][i] - buffer[1][i]) > change_threshold) {
			change_mask[i] = true;
			notify(i, buffer[which_buffer ? 1 : 0][i]);
		} else {
			change_mask[i] = false;
		}
	}
}

void ofApp::notify(int deg, float value) {
	float velocity = ofMap(value, bottom_distance, top_distance, 1.0, 0.0);
	if (velocity > note_threshold) { // Is Note On?
		if (note_state[deg]) // Note was already on
			notify_move(deg, velocity);
		else {
			note_state[deg] = true;
			notify_on(deg, velocity);
		}
	} else if (note_state[deg]) {
		note_state[deg] = false;
		notify_off(deg);
	}
}

void ofApp::notify_off(int deg) {
	ofxOscMessage msg;
	msg.setAddress("/lidar/off");
	msg.addIntArg(deg);

	osc_send.sendMessage(msg);
}

void ofApp::notify_on(int deg, float value) {
	ofxOscMessage msg;
	msg.setAddress("/lidar/on");
	msg.addIntArg(deg);
	msg.addFloatArg(value);

	osc_send.sendMessage(msg);
}

void ofApp::notify_move(int deg, float value) {
	ofxOscMessage msg;
	msg.setAddress("/lidar/move");
	msg.addIntArg(deg);
	msg.addFloatArg(value);

	osc_send.sendMessage(msg);
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
		ofSetColor(note_state[i] ? ofColor::purple : ofColor::gray);
		ofDrawCircle(0, ofMap(buffer[which_buffer ? 1 : 0][i], bottom_distance, top_distance, 10.0, 400.0), 5);

		ofRotateRad(angle);


	}
	ofPopMatrix();

	gui.draw();
}

void ofApp::draw_normal(int i, int radius, float angle, float x, float y) {
		ofColor fill_color(ofMap(buffer[which_buffer ? 1 : 0][i], bottom_distance, top_distance, 255.0, 0.0));
		ofSetColor(fill_color);

}

void ofApp::draw_debug(int i, int radius, float angle, float x, float y) {
	float value = ofMap(buffer[which_buffer ? 1 : 0][i], bottom_distance, top_distance, 255.0, 0.0);
	ofSetColor(change_mask[i] ? ofColor(value, 0, 0) : ofColor(0,0,0,0));
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
	if (key == ' ') {
		if (lidar->scanning()) {
			lidar->stopScan();
		} else {
			lidar->startScan();
		}
	}
}

void ofApp::newFrame(const ofx::rplidar::Measurement &data) {

	// TODO: Need to lock when a new frame is coming in and lock on the update and draw cycles

	// Clear current buffer
	memset(buffer[which_buffer ? 0 : 1], 0, sizeof(float) * BINCOUNT);

	// Accumulate Sums
	for(size_t i=0; i<data.count; i++) {
		 // TODO: Move this calculation and the other into the library
		int angle = ofWrap(ceil(data.data[i].angle_z_q14 * 90.f / (1 << 14)), 0, 359);
		float radius = data.data[i].dist_mm_q2 / 4.0f;
		int binned_angle = (int)(BINCOUNT*((float)angle / (float)360));

		buffer[which_buffer ? 0 : 1][binned_angle] += radius;;
		// NOTE: Equal Weight of Current Value and Previous Value might need to be reevaluated at some point in the future
	}

	// Average Buffer with previous value
	for(size_t i=0; i<BINCOUNT; i++) {
		float previous_value = buffer[which_buffer ? 0 : 1][i]; // TODO: MACRO buffer index calculation
		float current_value = buffer[which_buffer ? 1 : 0][i];

		// TODO: Need to calculate 10+1 based on BINCOUNT

		buffer[which_buffer ? 0 : 1][i] = ofClamp((previous_value + current_value) / 11.0, bottom_distance, top_distance);

	}


	which_buffer = !which_buffer;
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
