#include "WorldObject.h"

//Static(s)
Shader* WorldObject::basicShader = nullptr;
matrix4* WorldObject::pvMat = nullptr;

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

//Bind a specific (not the basic) program ( and the rest of the necc. binds)
void WorldObject::bindProgram(Shader const &shaderToUse){
	glUseProgram(shaderToUse.theProgram);
	glBindSampler(0, shaderToUse.sampler);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TEX_IMG);

	glEnableVertexAttribArray(0); // position
	glEnableVertexAttribArray(1); // uv tex coords
	glEnableVertexAttribArray(2); // normal
}


void WorldObject::bindTexture(){
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TEX_IMG);
}

void WorldObject::bind(){
	glUseProgram(basicShader->theProgram);
	glBindSampler(0, basicShader->sampler);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TEX_IMG);

	glEnableVertexAttribArray(0); // position
	glEnableVertexAttribArray(1); // uv tex coords
	glEnableVertexAttribArray(2); // normal
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

void WorldObject::quickDraw(){
	this->bind();
	this->draw();
}

//draw std
void WorldObject::draw(){
	matrix4 pvm = *pvMat * this->modelMat;
	glUniformMatrix4fv(basicShader->pvmUniformLoc, 1, GL_TRUE, pvm.matrix);

	//Bind the verticies buffer
	glBindBuffer(GL_ARRAY_BUFFER, model->VERT_BUFF_ID);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	//Bind the uv coords buffer
	glBindBuffer(GL_ARRAY_BUFFER, model->UV_BUFF_ID);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	//glBindTexture(GL_TEXTURE_2D, IMG_ID);
	glDrawArrays(GL_TRIANGLES, 0, model->verticies.size());
}

void WorldObject::drawLit(){
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