#version 330
layout(location = 0)in vec4 vert;

layout(std140) uniform GlobalPV
{
mat4 p;
mat4 v;
mat4 pv;
};

void main()
{
    gl_Position = pv * vert;
}