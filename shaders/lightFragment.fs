#version 400 core

out vec4 FragColor;

in vec3 fPosWorld;
in vec3 fNorWorld;
in vec2 uv;

uniform sampler2D text;

uniform vec3 ambientLightColor;
uniform float ambientLightStrength;
uniform vec3 lightPos;   // world-space light position
uniform vec3 lightColor; // rgb
uniform float lightStrength;
uniform float specularStrength;
uniform vec3 cameraPos;
uniform vec3 baseColor;

void main()
{
    vec3 normal = normalize(fNorWorld);

    vec3 direction = normalize(lightPos - fPosWorld);
    float NdotL = max(dot(normal, direction), 0.0);

    //vec4 diffuseTex = texture(text, uv);
    vec4 diffuseTex = vec4(1,1,1,1) * vec4(baseColor.rgb, 1);

    vec3 diffuse = diffuseTex.rgb * lightColor * NdotL;

    float diffuseStrength = clamp(0, 1, lightStrength - (length(lightPos - fPosWorld)));

    diffuse *= diffuseStrength;

    vec3 ambient = ambientLightColor * ambientLightStrength;

    vec3 viewDir = normalize(cameraPos - fPosWorld);
    vec3 reflectDir = reflect(-normalize(lightPos - fPosWorld), normal);

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = lightColor * specularStrength * spec;

    vec3 col = diffuse + ambient + specular;

    FragColor = vec4(col, 1);
}