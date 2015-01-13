#pragma once

#include <GL\glew.h>;
#include <matrix4.h>
#include <vec4VBO.h> //not using but has many includes


class Skybox
{
public:
	Skybox();
	Skybox(std::string SBdir);
	~Skybox();
	static void cleanAll();


	static GLfloat* cube_vertices;
	static GLushort* cube_indicies;
	static GLuint cube_VBO; 
	static GLuint cube_IBO;

	static Shader cubemapShader;
	//create the cube VBO and the cubemap sampler/shader
	static void skyboxInit();

	GLuint cubeMapTex;

	//draw it using the requisite shader. Should clear teh depth buffer afterwards
	void drawSB(vector3 cameraPos);

private:
	static std::string rootSBdir;
};

