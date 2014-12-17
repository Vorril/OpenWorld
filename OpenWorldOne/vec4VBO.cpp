#include "vec4VBO.h"
using namespace std;


////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////CONSTRUCTORS////////////////
//////////////////////////////////////////////////////////////////
vec4VBO::vec4VBO()
{
}


vec4VBO::~vec4VBO()
{
}

void vec4VBO::cleanup(){
	glDeleteBuffers(1, &VERT_BUFF_ID);
	glDeleteBuffers(1, &UV_BUFF_ID);//gets loaded at smae time even if unused, that might not be alright
	glDeleteBuffers(1, &NORM_BUFF_ID);
	
	verticies.clear(); verticies.shrink_to_fit();//warning on shrinktofit not guaranteed
	normals.clear(); normals.shrink_to_fit();
	texCoords.clear(); texCoords.shrink_to_fit();

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////// OPEN_GL BUFFERING METHODS ///////////////
//////////////////////////////////////////////////////////////////

//Call after filling
//upload the vertex data to the GPU
void vec4VBO::genVertBuffer(unsigned int hint){
	glGenBuffers(1, &VERT_BUFF_ID); // most important 
	glBindBuffer(GL_ARRAY_BUFFER, VERT_BUFF_ID);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(float) * verticies.size(),
		verticies.data(),
		hint);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

//call this when the vbo has grown in length and refresh when it has not
void vec4VBO::regenVertBuffer(unsigned int hint){
	glBindBuffer(GL_ARRAY_BUFFER, VERT_BUFF_ID);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(float)* verticies.size(),
		verticies.data(),
		hint);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
//Call when updating, could be private
//Only updates verts
void vec4VBO::refreshVBOdata(){
	glBindBuffer(GL_ARRAY_BUFFER, VERT_BUFF_ID);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * sizeof(float) * verticies.size(),
		verticies.data());
	glBindBuffer(GL_ARRAY_BUFFER, 0);
};

//Fill first, uptades norms to the GPU
void vec4VBO::genNormBuffer(){
	glGenBuffers(1, &NORM_BUFF_ID);
	glBindBuffer(GL_ARRAY_BUFFER, NORM_BUFF_ID);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) * normals.size(),
		normals.data(),
		GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void vec4VBO::genUVBuffer(){
	glGenBuffers(1, &UV_BUFF_ID); // most important 
	glBindBuffer(GL_ARRAY_BUFFER, UV_BUFF_ID);
	glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(float) * texCoords.size(),
		texCoords.data(),
		GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}




////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////METHODS & OPERATIONS//////////////
//////////////////////////////////////////////////////////////////
void vec4VBO::add(vector4 vecAdd){
	this->verticies.push_back(vecAdd);
}
void vec4VBO::add(float xAdd, float yAdd, float zAdd){
	this->add(vector4(xAdd, yAdd, zAdd));
}

void vec4VBO::add(vector3 vecAdd){
	add(vector4(vecAdd[0], vecAdd[1], vecAdd[2]));
}
/*
void vec4VBO::translate(vector4 vecTrans){
	for (vector4 &VBOedVec : this->verticies)
		VBOedVec += vecTrans;
}
void vec4VBO::translate(float xTrans, float yTrans, float zTrans){
	this->translate(vector4(xTrans, yTrans, zTrans));
}


//Rotates about the x axis theta degrees
void vec4VBO::rotateXaxis(float theta){
	matrix4 rotMat = matrix4::makeRotateXaxis(theta);
	for (vector4 &VBOedVec : this->verticies)
		VBOedVec = rotMat .rotMult(VBOedVec);
}
//Rotates about the y axis theta degrees
void vec4VBO::rotateYaxis(float theta){
	matrix4 rotMat = matrix4::makeRotateYaxis(theta);
	for (vector4 &VBOedVec : this->verticies)
		VBOedVec = rotMat .rotMult(VBOedVec);
}
//Rotates about the z axis theta degrees
void vec4VBO::rotateZaxis(float theta){
	matrix4 rotMat = matrix4::makeRotateZaxis(theta);
	for (vector4 &VBOedVec : this->verticies)
		VBOedVec = rotMat .rotMult(VBOedVec);
}

*/


vector4 &vec4VBO::operator[](int i){
	return this->verticies[i];
}