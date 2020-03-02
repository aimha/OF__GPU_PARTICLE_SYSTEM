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

uniform sampler2DRect posTex;
uniform sampler2DRect velTex;
uniform vec2 size;
uniform float maxSpeed;

in vec2 texCoordVarying;

void main()
{
    vec2 st = gl_FragCoord.xy / size;
    vec2 vel = texture(velTex, texCoordVarying).xy;
    float w = texture(velTex, texCoordVarying).z;
    float life = texture( posTex, texCoordVarying).z;

    outputColor = vec4(.25, .0, 1., life / 400.);
}
