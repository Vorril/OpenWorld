#include "matrix3.h"

#define degreesPerRadian 57.2974f
//						(180.0f / 3.1415f)
#define radiansPerDegree 0.0174532925f
//						(3.1415f / 180.0f)

matrix3::matrix3() :matrix()// ":matrix()" zeroes it
{														// 1 0 0   // 0 1 2
	matrix[0] = matrix[4] = matrix[8] = 1.0f;			// 0 1 0   // 3 4 5
}														// 0 0 1   // 6 7 8


matrix3::~matrix3()
{
}

//X:	1    0    0    		 Y:  cos   0   sin   		 z:  cos  -sin   0    
// 		0   cos -sin   			  0    1    0    		     sin   cos   0    
// 	 	0   sin  cos   			 -sin   0   cos   		      0     0    1    

matrix3 matrix3::makeRotateXaxis(float theta){
	matrix3 rotMat;
	theta *= radiansPerDegree;
	rotMat.matrix[8] = rotMat.matrix[4] = cosf(theta);
	rotMat.matrix[7] = sinf(theta);
	rotMat.matrix[5] = -rotMat.matrix[7];
	return rotMat;
}
matrix3 matrix3::makeRotateYaxis(float theta){
	matrix3 rotMat;
	theta *= radiansPerDegree;
	rotMat.matrix[8] = rotMat.matrix[0] = cosf(theta);
	rotMat.matrix[2] = sinf(theta);
	rotMat.matrix[6] = -rotMat.matrix[2];
	return rotMat;
}
matrix3 matrix3::makeRotateZaxis(float theta){
	matrix3 rotMat;
	theta *= radiansPerDegree;
	rotMat.matrix[4] = rotMat.matrix[0] = cosf(theta);
	rotMat.matrix[3] = sinf(theta);
	rotMat.matrix[1] = -rotMat.matrix[3];
	return rotMat;
}

#define ux axisNormal.coords[0]
#define uy axisNormal.coords[1]
#define uz axisNormal.coords[2]

//warning full rot not used
vector3 matrix3::rotateByArb_XYZ(vector3 pointToRot, vector3 axisNormal, float theta){
												// 0  1  2  	
	float cosine = cosf(theta*radiansPerDegree);// 3  4  5  	
	float sine = sinf(theta*radiansPerDegree);  // 6  7  8 	
	matrix3 rotMat;								
	rotMat[0] = cosine + ux*ux*(1 - cosine);
	rotMat[1] = ux*uy*(1 - cosine) - uz*sine;
	rotMat[2] = ux*uz*(1 - cosine) + uy*sine;

	rotMat[3] = uy*ux*(1 - cosine) + uz*sine;
	rotMat[4] = cosine + uy*uy*(1 - cosine);
	rotMat[5] = uy*uz*(1 - cosine) - ux*sine;

	rotMat[6] = uz*ux*(1 - cosine) - uy*sine;
	rotMat[7] = uz*uy*(1 - cosine) + ux*sine;
	rotMat[8] = cosine + uz*uz*(1 - cosine);

	return rotMat*(pointToRot);//should this be full mult?
}

//2 trig, 
vector3 matrix3::rotateByArb_XZ(vector3 pointToRot, vector3 axisNormal, float theta){
	// for htis all uy = 0
	float cosine = cosf(theta*radiansPerDegree);
	float sine = sinf(theta*radiansPerDegree);
	matrix3 rotMat;
	rotMat[0] = cosine + ux*ux*(1 - cosine);
	rotMat[1] = -uz*sine;
	rotMat[2] = ux*uz*(1 - cosine);

	rotMat[3] = uz*sine;
	rotMat[4] = cosine;
	rotMat[5] = -ux*sine;

	rotMat[6] = uz*ux*(1 - cosine);
	rotMat[7] = ux*sine;
	rotMat[8] = cosine + uz*uz*(1 - cosine);

	return rotMat*(pointToRot);//using rotMult here caused jerkiness
}
#undef ux
#undef uy
#undef uz

//ROW MAJOR
//UNIMPLEMENTED PV mats are all vec4 so this is questionably useful
matrix3 matrix3::operator*(const matrix3& dot){
	matrix3 multi;  /*          //dot
						// |a  b  c | 
						// |e  f  g | 
						// |i  j  k |
	//this				 
	// |a  b  c | 
	// |e  f  g | 
	// |i  j  k |
	 
	multi[0] = this->matrix[0] * dot[0] + this->matrix[1] * dot[4] + this->matrix[2] * dot[8] + this->matrix[3] * dot[12];
	multi[1] = this->matrix[0] * dot[1] + this->matrix[1] * dot[5] + this->matrix[2] * dot[9] + this->matrix[3] * dot[13];
	multi[2] = this->matrix[0] * dot[2] + this->matrix[1] * dot[6] + this->matrix[2] * dot[10] + this->matrix[3] * dot[14];
	multi[3] = this->matrix[0] * dot[3] + this->matrix[1] * dot[7] + this->matrix[2] * dot[11] + this->matrix[3] * dot[15];

	multi[4] = this->matrix[4] * dot[0] + this->matrix[5] * dot[4] + this->matrix[6] * dot[8] + this->matrix[7] * dot[12];
	multi[5] = this->matrix[4] * dot[1] + this->matrix[5] * dot[5] + this->matrix[6] * dot[9] + this->matrix[7] * dot[13];
	multi[6] = this->matrix[4] * dot[2] + this->matrix[5] * dot[6] + this->matrix[6] * dot[10] + this->matrix[7] * dot[14];
	multi[7] = this->matrix[4] * dot[3] + this->matrix[5] * dot[7] + this->matrix[6] * dot[11] + this->matrix[7] * dot[15];

	multi[8] = this->matrix[8] * dot[0] + this->matrix[9] * dot[4] + this->matrix[10] * dot[8] + this->matrix[11] * dot[12];
	multi[9] = this->matrix[8] * dot[1] + this->matrix[9] * dot[5] + this->matrix[10] * dot[9] + this->matrix[11] * dot[13];
	multi[10] = this->matrix[8] * dot[2] + this->matrix[9] * dot[6] + this->matrix[10] * dot[10] + this->matrix[11] * dot[14];
	multi[11] = this->matrix[8] * dot[3] + this->matrix[9] * dot[7] + this->matrix[10] * dot[11] + this->matrix[11] * dot[15];

	multi[12] = this->matrix[12] * dot[0] + this->matrix[13] * dot[4] + this->matrix[14] * dot[8] + this->matrix[15] * dot[12];
	multi[13] = this->matrix[12] * dot[1] + this->matrix[13] * dot[5] + this->matrix[14] * dot[9] + this->matrix[15] * dot[13];
	multi[14] = this->matrix[12] * dot[2] + this->matrix[13] * dot[6] + this->matrix[14] * dot[10] + this->matrix[15] * dot[14];
	multi[15] = this->matrix[12] * dot[3] + this->matrix[13] * dot[7] + this->matrix[14] * dot[11] + this->matrix[15] * dot[15];
*/
	return multi;
}

//28 flops, 4 assignments
vector3 matrix3::operator*(const vector3& dot){
	vector3 newVector;
	newVector.coords[0] = this->matrix[0] * dot.coords[0] +
						this->matrix[1] * dot.coords[1] +
						this->matrix[2] * dot.coords[2];
							

	newVector.coords[1] = this->matrix[3] * dot.coords[0] +
						this->matrix[4] * dot.coords[1] +
						this->matrix[5] * dot.coords[2];
						

	newVector.coords[2] = this->matrix[6] * dot.coords[0] +
						this->matrix[7] * dot.coords[1] +
						this->matrix[8] * dot.coords[2];
							

	

	return newVector;
}

#undef degreesPerRadian 

#undef radiansPerDegree 
