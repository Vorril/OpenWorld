#pragma once
#include <matrix4.h> //includes vector4
#include <matrix3.h>
#include <Shader.h>


class Camera
{
private:
	matrix4 viewMatrix;
	matrix4 projectionMatrix;
	vector3 lookDirectionXZ;
    
	//bool needsUpdated;considering

public: //really need to move these to private, /use get methods// actually try to pass the camera when possible
	//matrix4 PVmatrix; //for now going with on gpu combinations for flexibility
	matrix4 PV_Matrix;
	vector3 lookDirection;
	vector3 perpToLookDir;
	vector3 cameraPosition;//worldspace

	unsigned int keyDownCode = 0;
	

	Camera();
	Camera(vector3 look, vector3 translate);
	Camera(float xLook, float yLook, float zLook,
			float xTrans = 0.0f, float yTrans = 0.0f, float zTrans = 0.0f);

	~Camera();

	void translate(vector3 transVec);
	void translateVert(float yMovement);
	void translateTo(vector3 moveTo);
	void translateVertTo(float moveToY);

	void rotateCameraHorz(float angle);
	void rotateCameraVert(float angle);

	void moveCamera(float speedMod);//XZ plane "grounded" movement
	void moveCameraUnbound(float speedMod);//Full XYZ movement

	void update();

	void makeProjectionMatrix(float FoV, float aspectRatio, float NearClip, float farClip);
private:
	//Update means just for the uniform, remade means remake the matrix
	bool pvNeedsUpdate = false;
	bool viewNeedsUpdate = false;
	bool viewNeedsRemade = false;
	bool projNeedsUpdate = false;
	void makeViewMatrix(vector3 look, vector3 translate);
	void remakeViewMatrix();

	//void makePVMatrix();
};

