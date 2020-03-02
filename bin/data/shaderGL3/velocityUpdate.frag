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

uniform vec2 center;

in vec2 texCoordVarying;

void main()
{
  // Get the position and velocity from the pixel color.
  vec2 pos = texture( posData, texCoordVarying).xy;
  vec2 vel = texture( backbuffer, texCoordVarying ).xy;
  float weigth = texture( backbuffer, texCoordVarying ).z;

  float timeFrame = 0.00002;
  float limit = 1.;

  vec2 force = vec2(.5, .5) - pos;

  float dist = length(force);
  vec2 dir = normalize(force);

  float mag = (8.18 * weigth) / (dist * dist);

  vec2 acc = mag * dir / weigth;

  vel += acc * timeFrame;

  vel = clamp(vel, -limit, limit);
  
  outputColor = vec4(vel.x, vel.y, weigth, 1.0);
}
