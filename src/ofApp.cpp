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

	for(int i=0; i< 360; i++) {
		present[i] = false;
	}

	ofxGuiEnableHiResDisplay();
	gui.setup("Controls");
	gui.add(threshold.set("Detection Threshold", 100, 10, 1000));

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
	for(int i=0; i< 360; i++) {
		ofSetColor(present[i] ? ofColor::red : ofColor::white);
		ofDrawCircle(0, 500, 3);
		ofRotateDeg(1);
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
	for(int i=0; i< 360; i++) {
		present[i] = false;
	}
	for(size_t i=0; i<data.count; i++) {
		int angle = ofWrap(ceil(data.data[i].angle_z_q14 * 90.f / (1 << 14)), 0, 359);
		float radius = data.data[i].dist_mm_q2 / 4.0f;
		present[angle] = radius < threshold;
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
