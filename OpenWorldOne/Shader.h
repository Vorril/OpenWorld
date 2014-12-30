#pragma once
#include <GL/glew.h>

#include <matrix4.h>
#include <vector>
#include <fstream>
#include <string>

using namespace std;
/*
HOW TO USE THIS:

TODO


*/

//model matrix is translations, rotations, and scaling
//view matrix translates to camera/eye viewing pos


class Shader
{

public:
	Shader();
	~Shader();

	GLuint theProgram; 
	GLuint sampler;// for texture programs
	GLuint pvmUniformLoc;//could make this more dynamic since not everything uses it

	static GLuint pv_UBO_index;//UBO unique index thing
	static GLuint pv_UBO;//location

	string readFile(const char * fileName){
		std::ifstream read;
		
		std::string holder = "";
		std::string str;

		read.open(fileName);

		if (read.is_open()){
			std::getline(read, str);
			holder += str;
			holder.append("\n");//important
		
		while (!read.eof()){
			std::getline(read, str);
			//cout << str.c_str() << endl;
			holder += str;
			}
		}
		return holder;
	}

	void InitializeProgram(const char* vertFile, const char* fragFile){
		std::vector<GLuint> shaderList;

		shaderList.push_back(CreateShader(GL_VERTEX_SHADER, readFile(vertFile)));
		shaderList.push_back(CreateShader(GL_FRAGMENT_SHADER, readFile(fragFile)));

		theProgram = glCreateProgram();
		for (GLuint shader : shaderList){
			glAttachShader(theProgram, shader);
		}
		//////////////////////////////////////////////////////
		glLinkProgram(theProgram);
		//////////////////////////////////////////////////////
		for (GLuint shader : shaderList){
			glDeleteShader(shader);
		}


	
}//program from file

	//Should make private or protected
	static GLuint CreateShader(GLenum eShaderType, const std::string &strShaderFile)
	{
		GLuint shader = glCreateShader(eShaderType);
		const char *strFileData = strShaderFile.c_str();
		glShaderSource(shader, 1, &strFileData, NULL);

		glCompileShader(shader);

		GLint status;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
		if (status == GL_FALSE)
		{
			GLint infoLogLength;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

			GLchar *strInfoLog = new GLchar[infoLogLength + 1];
			glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);

			const char *strShaderType = NULL;
			switch (eShaderType)
			{
			case GL_VERTEX_SHADER: strShaderType = "vertex"; break;
			case GL_GEOMETRY_SHADER: strShaderType = "geometry"; break;
			case GL_FRAGMENT_SHADER: strShaderType = "fragment"; break;
			}

			fprintf(stderr, "Compile failure in %s shader:\n%s\n", strShaderType, strInfoLog);
			delete[] strInfoLog;
		}

		return shader;
	}

	//create the global UBO with an existing program
	static void setGlobalPV_UBO(Shader * existingShader){
		pv_UBO_index = glGetUniformBlockIndex(existingShader->theProgram, "GlobalPV");

		glGenBuffers(1, &pv_UBO);
		glBindBuffer(GL_UNIFORM_BUFFER, pv_UBO);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 52, NULL, GL_STREAM_DRAW);
			//1-16:Persp. mat//16-32 view mat.// 32-48pers*view mat//48-51 sun direction 52 sun intensity
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glUniformBlockBinding(existingShader->theProgram, pv_UBO_index, 1);//the association, 1 is a binding point
		glBindBufferRange(GL_UNIFORM_BUFFER, 1, pv_UBO, 0, sizeof(float) * 52);//association pt 2, they look like they combine through index 1 now
	}
	
	static void updateUPO_p(matrix4 * pMat){
		matrix4 transp = pMat->transpose();
		glBindBuffer(GL_UNIFORM_BUFFER, pv_UBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float)*16, &transp[0]);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
	static void updateUPO_v(matrix4 * vMat){
		matrix4 transp = vMat->transpose();
		glBindBuffer(GL_UNIFORM_BUFFER, pv_UBO);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float) * 16, sizeof(float) * 16, &transp[0]);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
	static void updateUPO_pv(matrix4 * pvMat){
		matrix4 transp = pvMat->transpose();
		glBindBuffer(GL_UNIFORM_BUFFER, pv_UBO);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float) * 32, sizeof(float) * 16, &transp[0]); //&pvMat->matrix[0]);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	static void updateUPO_light(vector4 light){
		glBindBuffer(GL_UNIFORM_BUFFER, pv_UBO);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float) * 48, sizeof(float) * 4, &light.coords[0]); //&pvMat->matrix[0]);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
};//class


