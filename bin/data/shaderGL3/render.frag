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
uniform sampler2DRect noiseField;      // life texture
uniform vec2 size;

in vec2 texCoordVarying;

#define PI 3.14159265359 

void main()
{
    vec2 st = gl_FragCoord.xy / size;

    float initialLife = texture( lifeFbo, texCoordVarying).x;
    float life = texture( lifeFbo, texCoordVarying).y;
    float w = texture( lifeFbo, texCoordVarying).z;

    vec3 n = texture( noiseField, texCoordVarying).xyz;

    outputColor = vec4(1., 1., 1., sin(life / initialLife * PI) * w);
}
