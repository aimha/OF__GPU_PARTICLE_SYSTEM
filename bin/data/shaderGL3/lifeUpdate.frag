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
uniform float time;

in vec2 texCoordVarying;

float random (vec2 st) {
  return fract(sin(dot( st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

void main()
{
  // Get life data
  float initialLife = texture( lifeFbo, texCoordVarying).x;
  float life = texture( lifeFbo, texCoordVarying).y;
  float w = texture( lifeFbo, texCoordVarying).z;

  // update life
  life = clamp(life - .01, 0., 1.);

  // check if restart life
  if ( life < 0.1 ) {
    life = 1.;
    initialLife = 1.;
  }
  
  outputColor = vec4(initialLife, life, w, 1.);
}