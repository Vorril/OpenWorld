#version 330
layout(location = 0)in vec4 vert;
layout(location = 1) in vec2 vertUV;

out vec2 UV;

layout(std140) uniform GlobalPV
{
mat4 p;
mat4 v;
mat4 pv;
};

uniform mat4 pvm;

void main()
{
    gl_Position = pvm * vert;
    UV = vertUV;
}