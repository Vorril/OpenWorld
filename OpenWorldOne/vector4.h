#pragma once
#include <math.h>
#include <GL/glew.h>
#include <iostream> //remove when done
#include <vector3.h>

class vector4 //a 4vector class basically
{
public:
	static const vector4 UNIT_X;
	static const vector4 UNIT_Y;
	static const vector4 UNIT_Z;

	float coords[4];


	void normalize();
	void multiplyBy(float m);

	void copy(vector4 copythis);

	static float fastInvSqrt(float number);

	vector4 operator-();
	vector4 operator-()const{ //dont really know why this works...
		return vector4(-coords[0], -coords[1], -coords[2]); //this worked
	}

	vector4 operator-(const vector4 &sub);

	vector4 operator+(const vector4& add);
	void operator+=(const vector4& add);

	float operator*(const vector4& dot);
	vector4 operator*(const float factor);
	vector4 cross( vector4 cross)const;
	vector4 operator%(const vector4 &cross);
	
	float operator[](int i) const{ return coords[i]; }
	float & operator[](int i){ return coords[i]; }

	operator const vector3(){
		return vector3(coords[0], coords[1], coords[2]);
	}

	vector4(float xCoord, float yCoord, float zCoord, float wVal = 1.0f);
	vector4(vector3 dir, float wVal = 1.0f);
	vector4();

	~vector4();

	

};

