#version 400 core
out vec4 FragColor;
in vec3 fPos;
in vec3 fNor;
in vec2 uv;
uniform mat4 modelMatrix;

void main()
{
   vec4 norm = vec4(fNor, 1) * modelMatrix;
   FragColor = vec4(norm);
}