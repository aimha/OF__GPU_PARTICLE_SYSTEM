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

uniform sampler2DRect backbuffer;   // previous velocity texture
uniform sampler2DRect posData;      // position texture
uniform sampler2DRect noiseField;      // noise texture

uniform vec2 center;
uniform float maxSpeed, maxForce, noiseRes;

in vec2 texCoordVarying;

void main()
{
  // Get the position and velocity from the pixel color.
  vec2 pos = texture( posData, texCoordVarying).xy;
  float life = texture( posData, texCoordVarying).z;
  vec2 vel = texture( backbuffer, texCoordVarying ).xy;
  float weigth = texture( backbuffer, texCoordVarying ).z;
  vec2 acc = texture( noiseField, vec2(pos.x * noiseRes, pos.y * noiseRes)).xy;

  // compute steering behaviour
  vec2 desired = (acc * 2. - 1.) * maxSpeed;
  vec2 steer = desired - vel;
  float steerMag = clamp(length(steer), 0., maxForce);
  steer = steerMag * normalize(steer);

  vel += steer / weigth;

  // limit velocity
  float velMag = length(vel);
  velMag = clamp(velMag, 0., maxSpeed);
  vel = velMag * normalize(vel);

  outputColor = vec4(vel.x, vel.y, weigth, 1.0);
}
