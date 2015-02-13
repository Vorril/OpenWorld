#pragma once

#include <GL/glew.h>
#include <stdio.h>
#include <utility>
#include <vec4VBO.h>
#include <tuple>

/*DESCRIPTION
Library runs roughly on the flyweight design pattern.
Models, textures, (etc), are asked for by enum reference (with possible hash implentation in future)
And the correspnding vec4VBO or GLuint correspnding to texture is returned
OR created if it did not exist

Semantics for preinitializing things are worth consideration (poss in loadscreens or at least multithreaded)
Should add unloading process in the future

POSS TODO: 
-- Add a flywieght member on demand at run time
-- use png or something
*/

static enum resource{NULL_ENUM, DEFAULT, SLIME, SPHERE, ROCK, CUBE_FANCY, DIRT_ROUGH, BOULDER, GRASS, ROCK_ARID, DIAMOND_SML};//should probly have multiple enums or something


namespace textureLib{
	typedef std::tuple<GLuint, bool, const char*> textureUnit;

	//Fetch the GLuint texture ID
	GLuint fetchTexture(resource enumCode);
	void cleanAll();


};

namespace modelLib{
	typedef std::tuple<vec4VBO*, bool, const char*> modelUnit;

	vec4VBO* fetchModel(resource enumCode);//careful not to declare it static even the cpp wont see it 
	void cleanAll();
};
