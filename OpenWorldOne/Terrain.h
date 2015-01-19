#pragma once

#include <thread>
#include <mutex>
#include <map>


#include <vec4VBO.h>
#include <Shader.h>
#include <Camera.h>
#include <Library.h>


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

	unsigned char * terrainLinear;

	std::pair<int, int> chunkCoords;
	//the linear position equation needs to be all positive
	//abs<x,y> = norm<x,y> - <lowX, lowY>
	GLuint texture;
	vec4VBO surfaceMesh;

//Offsets are by default -1 which will offset to center i.e. -1/2 width and length translations
	//UPDATE: create through a terrainMap 
	void loadTerrain(const char* file, float unitsWide, float unitsLength, float maxDepth,
					float textureRepeats =1.0f, float xOffset = -1.0f, float zOffset = -1.0f);

	//dont call
	float getLocalHeight(float worldX, float worldZ);

	//dont call
	void draw();

private:
	
	float xOffset;
	float zOffset;
	//units in world coordinates
	float xUnitsWide;
	float zUnitsWide;
	float depth;
	//units in verticies
	int xGridUnits;
	int zGridUnits;

	
};

//Holds a grid of terrain objects to allow for very large maps to be made. 
//Handles creation, intersection, view culling, and poss in conj. with something else (probably a level-loader class) misc objects
//Varying textures is going to be hard
class TerrainMap
{
public:
	TerrainMap();
	TerrainMap(Shader* ShaderToUse);
	~TerrainMap();
	std::mutex mutexLock;

	Shader* terrainShader;//pvOnly shader, should completely transfer ownership to this class, maybe, but using pointer for now

	vector<Terrain*> chunks;
	map<int, Terrain*> mapChunks;
	void loadChunk(const char* htMap, int x, int y, resource TEXTURE = resource::DEFAULT);
	void draw(const Camera* cam);//cull consideration: are we standing on the chunk || is any corner of chunk in FoV(prob wrong)?
	float getLocalHeight(float worldX, float worldZ);

private:
	//vector<bool> chunkExists;
	//some misc immutables, should/could do these with macros. These are assuming/forcing square chunks
	float const chunkSize = 64.0f; int const chunkSizei = 64;
	float const absOffset = -32.0f; int const absOffseti = -32;
	float const absDepth = 5.0f;

	//The storage is a (possibly sparse) grid of chunks// use a map with [i] as key// dynamically update when the map grows (probably remake the map)
	//terrainLinear[(int)((x)*height + y)]
	int chunksWide = 1; int lowestX = 0; int highestX = 0;
	int chunksTall = 1; int lowestY = 0; int highestY = 0;

	int linearPosition(int x, int y);
	int linearPosition(std::pair<int, int> chunkCoordpair);

	//chunks are loaded multithreaded but the ogl context thread must do the buffering! :
	bool needsBuffering = false;
	vector<Terrain*> chunksNeedGLbuffered;
};

