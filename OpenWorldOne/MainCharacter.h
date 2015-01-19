#pragma once

#include <Camera.h>
#include <Terrain.h>
#include <GL/glfw3.h>
class MainCharacter
{
public:
	MainCharacter();
	~MainCharacter();

	//consider a pointer to a physics controlling object to vary gravity etc

	Camera eyeCam;
	//base character movement speed
	float speed = 7.0f;
	//velocity for falling, could make this a vec3 in the future
	float velocity = 0.0f;
	float eyeHeight = 2.25f;
	bool jumping = false;
	bool boundMovement = false;


	struct coreTimer{
		double prevTime = 0.0;
		double deltaT = 0.0;
	};
	double jumpTime;


	void move(coreTimer* times, TerrainMap* ground);
	void jump();
	void updateCam();
	vector3 position();//do these by reference or something?
	vector3 lookDirection();
	//really need to move some things from cam into here
};

