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
uniform float maxSpeed, maxForce, lifeSpeed, noiseSize;

in vec2 texCoordVarying;

void main()
{
  // Get the position and velocity from the pixel color.
  vec2 pos = texture( positionFbo, texCoordVarying).xy;
  vec2 vel = texture( velocityFbo, texCoordVarying ).xy;

  float life = texture( lifeFbo, texCoordVarying ).y;
  float w = texture( lifeFbo, texCoordVarying ).z;

  vec2 acc = texture( noiseField, vec2(pos.x * noiseSize, pos.y * noiseSize)).xy;

  // compute steering

  // !!! for legacy purpouse only !!! //
  // acc.x = acc.x * 2. - 1.; // from [0, 1] to [-1, 1]
  // acc.y = acc.y * 2. - 1.; // from [0, 1] to [-1, 1]

  // vec2 desired = normalize(acc) * maxSpeed;

  // vec2 steer = desired - vel;
  // steer = maxForce * normalize(steer);

  // vec2 newVel = vel + steer / (1. + 100. * w);

  vec2 newVel = vel + acc / (1. + 100. * w) * maxForce;

  float newVelMag = length(newVel);
  vec2 newVelDir = normalize(newVel);

  newVel = clamp(newVelMag, 0., maxSpeed) * newVelDir;

  // update life
  life = clamp(life - lifeSpeed, 0., 1.);

  // check if respawn
  if ( life < lifeSpeed * 2. ) {
    newVel = vec2(.0);
  }

  outputColor = vec4(newVel.x, newVel.y, .0, 1.);
}

