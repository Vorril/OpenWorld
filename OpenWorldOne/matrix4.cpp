#include "matrix4.h"

matrix4::matrix4() :matrix()// ":matrix()" zeroes it
{
	matrix[0] = matrix[5] = matrix[10] = matrix[15] = 1.0f;
}


matrix4::~matrix4()
{
}

// 0  1  2  3		X:  1    0    0    -
// 4  5  6  7			0   cos -sin   -
// 8  9  10 11			0   sin  cos   -
// 12 13 14 15			-    -	  -    -
//
// Y:  cos   0   sin   -		 z:  cos  -sin   0    -
// 	    0    1    0    -		     sin   cos   0    -
// 	  -sin   0   cos   -		      0     0    1    -
// 	    -    -    -    -		      -     -    -    -

matrix4 matrix4::makeRotateXaxis(float theta){
	matrix4 rotMat;
	theta *= radiansPerDegree;
	rotMat.matrix[10] = rotMat.matrix[5] = cosf(theta);
	rotMat.matrix[9] = sinf(theta);
	rotMat.matrix[6] = -rotMat.matrix[9];
	return rotMat;
}
matrix4 matrix4::makeRotateYaxis(float theta){
	matrix4 rotMat;
	theta *= radiansPerDegree;
	rotMat.matrix[10] = rotMat.matrix[0] = cosf(theta);
	rotMat.matrix[2] = sinf(theta);
	rotMat.matrix[8] = -rotMat.matrix[2];
	return rotMat;
}
matrix4 matrix4::makeRotateZaxis(float theta){
	matrix4 rotMat;
	theta *= radiansPerDegree;
	rotMat.matrix[5] = rotMat.matrix[0] = cosf(theta);
	rotMat.matrix[4] = sinf(theta);
	rotMat.matrix[1] = -rotMat.matrix[4];
	return rotMat;
}

#define ux axisNormal.coords[0]
#define uy axisNormal.coords[1]
#define uz axisNormal.coords[2]
// 2 trig, 73 flops, 13 asignments
vector4 matrix4::rotateByArb_XYZ(vector4 pointToRot, vector4 axisNormal, float theta){
												// 0  1  2  3	
	float cosine = cosf(theta*radiansPerDegree);// 4  5  6  7	
	float sine = sinf(theta*radiansPerDegree);  // 8  9  10 11	
	matrix4 rotMat;								// 12 13 14 15
	rotMat[0] = cosine + ux*ux*(1 - cosine);
	rotMat[1] = ux*uy*(1 - cosine) - uz*sine;
	rotMat[2] = ux*uz*(1 - cosine) + uy*sine;

	rotMat[4] = uy*ux*(1 - cosine) + uz*sine;
	rotMat[5] = cosine + uy*uy*(1 - cosine);
	rotMat[6] = uy*uz*(1 - cosine) - ux*sine;

	rotMat[8] = uz*ux*(1 - cosine) - uy*sine;
	rotMat[9] = uz*uy*(1 - cosine) + ux*sine;
	rotMat[10] = cosine + uz*uz*(1 - cosine);

	return rotMat.rotMult(pointToRot);//should this be full mult?
}

//2 trig, 
vector4 matrix4::rotateByArb_XZ(vector4 pointToRot, vector4 axisNormal, float theta){
	// for htis all uy = 0
	float cosine = cosf(theta*radiansPerDegree);
	float sine = sinf(theta*radiansPerDegree);
	matrix4 rotMat;								
	rotMat[0] = cosine + ux*ux*(1 - cosine);
	rotMat[1] =  - uz*sine;
	rotMat[2] = ux*uz*(1 - cosine);

	rotMat[4] = uz*sine;
	rotMat[5] = cosine;
	rotMat[6] = - ux*sine;

	rotMat[8] = uz*ux*(1 - cosine);
	rotMat[9] = ux*sine;
	rotMat[10] = cosine + uz*uz*(1 - cosine);

	return rotMat*(pointToRot);//using rotMult here cause jerkiness!!
}
#undef ux
#undef uy
#undef uz

//useful to make VP or MVP matrixes to pass
//ROW MAJOR
matrix4 matrix4::operator*(const matrix4& dot){
	matrix4 multi;            //dot
						// |a  b  c  d| 
						// |e  f  g  h| 
						// |i  j  k  l|
		//this			// |m  n  o  p| 
	// |a  b  c  d| 
	// |e  f  g  h| 
	// |i  j  k  l|
	// |m  n  o  p| 
	multi[0] = this->matrix[0] * dot[0]     + this->matrix[1] * dot[4]     + this->matrix[2] * dot[8]     + this->matrix[3] * dot[12];
	multi[1] = this->matrix[0] * dot[1]     + this->matrix[1] * dot[5]     + this->matrix[2] * dot[9]     + this->matrix[3] * dot[13];
	multi[2] = this->matrix[0] * dot[2]     + this->matrix[1] * dot[6]     + this->matrix[2] * dot[10]     + this->matrix[3] * dot[14];
	multi[3] = this->matrix[0] * dot[3]     + this->matrix[1] * dot[7]     + this->matrix[2] * dot[11]     + this->matrix[3] * dot[15];

	multi[4] = this->matrix[4] * dot[0]     + this->matrix[5] * dot[4]     + this->matrix[6] * dot[8]     + this->matrix[7] * dot[12];
	multi[5] = this->matrix[4] * dot[1]     + this->matrix[5] * dot[5]     + this->matrix[6] * dot[9]     + this->matrix[7] * dot[13];
	multi[6] = this->matrix[4] * dot[2]     + this->matrix[5] * dot[6]     + this->matrix[6] * dot[10]     + this->matrix[7] * dot[14];
	multi[7] = this->matrix[4] * dot[3]     + this->matrix[5] * dot[7]     + this->matrix[6] * dot[11]     + this->matrix[7] * dot[15];

	multi[8] = this->matrix[8] * dot[0]     + this->matrix[9] * dot[4]     + this->matrix[10] * dot[8]     + this->matrix[11] * dot[12];
	multi[9] = this->matrix[8] * dot[1]     + this->matrix[9] * dot[5]     + this->matrix[10] * dot[9]     + this->matrix[11] * dot[13];
	multi[10] = this->matrix[8] * dot[2]     + this->matrix[9] * dot[6]     + this->matrix[10] * dot[10]     + this->matrix[11] * dot[14];
	multi[11] = this->matrix[8] * dot[3]     + this->matrix[9] * dot[7]     + this->matrix[10] * dot[11]     + this->matrix[11] * dot[15];

	multi[12] = this->matrix[12] * dot[0]     + this->matrix[13] * dot[4]     + this->matrix[14] * dot[8]     + this->matrix[15] * dot[12];
	multi[13] = this->matrix[12] * dot[1]     + this->matrix[13] * dot[5]     + this->matrix[14] * dot[9]     + this->matrix[15] * dot[13];
	multi[14] = this->matrix[12] * dot[2]     + this->matrix[13] * dot[6]     + this->matrix[14] * dot[10]     + this->matrix[15] * dot[14];
	multi[15] = this->matrix[12] * dot[3]     + this->matrix[13] * dot[7]     + this->matrix[14] * dot[11]     + this->matrix[15] * dot[15];

	return multi;
}

//28 flops, 4 assignments
vector4 matrix4::operator*(const vector4& dot){				
	vector4 newVector;											
	newVector.coords[0] = this->matrix[0] * dot.coords[0] +    
							this->matrix[1] * dot.coords[1] +   
							this->matrix[2] * dot.coords[2] +
							this->matrix[3] * dot.coords[3];	

	newVector.coords[1] = this->matrix[4] * dot.coords[0] +     
							this->matrix[5] * dot.coords[1] +   
							this->matrix[6] * dot.coords[2] +
						    this->matrix[7] * dot.coords[3];

	newVector.coords[2] = this->matrix[8] * dot.coords[0] +   				
							this->matrix[9] * dot.coords[1] + 				
							this->matrix[10] * dot.coords[2] +					
						    this->matrix[11] * dot.coords[3];						

	newVector.coords[3] = this->matrix[12] * dot.coords[0] +   
							this->matrix[13] * dot.coords[1] + 
							this->matrix[14] * dot.coords[2] + 
							this->matrix[15] * dot.coords[3];  

	return newVector;
}

//Multiplication streamlined for a rotation matrix multiplication
//ROW MAJOR
vector4 matrix4::rotMult(vector4 dot){							// x        0
	vector4 newVector;											//    y     0
	newVector.coords[0] = this->matrix[0] * dot.coords[0] +     //       z  0  can pre-cull these zeroes
							this->matrix[1] * dot.coords[1] +	// 0  0  0  w
							this->matrix[2] * dot.coords[2];
							//+	this->matrix[3] * dot.coords[3];					// 0  1  2  3
																					// 4  5  6  7
						newVector.coords[1] = this->matrix[4] * dot.coords[0] +     // 8  9  10 11
						this->matrix[5] * dot.coords[1] +							// 12 13 14 15
						this->matrix[6] * dot.coords[2];
						//this->matrix[7] * dot.coords[3];

	newVector.coords[2] = this->matrix[8] * dot.coords[0] +   //					| x |
							this->matrix[9] * dot.coords[1] + //					| y |
							this->matrix[10] * dot.coords[2]; //					| z |
						//this->matrix[11] * dot.coords[3];							| w |
		
	newVector.coords[3] = //this->matrix[12] * dot.coords[0] +   // |a  b  c  d| x*a + y*b + z*c + 0
							//this->matrix[13] * dot.coords[1] + // |e  f  g  h| x*e + y*f + z*g + 0
							//this->matrix[14] * dot.coords[2] + // |i  j  k  l| x*i + y*j + z*k + 0
							this->matrix[15] * dot.coords[3];    // |m  n  o  p|  0     0     0  + w*p

	return newVector;
}

matrix4 matrix4::transpose()const{
	// RM | 0  1  2  3 |	CM	| 0  4  8   12 |
	//	  | 4  5  6  7 |		| 1  5  9   13 |
	//	  | 8  9 10 11 |		| 2  6  10  14 |
	//	  |12 13 14 15 |		| 3  7  11  15 |

	matrix4 transposed;
	transposed[0] = matrix[0];
	transposed[5] = matrix[5];
	transposed[10] = matrix[10];
	transposed[15] = matrix[15];

	transposed[4] = matrix[1];
	transposed[1] = matrix[4];
	transposed[2] = matrix[8];
	transposed[8] = matrix[2];
	transposed[6] = matrix[9];
	transposed[9] = matrix[6];
	transposed[3] = matrix[12];
	transposed[12] = matrix[3];
	transposed[7] = matrix[13];
	transposed[13] = matrix[7];
	transposed[11] = matrix[14];
	transposed[14] = matrix[11];

	return transposed;

}

// 0  1  2  3      // 0  1  2  -
// 4  5  6  7      // 3  4  5  -
// 8  9  10 11     // 6  7  8  -
// 12 13 14 15        -  -  -  -
matrix4::operator matrix3()const{
	matrix3 topLeft;
	topLeft[0] = matrix[0];
	topLeft[1] = matrix[1];
	topLeft[2] = matrix[2];

	topLeft[3] = matrix[4];
	topLeft[4] = matrix[5];
	topLeft[5] = matrix[6];

	topLeft[6] = matrix[8];
	topLeft[7] = matrix[9];
	topLeft[8] = matrix[10];

	return topLeft;
}