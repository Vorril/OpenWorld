#include "SlimeWO.h"

//Statics
assetImmutables SlimeWO::slimeAssets;

SlimeWO::SlimeWO()
{
	//if (loaded){ return; }// review flywieght this may be over static
//	loaded = true;
	//UtilsWO::loadObj(fileModel, this->model);
	//UtilsWO::loadTexture()
}


SlimeWO::~SlimeWO()
{
}

///////////////////////////////////////////////////////
///// Virtual functions //////////////
////////////////////////////////////////////

GLuint SlimeWO::getTexture(){
	return slimeAssets.objectTexture;

}

void SlimeWO::setShader(Shader* shaderToUse){

}

