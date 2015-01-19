#include "Terrain.h"

#define terrain(x, y) terrainLinear[(int)((x)*height + y)]


Terrain::Terrain()
{
}

Terrain::~Terrain()
{
	SOIL_free_image_data(terrainLinear);
}

//this might be off by half a texel, 0 to <127 inclusive or non inclusive?
//I think using w/(width-1) will work
//Nevermind the +1s in the triangle work out, a 128 pixelwidth makes 127 squares
void Terrain::loadTerrain(const char* file, float unitsWide, float unitsLength, float maxDepth,
	float textureRepeats, float xOffset, float zOffset){
	
	if (zOffset == -1.0f) zOffset = -unitsLength / 2.0f;
	if (xOffset == -1.0f) xOffset = -unitsWide / 2.0f;

	int width, height, channels;
	
	//Width and hieght are going to be 128x128 (or maybe 256) to allow for using a single indices buffer
	//The code technically is general enough for any rectangle
	//Might not work for non pow 2 rects dont remember
	terrainLinear = SOIL_load_image(file, &width, &height, &channels, 1);  //    1---3.4
	//array[i][k] = array[i * width + k];								   //    |  /  |
	float widthf = (float)width;  										   //    2.5---6
	float heightf = (float)height;
	
	for (int w = 0; w < width-1; w++){ //w = x = width               
		for (int h = 0; h < height-1; h++){  //h = y = height
		//someday using indices would be really nice here
			surfaceMesh.add(unitsWide * (w / (widthf-1)) + xOffset, terrain(w, h) * maxDepth / 255.0f, unitsLength * (h / (heightf-1)) + zOffset);
			surfaceMesh.texCoords.push_back(vector2(w*(textureRepeats / (widthf-1)), h*(textureRepeats / (heightf-1))));
			surfaceMesh.add(unitsWide * (w / (widthf-1)) + xOffset, terrain(w, h+1) * maxDepth / 255.0f, unitsLength * ((h+1) / (heightf-1)) + zOffset);
			surfaceMesh.texCoords.push_back(vector2(w*(textureRepeats / (widthf-1)), (h+1)*(textureRepeats / (heightf-1))));
			surfaceMesh.add(unitsWide * ((w + 1) / (widthf-1)) + xOffset, terrain(w+1, h) * maxDepth / 255.0f, unitsLength * (h / (heightf-1)) + zOffset);
			surfaceMesh.texCoords.push_back(vector2((w + 1)*(textureRepeats / (widthf-1)), h*(textureRepeats / (heightf-1))));
			surfaceMesh.add(surfaceMesh.verticies[surfaceMesh.verticies.size() - 1]);//repeating
			surfaceMesh.texCoords.push_back(surfaceMesh.texCoords[surfaceMesh.texCoords.size() - 1]);//repeating
			surfaceMesh.add(surfaceMesh.verticies[surfaceMesh.verticies.size() - 3]);//repeating
			surfaceMesh.texCoords.push_back(surfaceMesh.texCoords[surfaceMesh.texCoords.size() - 3]);//repeating
			surfaceMesh.add(unitsWide * ((w + 1) / (widthf-1)) + xOffset, terrain(w + 1, h + 1) * maxDepth / 255.0f, unitsLength * ((h + 1) / (heightf-1)) + zOffset);
			surfaceMesh.texCoords.push_back(vector2((w + 1)*(textureRepeats / (widthf-1)), (h + 1)*(textureRepeats / (heightf-1))));
		}//trying widthf to (widthf-1) // worked
	}
	//assign vars to the object, some of these will be redundant in the push to force sq chunks
	xGridUnits = width;
	zGridUnits = height;
	this->xOffset = xOffset;
	this->zOffset = zOffset;
	xUnitsWide = unitsWide;
	zUnitsWide = unitsLength;
	depth = maxDepth;

	//gen buffers
	//surfaceMesh.genVertBuffer(GL_STATIC_DRAW);//draw and draw arb have same code
	//surfaceMesh.genUVBuffer();
	//this is screwing up MTing

	//TODO:
	//normal calcs
	//Multithread this

	//delete[] terrainLinear;//may be supposed to call soil_free_image_data(terrainLinear)


}

float Terrain::getLocalHeight(float worldX, float worldZ){
	//DO the math in local terrain coordinate system // this is readable and allowes for linearPosition lookup to work!
	float terrX = worldX - this->xOffset;
	float terrZ = worldZ - this->zOffset;
	float gridUnitWidth =  this->xUnitsWide / (this->xGridUnits - 1);//this is only for square meshes so far then
	float gridX = terrX / gridUnitWidth; 
	float gridZ = terrZ / gridUnitWidth;
	int gridBaseX = (int)floorf(gridX);
	int gridBaseZ = (int)floorf(gridZ);
	float xRemainder = gridX - floorf(gridX); // remainder of 12.34 = 0.34
	float zRemainder = gridZ - floorf(gridZ);

#define height zGridUnits
	vector3 normal;//calculate normal in terrain coordinates
	//this next line needs to change for (if) non square meshes, also would have to double check creation and macroing
	//can skip A LOT of this with saving the normals beforehand

	if (zRemainder > xRemainder){//top left triangle in a mesh sq															//top left
		normal = (vector3(gridUnitWidth, terrain(gridBaseX + 1, gridBaseZ + 1) * (this->depth / 256), gridUnitWidth) + -vector3(0.0f, terrain(gridBaseX, gridBaseZ + 1) * (this->depth / 256), gridUnitWidth))
			%    (vector3(0.0f,          terrain(gridBaseX, gridBaseZ) * (this->depth / 256),         0.0f) 		 + -vector3(0.0f, terrain(gridBaseX, gridBaseZ + 1) * (this->depth / 256), gridUnitWidth));
		}
	else{ //bot right tri		V^V^ top right and bot left ^V^V															//bot right
		normal = (vector3(gridUnitWidth, terrain(gridBaseX + 1, gridBaseZ + 1) * (this->depth / 256), gridUnitWidth) + -vector3(gridUnitWidth, terrain(gridBaseX + 1, gridBaseZ) * (this->depth / 256), 0.0f))
			%    (vector3(0.0f,          terrain(gridBaseX,     gridBaseZ) * (this->depth / 256),     0.0f)			 + -vector3(gridUnitWidth, terrain(gridBaseX + 1, gridBaseZ) * (this->depth / 256), 0.0f));
	}
	normal.normalize();

	//if P0 is a point on the plane; n is the plane normal; L0 is a point on the line; L is ray normal
	// t = (P0 - L0) * n / (L * n);							P0 = point on the plane

	return ((vector3(gridBaseX*gridUnitWidth, terrain(gridBaseX, gridBaseZ) * (this->depth / 256), gridBaseZ*gridUnitWidth) + vector3(-terrX, 0.0f, -terrZ)) * normal)
		/// (vector3(0.0f, 1.0f, 0.0f) * normal); //simplified
		/ normal[1];


#undef height
}

void Terrain::draw(){
	
	
	glBindTexture(GL_TEXTURE_2D, texture);

	//Bind the verticies buffer
	glBindBuffer(GL_ARRAY_BUFFER, surfaceMesh.VERT_BUFF_ID);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	//Bind the uv coords buffer
	glBindBuffer(GL_ARRAY_BUFFER, surfaceMesh.UV_BUFF_ID);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	//glBindTexture(GL_TEXTURE_2D, IMG_ID);
	glDrawArrays(GL_TRIANGLES, 0, surfaceMesh.verticies.size());

}



////////////////////////////////////
////////////// TerrainMap ///////////////////////////////////////////////
/////////////////////////////////////////////////
TerrainMap::TerrainMap()
{
}
TerrainMap::TerrainMap(Shader* ShaderToUse)
{
	terrainShader = ShaderToUse;
}

TerrainMap::~TerrainMap()
{
	typedef std::map<int, Terrain*>::iterator it_type;
	for (it_type iterator = mapChunks.begin(); iterator != mapChunks.end(); iterator++) {
		// iterator->first = key
		// iterator->second = value
		iterator->second->surfaceMesh.cleanup();//cleans ogl stuff
		delete iterator->second;//the array of verts is deleted in this destructor
	}
}

//Load a new chink. Should ignore calls to already loaded ones
void TerrainMap::loadChunk(const char* htMap, int chunkX, int chunkY, resource TEXTURE){
	//absXY = XY - lowXY
	//Terrain** chunk = &mapChunks[linearPosition(chunkX-lowestX, chunkY-lowestY)];//this adds it if it doesnt exist // it creates a null Terrain pointer
	//^^ doesnt need to be declared yet. Can bring it closer to changes for thread changes
	//Within bounds of the grid of existing chunks
	if (chunkX <= highestX && chunkX >= lowestX && chunkY <= highestY && chunkY >= lowestY){
		int key = linearPosition(chunkX - lowestX, chunkY - lowestY);
		if (mapChunks.count(key)){
			return; //it was already loaded this was a miss-call
		}
		else{//chunk not loaded://previous linPosition call had valid parameters though
			Terrain* chunk = ( new Terrain());
			(chunk)->chunkCoords = std::make_pair(chunkX, chunkY);  //File // width // height   //depth  //TexRep // xOffset //zOffset
										//x corresponds to width and z to height/y
			chunk->loadTerrain(htMap, chunkSize, chunkSize, absDepth, 8.0f, (chunkSize*chunkX) + absOffset, (chunkSize*chunkY) + absOffset);
			chunk->texture = textureLib::fetchTexture(TEXTURE);
			
			//  !- Begins being thread unsafe -!
			mutexLock.lock();
			chunksWide = highestX - lowestX + 1;
			chunksTall = highestY - lowestY + 1;//really just for the first chunk added if the first is 0,0
			//this is all sort of assuming 0,0 is loaded first which is bad
			mapChunks[linearPosition(chunk->chunkCoords.first - lowestX, chunk->chunkCoords.second - lowestY)] = chunk;
			chunks.push_back(chunk);
			mutexLock.unlock();
			chunksNeedGLbuffered.push_back(chunk);
			// !- Thread safe again -!

			needsBuffering = true;
			return;
		}
	}//it was in bounds of the extremes

	///////
	//It was out of bounds, is def new, list may need reordering (only one of four cases doesn't need reoredered but we'll always do it it's not hard:
	Terrain* chunk = new Terrain();
	(chunk)->chunkCoords = std::make_pair(chunkX, chunkY);
	chunk->texture = textureLib::fetchTexture(TEXTURE);
	chunk->loadTerrain(htMap, chunkSize, chunkSize, absDepth, 8.0f, (chunkSize*chunkX) + absOffset, (chunkSize*chunkY) + absOffset);

		if (chunkX > highestX){
			chunksWide = chunkX - lowestX + 1;
			highestX = chunkX;
		}//grows in width to the right

		else if (chunkX < lowestX){
			chunksWide = highestX - chunkX + 1;
			lowestX = chunkX;
		}//grew left
		if (chunkY > highestY){
			chunksTall = chunkY - lowestY + 1;
			highestY = chunkY;
		}//grew up
		else if (chunkY < lowestY){
			chunksTall = highestY - chunkY + 1;
			lowestY = chunkY;
		}//grew down

			
			
			//Now the map needs remade because chunksTall has changed
			//Well use the stupid list to refill it, USING ABSCHUNKCOORDS
		  //  !- Begins being thread unsafe -!
		mutexLock.lock(); 
		chunks.push_back(chunk);
		mapChunks.clear();
		for (Terrain* chunkPointer : chunks){
		mapChunks[linearPosition(chunkPointer->chunkCoords.first-lowestX, chunkPointer->chunkCoords.second-lowestY)] = chunkPointer;//this is a map assignment syntax
		}
		chunksNeedGLbuffered.push_back(chunk);
		mutexLock.unlock();
		  // !- Thread safe again -!

		needsBuffering = true;
		return;
		//pre MT code
			/*
			//absXY = XY - lowXY
	Terrain** chunk = &mapChunks[linearPosition(chunkX-lowestX, chunkY-lowestY)];//this adds it if it doesnt exist // it creates a null Terrain pointer
	//^^ doesnt need to be declared yet. Can bring it closer to changes for thread changes
	//Within bounds of the grid of existing chunks
	if (chunkX <= highestX && chunkX >= lowestX && chunkY <= highestY && chunkY >= lowestY){
		if (*chunk){
			return; //it was already loaded this was a miss-call
		}
		else{//chunk not loaded://previous linPosition call had valid parameters though
			*chunk = ( new Terrain());
			(*chunk)->chunkCoords = std::make_pair(chunkX, chunkY);
			chunks.push_back(*chunk);  //File // width // height   //depth  //TexRep // xOffset //zOffset
										//x corresponds to width and z to height/y
			chunks.back()->loadTerrain(htMap, chunkSize, chunkSize, absDepth, 8.0f, (chunkSize*chunkX) + absOffset, (chunkSize*chunkY) + absOffset);
			chunks.back()->texture = textureLib::fetchTexture(TEXTURE);
			
			chunksWide = highestX - lowestX + 1;
			chunksTall = highestY - lowestY + 1;//really just for the first chunk added if the first is 0,0
			//this is all sort of assuming 0,0 is loaded first which is bad
			return;
		}
	}//it was in bounds of the extremes

	///////
	//It was out of bounds, is def new, list may need reordering (only one of four cases doesn't need reoredered but we'll always do it it's not hard:
	*chunk = new Terrain();
	(*chunk)->chunkCoords = std::make_pair(chunkX, chunkY);
	chunks.push_back(*chunk);

		if (chunkX > highestX){
			chunksWide = chunkX - lowestX + 1;
			highestX = chunkX;
		}//grows in width to the right

		else if (chunkX < lowestX){
			chunksWide = highestX - chunkX + 1;
			lowestX = chunkX;
		}//grew left
		if (chunkY > highestY){
			chunksTall = chunkY - lowestY + 1;
			highestY = chunkY;
		}//grew up
		else if (chunkY < lowestY){
			chunksTall = highestY - chunkY + 1;
			lowestY = chunkY;
		}//grew down

			
			//last changes:
			chunks.back()->texture = textureLib::fetchTexture(TEXTURE);
			chunks.back()->loadTerrain(htMap, chunkSize, chunkSize, absDepth, 8.0f, (chunkSize*chunkX) + absOffset, (chunkSize*chunkY) + absOffset);
			//Now the map needs remade because chunksTall has changed
			//Well use the stupid list to refill it, USING ABSCHUNKCOORDS
			mapChunks.clear();
			for (Terrain* chunkPointer : chunks){
			mapChunks[linearPosition(chunkPointer->chunkCoords.first-lowestX, chunkPointer->chunkCoords.second-lowestY)] = chunkPointer;//this is a map assignment syntax
			}*/
}

//TODO: CULL and ELEMENT ARRAY IT UP
void TerrainMap::draw(const Camera* cam){
	//Check if all the data is buffered: (pretty inconsistent need but abs. necc.)
	for (Terrain* terrPointer : chunksNeedGLbuffered){
		terrPointer->surfaceMesh.genVertBuffer(GL_STATIC_DRAW);//draw and draw arb have same code
		terrPointer->surfaceMesh.genUVBuffer();
	}
	chunksNeedGLbuffered.clear();
	needsBuffering = false;


	glUseProgram(terrainShader->theProgram);
	glBindSampler(0, terrainShader->sampler);

	glEnableVertexAttribArray(0); // position
	glEnableVertexAttribArray(1); // uv tex coords

	glActiveTexture(GL_TEXTURE0);

	for (Terrain* eachChunk : chunks){
		eachChunk->draw();
	}

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glUseProgram(0);
}

//Working but doesnt catch out of bounds
float TerrainMap::getLocalHeight(float worldX, float worldZ){
	float terrSpaceX = worldX - absOffset;
	float terrSpaceZ = worldZ - absOffset;

	//0-63.999 -> 0 //64-127.999 -> 1 //128->191.999 -> 3
	//-64.0 -0.001 -> -1 floor should still be correct
	int xChunkCoord = (int)floorf(terrSpaceX / chunkSize);
	int zChunkCoord = (int)floorf(terrSpaceZ / chunkSize);//could precalculate this into a variable when they get updated

	//return mapChunks[linearPosition(xChunkCoord + lowestX, zChunkCoord + lowestY)]->getLocalHeight(worldX, worldZ);
	//  ^^ //turns out that offset is unnecc. b/c world coordinates are based off chunk coords in the first place ^^

	/*The super safe way
	//should wrap this in a 'bool careful' with a default param if ever seemes necc.
	int key = linearPosition(xChunkCoord - lowestX, zChunkCoord - lowestY);
	if (mapChunks.count(key))
	return mapChunks[key]->getLocalHeight(worldX, worldZ);
	else return 5.0f;
	*/

	/*The be-careful-half the lookups way*/
	return mapChunks[linearPosition(xChunkCoord - lowestX, zChunkCoord - lowestY)]->getLocalHeight(worldX, worldZ);
}

int TerrainMap::linearPosition(int x, int y){
	return (x*chunksTall) + y;
}

int TerrainMap::linearPosition(std::pair<int, int> chunkCoordpair){
	return linearPosition(chunkCoordpair.first, chunkCoordpair.second);
}
