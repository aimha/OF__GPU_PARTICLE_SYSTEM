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

uniform sampler2DRect prevPosData;  // previous position texture
uniform sampler2DRect velData;      // velocity texture

in vec2 texCoordVarying;

void main()
{
    // Get the position and velocity from the pixel color.
    vec2 pos = texture( prevPosData, texCoordVarying).xy;
    vec2 vel = texture( velData, texCoordVarying).xy;

    // Update the position.
    pos += vel * 0.0005;

    outputColor = vec4(pos.x, pos.y, 1.0, 1.0);
}
