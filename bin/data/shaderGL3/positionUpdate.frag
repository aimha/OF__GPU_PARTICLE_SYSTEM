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
uniform sampler2DRect positionFbo;  // previous position texture
uniform sampler2DRect velocityFbo;      // velocity texture
uniform float time, lifeSpeed;

in vec2 texCoordVarying;

float random (vec2 st) {
  return fract(sin(dot( st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

void main()
{
  // Get the position, velocity and life from the pixel color.
  vec2 pos = texture( positionFbo, texCoordVarying).xy;
  vec2 vel = texture( velocityFbo, texCoordVarying).xy;

  float life = texture( lifeFbo, texCoordVarying).y;
  float w = texture( lifeFbo, texCoordVarying).z;

  // Update the position.
  pos += vel * .0005; // * .0005

  // update life
  life = clamp(life - lifeSpeed, 0., 1.);

  // check if respawn
  if ( life < lifeSpeed * 2. ) {
    pos.x = random(vec2(w, time) + vec2(.1));
    pos.y = random(vec2(w, time));
  }

  outputColor = vec4(pos.x, pos.y, 0., 1.);
}
