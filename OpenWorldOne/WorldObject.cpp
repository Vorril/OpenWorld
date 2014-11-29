#include "WorldObject.h"


WorldObject::WorldObject()
{
}


WorldObject::~WorldObject()
{
}

WorldObject::WorldObject(resource enumCode){
	model = modelLib::fetchModel(enumCode);
	modelMat[7] = 5.5f;//testing
	model->TEX_IMG = textureLib::fetchTexture(enumCode);
}

//draw with given shader
void WorldObject::draw(GLuint *program){
	glUseProgram(*program);
}
//draw maintaining GL state bindings
void WorldObject::drawBound(){
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, model->TEX_IMG);

	//Bind the verticies buffer
	glBindBuffer(GL_ARRAY_BUFFER, model->VERT_BUFF_ID);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	//Bind the uv coords buffer
	glBindBuffer(GL_ARRAY_BUFFER, model->UV_BUFF_ID);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	//glBindTexture(GL_TEXTURE_2D, IMG_ID);
	glDrawArrays(GL_TRIANGLES, 0, model->verticies.size());
}
