#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofBackground(255);
	ofSetVerticalSync(true);

	ofSetFrameRate(120);

	// Gui setup
	guiControl.setup("General controls", "settings.xml");
	guiControl.setPosition(1000, 10);

	guiControl.add(maxSpeed.setup("maxSpeed", 2., .1, 5.));
	guiControl.add(maxForce.setup("maxForce", 1., .1, 10.));
	guiControl.add(trail.setup("trail", 30, 0, 255));
	guiControl.add(lifeLength.setup("life expectancy", .001, .0001, .002));

	guiNoises.setup("Noise Settings", "noiseSettings.xml");
	guiNoises.setPosition(1000, 160);

	guiNoises.add(gradientNoise.setup("gradient noise", true));
	guiNoises.add(gradientNoiseScale.setup("gradient noise scale", .5, 0., 4.));
	guiNoises.add(gradientNoiseAmnt.setup("gradient noise amount", 1., 0., 2.));
	guiNoises.add(simplexNoise.setup("simplex noise", true));
	guiNoises.add(simplexNoiseScale.setup("simplex noise scale", .5, 0., 4.));
	guiNoises.add(simplexNoiseAmnt.setup("simplex noise amount", .5, 0., 4.));
	guiNoises.add(voronoiNoise.setup("voronoi noise", true));
	guiNoises.add(voronoiNoiseScale.setup("voronoi noise scale", .5, 0., 4.));
	guiNoises.add(voronoiNoiseAmnt.setup("voronoi noise amount", 1., 0., 2.));

	guiNoises.add(channelOffsetX.setup("offset x", .0, -5., 5.));
	guiNoises.add(channelOffsetY.setup("offset y", .0, -5., 5.));

	guiNoises.add(fbmHurst.setup("FBM Hurst", .5, 0., 1.));
	guiNoises.add(fbmFrequency.setup("FBM frequency", 2., 0., 4.));
	guiNoises.add(fbmOctaves.setup("FBM Octaves", 8, 1, 12));
	guiNoises.add(fbmWarp.setup("FBM Warp", 3, 1, 4));
	guiNoises.add(fbmRotation.setup("FBM Rot", 0., 0., 2.));
	guiNoises.add(warpRotation.setup("WARP Rot", 0., 0., 2.));

	guiNoises.add(noiseSeed.setup("noise seed", 1., 1., 2.));
	guiNoises.add(contrast.setup("contrast", 1., 0., 10.));
	
	// General setup
	numParticles = 50000;
	center.set(.5, .5);

	// Width and Heigth of the fbo
	w = 1440;
	h = 1440;
	noiseSize = 500;

	size.set(w, h);

	// Load shaders
	render.load("shaderGL3/render");
	positionUpdate.load("shaderGL3/passthru.vert", "shaderGL3/positionUpdate.frag");
	velocityUpdate.load("shaderGL3/passthru.vert", "shaderGL3/velocityUpdate.frag");
	lifeUpdate.load("shaderGL3/passthru.vert", "shaderGL3/lifeUpdate.frag");
	fbmShader.load("shaderGL3/passthru.vert", "shaderGL3/fbm.frag");

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
			pos[i * 3 + 0] = ofRandomuf(); // position x
			pos[i * 3 + 1] = ofRandomuf(); // position y
			pos[i * 3 + 2] = ofRandomuf(); // position z
		}
	}

	for (int x = 0; x < textureRes; x++) {
		for (int y = 0; y < textureRes; y++) {
			int i = textureRes * y + x;
			vel[i * 3 + 0] = 0.; // initial x vel
			vel[i * 3 + 1] = 0.; // initial y vel
			vel[i * 3 + 2] = 0.; // initial z vel
		}
	}

	for (int x = 0; x < textureRes; x++) {
		for (int y = 0; y < textureRes; y++) {
			int i = textureRes * y + x;
			float l = ofRandomuf();
			life[i * 3 + 0] = l; // initial life
			life[i * 3 + 1] = l; // current life
			life[i * 3 + 2] = ofRandomuf(); // w
		}
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

	// Allocate the FBM fbo
	fbmFbo.allocate(noiseSize, noiseSize, GL_RGB32F);

	// FBM plane setup
	fbmPlane.set(noiseSize, noiseSize);
	fbmPlane.setPosition(0, 0, 0);
	fbmPlane.setResolution(2, 2);

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
	
	// Update noise
	noiseOffset.set(channelOffsetX, channelOffsetY);

	fbmFbo.begin();
	ofClear(0);

		fbmShader.begin();

		fbmShader.setUniform2f("planeSize", size);
		fbmShader.setUniform2f("noiseOffset", noiseOffset);
		fbmShader.setUniform1f("contrast", contrast);
		fbmShader.setUniform1f("seed", noiseSeed);

		fbmShader.setUniform1f("gradientNoise", gradientNoise);
		fbmShader.setUniform1f("gradientScale", gradientNoiseScale);
		fbmShader.setUniform1f("gradientAmnt", gradientNoiseAmnt);
		fbmShader.setUniform1f("simplexNoise", simplexNoise);
		fbmShader.setUniform1f("simplexScale", simplexNoiseScale);
		fbmShader.setUniform1f("simplexAmnt", simplexNoiseAmnt);
		fbmShader.setUniform1f("voronoiNoise", voronoiNoise);
		fbmShader.setUniform1f("voronoiScale", voronoiNoiseScale);
		fbmShader.setUniform1f("voronoiAmnt", voronoiNoiseAmnt);

		fbmShader.setUniform1f("fbmHurst", fbmHurst);
		fbmShader.setUniform1f("fbmFrequency", fbmFrequency);
		fbmShader.setUniform1i("fbmOctaves", fbmOctaves);
		fbmShader.setUniform1i("fbmWarp", fbmWarp);
		fbmShader.setUniform1f("fbmRotation", fbmRotation);
		fbmShader.setUniform1f("warpRotation", warpRotation);

		ofPushMatrix();
			ofTranslate(noiseSize / 2., noiseSize / 2.);

			fbmPlane.draw();

		ofPopMatrix();

		fbmShader.end();

	fbmFbo.end();
	
	//********************************************************//

	// Update life
	
	lifePingPong.dst->begin();
		ofClear(0);

		lifeUpdate.begin();
			lifeUpdate.setUniformTexture("lifeFbo", lifePingPong.src->getTexture(), 1);  // passing the position information
			lifeUpdate.setUniform1f("time", ofGetElapsedTimef());
			lifeUpdate.setUniform1f("lifeSpeed", lifeLength);
			
			// draw the source life texture to be updated
			lifePingPong.src->draw(0, 0);
		
		lifeUpdate.end();

	lifePingPong.dst->end();

	//********************************************************//

	// Update velocity

	velPingPong.dst->begin();
		ofClear(0);
		
		velocityUpdate.begin();
			velocityUpdate.setUniformTexture("positionFbo", posPingPong.src->getTexture(), 1);  // passing the position information
			velocityUpdate.setUniformTexture("velocityFbo", velPingPong.src->getTexture(), 2);   // passing the previus velocity information
			positionUpdate.setUniformTexture("lifeFbo", lifePingPong.src->getTexture(), 3);  // passing the life information
			velocityUpdate.setUniformTexture("noiseField", fbmFbo.getTexture(), 4);  // passing noise field information
			velocityUpdate.setUniform2f("center", center);
			velocityUpdate.setUniform1f("maxSpeed", maxSpeed);
			velocityUpdate.setUniform1f("maxForce", maxForce);
			velocityUpdate.setUniform1f("lifeSpeed", lifeLength);
			velocityUpdate.setUniform1f("noiseSize", noiseSize);

			// draw the source velocity texture to be updated
			velPingPong.src->draw(0, 0);

		velocityUpdate.end();

	velPingPong.dst->end();

	//********************************************************//

	// Update position

	posPingPong.dst->begin(); // Open new postion FBO
		ofClear(0); // clear FBO
		
		positionUpdate.begin(); // start shader

			positionUpdate.setUniformTexture("positionFbo", posPingPong.src->getTexture(), 1); // Pass previus position to shader
			positionUpdate.setUniformTexture("velocityFbo", velPingPong.src->getTexture(), 2);  // Pass current Velocity to shader
			positionUpdate.setUniformTexture("lifeFbo", lifePingPong.src->getTexture(), 3);  // passing the life information
			positionUpdate.setUniform1f("time", ofGetElapsedTimef());
			positionUpdate.setUniform1f("lifeSpeed", lifeLength);

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
		render.setUniformTexture("noiseField", fbmFbo.getTexture(), 3); // send the "pong" FBO to shader
		render.setUniform2f("size", size);

		ofPushStyle();
			ofEnableBlendMode(OF_BLENDMODE_ADD);
		
				ofSetColor(255);
				glPointSize(1.);
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
	
	renderFbo.draw(0, 0, w, h);
	
	lifePingPong.src->draw(10., 10., 100., 100.);
	posPingPong.src->draw(10., 120., 100., 100.);
	velPingPong.src->draw(10., 230., 100., 100.);
	fbmFbo.draw(10., 450., 100., 100.);

	// draw gui
	guiControl.draw();
	guiNoises.draw();

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
