#pragma once
#include <vector4.h>
#include <matrix3.h>
#include <iostream> //DEBUG NOTE: REMOVE LATER
//#include <GL/glew.h>

#define degreesPerRadian 57.2974f
//						(180.0f / 3.1415f)
#define radiansPerDegree 0.0174532925f
//						(3.1415f / 180.0f)

class matrix4
{
public:

	float matrix[16];

	matrix4();
	//matrix4(matrix4& copy);

	~matrix4();

	static matrix4 makeRotateXaxis(float theta);
	static matrix4 makeRotateYaxis(float theta);
	static matrix4 makeRotateZaxis(float theta);

	static vector4 rotateByArb_XYZ(vector4 pointToRot, vector4 axisNormal, float theta);
	static vector4 rotateByArb_XZ(vector4 pointToRot, vector4 axisNormal, float theta);

	matrix4  transpose()const;

	matrix4 operator*(const matrix4& dot);
	vector4 operator*(const vector4& dot);
	vector4 rotMult(vector4 dot);

	float operator[](int i) const{ return matrix[i]; }
	float & operator[](int i){ return matrix[i]; }
	operator matrix3()const;

	void printMat(char* name){
		std::cout << name << std::endl;
		printf("%8.3lf", matrix[0]); printf("%8.3lf", matrix[1]); printf("%8.3lf", matrix[2]); printf("%8.3lf", matrix[3]); std::cout << std::endl;
		printf("%8.3lf", matrix[4]); printf("%8.3lf", matrix[5]); printf("%8.3lf", matrix[6]); printf("%8.3lf", matrix[7]); std::cout << std::endl;
		printf("%8.3lf", matrix[8]); printf("%8.3lf", matrix[9]); printf("%8.3lf", matrix[10]); printf("%8.3lf", matrix[11]); std::cout << std::endl;
		printf("%8.3lf", matrix[12]); printf("%8.3lf", matrix[13]); printf("%8.3lf", matrix[14]); printf("%8.3lf", matrix[15]); std::cout << std::endl << std::endl;
	}
};

