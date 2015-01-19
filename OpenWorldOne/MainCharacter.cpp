#include "MainCharacter.h"


MainCharacter::MainCharacter()
{
}


MainCharacter::~MainCharacter()
{
}

vector3 MainCharacter::position(){
	return eyeCam.cameraPosition;
}

vector3 MainCharacter::lookDirection(){
	return eyeCam.lookDirection;
}

void MainCharacter::jump(){

	jumping = true;
	jumpTime = glfwGetTime();
}

void MainCharacter::move(coreTimer* times, TerrainMap* ground){
	if (!boundMovement)
		// need to make sure you cant stand on anything too steep
		eyeCam.moveCameraUnbound(speed*times->deltaT);//fly
	if (boundMovement){
		eyeCam.moveCamera(speed * times->deltaT);//bound XZ
		float yIntercept = ground->getLocalHeight(eyeCam.cameraPosition.coords[0], eyeCam.cameraPosition.coords[2]) + eyeHeight;//poss return the normal collided with

		if (jumping){
			float yMove;
			//camVelocity -= deltaT * gravity * 1.0f;
			double timeSinceJump = times->prevTime - jumpTime;
			if ((timeSinceJump) < 0.18){ //0.18s // prevtime is effectively currenttime
				velocity = (timeSinceJump)* 33.0;
				if (velocity < 0.0) velocity = -velocity;
			}//pulsing
			else{
				velocity -= times->deltaT * 10.0f;
			}//gravity is in control

			yMove = times->deltaT * velocity;
			eyeCam.translateVert(yMove);

			if (yIntercept > eyeCam.cameraPosition[1] && timeSinceJump > 0.18){ //moved below the ground // consider jumptime otherwise its imposs to jump uphill
				eyeCam.translateVertTo(yIntercept);
				jumping = false;
			}
			//else{//still above the ground

			//}
		}//was jumping
		else eyeCam.translateVertTo(yIntercept);
	}//bound movement
	eyeCam.update();
}
