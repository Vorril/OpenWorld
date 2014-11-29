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
	~WorldObject();

	matrix4 modelMat;
	vec4VBO* model;
	//draw with given shader
	void draw(GLuint* program);
	//draw maintaining GL state bindings
	void drawBound();
	
};

