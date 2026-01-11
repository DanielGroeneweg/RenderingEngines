#version 400 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D scene;
uniform float threshold;

void main()
{
    vec3 color = texture(scene, TexCoords).rgb;
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));

    // Smooth bloom mask
    float factor = smoothstep(threshold, threshold + 0.3, brightness);
    vec3 bloomPart = color * factor; // scale bright parts, keep dark parts

    FragColor = vec4(bloomPart, 1.0);
}