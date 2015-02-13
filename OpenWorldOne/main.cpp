#include <iostream>
#include <Dependencies/GL/glew.h>//using the d library nd dll
#include <Dependencies/GL/glfw3.h> // includes windows.h
#include <math.h>

#include <thread>

//My classes:
#include <Shader.h>
#include <World.h> //hold the state of the world, display lists etc, reduce as much as possible from living in main
#include <Camera.h> //includes mats and shader
#include <Terrain.h> //should probably moce to world
#include <Skybox.h>
#include <MainCharacter.h> // includes cam and terrain
/*
TODO
-- Consider terrain view culling
-- Make sure calls to draw are grouped very well
-- Column major matrixes
-- VAOs
-- Particle system
-- Get number of lines to draw something down to (1)
-- Consider locking downview at like 30 degrees
-- Abstract base classes for levels and or objects
-- Do something with misc function placement
-- Make a physics function/loop
-- Restrict jump movement
-- Keep pulling unecc. out of main//use world to do this >!>!
-- Just use inheritance for worldobjects but keep lib for common things
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
std::vector<WorldObject> diamonds;

bool tbool3 = false;
bool tbool2 = true;
bool tbool = false;

int countTemp = 0;
int mycount = 0;

double gravity = 1.3;

////////////////////////////////////////////////
//// PERMANENT VARS /////
////////////////////////////////////////////////
vector<WorldObject*> objectList;
vector<WorldObject*> drawList;

//Camera and Screen Properties
//Camera camera;
MainCharacter player;

//Shaders
Shader pvOnlyShader; // could hold shaders elsewhere
Shader pvSolidColorShader; //inside inherited classes
Shader pvmSolidColorShader; 
Shader modelTexShader;
Shader modelTexLitShader;

//Misc Important
TerrainMap ground; 
Skybox sky;

bool paused = false;



struct windowData{ //Data for a window, width, height,  and mouse coords etc
	int width; int height;
	double mouseX;
	double mouseY;
	GLFWwindow* primaryWindow;
	float FoV = 56.0f;//degrees (vert)//these are windows properties should put them in the struct
	float HorzFoV = 0.0f;// a function of FoV(vert) and screen dims
	float degreesPerPixelVert = 1.0f;
	float degreesPerPixelHorz = 1.0f;//fov relation useful for mouse movement
	float dotCullLimit = 0.0f;
}window;
GLFWmonitor* myMonitor = nullptr;

MainCharacter::coreTimer times; 


//Misc functions should think of a good place

static bool inFrame(vector3  objectpos){// ushort precision 0, 1, 2 considering multiple angles cull dist etc
	vector3 objToCam = (objectpos - player.position());
	objToCam.normalize();
	float dot = objToCam * player.lookDirection();
	return dot < window.dotCullLimit;
}

//apply gravity and bounce if neccessary
static void moveGravity(WorldObject &WorldObjVar){
	float yIntercept = 0.0f;
	yIntercept = ground.getLocalHeight(WorldObjVar.modelMat[3], WorldObjVar.modelMat[11]) +0.6f;
	WorldObjVar.velocity[1] -= (float)(gravity * times.deltaT * 8.0);
	WorldObjVar.modelMat[7] += WorldObjVar.velocity[1] * times.deltaT;
	if (WorldObjVar.modelMat[7] < yIntercept){
		WorldObjVar.velocity[1] *= -1.0f;// -WorldObjVar.velocity[1]; //;
		WorldObjVar.modelMat[7] = yIntercept;
	}
}

//move the character (camera) also sets cam to update

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void display(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	times.deltaT = glfwGetTime() - times.prevTime;
	//getTime ~ deltaT + prevTime
	//newPrevTime = getTime
	times.prevTime = times.deltaT + times.prevTime;//dont have to call getTime a second time, should be very close to true maybe better


	glfwPollEvents();
	if (!paused)checkMouse();

	
	//consider saving the groundmap in player, it will never change
	player.move(&times, &ground);

		
	for(auto &WorldObjVar : testSpheres){
		moveGravity(WorldObjVar);
	}


	//DRAW:
	if (tbool)
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);///!!!//TEMP//!!!/// undo is way at the bottom
	ground.draw(&player.eyeCam);
	
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
	matrix4 pvmMat = player.eyeCam.PV_Matrix * (testSlime.modelMat);
	//this needs so much cleaning^^

	testSlime.bindProgram(modelTexLitShader);

	glUniformMatrix4fv(modelTexLitShader.pvmUniformLoc, 1, GL_TRUE, pvmMat.matrix);
	glUniformMatrix4fv(modelTexLitShader.mUniformLoc, 1, GL_TRUE, testSlime.modelMat.matrix);//this isnt copying is it? it doesnt need to.
	testSlime.bindTexture();///
	testSlime.drawBound();

	testSlimeTwo.modelMat = testSlimeTwo.modelMat * matrix4::makeRotateYaxis(-1.0f);
	pvmMat = player.eyeCam.PV_Matrix * (testSlimeTwo.modelMat);
	glUniformMatrix4fv(modelTexLitShader.pvmUniformLoc, 1, GL_TRUE, pvmMat.matrix);
	glUniformMatrix4fv(modelTexLitShader.mUniformLoc, 1, GL_TRUE, testSlimeTwo.modelMat.matrix);
	testSlimeTwo.drawBound();

	testError.quickDraw();

	mycount = 0;
	testSpheres[0].bind();
	for (WorldObject &sphere : testSpheres){
		if (inFrame(sphere.position())){
			mycount++;
			sphere.draw();
		}
	}

	

	testBoulder.bind(); 
	testBoulder.draw();

	glUseProgram(pvmSolidColorShader.theProgram);
	for (WorldObject &dia : diamonds){
		pvmMat = player.eyeCam.PV_Matrix * dia.modelMat;
		glUniformMatrix4fv(pvmSolidColorShader.pvmUniformLoc, 1, GL_TRUE, pvmMat.matrix);
		glBindBuffer(GL_ARRAY_BUFFER, dia.model->VERT_BUFF_ID);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glDrawArrays(GL_TRIANGLES, 0, dia.model->verticies.size());
	}


	if (tbool)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);///!!!//TEMP//!!!///


	//Skybox last (supposed efficiency)
	glDepthFunc(GL_LEQUAL);  
	sky.drawSB(player.eyeCam.cameraPosition);
	glDepthFunc(GL_LESS); 


//put these somewhere better:
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

	glUseProgram(0);
	//glFlush();
	glfwSwapBuffers(window.primaryWindow);
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////   CALLBACKS   //////////////
////////////////////////////////////////////////////////////////
static void key_callback(GLFWwindow* windowP, int key, int scancode, int action, int mods){

	if (action == 1){//press 

		switch (key){
	//Movekeys:
		case 32://space
			if (!player.jumping){
				player.jump();
			}
			break;
			{ 	
		case 65://a
			player.eyeCam.keyDownCode += 1;
			break;
		case 68://d
			player.eyeCam.keyDownCode += 2;
			break;
		case 87://w
			player.eyeCam.keyDownCode += 4;
			break;
		case 83://s
			player.eyeCam.keyDownCode += 8;
			break; 
		case 340://s
			player.speed += 5.0f;
			break;
			}
	//Misc keys:
		case 69:{//e
			double timeStart = glfwGetTime();

			std::thread t1(&TerrainMap::loadChunk, &ground, "Textures/Terrain/hill.tga", 1, 1, resource::GRASS);
			t1.detach();

			cout << "Terrain Load: " << (glfwGetTime() - timeStart) * 1000.0 << " ms" << endl;
		}
			break;
		case 81://q
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
			player.boundMovement = !player.boundMovement;
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
			player.eyeCam.keyDownCode -= 1;
			break;
		case 68://d
			player.eyeCam.keyDownCode -= 2;
			break;
		case 87://w
			player.eyeCam.keyDownCode -= 4;
			break;
		case 83://s
			player.eyeCam.keyDownCode -= 8;
			break;
		case 340://left shift
			player.speed -= 5.0f;
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
		player.eyeCam.rotateCameraHorz(float(deltaX) * window.degreesPerPixelHorz);
		player.eyeCam.rotateCameraVert(float(deltaY) * window.degreesPerPixelVert);
	}
}

void mouse_button_callback(GLFWwindow* windowP, int button, int action, int mods){
	if (button == 0 && action == 1){
	
			vector3 perperp = player.eyeCam.lookDirection % player.eyeCam.perpToLookDir;
			for (unsigned int i = 0; i < 13; i++){
				lines.add(player.eyeCam.cameraPosition + (player.eyeCam.lookDirection * -1.5f));
				lines.add((player.eyeCam.cameraPosition + (player.eyeCam.lookDirection * -35.0f))
				//+ camera.perpToLookDir*(cosf(i / 13.0f*6.283f) * spread)
				//+ perperp*(sinf(i / 13.0f*6.283f) * spread));
				+ player.eyeCam.perpToLookDir*(spread - 2 * rand()*spread / RAND_MAX)
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
				
	
	if (button == 1 && action == 1){
		cout << mycount << endl;
	}
}

void scroll_callback(GLFWwindow* windowP, double xpos, double ypos){
	gravity += ypos * 0.1; 
	//cout << spread << endl;
	//parallelLightSource[3] += (float)ypos*0.1f;
	//if (parallelLightSource[3] < 0.0f) parallelLightSource[3] = 0.0f;
	//calc in clip space:
	//parallelLightSource[3] += (float)ypos*0.1f;
		//if (parallelLightSource[3] < 0.0f) parallelLightSource[3] = 0.0f;
		//Shader::updateUPO_light(parallelLightSource);
	
}

static void window_size_callback(GLFWwindow* windowP, int widthNew, int heightNew){
	window.width = widthNew;
	window.height = heightNew;
	glViewport(0, 0, widthNew, heightNew);

	player.eyeCam.makeProjectionMatrix(window.FoV, (float)window.width / window.height, 0.1f, 100.0f);
	
	

	/////Necc. updates:
	//for mouse movement:
	window.degreesPerPixelVert = (window.FoV / window.height);
	float d = (window.height / 2.0f) / tanf(window.FoV*radiansPerDegree / 2);
	float halfHorzFoV = atan((window.width / 2.0f) / d)*degreesPerRadian;
	window.HorzFoV = 2 * halfHorzFoV;//saving
	window.degreesPerPixelHorz = halfHorzFoV / (window.width / 2.0f);
	//For view culling:
	window.dotCullLimit = -cosf(window.HorzFoV*radiansPerDegree / 2);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////  SETUP & EXIT  //////////////
////////////////////////////////////////////////////////////////

void initialize(){
////////Shaders///////////////
	//init skybox(first b/c it owns a Shader)
	Skybox::skyboxInit();
	sky = Skybox(std::string("CloudyBlur"));

	//Create Shaders
	pvOnlyShader.InitializeProgram("Shaders/primary.vert", "Shaders/primary.frag");//uses only PV from UBO and texture//the ground shader
	modelTexShader.InitializeProgram("Shaders/modelTex.vert", "Shaders/modelTex.frag");//uses mpv and texture//most obects
	modelTexLitShader.InitializeProgram("Shaders/modelTexLit.vert", "Shaders/modelTexLit.frag");//uses mpv and texture and modelmat
	pvSolidColorShader.InitializeProgram("Shaders/solidColor.vert", "Shaders/solidColor.frag");//uses only pv from UBO and is solid red
	pvmSolidColorShader.InitializeProgram("Shaders/pvmSolidColor.vert", "Shaders/pvmSolidColor.frag");//same but uses pvm
	//Create Sampler(s)
	pvOnlyShader.sampler = glGetUniformLocation(pvOnlyShader.theProgram, "diffuseSampler");
	glSamplerParameteri(pvOnlyShader.sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(pvOnlyShader.sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glSamplerParameteri(pvOnlyShader.sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glSamplerParameteri(pvOnlyShader.sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glSamplerParameterf(pvOnlyShader.sampler, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.0f);
	
	modelTexShader.sampler = glGetUniformLocation(modelTexShader.theProgram, "diffuseSampler");
	glSamplerParameteri(modelTexShader.sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);//minmaps are minification only
	glSamplerParameteri(modelTexShader.sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glSamplerParameteri(modelTexShader.sampler, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glSamplerParameteri(modelTexShader.sampler, GL_TEXTURE_WRAP_T, GL_CLAMP);
	//glProgramUniform1i(modelTexShader.theProgram, modelTexShader.sampler, 0);
	modelTexLitShader.sampler = modelTexShader.sampler;

	////Uniforms (Global UBO and misc.)
	//remember that everything which uses the UBO needs tipped off to its index
	Shader::setGlobalPV_UBO(&pvOnlyShader);
	glUniformBlockBinding(pvSolidColorShader.theProgram, Shader::pv_UBO_index, 1);
	glUniformBlockBinding(modelTexShader.theProgram, Shader::pv_UBO_index, 1);
	glUniformBlockBinding(modelTexLitShader.theProgram, Shader::pv_UBO_index, 1);
	glUniformBlockBinding(Skybox::cubemapShader.theProgram, Shader::pv_UBO_index, 1);

	modelTexShader.pvmUniformLoc = glGetUniformLocation(modelTexShader.theProgram, "pvm");
	pvmSolidColorShader.pvmUniformLoc = glGetUniformLocation(modelTexShader.theProgram, "pvm");
	modelTexLitShader.pvmUniformLoc = glGetUniformLocation(modelTexLitShader.theProgram, "pvm");
	modelTexLitShader.mUniformLoc = glGetUniformLocation(modelTexLitShader.theProgram, "m");
	
	//light
	vector3 parallelLightDir = vector3(1.1f, 1.0f, 0.4f);
	parallelLightDir.normalize();
	parallelLightSource = vector4(parallelLightDir, 2.0f);
	Shader::updateUPO_light(parallelLightSource);

////Camera///////////////////
	player.eyeCam = Camera(20.2f, -0.1f, 0.7f, 0.0f, 6.0f, 0.0f);
	player.eyeCam.makeProjectionMatrix(window.FoV, (float)window.width / window.height, 0.1f, 100.0f);
	player.eyeCam.update();

	window.degreesPerPixelVert = (window.FoV / window.height);
	float d = (window.height / 2.0f) / tanf(window.FoV*radiansPerDegree / 2);
	float halfHorzFoV = atan((window.width / 2.0f) / d)*degreesPerRadian; 
	window.HorzFoV = 2 * halfHorzFoV;
	window.degreesPerPixelHorz = halfHorzFoV / (window.width / 2.0f);
	window.dotCullLimit = -cosf(window.HorzFoV*radiansPerDegree / 2);

////Terrain//////////////////
	ground.createConstantArrays128();//TODO maybe make this variable, probably clear the mem after GPU upload

	ground.terrainShader = &pvOnlyShader;
	ground.loadChunk("Textures/Terrain/MapChunks/tile5.tga", 0, 0, resource::GRASS);//loading 0,0 first might be safest
	std::thread t1(&TerrainMap::loadChunk, &ground, "Textures/Terrain/MapChunks/tile6.tga", 0, 1, resource::GRASS);
	t1.detach();
	std::thread t2(&TerrainMap::loadChunk, &ground, "Textures/Terrain/MapChunks/tile9.tga", 1, 0, resource::GRASS);
	t2.detach();
	std::thread t3(&TerrainMap::loadChunk, &ground, "Textures/Terrain/MapChunks/tile10.tga", 1, 1, resource::GRASS);
	t3.detach();

	/*
	if (t1.joinable())
	t1.join();
	if (t2.joinable())
	t2.join();
	if (t3.joinable())
		t3.join();
*/
	//ground.loadChunk("Textures/Terrain/MapChunks/tile7.tga", 0, 2, resource::GRASS);
	//ground.loadChunk("Textures/Terrain/MapChunks/tile11.tga", 1, 2, resource::GRASS);
	//std::thread tTrack(&TerrainMap::monitorLoading, &ground, &player.eyeCam);
	//tTrack.detach();


////Misc. sets////////
	WorldObject::basicShader = &modelTexShader;
	WorldObject::pvMat = &player.eyeCam.PV_Matrix;


////TEMP/////////
	testSlime = WorldObject(resource::SLIME); 
	testSlime.modelMat[7] = ground.getLocalHeight(testSlime.modelMat[7], testSlime.modelMat[11]);
	testSlimeTwo = WorldObject(resource::SLIME);
	testSlimeTwo.modelMat[7] = ground.getLocalHeight(testSlimeTwo.modelMat[7], testSlimeTwo.modelMat[11]);
	testError = WorldObject(resource::ROCK, resource::SPHERE); testError.modelMat[3] += 5.0f; testError.modelMat[11] -= 5.0f;
	for(int i=0; i<10; i++){
		testSpheres.push_back(WorldObject(resource::SPHERE, resource::ROCK));//doesnt call new?
		testSpheres[i].modelMat[3] -= (20.0f - 2 * rand()*20.0f / RAND_MAX);
		testSpheres[i].modelMat[11] += (20.0f - 2 * rand()*20.0f / RAND_MAX);
		testSpheres[i].modelMat[7] = ( rand()*15.0f / RAND_MAX) + 7.0f;
	}
	
	diamonds.push_back(WorldObject(resource::DIAMOND_SML));
	diamonds[0].modelMat[7] = ground.getLocalHeight(-5.0f, -5.0f);
	diamonds[0].modelMat[3] = -5.0f;
	diamonds[0].modelMat[11] = -5.0f;

	testBoulder = WorldObject(resource::BOULDER, resource::ROCK_ARID);
	testBoulder.modelMat[3] = testBoulder.modelMat[11] = 10.0f;
	testBoulder.modelMat[7] = ground.getLocalHeight(10.0f, 10.0f) - 0.25f;

	lines.genVertBuffer();


	times.deltaT = glfwGetTime() - times.prevTime;
	//getTime ~ deltaT + prevTime
	//newPrevTime = getTime
	times.prevTime = times.deltaT + times.prevTime;
}

void exit(){
	ground.keepRunning = false;//signal thread to exit

	glDeleteSamplers(1, &pvOnlyShader.sampler);
	glDeleteSamplers(1, &Skybox::cubemapShader.sampler);
	glDeleteSamplers(1, &modelTexShader.sampler);
	glDeleteShader(pvOnlyShader.theProgram);
	glDeleteShader(pvSolidColorShader.theProgram);
	glDeleteShader(modelTexShader.theProgram);
	glDeleteShader(modelTexLitShader.theProgram);
	glDeleteShader(Skybox::cubemapShader.theProgram);
	glDeleteShader(pvmSolidColorShader.theProgram);

	//ground.surfaceMesh.cleanup(); // tied into TerrainMap destructor

	Skybox::cleanAll;
	textureLib::cleanAll();
	modelLib::cleanAll();

	//maybe wait for detached threads? worth?
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
	
	cout << "GL version: " << glGetString(GL_VERSION) << endl << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

	initialize();

	while (!glfwWindowShouldClose(window.primaryWindow))
	{

		display();


	}




}