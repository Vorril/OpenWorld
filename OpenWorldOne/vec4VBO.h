#pragma once
/*
A class to store and handle the data and only the data of an object
wrap inside a higher order object
Use:
Fill the buffers by passing this object to modelLoader.h then call the genFunctions
draw functions are mostly being phased out of this class

genTex includes genUV

*/
#include <matrix4.h> //includes vec4.h and math.h
#include <vector3.h>
#include <vector2.h>
#include <vector>


#include <Shader.h>//glew
#include <GL\SOIL.h>//can remove if vbo generation is entirely abstracted I think I might still be used directly 

#include <iostream>//temporary

using std::vector;

class vec4VBO
{
public:
	//Members/////////////////////////////////////////////////////
	vector<vector4> verticies;
	vector<vector3> normals;
	vector<vector2> texCoords;

	GLuint VAO_ID;//curr unused

	GLuint VERT_BUFF_ID;
	GLuint UV_BUFF_ID; 
	GLuint NORM_BUFF_ID;

	GLuint TEX_IMG;// can shift this responsibility to the object class


	//Construction and cleanup/////////////////////////////////////////////////////
	vec4VBO();
	~vec4VBO();
	void cleanup();

	//OGL buffering methods/////////////////////////////////////////////////////
	void genVertBuffer(unsigned int hint = GL_DYNAMIC_DRAW);
	void genUVBuffer();
	void genNormBuffer();
	void refreshVBOdata();

	//Drawing methods/////////////////////////////////////////////////////
	void drawGL(Shader* shader); // can shift this responsibility to the object class
	void drawGLTex(Shader* shader);
	
	
	void add(float xAdd, float yAdd, float zAdd);
	void add(vector4 vecAdd);
	void add(vector3 vecAdd);

	//Manipulator methods, generally will be replaced by model matrix implementation//////////////////////

	/*
	void translate(float xTrans, float yTrans, float zTrans);
	void translate(vector4 vecTrans);

	void rotateXaxis(float theta);
	void rotateYaxis(float theta);
	void rotateZaxis(float theta);
	*/

	vector4 & operator[](int i);

};

