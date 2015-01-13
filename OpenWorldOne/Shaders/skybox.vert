#version 330
layout(location = 0)in vec3 vert;
out vec3 texCoord;

layout(std140) uniform GlobalPV
{
mat4 p;
mat4 v;
mat4 pv;
vec4 l;
};

uniform vec3 m;

void main()
{
  gl_Position =  pv * vec4((50.0*vert)+m, 1.0);
  texCoord = vert;
}