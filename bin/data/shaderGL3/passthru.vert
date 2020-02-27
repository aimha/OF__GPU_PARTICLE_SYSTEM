// vertex shader

#version 150

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 textureMatrix;
uniform mat4 modelViewProjectionMatrix;

in vec4 position;
in vec3 normal;
in vec4 color;
in vec2 texcoord;

out vec2 texCoordVarying;

void main(){

    vec4 modifiedPosition = modelViewProjectionMatrix * position;

    gl_Position = modifiedPosition;

    texCoordVarying = texcoord;
}