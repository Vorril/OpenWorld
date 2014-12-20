#include "Library.h"

#define getFile(v) std::get<2>(v)
#define isLoaded(v) std::get<1>(v)
#define getVBO(v) std::get<0>(v)
#define getTexture(v) std::get<0>(v)

static void checkModel(modelLib::modelUnit* model);
static bool loadObj(const char* filePath, vec4VBO* loadToMe);
static GLuint checkTexture(textureLib::textureUnit *texture);

//////////////////////TextureLib//////////////////////
namespace textureLib{
std::tuple <GLuint, bool, const char*> Default = textureLib::textureUnit(0, false, "Textures/default.bmp");
std::tuple <GLuint, bool, const char*> Slime = textureLib::textureUnit(0, false, "Textures/slime.bmp");
std::tuple <GLuint, bool, const char*> Test = textureLib::textureUnit(0, false, "Textures/testTexture.bmp");//should prob use only default
std::tuple <GLuint, bool, const char*> Rock = textureLib::textureUnit(0, false, "Textures/rockRough.bmp");

	GLuint textureLib::fetchTexture(resource enumCode){
		switch (enumCode){
		case NULL_ENUM:
			return checkTexture(&Test);
		case SLIME:
			return checkTexture(&Slime);
		case ROCK:
			return checkTexture(&Rock);
		default:
			return checkTexture(&Default);
		}//switch
	}//fetch

	void textureLib::cleanAll(){
		if (isLoaded(Slime)) glDeleteTextures(1, &getTexture(Slime));
		if (isLoaded(Rock)) glDeleteTextures(1, &getTexture(Rock));
		if (isLoaded(Default)) glDeleteTextures(1, &getTexture(Default));
		if (isLoaded(Test)) glDeleteTextures(1, &getTexture(Test));
	}

}//ns

	static GLuint checkTexture(textureLib::textureUnit *texture){
		if (!isLoaded(*texture)){
			isLoaded(*texture) = true;
			getTexture(*texture) =  SOIL_load_OGL_texture(getFile(*texture),
			SOIL_LOAD_AUTO,
			SOIL_CREATE_NEW_ID,
			SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
			);
		}
		return getTexture(*texture);
	}



//////////////////////ModelLib////////////////////////
namespace modelLib{// const correctness here ?
std::tuple<vec4VBO*, bool, const char*> Default = modelLib::modelUnit(nullptr, false, "Models/default.mdl");
std::tuple<vec4VBO*, bool, const char*> Slime = modelLib::modelUnit(nullptr, false, "Models/slime.mdl");
std::tuple<vec4VBO*, bool, const char*> Sphere = modelLib::modelUnit(nullptr, false, "Models/sphere.mdl");

	vec4VBO* fetchModel(resource enumCode){
		//dont call the object class should interact with this

		switch (enumCode){
		case NULL_ENUM:
			break;
		case SLIME:
			checkModel(&Slime);
			return getVBO(Slime);
		case SPHERE:
			checkModel(&Sphere);
			return getVBO(Sphere);
		default:
			checkModel(&Default);
			return getVBO(Default);
		}

		return nullptr;
	}//fetch

	void modelLib::cleanAll(){//can probly improve 
		if (isLoaded(Slime)) getVBO(Slime)->cleanup();
		if (isLoaded(Sphere)) getVBO(Sphere)->cleanup();
		if (isLoaded(Default)) getVBO(Default)->cleanup();
	}

}//ns

//I have no idea what scope this is in:
static void checkModel(modelLib::modelUnit* model){
	if (isLoaded(*model)) return;
	else{
		isLoaded(*model) = true;
		getVBO(*model) = new vec4VBO();
		loadObj(getFile(*model), getVBO(*model));

		getVBO(*model)->genVertBuffer();
		getVBO(*model)->genUVBuffer();
		getVBO(*model)->genNormBuffer();
	}
}
static bool loadObj(const char* filePath, vec4VBO* loadToMe){
	FILE * file = fopen(filePath, "r");
	if (file == NULL){
		printf("Impossible to open the file !\n");
		return false;
	}
	std::vector< unsigned int > vertexIndices, uvIndices, normalIndices;
	std::vector< vector4 > temp_vertices;
	std::vector< vector2 > temp_uvs;
	std::vector< vector3 > temp_normals;

	while (1){

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		if (strcmp(lineHeader, "v") == 0){
			vector4 vertex;
			fscanf(file, "%f %f %f\n", &vertex.coords[0], &vertex.coords[1], &vertex.coords[2]);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0){
			vector2 uv;
			fscanf(file, "%f %f\n", &uv.coords[0], &uv.coords[1]);
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0){
			vector3 normal;
			fscanf(file, "%f %f %f\n", &normal.coords[0], &normal.coords[1], &normal.coords[2]);
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0){
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9){
				printf("File can't be read by our simple parser : ( Try exporting with other options\n");
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
	}//while
	// For each vertex of each triangle
	for (unsigned int i = 0; i < vertexIndices.size(); i++){
		unsigned int vertexIndex = vertexIndices[i];
		vector4 vertex = temp_vertices[vertexIndex - 1];
		loadToMe->verticies.push_back(vertex);

		unsigned int uvIndex = uvIndices[i];
		vector2 uvVert = temp_uvs[uvIndex - 1];
		loadToMe->texCoords.push_back(uvVert);

		unsigned int normIndex = normalIndices[i];
		vector3 normVert = temp_normals[normIndex - 1];
		loadToMe->normals.push_back(normVert);
	}

	return true;

}//loadObj



#undef getFile
#undef isLoaded
#undef getVBO
#undef getTexture