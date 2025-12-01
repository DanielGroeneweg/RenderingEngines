#version 400

layout (location = 0) in vec2 aPos;   // fullscreen quad XY
layout (location = 1) in vec2 aTex;   // fullscreen quad UV

out vec2 TexCoords;

void main()
{
    TexCoords = aTex;
    gl_Position = vec4(aPos, 0.0, 1.0);
}