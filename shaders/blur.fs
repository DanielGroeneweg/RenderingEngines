#version 400 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D image;
uniform bool horizontal;
uniform float blurRadius; // new: multiplier for distance

const float weights[9] = float[](0.05, 0.09, 0.12, 0.15, 0.16, 0.15, 0.12, 0.09, 0.05);

void main()
{
    vec2 texOffset = 1.0 / textureSize(image, 0); // 1 pixel
    vec3 result = texture(image, TexCoords).rgb * weights[0];

    for (int i = -4; i <= 4; i++)
    {
        vec2 offset = horizontal
            ? vec2(texOffset.x * i * blurRadius, 0.0)
            : vec2(0.0, texOffset.y * i * blurRadius);

        result += texture(image, TexCoords + offset).rgb * weights[i + 4];
    }

    FragColor = vec4(result, 1.0);
}