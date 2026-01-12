#version 400 core
out vec4 FragColor;
in vec3 fNor;
in vec2 uv;
uniform sampler2D text;
uniform float shift;

void main() {
    vec4 color = texture2D(text, uv);

    const mat3 rgb2yiq = mat3(
        0.299,  0.587,  0.114,
        0.596, -0.275, -0.321,
        0.212, -0.523,  0.311
    );

    const mat3 yiq2rgb = mat3(
        1.0,  0.956,  0.621,
        1.0, -0.272, -0.647,
        1.0, -1.106,  1.703
    );

    vec3 yiq = rgb2yiq * color.rgb;

    float cosH = cos(shift);
    float sinH = sin(shift);

    float i = yiq.y * cosH - yiq.z * sinH;
    float q = yiq.y * sinH + yiq.z * cosH;

    vec3 shifted = yiq2rgb * vec3(yiq.x, i, q);
    gl_FragColor = vec4(shifted, color.a);
}