#include "vector4.h"

#define x coords[0]
#define y coords[1]
#define z coords[2]
#define w coords[3]

vector4::vector4(float xCoord, float yCoord, float zCoord, float wVal){

	coords[0] = xCoord;
	coords[1] = yCoord;
	coords[2] = zCoord;
	coords[3] = wVal;

}
vector4::vector4(){
	coords[0] = 0.0f;
	coords[1] = 0.0f;
	coords[2] = 0.0f;
	coords[3] = 1.0f;
}

vector4::vector4(vector3 dir, float wVal){
	coords[0] = dir[0];
	coords[1] = dir[1];
	coords[2] = dir[2];
	coords[3] = wVal;

}

vector4::~vector4()
{
}

const vector4 vector4::UNIT_X = vector4(1.00f, 0.0f, 0.0f);
const vector4 vector4::UNIT_Y = vector4(0.0f, 1.00f, 0.0f);
const vector4 vector4::UNIT_Z = vector4(0.0f, 0.0f, 1.00f);

//does not include w in normalization
void vector4::normalize(){
	float invMagnitude = fastInvSqrt( (x*x + y*y + z*z));
	this->x *= invMagnitude;
	this->y *= invMagnitude;
	this->z *= invMagnitude;
}

void vector4::multiplyBy(float m){
	x *= m;
	y *= m;
	z *= m;
}

void vector4::copy(vector4 copythis){
	this->x = copythis.x;
	this->y = copythis.y;
	this->z = copythis.z;
	this->w = copythis.w;
}

float vector4::fastInvSqrt(float number)
{
	long i;
	float x2, yVal;
	const float threehalfs = 1.5F;

	x2 = number * 0.5F;
	yVal = number;
	i = *(long *)&yVal;                       // evil floating point bit level hacking
	i = 0x5f3759df - (i >> 1);               // what the fuck?
	yVal = *(float *)&i;
	yVal = yVal * (threehalfs - (x2 * yVal * yVal));   // 1st iteration
	yVal  = yVal * ( threehalfs - ( x2 * yVal * yVal ) );   // 2nd iteration, this can be removed

	return yVal;
}

vector4 vector4::operator-(const vector4 &sub){
	return vector4(this->x - sub.x, this->y - sub.y, this->z - sub.z);
}

vector4 vector4::operator-(){

	return vector4(-x, -y, -z);
};

vector4 vector4::operator+(const vector4& add){
	vector4 newVertex(this->x + add.x, this->y + add.y, this->z + add.z);
	return newVertex;
}

void vector4::operator+=(const vector4& add){
	this->x += add.x;
	this->y += add.y;
	this->z += add.z;
	//return *this;
}

//does not include w in calculation
float vector4::operator*(const vector4& dot){
	float dotProduct = (this->x * dot.x) + (this->y * dot.y) + (this->z * dot.z);
	return dotProduct;
}

vector4 vector4::operator*(const float factor){
	return vector4(this->x*factor, this->y*factor, this->z*factor);
}
vector4 vector4::cross( vector4 cross)const{
	//  <a, b, c> X <d, e, f>

	//  | x  y  z |
	//  | a  b  c | = x|b c| - y|a c| + z|a b| = < b*f-c*e, a*f-c*d, a*e-b*d >
	//  | d  e  f |    |e f|    |d f|    |d e|

	return vector4( coords[1] * cross.coords[2] - coords[2] * cross.coords[1],
					-(coords[0] * cross.coords[2] - coords[2] * cross.coords[0]),
					coords[0] * cross.coords[1] - coords[1] * cross.coords[0]);
}
vector4 vector4::operator%(const vector4 &cross){
	return this->cross(cross); 
}

#undef x
#undef y
#undef z