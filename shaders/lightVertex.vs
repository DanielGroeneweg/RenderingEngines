#version 400 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNor;
layout (location = 2) in vec2 aUv;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 fPosWorld;
out vec3 fNorWorld;
out vec2 uv;

void main()
{
    // World-space position
    vec4 worldPos = model * vec4(aPos, 1.0);
    fPosWorld = worldPos.xyz;

    // Correct world-space normal
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    fNorWorld = normalize(normalMatrix * aNor);

    // Pass UVs
    uv = aUv;

    // Final clip-space position
    gl_Position = projection * view * worldPos;
}