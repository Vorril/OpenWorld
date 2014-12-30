#version 330
in vec2 UV;
in float lightPow;
in float incidence;
out vec3 fragColor;

uniform sampler2D diffuseSampler;

void main()
{
    vec3 diffuseCol = texture(diffuseSampler, UV).rgb;
    float result = max(incidence, 0.0);
    fragColor = (0.27*diffuseCol) + (diffuseCol*result*lightPow);
}