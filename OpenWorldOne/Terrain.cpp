#include "Terrain.h"

#define terrain(x, y) terrainLinear[(int)((x)*width + y)]

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
	
	surfaceMesh.genVertBuffer(GL_STATIC_DRAW);//draw and draw arb have same code
	surfaceMesh.genUVBuffer();



	//delete[] terrainLinear;//may be supposed to call soil_free_image_data(terrainLinear)
	SOIL_free_image_data(terrainLinear);

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
