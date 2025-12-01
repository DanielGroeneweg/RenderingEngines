#version 400 core
out vec4 FragColor;
in vec3 fNor;
in vec2 uv;
uniform sampler2D text;
uniform int invertColors;

void main() {
    vec4 diffuse = texture(text, uv);
    if (invertColors == 1) {
    diffuse = vec4(vec3(1.0) - diffuse.rgb, 1.0);
    }
    FragColor = diffuse;
}