// fragment shader

#version 150

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 textureMatrix;
uniform mat4 modelViewProjectionMatrix;

in vec4 position;
in vec3 normal;
in vec4 color;
in vec2 texcoord;

out vec4 outputColor;

uniform sampler2DRect lifeFbo;      // life texture
uniform sampler2DRect positionFbo;      // position texture
uniform sampler2DRect velocityFbo;   // previous velocity texture
uniform sampler2DRect noiseField;      // noise texture

uniform vec2 center;
uniform float maxSpeed, maxForce, noiseRes;

in vec2 texCoordVarying;

void main()
{
  // Get the position and velocity from the pixel color.
  vec2 pos = texture( positionFbo, texCoordVarying).xy;
  vec2 vel = texture( velocityFbo, texCoordVarying ).xy;
  vec2 acc = texture( noiseField, vec2(pos.x * noiseRes, pos.y * noiseRes)).xy;

  float initialLife = texture( lifeFbo, texCoordVarying ).x;
  float life = texture( lifeFbo, texCoordVarying ).y;
  float w = texture( lifeFbo, texCoordVarying ).z;

  // compute steering behaviour

  vec2 desired = (acc * 2. - 1.) * maxSpeed;
  vec2 steer = desired - vel;
  float steerMag = clamp(length(steer), 0., maxForce);
  steer = steerMag * normalize(steer);

  vec2 newVel = vel + steer / (60. + w * 100.);
  
  // limit velocity
  float newVelMag = length(newVel);
  newVelMag = clamp(newVelMag, 0., maxSpeed);
  newVel = newVelMag * normalize(newVel);

  //   // Update the position.
  // pos += vel * .0005; // * .0005

  // // check if respawn
  // if ( pos.x < 0. || pos.x > 1. || pos.y < 0. || pos.y > 1. ) {
  //   newVel = vec2(.0);
  // }

  outputColor = vec4(newVel.x, newVel.y, .0, 1.);
}
