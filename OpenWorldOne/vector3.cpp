#include "vector3.h"

#define x coords[0]
#define y coords[1]
#define z coords[2]

const vector3 vector3::UNIT_X = vector3(1.00f, 0.0f, 0.0f);
const vector3 vector3::UNIT_Y = vector3(0.0f, 1.00f, 0.0f);
const vector3 vector3::UNIT_Z = vector3(0.0f, 0.0f, 1.00f);

vector3::vector3()
{
}

vector3::vector3(float xP, float yP, float zP){
	coords[0] = xP;
	coords[1] = yP;
	coords[2] = zP;
}

vector3::~vector3()
{
}

float vector3::fastInvSqrt(float number)
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
	yVal = yVal * (threehalfs - (x2 * yVal * yVal));   // 2nd iteration, this can be removed

	return yVal;
}

void vector3::normalize(){
	float invMag = fastInvSqrt(coords[0] * coords[0] + coords[1] * coords[1] + coords[2] * coords[2]);
	coords[0] *= invMag;
	coords[1] *= invMag;
	coords[2] *= invMag;

}


vector3 vector3::operator-(){

	return vector3(-x, -y, -z);
};

vector3 vector3::operator-(const vector3 &sub){
	return vector3(this->x - sub.x, this->y - sub.y, this->z - sub.z);
}

vector3 vector3::operator+(const vector3& add){
	vector3 newVertex(this->x + add.x, this->y + add.y, this->z + add.z);
	return newVertex;
}

void vector3::operator+=(const vector3& add){
	this->x += add.x;
	this->y += add.y;
	this->z += add.z;
	//return *this;
}

float vector3::operator*(const vector3& dot){
	float dotProduct = (this->x * dot.x) + (this->y * dot.y) + (this->z * dot.z);
	return dotProduct;
}

vector3 vector3::operator*(const float factor){
	return vector3(this->x*factor, this->y*factor, this->z*factor);
}
vector3 vector3::cross(vector3 cross)const{
	//  <a, b, c> X <d, e, f>

	//  | x  y  z |
	//  | a  b  c | = x|b c| - y|a c| + z|a b| = < b*f-c*e, a*f-c*d, a*e-b*d >
	//  | d  e  f |    |e f|    |d f|    |d e|

	return vector3(coords[1] * cross.coords[2] - coords[2] * cross.coords[1],
		-(coords[0] * cross.coords[2] - coords[2] * cross.coords[0]),
		coords[0] * cross.coords[1] - coords[1] * cross.coords[0]);
}
vector3 vector3::operator%(const vector3 &cross){
	return this->cross(cross);
}

#undef x
#undef y
#undef z