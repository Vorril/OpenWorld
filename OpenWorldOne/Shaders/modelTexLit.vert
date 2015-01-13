#version 330
layout(location = 0)in vec4 vert;
layout(location = 1) in vec2 vertUV;
layout(location = 2) in vec3 normal;

out vec2 UV;
out float lightPow;
out float incidence;

layout(std140) uniform GlobalPV
{
mat4 p;
mat4 v;
mat4 pv;
vec4 l;
};

uniform mat4 pvm;
uniform mat4 m;

void main()
{
    gl_Position = pvm * vert;
    UV = vertUV;
    incidence = dot(l.rgb, mat3(m)*normal);
    lightPow = l.a;
}