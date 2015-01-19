#include "WorldObject.h"


WorldObject::WorldObject()
{
}


WorldObject::~WorldObject()
{
}

WorldObject::WorldObject(resource enumCode){
	model = modelLib::fetchModel(enumCode);
	modelMat[7] = 2.9f;//testing
	TEX_IMG = textureLib::fetchTexture(enumCode);

}
WorldObject::WorldObject(resource modelCode, resource texCode){
	velocity = vector3(0.0f, 0.0f, 0.0f);
	model = modelLib::fetchModel(modelCode);
	TEX_IMG = textureLib::fetchTexture(texCode);

}

vector3 WorldObject::position(){
	return vector3(modelMat[3], modelMat[7], modelMat[11]);
}

//TODO: Make this inherited or something more easily varied
void WorldObject::bindProgram(Shader const &shaderToUse){
	glUseProgram(shaderToUse.theProgram);
	glBindSampler(0, shaderToUse.sampler);
	glEnableVertexAttribArray(0); // position
	glEnableVertexAttribArray(1); // uv tex coords
	glEnableVertexAttribArray(2); // normal
}

void WorldObject::draw(GLuint *program){
	glUseProgram(*program);
}
void WorldObject::bindTexture(){
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TEX_IMG);
}
//draw maintaining GL state bindings
void WorldObject::drawBound(){
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, TEX_IMG);

	//Bind the verticies buffer
	glBindBuffer(GL_ARRAY_BUFFER, model->VERT_BUFF_ID);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	//Bind the uv coords buffer
	glBindBuffer(GL_ARRAY_BUFFER, model->UV_BUFF_ID);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	//Bind the normal buffer
	glBindBuffer(GL_ARRAY_BUFFER, model->NORM_BUFF_ID);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	//glBindTexture(GL_TEXTURE_2D, IMG_ID);
	glDrawArrays(GL_TRIANGLES, 0, model->verticies.size());
}

