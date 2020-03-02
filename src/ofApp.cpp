#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofBackground(0);
	
	// General setup
	numParticles = 100000;

	// Width and Heigth of the fbo
	w = ofGetWidth();
	h = ofGetHeight();

	size.set(w, h);

	// Load shaders
	render.load("shaderGL3/render");
	positionUpdate.load("shaderGL3/passthru.vert", "shaderGL3/positionUpdate.frag");
	velocityUpdate.load("shaderGL3/passthru.vert", "shaderGL3/velocityUpdate.frag");

	// Setting the textures where the information will be stored
	textureRes = (int)sqrt((float)numParticles);
	numParticles = textureRes * textureRes;

	// Making arrays of float pixels with position and velocity information
	vector<float> pos(numParticles * 3);
	vector<float> vel(numParticles * 3);

	for (int x = 0; x < textureRes; x++) {
		for (int y = 0; y < textureRes; y++) {
			int i = textureRes * y + x;
			pos[i * 3 + 0] = ofRandom(1.);
			pos[i * 3 + 1] = ofRandom(1.);
			pos[i * 3 + 2] = .0;
		}
	}

	for (int i = 0; i < numParticles; i++) {
		vel[i * 3 + 0] = ofRandom(-2.0, 2.0);
		vel[i * 3 + 1] = ofRandom(-2.0, 2.0);
		vel[i * 3 + 2] = ofRandom(.1, 1.);
	}

	// Allocate pingpong FBOs and load arrays
	posPingPong.allocate(textureRes, textureRes, GL_RGB32F);
	posPingPong.src->getTexture().loadData(pos.data(), textureRes, textureRes, GL_RGB);
	posPingPong.dst->getTexture().loadData(pos.data(), textureRes, textureRes, GL_RGB);

	velPingPong.allocate(textureRes, textureRes, GL_RGB32F);
	velPingPong.src->getTexture().loadData(vel.data(), textureRes, textureRes, GL_RGB);
	velPingPong.dst->getTexture().loadData(vel.data(), textureRes, textureRes, GL_RGB);

	// Allocate the render FBO 
	renderFbo.allocate(w, h, GL_RGB32F);
	renderFbo.begin();
		ofClear(0, 0, 0, 255); // clear fbo
	renderFbo.end();

	// Setup Mesh
	mesh.setMode(OF_PRIMITIVE_POINTS); // set mesh to points

	for (int x = 0; x < textureRes; x++) {
		for (int y = 0; y < textureRes; y++) {
			mesh.addVertex({ x, y, 0 }); // add an equal number of vertex to mesh
			mesh.addTexCoord({ x, y });	// add texture coordinates to vertex
		}
	}
}

//--------------------------------------------------------------
void ofApp::update(){

	velPingPong.dst->begin();
		ofClear(0);
		
		velocityUpdate.begin();
			velocityUpdate.setUniformTexture("backbuffer", velPingPong.src->getTexture(), 0);   // passing the previus velocity information
			velocityUpdate.setUniformTexture("posData", posPingPong.src->getTexture(), 1);  // passing the position information

			// draw the source velocity texture to be updated
			velPingPong.src->draw(0, 0);

		velocityUpdate.end();

	velPingPong.dst->end();

	velPingPong.swap();

	//********************************************************//

	// Update position computing from the vel FBO

	posPingPong.dst->begin(); // Open new postion FBO
		ofClear(0); // clear FBO
		
		positionUpdate.begin(); // start shader
			
			positionUpdate.setUniformTexture("prevPosData", posPingPong.src->getTexture(), 0); // Pass previus position to shader
			positionUpdate.setUniformTexture("velData", velPingPong.src->getTexture(), 1);  // Pass current Velocity to shader
			
			posPingPong.src->draw(0, 0); // Draw previous position FBO inside new position FBO

		positionUpdate.end(); // close shader with calculation done from the previous position FBO
	
	posPingPong.dst->end(); // Close new position FBO with new positions calculated

	posPingPong.swap(); // New position becomes previous position

	//********************************************************//

	// Draw mesh into renderFbo displaced with the computed position texture (dst)
	renderFbo.begin();
		ofClear(0, 0, 0, 0);
		
		render.begin();
		render.setUniformTexture("posTex", posPingPong.dst->getTexture(), 0); // send the "pong" FBO to shader
		velocityUpdate.setUniform2f("size", size);
			mesh.draw();
		render.end();

	renderFbo.end();
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofBackground(0);

	renderFbo.draw(0., 0., w, h);

	//posPingPong.src->draw(0., 0., 500, 500);
	//velPingPong.src->draw(0., 500 + 10., 500, 500);

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

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
