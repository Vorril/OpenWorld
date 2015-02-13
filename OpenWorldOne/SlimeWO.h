#pragma once
#include "WorldObject.h"
class SlimeWO :
	public WorldObject
{
public:
	SlimeWO();
	~SlimeWO();

	static assetImmutables slimeAssets;

	

	GLuint getTexture();
	void setShader(Shader* shaderToUse);
};

