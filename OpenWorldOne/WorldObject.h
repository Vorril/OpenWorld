#pragma once
#include <GL\glew.h>;
#include <matrix4.h>
#include <UtilsWO.h>

/*
This class:
Should be most anything in the world that has a model matrix and is not the terrain

Basic objects like rocks etc will be just WorldObjects and will use the default methods ie a diffuse shader
Beyond that there will still be some intermediate complexity functions but things should inherit and replace as necc.

*/

//Each inheritor of WorldObjects is  an entity/asset that will share the same texture and model with instances of itself
//Hence static immutables. Each inheritor will have a static set of these for flywieght design/ easy initialization
//consider just a master map somewhere though to check things
struct assetImmutables{
	bool isLoaded = false;
	GLuint objectTexture = 0;
	vec4VBO* objectModel = nullptr;
	Shader* appropriateShader = nullptr;
};

class WorldObject
{
public:
	/////////////////////////////////////////////////////
	////// CONSTRUCTORS \\ DESTRUCTORS /////////
	////////////////////////////////////////////////////
	WorldObject();
	WorldObject(resource enumCode);
	WorldObject(resource modelCode, resource texCode);
	~WorldObject();


	/////////////////////////////////////////////////////
	////// MEMBER VARIABLES /////////
	////////////////////////////////////////////////////
	matrix4 modelMat;
	vec4VBO* model;
	GLuint TEX_IMG;
	vector3 velocity;
	vector3 position();//returns the correspond modelMatrix column

	static matrix4* pvMat;
	static Shader* basicShader;//shader to use for basic WO, simple diffuse tex
	/////////////////////////////////////////////////////
	//////MEMEBR FUNCTIONS ///////// TODO GET RID OF SOME OF THESE 
	////////////////////////////////////////////////////

	//Standard draw call. Inherited members may replace with specializations
	//Executes the draw AND uploads the modelMat in the process
	void draw();
	//Bind everything required for a draw that will be the same for objects of a single type 
	// ie bind for a batch
	void bind();
	//completely bind and draw. OK if only drawing one thing otherwise redundant bind gl calls dont do it!
	void quickDraw();
	void bindProgram(Shader const &shaderToUse);
	//bind texture to follow with drawBound()
	void bindTexture();
	//draw maintaining GL state bindings
	void drawBound();

	//TODO HERE: in init possibly with a bool classHasBeenInit, to be checked in the constructor for inherited objects

	//A get method to retrive static member texture unique to each WorldObect descendant
	//Basic WOs just use a texture from Library.h
	virtual GLuint getTexture(){ return 0; };
	//Set the static member shader a class should use
	virtual void setShader(Shader* shaderToUse){};

protected:
	void drawLit();
	void updateUniforms();
};

