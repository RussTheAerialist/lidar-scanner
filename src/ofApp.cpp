#include "ofApp.h"
#include "ofSerial.h"
#include <math.h>

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetLogLevel(OF_LOG_VERBOSE);
	lidar = ofx::rplidar::ILidar::create("/dev/ttyUSB0");
	if (lidar == nullptr) {
		ofLog(OF_LOG_ERROR) << "Unable to open lidar device, exiting";
		ofExit();
	}

	for(int i=0;i<32;i++) { // TODO: Move BinCount to a #define
		buffer[i][0] = 0;
		buffer[i][1] = 0;
	}

	ofxGuiEnableHiResDisplay();
	gui.setup("Controls");
	gui.add(bottom_distance.set("Bottom", 100, 10, 8000));
	gui.add(top_distance.set("Top", 8000, 10, 8000));

	ofAddListener(lidar->onNewFrameAvailable, this, &ofApp::newFrame);
	ofBackground(ofColor::black);

	lidar->startScan();
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){

	ofFill();
	ofPushMatrix();
	ofTranslate(ofGetWidth() / 2., ofGetHeight() / 2.);
	int radius = 400;
	float angle = ofDegToRad(360 / 32);
	float x = radius * cos(angle);
	float y = radius * sin(angle);

	for(int i=0; i< 32; i++) {
		ofSetColor(ofColor(ofMap(buffer[i][which_buffer ? 1 : 0], bottom_distance, top_distance, 255.0, 0.0)));
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
	for(size_t i=0; i<data.count; i++) {
		 // TODO: Move this calculation and the other into the library
		int angle = ofWrap(ceil(data.data[i].angle_z_q14 * 90.f / (1 << 14)), 0, 359);
		float radius = data.data[i].dist_mm_q2 / 4.0f;
		int binned_angle = (int)(32*((float)angle / (float)360));

		float previous_value = buffer[binned_angle][which_buffer ? 0 : 1]; // TODO: MACRO buffer index calculation

		buffer[binned_angle][which_buffer ? 0 : 1] = ofClamp((previous_value + 2.0 * radius) / 3.0, bottom_distance, top_distance);
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
