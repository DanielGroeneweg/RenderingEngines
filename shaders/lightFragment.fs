#version 400 core

out vec4 FragColor;

in vec3 fPosWorld;
in vec3 fNorWorld;
in vec2 uv;

uniform sampler2D text;

uniform vec3 lightPos;   // world-space light position
uniform vec3 lightColor; // rgb
uniform vec3 cameraPos;

void main()
{
    vec3 normal = normalize(fNorWorld);

    vec3 L = normalize(lightPos - fPosWorld);
    float NdotL = max(dot(normal, L), 0.0);

    //vec4 diffuseTex = texture(text, uv);
    vec4 diffuseTex = vec4(1,1,1,1);

    vec3 diffuse = diffuseTex.rgb * lightColor * NdotL;

    FragColor = vec4(diffuse, diffuseTex.a);
}