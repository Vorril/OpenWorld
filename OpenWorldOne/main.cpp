#include <iostream>
#include <GL/glew.h>//using the d library nd dll
#include <GL/glfw3.h> // includes windows.h
#include <math.h>

//My classes:
#include <Shader.h>
#include <World.h> //hold the state of the world, display lists etc, reduce as much as possible from living in main
#include <Camera.h> //includes mats and shader
#include <Terrain.h> //should probably moce to world

/*
TODO
+- Implement model matrices//(re)consider what multiplications to do on the GPU
++ Implement movement on terrain 
-- Consider terrain view culling
++ Properly set build settings
-- Make sure calls to draw are grouped very well
-- Column major matrixes
-- VAOs
-- Gravity
-- Implement jump as a  non instant pulse

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

WorldObject testSlime;
WorldObject testSlimeTwo;
WorldObject testError;
WorldObject testSphere;

bool tbool3 = false;
bool tbool2 = false;
bool tbool = false;

int countTemp = 0;

float gravity = 1.0f;
float camVelocity = 0.0f;
bool camIsJumping = false;
double jumpTime = 0.0;
////////////////////////////////////////////////
//// PERMANENT VARS /////
////////////////////////////////////////////////

//Camera and Screen Properties
Camera camera;
float FoV = 56.0f;//degrees
float degreesPerPixelVert = 1.0f;
float degreesPerPixelHorz = 1.0f;//fov relation useful for mouse movement

//Shaders
Shader pvOnlyShader; // could hold shaders elsewhere
Shader pvSolidColorShader;
Shader modelTexShader;

//Misc Important
Terrain ground; 

bool paused = false;
double prevTime = 0.0;
double deltaT = 0.0;


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

	if (!tbool2)//need to make all this velocity relative
		//also consider another way of pulling out eyeHeight its too complicated
	camera.moveCameraUnbound(7.10*deltaT);//fly
	if (tbool2){
		camera.moveCamera(7.10 * deltaT);//bound XZ
		float yIntercept  = ground.getLocalHeight(camera.cameraPosition.coords[0], camera.cameraPosition.coords[2])+ 0.75f;//clean
		
		if (camIsJumping){
			float yMove;
			//camVelocity -= deltaT * gravity * 1.0f;
			if ((prevTime - jumpTime) < 0.18){ //0.18s // prevtime is effectively currenttime
				camVelocity = (prevTime- jumpTime) * 33.0;
			}//pulsing
			else{
				camVelocity -= deltaT * gravity * 8.0f;
			}//gravity is in control

			yMove = deltaT * camVelocity;

			if (yIntercept > camera.cameraPosition[1] + yMove + 0.75f){ //moved below the ground // 0.75f is eyeHeight
				camera.translateVertTo(yIntercept);
				camIsJumping = false;
			}
			else{//still above the ground

			}
		}//was jumping
		else camera.translateVertTo(yIntercept);
	}//bound movement
	camera.update();


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
	
	//initialize pvm shader
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

	pvmMat = camera.PV_Matrix * testError.modelMat;
	glUniformMatrix4fv(modelTexShader.pvmUniformLoc, 1, GL_TRUE, pvmMat.matrix);
	testError.drawBound();

	pvmMat = camera.PV_Matrix * testSphere.modelMat;
	glUniformMatrix4fv(modelTexShader.pvmUniformLoc, 1, GL_TRUE, pvmMat.matrix);
	testSphere.drawBound();



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

}

void scroll_callback(GLFWwindow* windowP, double xpos, double ypos){
	spread += (float)ypos * 0.15f; 
	//cout << spread << endl;
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
	pvSolidColorShader.InitializeProgram("Shaders/solidColor.vert", "Shaders/solidColor.frag");//uses only pv from UBO and is solid red
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
	//Shader::setGlobalPV_UBO(&pvOnlyShader);
	//remember that everything which uses the UBO needs tipped off to its index
	Shader::setGlobalPV_UBO(&pvOnlyShader);
	glUniformBlockBinding(pvSolidColorShader.theProgram, Shader::pv_UBO_index, 1);
	glUniformBlockBinding(modelTexShader.theProgram, Shader::pv_UBO_index, 1);
	modelTexShader.pvmUniformLoc = glGetUniformLocation(modelTexShader.theProgram, "pvm");

////Camera///////////////////
	camera = Camera(1.2f, -0.6f, 2.5f, 5.0f, 7.0f, 12.0f);
	camera.makeProjectionMatrix(FoV, (float)window.width / window.height, 0.1f, 100.0f);
	camera.update();
	degreesPerPixelVert = (FoV/window.height);
	float d = (window.height / 2.0f) / tanf(FoV*radiansPerDegree / 2); 
	float halfHorzFoV = atan((window.width / 2.0f) / d)*degreesPerRadian; 
	degreesPerPixelHorz = halfHorzFoV / (window.width / 2.0f);

////Terrain//////////////////
//	ground.loadTerrain("Textures/Terrain/testTerrainFlat.tga", 32.0f, 32.0f, 3.5f, 16.0f);
	ground.loadTerrain("Textures/Terrain/terrainLarge.tga", 80.0f, 80.0f, 5.5f, 16.0f);
	ground.texture = textureLib::fetchTexture(resource::NULL_ENUM);
	glBindTexture(GL_TEXTURE_2D, ground.texture);
	glBindTexture(GL_TEXTURE_2D, 0);

///TEMP/////////
	testSlime = WorldObject(resource::SLIME);
	testSlimeTwo = WorldObject(resource::SLIME);
	testError = WorldObject(resource::ROCK, resource::SPHERE); testError.modelMat[3] += 5.0f; testError.modelMat[11] -= 5.0f;
	testSphere = WorldObject(resource::SPHERE, resource::ROCK); testSphere.modelMat[3] -= 5.0f; testSphere.modelMat[11] += 5.0f;
	lines.genVertBuffer();
}

void exit(){
	glDeleteShader(pvOnlyShader.theProgram);
	glDeleteShader(pvSolidColorShader.theProgram);
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

	glfwSwapInterval(1);

	initialize();
	while (!glfwWindowShouldClose(window.primaryWindow))
	{

		display();


	}




}