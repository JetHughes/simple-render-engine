/*
 * renderApp.cpp
 *
 * by Stefanie Zollmann
 *
 * Basic model loading with assimp library
 *
 */

 // Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include <glad/gl.h>

#define GLAD_GL_IMPLEMENTATION

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/Shader.hpp>
#include <common/Object.hpp>
#include <common/Scene.hpp>
#include <common/Triangle.hpp>
#include <common/BasicMaterialShader.hpp>
#include <common/Mesh.hpp>
#include <common/Controls.hpp>
#include <common/Group.hpp>
#include <common/Objloader.hpp>
#include <common/Quad.hpp>
#include <common/PostProcessingShader.hpp>

#include <iostream>

bool initWindow(std::string windowName)
{

	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return false;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(1024, 768, windowName.c_str(), NULL, NULL);
	if (window == NULL)
	{
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible.\n");
		getchar();
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(window);

	return true;
}

int main(int argc, char* argv[])
{

	initWindow("renderEngine Skeleton");
	glfwMakeContextCurrent(window);

	int version = gladLoadGL(glfwGetProcAddress);
	if (version == 0)
	{
		printf("Failed to initialize OpenGL context\n");
		return -1;
	}

	std::cout << "Kia ora" << std::endl;

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	// Hide the mouse and enable unlimited mouvement
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set the mouse at the center of the screen
	glfwPollEvents();
	glfwSetCursorPos(window, 1024 / 2, 768 / 2);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	// Enable transparency
	glEnable(GL_BLEND);

	// Set the blending function
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// create a Vertex Array Object and set it as the current one
	// we will not go into detail here. but this can be used to optimise the performance by storing all of the state needed to supply vertex data
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	/*
	 * Create Scene
	 * With a object and texture loaded from loader
	 */

	Scene* myScene = new Scene();

	// Read our .obj files - this is hard coded for testing - you can pass obj file names as arguments instead to make the code more flexible

	if (argc == 1)
	{
		// if nothing specified we load default objects
		// person obj
		Group* person = new Group();
		bool res = loadOBJMTL("person.obj", person);
		person->init();

		// earth obj
		Group* sphere = new Group();
		res = loadOBJMTL("earthobj.obj", sphere);
		sphere->init();

		// table obj
		Group* table = new Group();
		res = loadOBJMTL("CarvedTable.obj", table);
		table->init();

		// bottle obj
		Group* bottle = new Group();
		res = loadOBJMTL("glassBottle.obj", bottle);
		bottle->init();

		// add objects to the scene
		myScene->addObject(person);
		myScene->addObject(sphere);
		myScene->addObject(table);
		myScene->addObject(bottle);
	}
	else
	{

		for (int a = 1; a < argc; ++a)
		{
			Group* objGroup = new Group();
			bool res = loadOBJMTL(argv[a], objGroup);
			if (res)
			{
				objGroup->init();
				// add objects to the scene
				myScene->addObject(objGroup);
			}
		}
	}

	Camera* myCamera = new Camera();
	myCamera->setPosition(glm::vec3(0, 100, 200)); // set camera to show the models
	Controls* myControls = new Controls(myCamera);
	myControls->setSpeed(30);

	// ***************************************
	// Render to texture stuff
	int windowWidth = 1024;
	int windowHeight = 768;
	glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
	// The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
	GLuint RenderToTextureFrameBuffer = 0;
	glGenFramebuffers(1, &RenderToTextureFrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, RenderToTextureFrameBuffer);
	// The texture we're going to render to
	GLuint renderedTexture;
	glGenTextures(1, &renderedTexture);
	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, renderedTexture);
	// Give an empty image to OpenGL ( the last "0" means "empty" )
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	// Poor filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// We also need a depth buffer
	GLuint depthrenderbuffer;
	glGenRenderbuffers(1, &depthrenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1024, 768);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);
	// Set "renderedTexture" as our colour attachement #0
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0);
	// Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return false;
	// the quad that we use to render the framebuffer texture to the screen
	Quad* outputQuad = new Quad();
	PostProcessingShader* postEffectShader = new PostProcessingShader("Passthrough.vert", "PostEffect.frag");
	// End Render to texture
	// ****************************

	// For speed computation
	double lastTime = glfwGetTime();
	int nbFrames = 0;
	// Render loop
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0)
	{ // Clear the screen

		// Render to our framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, RenderToTextureFrameBuffer);
		 //glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Measure speed
		double currentTime = glfwGetTime();
		nbFrames++;
		if (currentTime - lastTime >= 1.0)
		{ // If last prinf() was more than 1sec ago
		  // printf and reset
			printf("%f ms/frame\n", 1000.0 / double(nbFrames));
			nbFrames = 0;
			lastTime += 1.0;
		}

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Also clear the depth buffer!!!

		// update camera controls with mouse input
		myControls->update();
		// takes care of all the rendering automatically
		myScene->render(myCamera);

		// *********************
		// Render to texture
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, windowWidth, windowHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Use our shader
		postEffectShader->bind();
		// Bind our texture in Texture Unit 0
		// the one from the famebuffer = render texture the one used in the shader texid
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, renderedTexture);
		// Set our "renderedTexture" sampler to user Texture Unit 0
		postEffectShader->bindTexture();
		if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
			postEffectShader->setShaderMode(0);
		else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
			postEffectShader->setShaderMode(1);
		else if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
			postEffectShader->setShaderMode(2);
		else if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
			postEffectShader->setShaderMode(3);

		postEffectShader->setTime((float)(glfwGetTime() * 10.0f)); // set time to get animated effect in shader
		outputQuad->directRender();                                // call render directly to render quad only without transformations
		// End render to texture
		// ******************

// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VertexArrayID);
	// delete texture;
	delete myScene;
	delete myCamera;
	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}
