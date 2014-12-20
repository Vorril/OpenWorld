#include "Terrain.h"

#define terrain(x, y) terrainLinear[(int)((x)*height + y)]

unsigned char * Terrain::terrainLinear = nullptr;

Terrain::Terrain()
{
}


Terrain::~Terrain()
{
}

//this might be off by half a texel, 0 to <127 inclusive or non inclusive?
//I think using w/(width-1) will work
//Nevermind the +1s in the triangle work out, a 128 pixelwidth makes 127 squares
void Terrain::loadTerrain(const char* file, float unitsWide, float unitsLength, float maxDepth,
	float textureRepeats, float xOffset, float zOffset){
	
	if (zOffset == -1.0f) zOffset = -unitsLength / 2.0f;
	if (xOffset == -1.0f) xOffset = -unitsWide / 2.0f;

	int width, height, channels;
	terrainLinear = SOIL_load_image(file, &width, &height, &channels, 1);  //    1---3.4
	//array[i][k] = array[i * width + k];                                  //    |  /  |
	for (float w = 0.0f; w < width-1; w++){ //w = x = width                  //    2.5---6
		for (float h = 0.0f; h < height-1; h++){  //h = y = height
		//someday using indices would be reall ynice here
			surfaceMesh.add(unitsWide * (w / width) + xOffset, terrain(w, h) * maxDepth / 255.0f, unitsLength * (h / height) + zOffset);
			surfaceMesh.texCoords.push_back(vector2(w*(textureRepeats / width), h*(textureRepeats / height)));
			surfaceMesh.add(unitsWide * (w / width) + xOffset, terrain(w, h+1) * maxDepth / 255.0f, unitsLength * ((h+1) / height) + zOffset);
			surfaceMesh.texCoords.push_back(vector2(w*(textureRepeats / width), (h+1)*(textureRepeats / height)));
			surfaceMesh.add(unitsWide * ((w + 1) / width) + xOffset, terrain(w+1, h) * maxDepth / 255.0f, unitsLength * (h / height) + zOffset);
			surfaceMesh.texCoords.push_back(vector2((w + 1)*(textureRepeats / width), h*(textureRepeats / height)));
			surfaceMesh.add(surfaceMesh.verticies[surfaceMesh.verticies.size() - 1]);//repeating
			surfaceMesh.texCoords.push_back(surfaceMesh.texCoords[surfaceMesh.texCoords.size() - 1]);//repeating
			surfaceMesh.add(surfaceMesh.verticies[surfaceMesh.verticies.size() - 3]);//repeating
			surfaceMesh.texCoords.push_back(surfaceMesh.texCoords[surfaceMesh.texCoords.size() - 3]);//repeating
			surfaceMesh.add(unitsWide * ((w + 1) / width) + xOffset, terrain(w + 1, h + 1) * maxDepth / 255.0f, unitsLength * ((h + 1) / height) + zOffset);
			surfaceMesh.texCoords.push_back(vector2((w + 1)*(textureRepeats / width), (h + 1)*(textureRepeats / height)));
		}
	}
	//assign vars to the object
	xGridUnits = width;
	zGridUnits = height;
	this->xOffset = xOffset;
	this->zOffset = zOffset;
	xUnitsWide = unitsWide;
	zUnitsWide = unitsLength;
	depth = maxDepth;

	//gen buffers
	surfaceMesh.genVertBuffer(GL_STATIC_DRAW);//draw and draw arb have same code
	surfaceMesh.genUVBuffer();

	//TODO
	//normal calcs

	//delete[] terrainLinear;//may be supposed to call soil_free_image_data(terrainLinear)
//	SOIL_free_image_data(terrainLinear);

}

float Terrain::getLocalHeight(float worldX, float worldZ){
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
	//this next line needs to change for non square meshes, also would have to double check creation and macroing
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


void Terrain::draw(GLuint * shaderProgram){
	glUseProgram(*shaderProgram);
	glEnableVertexAttribArray(0); // position
	glEnableVertexAttribArray(1); // uv tex coords

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	//Bind the verticies buffer
	glBindBuffer(GL_ARRAY_BUFFER, surfaceMesh.VERT_BUFF_ID);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	//Bind the uv coords buffer
	glBindBuffer(GL_ARRAY_BUFFER, surfaceMesh.UV_BUFF_ID);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	//glBindTexture(GL_TEXTURE_2D, IMG_ID);
	glDrawArrays(GL_TRIANGLES, 0, surfaceMesh.verticies.size());

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glUseProgram(0);
}
