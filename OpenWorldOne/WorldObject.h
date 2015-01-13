#pragma once
#include <GL\glew.h>;
#include <matrix4.h>
#include <vec4VBO.h>
#include <Library.h>

/*
This class:
Should be most anything in the world that has a model matrix and is not the terrain
Should be extended for specializations
*/


class WorldObject
{
public:
	WorldObject();
	WorldObject(resource enumCode);
	WorldObject(resource modelCode, resource texCode);
	~WorldObject();

	matrix4 modelMat;
	vec4VBO* model;
	GLuint TEX_IMG;
	vector3 velocity;

	vector3 position();


	//draw with given shader
	void draw(GLuint* program);
	//bind gl program, sampler, pointers 
	//should probably make this variable or abstract and inherited
	void bindProgram(Shader const &shaderToUse);
	//bind texture to follow with drawBound()
	void bindTexture();
	//draw maintaining GL state bindings
	void drawBound();

};

class WorldObjectLit :WorldObject{

};