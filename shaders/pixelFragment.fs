#version 400 core
out vec4 FragColor;
in vec3 fNor;
in vec2 uv;
uniform sampler2D text;
uniform int pixelate;
uniform int pixelSize;
uniform vec2 screenSize;

void main() {
    if (pixelate == 1) {
    vec2 pixelCoord = uv * screenSize;

    pixelCoord = floor(pixelCoord / pixelSize) * pixelSize;

    vec2 snappedUV = pixelCoord / screenSize;

    FragColor = texture(text, snappedUV);
    }

    else {
    FragColor = texture(text, uv);
    }
}