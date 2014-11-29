#pragma once
#include <math.h>
#include <vector3.h>



class matrix3
{
public:
	matrix3();
	~matrix3();

	float matrix[9];

	static matrix3 makeRotateXaxis(float theta);
	static matrix3 makeRotateYaxis(float theta);
	static matrix3 makeRotateZaxis(float theta);

	static vector3 rotateByArb_XYZ(vector3 pointToRot, vector3 axisNormal, float theta);
	static vector3 rotateByArb_XZ(vector3 pointToRot, vector3 axisNormal, float theta);


	matrix3 transpose();

	matrix3 operator*(const matrix3& dot);
	vector3 operator*(const vector3& dot);

	float operator[](int i) const{ return matrix[i]; }
	float & operator[](int i){ return matrix[i]; }
};

