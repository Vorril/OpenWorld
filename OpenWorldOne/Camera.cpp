#include "Camera.h"

#define invsqrt2 0.707107f


Camera::Camera()
{
}

//Making the viewMatrix ROW MAJOR
//DOESNT Make the proj matrix (too much window data to toss in here)
//DOESNT UPDATE the UBO call update() only when programs exist
Camera::Camera(vector3 lookAt, vector3 cameraPos){
	makeViewMatrix(lookAt, cameraPos);
}
Camera::Camera(float xLook, float yLook, float zLook,
	float xTrans, float yTrans, float zTrans){
	*this = Camera(vector3(xLook, yLook, zLook), 
				   vector3(xTrans, yTrans, zTrans));
}

Camera::~Camera()
{
}

void Camera::makeViewMatrix(vector3 lookAt, vector3 cameraPos){
	cameraPosition = cameraPos;//save that to camera obj
	vector3 zAxis = cameraPos + -lookAt;
	zAxis.normalize(); 
	vector3 xAxis = vector3::UNIT_Y.cross(zAxis);//Y by convention
	xAxis.normalize();
	vector3 yAxis = zAxis % xAxis; 


	/// Side doings ~~ Saving some results ////////////////////
	lookDirection = zAxis;//save that to camera obj  //this might be backwards
	lookDirectionXZ = (vector4(zAxis.coords[0], 0.0f, zAxis.coords[2]));
	lookDirectionXZ.normalize();//important to normalize for VBM later
	perpToLookDir = xAxis; 
	///////////////////////////////////////

	

	//World -> View matrix =	|x1, x2, x3  -xAxis * eye |	     	RM | 0  1  2  3 | /// CM	| 0  4  8   12 |
	//							|y1, y2, y3  -yAxis * eye |			   | 4  5  6  7 |		    | 1  5  9   13 |
	//							|z1, z2, z3  -zAxis * eye |			   | 8  9 10 11 |		    | 2  6  10  14 |
	//							| 0   0   0        1      |			   |12 13 14 15 |		    | 3  7  11  15 |

	// ROW MAJOR
	viewMatrix[0] = xAxis.coords[0]; viewMatrix[1] = xAxis.coords[1]; viewMatrix[2] = xAxis.coords[2];
	viewMatrix[4] = yAxis.coords[0]; viewMatrix[5] = yAxis.coords[1]; viewMatrix[6] = yAxis.coords[2];
	viewMatrix[8] = zAxis.coords[0]; viewMatrix[9] = zAxis.coords[1]; viewMatrix[10] = zAxis.coords[2];
	//That was R^T(R transpose) stretched to a 4 matrix
	//column 4 = -R^T * transposeVector is eye coords

	viewMatrix[3] = -(xAxis * cameraPos);
	viewMatrix[7] = -(yAxis * cameraPos);
	viewMatrix[11] = -(zAxis * cameraPos);

	viewNeedsUpdate = true;
}

//remake the viewMatrix intended to be called after manually adjusting the look direction and/or position
//Relative axes already exist
void Camera::remakeViewMatrix(){
	//vector4 xAxis = vector4::UNIT_Y.cross(lookDirection);
	//xAxis.normalize();
	vector3 yAxis = lookDirection % perpToLookDir; // perptolook <=< xAxis

	viewMatrix[0] = perpToLookDir.coords[0]; viewMatrix[1] = perpToLookDir.coords[1]; viewMatrix[2] = perpToLookDir.coords[2];
	viewMatrix[4] = yAxis.coords[0]; viewMatrix[5] = yAxis.coords[1]; viewMatrix[6] = yAxis.coords[2];
	viewMatrix[8] = lookDirection.coords[0]; viewMatrix[9] = lookDirection.coords[1]; viewMatrix[10] = lookDirection.coords[2];
	
	viewMatrix[3] = -(perpToLookDir * cameraPosition);
	viewMatrix[7] = -(yAxis * cameraPosition);
	viewMatrix[11] = -(lookDirection * cameraPosition);

	viewNeedsUpdate = true;
}

//field of view in degrees b/w 30 and 90
void Camera::makeProjectionMatrix(float FoV, float aspectRatio, float nearClip, float farClip){
	
	float d = 1.0f / ( tanf(FoV*radiansPerDegree/2) );
	projectionMatrix[0] = d / aspectRatio;
	projectionMatrix[5] = d;
	projectionMatrix[10] = (nearClip + farClip) / (nearClip - farClip);
	projectionMatrix[11] = (2 * nearClip*farClip) / (nearClip - farClip); // 14 column   |   11 row
	projectionMatrix[14] = -1.0f;										  // 11 major    |   14 major
	projectionMatrix[15] = 0.0f;

	projNeedsUpdate = true;
	
}

/* //not doing it this way
void Camera::makePVMatrix(){
	PVmatrix = projectionMatrix * viewMatrix;
	////////////////////////////////////////
	///// IMPORTANT//////////////
	needsUpdate = false; //could still need the uniform updated!
	////////////////////////////////////////////////////
	/////////////////////
}
*/

void Camera::translate(vector3 transVec){
	cameraPosition += transVec;
	viewNeedsRemade = true;
}

//Proper encapsulation for gravitation movement
void Camera::translateVert(float yMovement){
	cameraPosition[1] += yMovement;
	viewNeedsRemade = true;
}

void Camera::translateTo(vector3 moveTo){
	cameraPosition = moveTo;
	viewNeedsRemade = true;
}

void Camera::translateVertTo(float moveToY){
	cameraPosition[1] = moveToY;
	viewNeedsRemade = true;
}

//rotate about the y axis
//You need to update the V, PV mats later yourself
//In case both the position and looking vecs both change
void Camera::rotateCameraHorz(float angle){
	lookDirection = matrix3::makeRotateYaxis(angle)*lookDirection;//efficiency concerns
	//perpToLookDir = vector4::UNIT_Y.cross(lookDirection);
	
	lookDirectionXZ = (vector3(lookDirection.coords[0], 0.0f, lookDirection.coords[2]));
	lookDirectionXZ.normalize();//important to normalize for VBM later
	perpToLookDir = vector4(lookDirectionXZ.coords[2], 0.0f, -lookDirectionXZ.coords[0]);

	viewNeedsRemade = true;

}

void Camera::rotateCameraVert(float angle){
	vector3 lookCopy = lookDirection;
	lookDirection = matrix3::rotateByArb_XZ(lookDirection, perpToLookDir, angle);
	lookDirection.normalize();
	if (lookDirection[1] * lookDirection[1] > 0.95) lookDirection = lookCopy;//undo went too extreme
	else viewNeedsRemade = true;
	//}
}

//More like keepUpToDate
//Remake if necc otherwise update
//If proj changes are moved to entirely within Camera it would
	//make sene to not make this distinction
	//Otherwise its useful to not remake the view mat an unnec amount of times
void Camera::update(){
	if (viewNeedsRemade){
		remakeViewMatrix();
		viewNeedsRemade = false;
		pvNeedsUpdate = true;
	}
	if (viewNeedsUpdate){
		Shader::updateUPO_v(&viewMatrix);
		viewNeedsUpdate = false;
		pvNeedsUpdate = true;
	}
	if (projNeedsUpdate){
		Shader::updateUPO_p(&projectionMatrix);
		projNeedsUpdate = false;
		pvNeedsUpdate = true;
	}
	if (pvNeedsUpdate){
		PV_Matrix = projectionMatrix * viewMatrix;
		Shader::updateUPO_pv(&PV_Matrix);
		pvNeedsUpdate = false;
	}
}

void Camera::moveCamera(float speedMod){
	
	switch (keyDownCode){

	case 0x1u://Left
	case 0xdu://Left
		translate(perpToLookDir*-speedMod);
		break;
	case 0x2u://Right
	case 0xeu://Right
		translate(perpToLookDir*speedMod);
		break;
	case 0x4u://Up
	case 0x7u://Up
		translate(lookDirectionXZ*-speedMod);
		break;
	case 0x5u://UpLeft
		translate((lookDirectionXZ + perpToLookDir) * (-speedMod* invsqrt2));
		break;
	case 0x6u://UpRight
		translate((-lookDirectionXZ + perpToLookDir) * (speedMod* invsqrt2));
		break;
	case 0x8u://Down
	case 0xbu://Down
		translate(lookDirectionXZ*speedMod);
		break;
	case 0x9u://DownLeft
		translate((lookDirectionXZ + -perpToLookDir) * (speedMod* invsqrt2));
		break;
	case 0xau://DownRight
		translate((lookDirectionXZ + perpToLookDir) * (speedMod* invsqrt2));
		break;
	

	//case 0x0u
	//case 0x3u:
	//case 0xcu:
	//case 0xfu:

	
	}//switch
	
	
}

void Camera::moveCameraUnbound(float speedMod){

	switch (keyDownCode){

	case 0x1u://Left
	case 0xdu://Left
		translate(perpToLookDir*-speedMod);
		break;
	case 0x2u://Right
	case 0xeu://Right
		translate(perpToLookDir*speedMod);
		break;
	case 0x4u://Up
	case 0x7u://Up
		translate(lookDirection*-speedMod);
		break;
	case 0x5u://UpLeft
		translate((lookDirection + perpToLookDir) * (-speedMod* invsqrt2));
		break;
	case 0x6u://UpRight
		translate((-lookDirection + perpToLookDir) * (speedMod* invsqrt2));
		break;
	case 0x8u://Down 
	case 0xbu://Down
		translate(lookDirection*speedMod);
		break;
	case 0x9u://DownLeft
		translate((lookDirection + -perpToLookDir) * (speedMod* invsqrt2));
		break;
	case 0xau://DownRight
		translate((lookDirection + perpToLookDir) * (speedMod* invsqrt2));
		break;


		//case 0x0u
		//case 0x3u:
		//case 0xcu:
		//case 0xfu:


	}//switch
}
