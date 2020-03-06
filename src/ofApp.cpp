#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	GLint maxTexSize;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
	cout << "Max texture size : " << maxTexSize << endl;

	ofBackground(0);
	ofSetVerticalSync(true);

	ofSetFrameRate(120);

	// Gui setup
	guiControl.setup("General controls", "settings.xml");
	guiControl.setPosition(50, 50);

	guiControl.add(maxSpeed.setup("maxSpeed", 5., .1, 10.));
	guiControl.add(maxForce.setup("maxForce", 2., .1, 2.));
	guiControl.add(trail.setup("trail", 255, 0, 255));
	
	// General setup
	numParticles = 40;
	center.set(.5, .5);

	// Width and Heigth of the fbo
	w = 1440;
	h = 1440;

	size.set(w, h);

	// Load shaders
	render.load("shaderGL3/render");
	positionUpdate.load("shaderGL3/passthru.vert", "shaderGL3/positionUpdate.frag");
	velocityUpdate.load("shaderGL3/passthru.vert", "shaderGL3/velocityUpdate.frag");
	lifeUpdate.load("shaderGL3/passthru.vert", "shaderGL3/lifeUpdate.frag");

	// Setting the textures where the information will be stored
	textureRes = (int)sqrt((float)numParticles);
	numParticles = textureRes * textureRes;

	// Making arrays of float pixels with position and velocity information
	vector<float> pos(numParticles * 3);
	vector<float> vel(numParticles * 3);
	vector<float> life(numParticles * 3);

	for (int x = 0; x < textureRes; x++) {
		for (int y = 0; y < textureRes; y++) {
			int i = textureRes * y + x;
			pos[i * 3 + 0] = ofRandomf(); // position x
			pos[i * 3 + 1] = ofRandomf(); // position y
			pos[i * 3 + 2] = ofRandomf(); // position z
		}
	}

	for (int i = 0; i < numParticles; i++) {
		vel[i * 3 + 0] = 0.; // initial x vel
		vel[i * 3 + 1] = 0.; // initial y vel
		vel[i * 3 + 2] = 0.; // initial z vel
	}

	for (int i = 0; i < textureRes; i++) {
		float l = 1.;
		life[i * 3 + 0] = l; // initial life
		life[i * 3 + 1] = l; // current life
		life[i * 3 + 2] = ofRandomf(); // w
	}

	// Allocate pingpong FBOs and load arrays
	posPingPong.allocate(textureRes, textureRes, GL_RGB32F);
	posPingPong.src->getTexture().loadData(pos.data(), textureRes, textureRes, GL_RGB);
	posPingPong.dst->getTexture().loadData(pos.data(), textureRes, textureRes, GL_RGB);

	velPingPong.allocate(textureRes, textureRes, GL_RGB32F);
	velPingPong.src->getTexture().loadData(vel.data(), textureRes, textureRes, GL_RGB);
	velPingPong.dst->getTexture().loadData(vel.data(), textureRes, textureRes, GL_RGB);

	lifePingPong.allocate(textureRes, textureRes, GL_RGB32F);
	lifePingPong.src->getTexture().loadData(life.data(), textureRes, textureRes, GL_RGB);
	lifePingPong.dst->getTexture().loadData(life.data(), textureRes, textureRes, GL_RGB);

	// Allocate the render FBO 
	renderFbo.allocate(w, h, GL_RGBA32F);
	renderFbo.begin();
		ofClear(0, 0, 0, 255); // clear fbo
	renderFbo.end();

	// Making noise FBO
	noiseRes = 3.;

	vector<float> noise(noiseRes * noiseRes * 3);

	for (int x = 0; x < noiseRes; x++) {
		for (int y = 0; y < noiseRes; y++) {
			int i = noiseRes * y + x;
			noise[i * 3 + 0] = ofRandomf();
			noise[i * 3 + 1] = ofRandomf();
			noise[i * 3 + 2] = 0.;
		}
	}

	noiseFbo.allocate(noiseRes, noiseRes, GL_RGB32F);
	noiseFbo.getTexture().loadData(noise.data(), noiseRes, noiseRes, GL_RGB);

	// Setup Mesh
	mesh.setMode(OF_PRIMITIVE_POINTS); // set mesh to points
	mesh.enableColors();

	for (int x = 0; x < textureRes; x++) {
		for (int y = 0; y < textureRes; y++) {
			mesh.addVertex({ x, y, 0 }); // add an equal number of vertex to mesh
			mesh.addTexCoord({ x, y });	// add texture coordinates to vertex
		}
	}
}

//--------------------------------------------------------------
void ofApp::update(){

	// Update life
	
	lifePingPong.dst->begin();
		ofClear(0);

		lifeUpdate.begin();
			lifeUpdate.setUniformTexture("lifeFbo", lifePingPong.src->getTexture(), 0);  // passing the position information
			lifeUpdate.setUniform1f("time", ofGetElapsedTimef());
			
			// draw the source life texture to be updated
			lifePingPong.src->draw(0, 0);
		
		lifeUpdate.end();

	lifePingPong.dst->end();

	//********************************************************//

	// Update velocity

	velPingPong.dst->begin();
		ofClear(0);
		
		velocityUpdate.begin();
			velocityUpdate.setUniformTexture("lifeFbo", lifePingPong.src->getTexture(), 0);  // passing the life information
			velocityUpdate.setUniformTexture("positionFbo", posPingPong.src->getTexture(), 1);  // passing the position information
			velocityUpdate.setUniformTexture("velocityFbo", velPingPong.src->getTexture(), 2);   // passing the previus velocity information
			velocityUpdate.setUniformTexture("noiseField", noiseFbo.getTexture(), 3);  // passing noise field information
			velocityUpdate.setUniform2f("center", center);
			velocityUpdate.setUniform1f("maxSpeed", maxSpeed);
			velocityUpdate.setUniform1f("maxForce", maxForce);
			velocityUpdate.setUniform1f("noiseRes", noiseRes);

			// draw the source velocity texture to be updated
			velPingPong.src->draw(0, 0);

		velocityUpdate.end();

	velPingPong.dst->end();

	//********************************************************//

	// Update position

	posPingPong.dst->begin(); // Open new postion FBO
		ofClear(0); // clear FBO
		
		positionUpdate.begin(); // start shader

			positionUpdate.setUniformTexture("lifeFbo", lifePingPong.src->getTexture(), 0);  // passing the life information
			positionUpdate.setUniformTexture("positionFbo", posPingPong.src->getTexture(), 1); // Pass previus position to shader
			positionUpdate.setUniformTexture("velocityFbo", velPingPong.src->getTexture(), 2);  // Pass current Velocity to shader
			positionUpdate.setUniform1f("time", ofGetElapsedTimef());

			posPingPong.src->draw(0, 0); // Draw previous position FBO inside new position FBO

		positionUpdate.end(); // close shader with calculation done from the previous position FBO
	
	posPingPong.dst->end(); // Close new position FBO with new positions calculated
	
	//********************************************************//

	// Draw mesh into renderFbo displaced with the computed position texture (dst)
	renderFbo.begin();
		//ofClear(0, 0, 0, 0);
		ofSetColor(0, 0, 0, trail);
		ofDrawRectangle(0, 0, w, h);

		render.begin();
		render.setUniformTexture("lifeFbo", lifePingPong.src->getTexture(), 0);  // passing the life information
		render.setUniformTexture("positionFbo", posPingPong.src->getTexture(), 1); // send the "pong" FBO to shader
		render.setUniform2f("size", size);

		ofPushStyle();
			ofEnableBlendMode(OF_BLENDMODE_ADD);
		
				ofSetColor(255);
				glPointSize(10.);
				mesh.draw();

			ofDisableBlendMode();
			glEnd();

			ofPopStyle();
		render.end();

	renderFbo.end();

	//********************************************************//

	// swap FBOs
	lifePingPong.swap(); // New life becomes previous life
	velPingPong.swap(); // New velocity becomes previous velocity
	posPingPong.swap(); // New position becomes previous position
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofBackground(0);
	
	renderFbo.draw(0., 0., w, h);
	
	lifePingPong.src->draw(10., 10., 100., 100.);
	posPingPong.src->draw(10., 120., 100., 100.);
	velPingPong.src->draw(10., 230., 100., 100.);
	noiseFbo.draw(10., 340., 100., 100.);

	// draw gui
	guiControl.draw();

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	vector<float> noise(noiseRes * noiseRes * 3);

	for (int x = 0; x < noiseRes; x++) {
		for (int y = 0; y < noiseRes; y++) {
			int i = noiseRes * y + x;
			noise[i * 3 + 0] = ofRandom(1.);
			noise[i * 3 + 1] = ofRandom(1.);
			noise[i * 3 + 2] = 0.;
		}
	}
	noiseFbo.getTexture().loadData(noise.data(), noiseRes, noiseRes, GL_RGB);
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
