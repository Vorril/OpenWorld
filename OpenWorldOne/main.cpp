#include <iostream>
#include <GL/glew.h>//using the d library nd dll
#include <GL/glfw3.h> // includes windows.h
#include <math.h>

//My classes:
#include <Shader.h>
#include <World.h> //hold the state of the world, display lists etc, reduce as much as possible from living in main
#include <Camera.h> //includes mats and shader
#include <Terrain.h> //should probably moce to world
#include <Skybox.h>

/*
TODO
+- Implement model matrices//(re)consider what multiplications to do on the GPU
-- Consider terrain view culling
-- Make sure calls to draw are grouped very well
-- Column major matrixes
-- VAOs
+- Implement jump as a  non instant pulse
-- Lookup proper initialization of array of objects
-- Particle system
-- Get number of lines to draw something down to (1)
-- Consider locking downview at like 30 degrees
-- Abstract base classes for levels and or objects
-- Do something with misc function placement
-- Make a physics function/loop
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////// PROTO FUNCTIONS ///////////
////////////////////////////////////////////////////////////////
void checkMouse();
void window_size_callback(GLFWwindow* windowP, int width, int height);
void scroll_callback(GLFWwindow* windowP, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* windowP, int button, int action, int mods);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////// CONSTANTS & VARIABLES ///////////
////////////////////////////////////////////////////////////////
// UNLIKELY TO KEEP(HERE)// TEMP VARS ////
///////////////////////////////////////////////
vec4VBO lines;
float spread = 1.05f;

vector4 parallelLightSource;//

WorldObject testSlime;
WorldObject testSlimeTwo;
WorldObject testError;
WorldObject testCube;
WorldObject testBoulder;
std::vector<WorldObject> testSpheres;

bool tbool3 = false;
bool tbool2 = true;
bool tbool = false;

int countTemp = 0;
int mycount = 0;

double gravity = 1.3;
float camVelocity = 0.0f;
bool camIsJumping = false;
double jumpTime = 0.0;
float camEyeHeight = 2.25f;
////////////////////////////////////////////////
//// PERMANENT VARS /////
////////////////////////////////////////////////
vector<WorldObject*> objectList;
vector<WorldObject*> drawList;

//Camera and Screen Properties
Camera camera;
float FoV = 56.0f;//degrees (vert)
float HorzFoV = 0.0f;// a function of FoV(vert) and screen dims
float degreesPerPixelVert = 1.0f;
float degreesPerPixelHorz = 1.0f;//fov relation useful for mouse movement

//Shaders
Shader pvOnlyShader; // could hold shaders elsewhere
Shader pvSolidColorShader;
Shader modelTexShader;
Shader modelTexLitShader;

//Misc Important
Terrain ground; 
Skybox sky;

bool paused = false;
double prevTime = 0.0;
double deltaT = 0.0;
float dotCullLimit = 0.0f;

struct windowData{ //Data for a window, width, height,  and mouse coords etc
	int width; int height;
	double mouseX;
	double mouseY;
	GLFWwindow* primaryWindow;
}window;
GLFWmonitor* myMonitor = nullptr;




static bool inFrame(vector3  objectpos){// ushort precision 0, 1, 2 considering multiple angles cull dist etc
	vector3 objToCam = (objectpos - camera.cameraPosition);
	objToCam.normalize();
	float dot = objToCam * camera.lookDirection;
	return dot < dotCullLimit;
}

//apply gravity and bounce if neccessary
static void moveGravity(WorldObject &WorldObjVar){
	float yIntercept = 0.0f;
	yIntercept = ground.getLocalHeight(WorldObjVar.modelMat[3], WorldObjVar.modelMat[11]);
	WorldObjVar.velocity[1] -= (float)(gravity *deltaT * 8.0);
	WorldObjVar.modelMat[7] += WorldObjVar.velocity[1] * deltaT;
	if (WorldObjVar.modelMat[7] < yIntercept){
		WorldObjVar.velocity[1] *= -1.0f;// -WorldObjVar.velocity[1]; //;
		WorldObjVar.modelMat[7] = yIntercept;
	}
}

//move the character (camera) also sets cam to update
static void moveMainChar(){
	if (!tbool2)//need to make all this velocity relative
		// need to make sure you cant stand on anything too steep
		camera.moveCameraUnbound(7.10*deltaT);//fly
	if (tbool2){
		camera.moveCamera(7.10 * deltaT);//bound XZ
		float yIntercept = ground.getLocalHeight(camera.cameraPosition.coords[0], camera.cameraPosition.coords[2]) + camEyeHeight;//poss return the normal collided with

		if (camIsJumping){
			float yMove;
			//camVelocity -= deltaT * gravity * 1.0f;
			double timeSinceJump = prevTime - jumpTime;
			if ((timeSinceJump) < 0.18){ //0.18s // prevtime is effectively currenttime
				camVelocity = (timeSinceJump)* 33.0;
				if (camVelocity < 0.0) camVelocity = -camVelocity;
			}//pulsing
			else{
				camVelocity -= deltaT * gravity * 8.0f;
			}//gravity is in control

			yMove = deltaT * camVelocity;
			camera.translateVert(yMove);

			if (yIntercept > camera.cameraPosition[1] && timeSinceJump > 0.18){ //moved below the ground // consider jumptime otherwise its imposs to jump uphill
				camera.translateVertTo(yIntercept);
				camIsJumping = false;
			}
			//else{//still above the ground

			//}
		}//was jumping
		else camera.translateVertTo(yIntercept);
	}//bound movement
	camera.update();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void display(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	deltaT = glfwGetTime() - prevTime;
	//getTime ~ deltaT + prevTime
	//newPrevTime = getTime
	prevTime = deltaT + prevTime;//dont have to call getTime a second time, should be very close to true maybe better


	glfwPollEvents();
	if (!paused)checkMouse();

	//testing
	sky.drawSB(camera.cameraPosition);

	moveMainChar();

	//if (tbool3){//damped? // could add a booling ifFalling
		
	for(auto &WorldObjVar : testSpheres){
		moveGravity(WorldObjVar);
	}


	//DRAW:
	//TODO: Organize by shader used etc
	if (tbool)
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);///!!!//TEMP//!!!///
	glBindSampler(0, pvOnlyShader.sampler);
	ground.draw(&pvOnlyShader.theProgram);
	if (tbool)
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);///!!!//TEMP//!!!///
	
	//draw temp
	glLineWidth(2.3f);
	glUseProgram(pvSolidColorShader.theProgram);
	glEnableVertexAttribArray(0); // position
	glBindBuffer(GL_ARRAY_BUFFER, lines.VERT_BUFF_ID);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glDrawArrays(GL_LINES, 0, lines.verticies.size());
	glUseProgram(0);
	glLineWidth(1.0f);


	//draw worldobjects:
	//rotate one of the slimes
	countTemp++; countTemp %= 360;
	testSlime.modelMat[3] = 5 * cos(countTemp*radiansPerDegree);
	testSlime.modelMat[11] = 5 * sin(countTemp*radiansPerDegree);
	testSlime.modelMat = testSlime.modelMat * matrix4::makeRotateYaxis(1.0f);
	matrix4 pvmMat = camera.PV_Matrix * (testSlime.modelMat);
	

	testSlime.bindProgram(modelTexLitShader);

	glUniformMatrix4fv(modelTexLitShader.pvmUniformLoc, 1, GL_TRUE, pvmMat.matrix);
	glUniformMatrix4fv(modelTexLitShader.mUniformLoc, 1, GL_TRUE, testSlime.modelMat.matrix);//this isnt copying is it? it doesnt need to.
	testSlime.bindTexture();///
	testSlime.drawBound();

	testSlimeTwo.modelMat = testSlimeTwo.modelMat * matrix4::makeRotateYaxis(-1.0f);
	pvmMat = camera.PV_Matrix * (testSlimeTwo.modelMat);
	glUniformMatrix4fv(modelTexLitShader.pvmUniformLoc, 1, GL_TRUE, pvmMat.matrix);
	glUniformMatrix4fv(modelTexLitShader.mUniformLoc, 1, GL_TRUE, testSlimeTwo.modelMat.matrix);
	testSlimeTwo.drawBound();

	//new shader
	testError.bindProgram(modelTexShader);
	pvmMat = camera.PV_Matrix * testError.modelMat;
	glUniformMatrix4fv(modelTexShader.pvmUniformLoc, 1, GL_TRUE, pvmMat.matrix);
	testError.drawBound();

	mycount = 0;
	testSpheres[0].bindTexture();
	for (WorldObject &sphere : testSpheres){
		if (inFrame(sphere.position())){
			mycount++;
		pvmMat = camera.PV_Matrix * sphere.modelMat;
		glUniformMatrix4fv(modelTexShader.pvmUniformLoc, 1, GL_TRUE, pvmMat.matrix);
		sphere.drawBound();
		}
	}

	testCube.bindTexture();
	pvmMat = camera.PV_Matrix * testCube.modelMat;
	glUniformMatrix4fv(modelTexShader.pvmUniformLoc, 1, GL_TRUE, pvmMat.matrix);
	testCube.drawBound();

	testBoulder.bindTexture();
	pvmMat = camera.PV_Matrix * testBoulder.modelMat;
	glUniformMatrix4fv(modelTexShader.pvmUniformLoc, 1, GL_TRUE, pvmMat.matrix);
	testBoulder.drawBound();

	//put these somewhere better:
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

	glUseProgram(0);


	glFlush();
	glfwSwapBuffers(window.primaryWindow);
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////   CALLBACKS   //////////////
////////////////////////////////////////////////////////////////
static void key_callback(GLFWwindow* windowP, int key, int scancode, int action, int mods){

	if (action == 1){//press 

		switch (key){
		case 32://space
			/*
			if (mods == 0x0001){//shift
				camera.cameraPosition[1] -= 0.5f;
			}
			else{
				camera.cameraPosition[1] += 0.5f;
			}*/
			if (!camIsJumping){
			//	camVelocity = 5.0f;
				camIsJumping = true;
				jumpTime = glfwGetTime();
			}



	//Movekeys:
			{ 
			break;
		case 65://a
			camera.keyDownCode += 1;
			break;
		case 68://d
			camera.keyDownCode += 2;
			break;
		case 87://w
			camera.keyDownCode += 4;
			break;
		case 83://s
			camera.keyDownCode += 8;
			break;
			}
	//Misc keys:
		case 69://e
			break;
		case 81://q
			//cout <<  ground.getLocalHeight(camera.cameraPosition.coords[0], camera.cameraPosition.coords[2]) << endl;
			break;
		case 256://esc
			if (paused) { glfwSetInputMode(window.primaryWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN); }
			else { glfwSetInputMode(window.primaryWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL); }
			paused = !paused;
			break;
		//# keys
		case 49:// 1
			tbool = !tbool;
			break;
		case 50:// 2
			tbool2 = !tbool2;
			break;
		case 51:// 3
			tbool3 = !tbool3;
			if (tbool3) glEnable(GL_MULTISAMPLE);
			else glDisable(GL_MULTISAMPLE);
			break;
		default:
			cout << key << endl;
			break;
		}//switch

	}//key was pressed >down<


	else if (action == 0){//release

		switch (key){

		case 65://a
			camera.keyDownCode -= 1;
			break;
		case 68://d
			camera.keyDownCode -= 2;
			break;
		case 87://w
			camera.keyDownCode -= 4;
			break;
		case 83://s
			camera.keyDownCode -= 8;
			break;
		default:{};
		}//switch

	}//key was released
}//keycall

static void checkMouse(){
	glfwGetCursorPos(window.primaryWindow, &window.mouseX, &window.mouseY);//relative top left
	if (window.mouseX != window.width * 0.50 || window.mouseY != window.height * 0.50){
		double deltaX = window.width*0.50 - window.mouseX;
		double deltaY = window.height*0.50 - window.mouseY;
		glfwSetCursorPos(window.primaryWindow, window.width * 0.50, window.height * 0.50);
		//rotation fxns take degrees, then convert in matrix_.h
		camera.rotateCameraHorz(float(deltaX) * degreesPerPixelHorz);
		camera.rotateCameraVert(float(deltaY) * degreesPerPixelVert);
	}
}

void mouse_button_callback(GLFWwindow* windowP, int button, int action, int mods){
	if (button == 0 && action == 1){
		vector3 camToSlime = -testSlimeTwo.position() + camera.cameraPosition;//flip
		camToSlime.normalize();
		cout << "camToSlime X: " << camToSlime[0] << "Y: " << camToSlime[1] << "Z: " << camToSlime[2] << endl;
		cout << "cameraLook X: " << camera.lookDirection[0] << "Y: " << camera.lookDirection[1] << "Z: " << camera.lookDirection[2] << endl;
		float dotProd = camToSlime * camera.lookDirection;
		cout << "Dot: " << dotProd << endl;
		cout << "Angle: " << acosf(dotProd) * degreesPerRadian << '\n' << endl;

		/*
				vector3 perperp = camera.lookDirection % camera.perpToLookDir;
				for (unsigned int i = 0; i < 13; i++){
				lines.add(camera.cameraPosition + (camera.lookDirection * -1.5f));
				lines.add((camera.cameraPosition + (camera.lookDirection * -35.0f))
				//+ camera.perpToLookDir*(cosf(i / 13.0f*6.283f) * spread)
				//+ perperp*(sinf(i / 13.0f*6.283f) * spread));
				+ camera.perpToLookDir*(spread - 2*rand()*spread / RAND_MAX)
				+ perperp*(spread - 2*rand()*spread / RAND_MAX));
				//+ vector3(2 * spread - rand()*spread / RAND_MAX,
				//2 * spread - rand()*spread / RAND_MAX,
				//2 * spread - rand()*spread / RAND_MAX));
				}
				lines.regenVertBuffer();
				}
				if (button == 1 && action == 1){
				lines.verticies.clear();
				lines.regenVertBuffer();
				}
				*/
	}
	if (button == 1 && action == 1){
		cout << mycount << endl;
	}
}

void scroll_callback(GLFWwindow* windowP, double xpos, double ypos){
	//gravity += ypos * 0.1; 
	//cout << spread << endl;
	//parallelLightSource[3] += (float)ypos*0.1f;
	//if (parallelLightSource[3] < 0.0f) parallelLightSource[3] = 0.0f;
	//calc in clip space:
	parallelLightSource[3] += (float)ypos*0.1f;
		if (parallelLightSource[3] < 0.0f) parallelLightSource[3] = 0.0f;
		Shader::updateUPO_light(parallelLightSource);
	
}

static void window_size_callback(GLFWwindow* windowP, int widthNew, int heightNew){
	window.width = widthNew;
	window.height = heightNew;
	glViewport(0, 0, widthNew, heightNew);

	camera.makeProjectionMatrix(FoV, (float)window.width / window.height, 0.1f, 100.0f);
	
	

	/////Necc. updates:
	//for mouse movement:
	degreesPerPixelVert = (FoV / window.height);
	float d = (window.height / 2.0f) / tanf(FoV*radiansPerDegree / 2);
	float halfHorzFoV = atan((window.width / 2.0f) / d)*degreesPerRadian;
	HorzFoV = 2 * halfHorzFoV;//saving
	degreesPerPixelHorz = halfHorzFoV / (window.width / 2.0f); 
	//For view culling:
	dotCullLimit = -cosf(HorzFoV*radiansPerDegree / 2);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////  SETUP & EXIT  //////////////
////////////////////////////////////////////////////////////////

void initialize(){
////////Shaders///////////////
	//init skybox(first b/c it owns a Shader)
	Skybox::skyboxInit();
	sky = Skybox(std::string("CloudySky"));

	//Create Shaders
	pvOnlyShader.InitializeProgram("Shaders/primary.vert", "Shaders/primary.frag");//uses only PV from UBO and texture
	modelTexShader.InitializeProgram("Shaders/modelTex.vert", "Shaders/modelTex.frag");//uses mpv and texture
	modelTexLitShader.InitializeProgram("Shaders/modelTexLit.vert", "Shaders/modelTexLit.frag");//uses mpv and texture and modelmat
	pvSolidColorShader.InitializeProgram("Shaders/solidColor.vert", "Shaders/solidColor.frag");//uses only pv from UBO and is solid red
	//Create Sampler(s)
	pvOnlyShader.sampler = glGetUniformLocation(pvOnlyShader.theProgram, "diffuseSampler");
	//glSamplerParameteri(pvOnlyShader.sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glSamplerParameteri(pvOnlyShader.sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glSamplerParameteri(pvOnlyShader.sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(pvOnlyShader.sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glSamplerParameteri(pvOnlyShader.sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glSamplerParameteri(pvOnlyShader.sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glSamplerParameterf(pvOnlyShader.sampler, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.0f);
	//glProgramUniform1i(pvOnlyShader.theProgram, pvOnlyShader.sampler, 0);
	modelTexShader.sampler = glGetUniformLocation(modelTexShader.theProgram, "diffuseSampler");
	glSamplerParameteri(modelTexShader.sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);//minmaps are minification only
	glSamplerParameteri(modelTexShader.sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glSamplerParameteri(modelTexShader.sampler, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glSamplerParameteri(modelTexShader.sampler, GL_TEXTURE_WRAP_T, GL_CLAMP);
	//glProgramUniform1i(modelTexShader.theProgram, modelTexShader.sampler, 0);
	modelTexLitShader.sampler = modelTexShader.sampler;

	//Uniforms (Global UBO and misc.)
	//Shader::setGlobalPV_UBO(&pvOnlyShader);
	//remember that everything which uses the UBO needs tipped off to its index
	Shader::setGlobalPV_UBO(&pvOnlyShader);
	glUniformBlockBinding(pvSolidColorShader.theProgram, Shader::pv_UBO_index, 1);
	glUniformBlockBinding(modelTexShader.theProgram, Shader::pv_UBO_index, 1);
	glUniformBlockBinding(modelTexLitShader.theProgram, Shader::pv_UBO_index, 1);
	glUniformBlockBinding(Skybox::cubemapShader.theProgram, Shader::pv_UBO_index, 1);

	modelTexShader.pvmUniformLoc = glGetUniformLocation(modelTexShader.theProgram, "pvm");

	vector3 parallelLightDir = vector3(1.1f, 1.0f, 0.4f);
	parallelLightDir.normalize();
	parallelLightSource = vector4(parallelLightDir, 2.0f);
	Shader::updateUPO_light(parallelLightSource);
	modelTexLitShader.pvmUniformLoc = glGetUniformLocation(modelTexLitShader.theProgram, "pvm");
	modelTexLitShader.mUniformLoc = glGetUniformLocation(modelTexLitShader.theProgram, "m");

////Camera///////////////////
	camera = Camera(20.2f, -0.1f, 0.7f, 0.0f, 6.0f, 0.0f);
	camera.makeProjectionMatrix(FoV, (float)window.width / window.height, 0.1f, 100.0f);
	camera.update();
	degreesPerPixelVert = (FoV/window.height);
	float d = (window.height / 2.0f) / tanf(FoV*radiansPerDegree / 2); 
	float halfHorzFoV = atan((window.width / 2.0f) / d)*degreesPerRadian; 
	HorzFoV = 2 * halfHorzFoV;
	degreesPerPixelHorz = halfHorzFoV / (window.width / 2.0f);
	dotCullLimit = -cosf(HorzFoV*radiansPerDegree / 2);

////Terrain//////////////////
//	ground.loadTerrain("Textures/Terrain/testTerrainFlat.tga", 32.0f, 32.0f, 3.5f, 16.0f);
//	ground.loadTerrain("Textures/Terrain/terrainDiag.tga", 64.0f, 64.0f, 3.5f, 16.0f);
	ground.loadTerrain("Textures/Terrain/testSine.tga", 128.0f, 128.0f, 4.5f, 16.0f);
	ground.texture = textureLib::fetchTexture(resource::GRASS);

///TEMP/////////
	testSlime = WorldObject(resource::SLIME);
	testSlimeTwo = WorldObject(resource::SLIME);
	testError = WorldObject(resource::ROCK, resource::SPHERE); testError.modelMat[3] += 5.0f; testError.modelMat[11] -= 5.0f;
	for(int i=0; i<10; i++){
		testSpheres.push_back(WorldObject(resource::SPHERE, resource::ROCK));
		testSpheres[i].modelMat[3] -= (30.0f - 2 * rand()*30.0f / RAND_MAX);
		testSpheres[i].modelMat[11] += (30.0f - 2 * rand()*30.0f / RAND_MAX);
		testSpheres[i].modelMat[7] = ( rand()*15.0f / RAND_MAX) + 7.0f;
	}
	
	testCube = WorldObject(CUBE_FANCY, resource::DEFAULT);
	testCube.modelMat[3] = 5.0f;
	testCube.modelMat[7] = 5.0f;
	testCube.modelMat[11] = 5.0f;

	testBoulder = WorldObject(resource::BOULDER, resource::ROCK_ARID);
	testBoulder.modelMat[3] = testBoulder.modelMat[11] = 10.0f;
	testBoulder.modelMat[7] = ground.getLocalHeight(10.0f, 10.0f) - 0.25f;

	lines.genVertBuffer();


	deltaT = glfwGetTime() - prevTime;
	//getTime ~ deltaT + prevTime
	//newPrevTime = getTime
	prevTime = deltaT + prevTime;
}

void exit(){
	glDeleteShader(pvOnlyShader.theProgram);
	glDeleteShader(pvSolidColorShader.theProgram);
	glDeleteShader(modelTexShader.theProgram);
	glDeleteShader(modelTexLitShader.theProgram);
	glDeleteSamplers(1, &pvOnlyShader.sampler);

	ground.surfaceMesh.cleanup();

	textureLib::cleanAll();
	modelLib::cleanAll();
}
int main(int argc, char **argv){


	if (!glfwInit())
		exit();//EXIT_FAILURE);

	//glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);//does anything?
	//glfwSetErrorCallback(errorCB);

	myMonitor = glfwGetPrimaryMonitor();
	glfwWindowHint(GLFW_SAMPLES, 4);
	GLFWwindow* windowGLFW = glfwCreateWindow(1280, 720, "Open World", NULL, NULL);
	//GLFWwindow* windowGLFW = glfwCreateWindow(1600, 900, "FRUSTRUM", myMonitor, NULL);//fullscreen mode, change view below
	glfwMakeContextCurrent(windowGLFW);

	if (!windowGLFW){
		exit();
	}



	GLenum err = glewInit();
	if (GLEW_OK != err){
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));

	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//// REGISTER CALLBACKS //////////////
	////////////////////////////////////////////////////////////////
	glfwSetKeyCallback(windowGLFW, key_callback);
	glfwSetWindowSizeCallback(windowGLFW, window_size_callback);
	glfwSetScrollCallback(windowGLFW, scroll_callback);
	glfwSetMouseButtonCallback(windowGLFW, mouse_button_callback);



	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//// IMPORTANT SETS //////////////////
	////////////////////////////////////////////////////////////////
	window.primaryWindow = windowGLFW; // is this best way to use the pointer?
	window.width = 1280; window.height = 720;
	glViewport(0, 0, 1280, 720);

	glfwSetInputMode(windowGLFW, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	glfwSetCursorPos(window.primaryWindow, window.width * 0.50, window.height * 0.50);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	glfwSwapInterval(1);

	initialize();
	while (!glfwWindowShouldClose(window.primaryWindow))
	{

		display();


	}




}