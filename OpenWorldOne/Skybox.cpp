#include "Skybox.h"

std::vector<Skybox*> Skybox::referenceCounts = {};
GLuint Skybox::cube_IBO = 0;
GLuint Skybox::cube_VBO = 0;
Shader Skybox::cubemapShader = Shader();
static float vboData[] = { -1.0f, 1.0f, 1.0f,
-1.0f, -1.0f, 1.0f,
1.0f, -1.0f, 1.0f,
1.0f, 1.0f, 1.0f,
-1.0f, 1.0f, -1.0f,
-1.0f, -1.0f, -1.0f,
1.0f, -1.0f, -1.0f,
1.0f, 1.0f, -1.0f, };
static GLushort iboData[] = {
	static_cast<unsigned short>(0), static_cast<unsigned short>(1), static_cast<unsigned short>(2), static_cast<unsigned short>(3),
	static_cast<unsigned short>(3), static_cast<unsigned short>(2), static_cast<unsigned short>(6), static_cast<unsigned short>(7),
	static_cast<unsigned short>(7), static_cast<unsigned short>(6), static_cast<unsigned short>(5), static_cast<unsigned short>(4),
	static_cast<unsigned short>(4), static_cast<unsigned short>(5), static_cast<unsigned short>(1), static_cast<unsigned short>(0),
	static_cast<unsigned short>(0), static_cast<unsigned short>(3), static_cast<unsigned short>(7), static_cast<unsigned short>(4),
	static_cast<unsigned short>(1), static_cast<unsigned short>(2), static_cast<unsigned short>(6), static_cast<unsigned short>(5)
};
GLfloat* Skybox::cube_vertices = vboData;
GLushort* Skybox::cube_indicies = iboData;
std::string Skybox::rootSBdir = std::string("Textures/Skybox/");

Skybox::Skybox()
{
}

Skybox::Skybox(std::string SBdir){
	cubeMapTex = SOIL_load_OGL_cubemap
		(
		(rootSBdir + SBdir + std::string("/nz.png")).c_str(),
		(rootSBdir + SBdir + std::string("/pz.png")).c_str(),
		(rootSBdir + SBdir + std::string("/py.png")).c_str(),
		(rootSBdir + SBdir + std::string("/ny.png")).c_str(),
		(rootSBdir + SBdir + std::string("/px.png")).c_str(),
		(rootSBdir + SBdir + std::string("/nx.png")).c_str(),
		/*
		"Textures/Skybox/CloudySky/nz.png",
		"Textures/Skybox/CloudySky/pz.png",
		"Textures/Skybox/CloudySky/py.png",
		"Textures/Skybox/CloudySky/ny.png",
		"Textures/Skybox/CloudySky/px.png",
		"Textures/Skybox/CloudySky/nx.png",
*/

		SOIL_LOAD_RGB,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS
		);


	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTex);
	//Def want to see if I can put htis on sampler or play around more:
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	Skybox::referenceCounts.push_back(this);
}

Skybox::~Skybox()
{
}

//could use improvement esp regaurding multiple skyboxes
void Skybox::cleanAll(){
	for (Skybox *box : Skybox::referenceCounts)
	glDeleteTextures(1, &box->cubeMapTex);
}

void Skybox::skyboxInit(){
	Skybox::cubemapShader.InitializeProgram("Shaders/skybox.vert", "Shaders/skybox.frag");
	Skybox::cubemapShader.sampler = glGetUniformLocation(Skybox::cubemapShader.theProgram, "myCubeSampler");
	Skybox::cubemapShader.mUniformLoc = glGetUniformLocation(Skybox::cubemapShader.theProgram, "m");
	
	glGenBuffers(1, &Skybox::cube_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, Skybox::cube_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*24, Skybox::cube_vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	
	glGenBuffers(1, &Skybox::cube_IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Skybox::cube_IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort)*24, Skybox::cube_indicies, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Skybox::drawSB(vector3 cameraPos){
	//glDisable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	glUseProgram(Skybox::cubemapShader.theProgram);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTex);
	glBindSampler(0, Skybox::cubemapShader.sampler);
	

	glUniform3f(Skybox::cubemapShader.mUniformLoc, cameraPos[0], cameraPos[1], cameraPos[2]);
	
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, Skybox::cube_VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Skybox::cube_IBO);

	glDrawElements(GL_QUADS, 24, GL_UNSIGNED_SHORT, 0);

	glCullFace(GL_BACK);
	//glEnable(GL_CULL_FACE);
//	glClear(GL_DEPTH_BUFFER_BIT);
}