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
uniform vec2 size;

in vec2 texCoordVarying;

void main()
{
    vec2 st = gl_FragCoord.xy / size;

    outputColor = vec4(st.x, st.y, 1., 1.);
}
