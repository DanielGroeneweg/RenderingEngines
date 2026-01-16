#version 400 core
out vec4 FragColor;
in vec3 fNor;
in vec2 uv;
uniform sampler2D text;
uniform vec3 baseColor;

void main()
{
    vec4 diffuse = texture(text, uv);
    FragColor = vec4(diffuse.rgb * baseColor.rgb, 1);
    //FragColor = vec4(uv, 0, 1);
}