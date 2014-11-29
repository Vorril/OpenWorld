#include <iostream>
#include <GL/glew.h>//using the d library nd dll
#include <GL/glfw3.h> // includes windows.h

//My classes:
#include <Shader.h>
#include <World.h> //hold the state of the world, display lists etc, reduce as much as possible from living in main
#include <Camera.h> //includes mats and shader
#include <Terrain.h> //should probably moce to world

/*
TODO
- Implement model matrices//(re)consider what multiplications to do on the GPU
- Implement movement on terrain 
- Consider terrain view culling
- Properly set build settings
- Make sure calls to draw are grouped very well

*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////// PROTO FUNCTIONS ///////////
////////////////////////////////////////////////////////////////
void checkMouse();
void window_size_callback(GLFWwindow* windowP, int width, int height);
void scroll_callback(GLFWwindow* windowP, double xpos, double ypos);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////// CONSTANTS & VARIABLES ///////////
////////////////////////////////////////////////////////////////
// UNLIKELY TO KEEP(HERE)// TEMP VARS ////
///////////////////////////////////////////////
float FoV = 56.0f;//degrees
float degreesPerPixelVert = 1.0f;
float degreesPerPixelHorz = 1.0f;//fov relation useful for mouse movement

WorldObject testSlime;
WorldObject testSlimeTwo;

bool tempbool = false;
////////////////////////////////////////////////
//// PERMANENT VARS /////
////////////////////////////////////////////////
Camera camera;
Shader pvOnlyShader; // could hold shaders elsewhere
Shader modelTexShader;
Terrain ground; 

bool paused = false;
double prevTime = 0.0;
double deltaT = 0.0;

int countTemp = 0;

struct windowData{ //Data for a window, width, height,  and mouse coords etc
	int width; int height;
	double mouseX;
	double mouseY;
	GLFWwindow* primaryWindow;
}window;
GLFWmonitor* myMonitor = nullptr;



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void display(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	deltaT = glfwGetTime() - prevTime;
	//getTime ~ deltaT + prevTime
	//newPrevTime = getTime
	prevTime = deltaT + prevTime;//dont have to call getTime a second time, should be very close to true maybe better


	glfwPollEvents();
	if (!paused)checkMouse();

	camera.moveCameraUnbound(7.10f*deltaT);
	camera.update();


	//DRAW:
	//TODO: Organize by shader used etc
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);///!!!//TEMP//!!!///
	glBindSampler(0, pvOnlyShader.sampler);
	ground.draw(&pvOnlyShader.theProgram);
	
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);///!!!//TEMP//!!!///
	
	
	//draw worldobjects
	countTemp++; countTemp %= 360;
	testSlime.modelMat[3] = 5 * cos(countTemp*radiansPerDegree);
	testSlime.modelMat[11] = 5 * sin(countTemp*radiansPerDegree);
	testSlime.modelMat = testSlime.modelMat * matrix4::makeRotateYaxis(1.0f);
	matrix4 pvmMat = camera.PV_Matrix * (testSlime.modelMat);
	
	
	glUseProgram(modelTexShader.theProgram);
	glBindSampler(0, modelTexShader.sampler);
	glUniformMatrix4fv(modelTexShader.pvmUniformLoc, 1, GL_TRUE, pvmMat.matrix);
	

	glEnableVertexAttribArray(0); // position
	glEnableVertexAttribArray(1); // uv tex coords

	testSlime.drawBound();

	testSlimeTwo.modelMat = testSlimeTwo.modelMat * matrix4::makeRotateYaxis(-1.0f);
	pvmMat = camera.PV_Matrix * (testSlimeTwo.modelMat);
	glUniformMatrix4fv(modelTexShader.pvmUniformLoc, 1, GL_TRUE, pvmMat.matrix);

	testSlimeTwo.drawBound();

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

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
			if (mods == 0x0001){//shift
				camera.cameraPosition[1] -= 0.5f;
			}
			else{
				camera.cameraPosition[1] += 0.5f;
			}
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
		case 256://esc
			if (paused) { glfwSetInputMode(window.primaryWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN); }
			else { glfwSetInputMode(window.primaryWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL); }
			paused = !paused;
		case 81://q
			tempbool = !tempbool;
			if (tempbool) glDisable(GL_MULTISAMPLE);
			else glEnable(GL_MULTISAMPLE);
			break;
		default:
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

static void window_size_callback(GLFWwindow* windowP, int widthNew, int heightNew){
	window.width = widthNew;
	window.height = heightNew;
	glViewport(0, 0, widthNew, heightNew);

	camera.makeProjectionMatrix(FoV, (float)window.width / window.height, 0.1f, 100.0f);
	
	degreesPerPixelVert = (FoV / window.height);
	float d = (window.height / 2.0f) / tanf(FoV*radiansPerDegree / 2);
	float halfHorzFoV = atan((window.width / 2.0f) / d)*degreesPerRadian;
	degreesPerPixelHorz = halfHorzFoV / (window.width / 2.0f);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////  SETUP & EXIT  //////////////
////////////////////////////////////////////////////////////////

void initialize(){
////////Shaders///////////////
	//Create Shaders
	pvOnlyShader.InitializeProgram("Shaders/primary.vert", "Shaders/primary.frag");//uses only PV from UBO and texture
	modelTexShader.InitializeProgram("Shaders/modelTex.vert", "Shaders/modelTex.frag");//uses mpv and texture
	//Create Sampler(s)
	pvOnlyShader.sampler = glGetUniformLocation(pvOnlyShader.theProgram, "diffuseSampler");
	glSamplerParameteri(pvOnlyShader.sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glSamplerParameteri(pvOnlyShader.sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glSamplerParameteri(pvOnlyShader.sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glSamplerParameteri(pvOnlyShader.sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glProgramUniform1i(pvOnlyShader.theProgram, pvOnlyShader.sampler, 0);
	modelTexShader.sampler = glGetUniformLocation(modelTexShader.theProgram, "diffuseSampler");
	glSamplerParameteri(modelTexShader.sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);//minmaps are minification only
	glSamplerParameteri(modelTexShader.sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glSamplerParameteri(modelTexShader.sampler, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glSamplerParameteri(modelTexShader.sampler, GL_TEXTURE_WRAP_T, GL_CLAMP);
	//glProgramUniform1i(modelTexShader.theProgram, modelTexShader.sampler, 0);

	//Uniforms (Global UBO and misc.)
	Shader::setGlobalPV_UBO(&pvOnlyShader);
	modelTexShader.pvmUniformLoc = glGetUniformLocation(modelTexShader.theProgram, "pvm");

////Camera///////////////////
	camera = Camera(1.2f, -0.6f, 5.5f, 0.0f, 7.0f);
	camera.makeProjectionMatrix(FoV, (float)window.width / window.height, 0.1f, 100.0f);
	camera.update();
	degreesPerPixelVert = (FoV/window.height);
	float d = (window.height / 2.0f) / tanf(FoV*radiansPerDegree / 2); 
	float halfHorzFoV = atan((window.width / 2.0f) / d)*degreesPerRadian; 
	degreesPerPixelHorz = halfHorzFoV / (window.width / 2.0f);

////Terrain//////////////////
	ground.loadTerrain("Textures/Terrain/testTerrain3.tga", 32.0f, 32.0f, 3.5f, 16.0f);
	ground.texture = textureLib::fetchTexture(resource::NULL_ENUM);
	glBindTexture(GL_TEXTURE_2D, ground.texture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);

///TEMP/////////
	testSlime = WorldObject(resource::SLIME);
	testSlimeTwo = WorldObject(resource::SLIME);
}

void exit(){
	glDeleteShader(pvOnlyShader.theProgram);
	glDeleteShader(modelTexShader.theProgram);
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
	//glfwSetScrollCallback(windowGLFW, scroll_callback);
	//glfwSetMouseButtonCallback(windowGLFW, mouse_button_callback);



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

	glfwSwapInterval(1);

	initialize();
	while (!glfwWindowShouldClose(window.primaryWindow))
	{

		display();


	}




}