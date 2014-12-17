#pragma once
#include <vec4VBO.h>
#include <Shader.h>

/*
Ideally this class will handle loading, collisions, and will somehow handle culling
based on structure

I can probably get away with a shader that doesn't require a model matrix and comit do actually
doing translations/manipulations once
*/

class Terrain
{
public:
	Terrain();
	~Terrain();

	static unsigned char * terrainLinear;

	vec4VBO surfaceMesh;
	GLuint texture;
	float xOffset;
	float zOffset;
	//units in world coordinates
	float xUnitsWide;
	float zUnitsWide;
	float depth;
	//units in verticies
	int xGridUnits;
	int zGridUnits;

	//Offsets are by default -1 which will offset to center i.e. -1/2 width and length translations
	void loadTerrain(const char* file, float unitsWide, float unitsLength, float maxDepth,
					float textureRepeats =1.0f, float xOffset = -1.0f, float zOffset = -1.0f);

	void draw(GLuint * shaderProgram);

	float getLocalHeight(float worldX, float worldZ);
};

