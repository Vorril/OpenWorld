#version 330
in vec3 texCoord;
out vec4 fragColor;

uniform samplerCube myCubeSampler;

void main(void){
  fragColor = texture(myCubeSampler, texCoord);
}