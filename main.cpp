#include <iostream>
#include <cmath>
#include <string>
#include <algorithm>

//GLEW
#define GLEW_STATIC
#include <glew.h>

//GLFW
#include <glfw3.h>

//GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//Other Libs
#include <SOIL.h>

// Other Includes
#include "Shader.h"
#include "Camera.h"
#include "objloader.h"
#include "RigidBody.h"
#include "AABB.h"

using namespace std;

//Funtion Prototypes
bool checkaabbX(AABB &a, AABB &b);
bool checkaabbY(AABB &a, AABB &b);
bool checkaabbZ(AABB &a, AABB &b);
void renderRigid(std::vector<glm::vec3> *vec,GLuint *va, GLuint *vb);
void renderBoundBox(GLuint *va, GLuint *vb, GLfloat (*boundbox)[108],Shader *shader, bool collide);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void do_movement();
GLuint loadTexture(GLchar* path);
GLuint loadCubemap(vector <const GLchar*> faces);

//Window Dimensions
const GLuint WIDTH = 1024, HEIGHT = 768;
GLuint screenWidth = 1024, screenHeight = 768;

//Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
GLfloat fov = 45.0f;
bool keys[1024];
bool firstMouse = true;
bool demoPause = false;

//Deltatime
GLfloat deltaTime = 0.0f;
GLfloat cameradeltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
GLfloat radius = 5.0f;

int table[10][10];


GLuint loadCubemap(vector<const GLchar*> faces)
{
	GLuint textureID;
	glGenTextures(1, &textureID);

	int width, height;
	unsigned char* image;

	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	for (GLuint i = 0; i < faces.size(); i++)
	{
		image = SOIL_load_image(faces[i], &width, &height, 0, SOIL_LOAD_RGB);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		SOIL_free_image_data(image);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return textureID;
}


// This function loads a texture from file. Note: texture loading functions like these are usually 
// managed by a 'Resource Manager' that manages all resources (like textures, models, audio). 
// For learning purposes we'll just define it as a utility function.
GLuint loadTexture(GLchar* path)
{
	//Generate texture ID and load texture data 
	GLuint textureID;
	glGenTextures(1, &textureID);
	int width, height;
	unsigned char* image = SOIL_load_image(path, &width, &height, 0, SOIL_LOAD_RGB);
	// Assign texture to ID
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	SOIL_free_image_data(image);
	return textureID;
}


int main(){

	//Init GLFW
	glfwInit();

	//Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	//Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Physics", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	//Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	//GLFW options
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//Set this to true so GLEW knows to use a modern approach to 
	//retreiving function pointers and extensions
	glewExperimental = GL_TRUE;
	//Initialize GLEW to setup the OpenGL Function pointer
	glewInit();
	

	//Define the viewport dimensions
	glViewport(0, 0, WIDTH, HEIGHT);
	
	//Setup OpenGL options
	glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_LESS);
	


#pragma region "object_initialization"

	// Set the object data (buffers, vertex attributes)	
	GLfloat cubeVertices[] = {
		// Positions          // Normals
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
	};

	GLfloat skyboxVertices[] = {
		// Positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f
	};

	GLfloat boundingBox[] = {
		// Positions          
		0,0,0,
		0,0,0,
		0,0,0,
		0,0,0,
		0,0,0,
		0,0,0,
		0,0,0,
		0,0,0,

		0,0,0,
		0,0,0,
		0,0,0,
		0,0,0,
		0,0,0,
		0,0,0,
		0,0,0,
		0,0,0
	};

	// Setup and compile our shaders
	Shader shader("shaders/advanced.vs", "shaders/advanced.frag");
	Shader skyboxShader("shaders/skybox.vs", "shaders/skybox.frag");
	Shader extraObject("shaders/extra.vs", "shaders/extra.frag");
	Shader refract("shaders/refract.vs", "shaders/refract.frag");
	Shader fresnel("shaders/fresnel.vs", "shaders/fresnel.frag");
	Shader box("shaders/box.vs", "shaders/box.frag");
	Shader boundBox("shaders/bound.vs", "shaders/bound.frag");
	Shader chromatic("shaders/chromatic.vs", "shaders/chromatic.frag");


	// Load model and convert to vertices and 

	vector<glm::vec3> vertices;
	vector<glm::vec2> uvs;
	vector<glm::vec3> normals;

	bool res = loadOBJ("obj/cube.obj", vertices, uvs, normals);

	vector<glm::vec3> worldvertex, worldvertex2 , worldvertex3, worldvertex4, worldvertex5, worldvertex6, worldvertex7,
					  worldvertex8, worldvertex9, worldvertex10;

	vector<glm::vec3> spherePosition;

	//cube dimensions (madeup)
	float h = 3.0f, d = 3.0f, w = 3.0f;


	float mass = 10000.0f;
	glm::mat3x3 Ibody((h*h + d*d) / mass, 0.0f, 0.0f,
						 0, (w*w + d*d) / mass, 0,
		                 0, 0, (h*h + w*w) / mass); 

	mat3x3 IbodyInv = glm::inverse(Ibody);

	/*mat3x3 rotation(0.5f,0.0f,0.866f,
					0.0f, 1.0f, 0.0f, 
					-0.866f, 0.0f, 0.5f);*/

	mat3x3 rotation(1.0f);
	//rotation = glm::transpose(rotation);

	vec3 pos(0.0, 0.0, 0.0),force(0.0,0.54,0.0),torque(0,0,0),omega(0.0,0.0,0);
	vec3 linMoment(0.0f, 0.0f, 0.0f), angMoment(0, 0, 0);
	
	RigidBody *rigidBody = new RigidBody(mass,Ibody,IbodyInv,pos,rotation,omega,angMoment,linMoment,force,torque);
	RigidBody *rigidBody2 = new RigidBody(mass, Ibody, IbodyInv, vec3(3,3,0), rotation, omega, angMoment, linMoment, vec3(0.05, 0.32, 0.0), torque);
	RigidBody *rigidBody3 = new RigidBody(mass, Ibody, IbodyInv, vec3(0,8,4), rotation, omega, angMoment, linMoment, vec3(0.03, 0.06, 0.11), torque);
	RigidBody *rigidBody4 = new RigidBody(mass, Ibody, IbodyInv, vec3(3, 1, 4), rotation, omega, angMoment, linMoment, vec3(0.17, 0.16, 0.11), torque);
	RigidBody *rigidBody5 = new RigidBody(mass, Ibody, IbodyInv, vec3(4, 8, 1), rotation, omega, angMoment, linMoment, vec3(0.03, 0.26, 0.01), torque);
	RigidBody *rigidBody6 = new RigidBody(mass, Ibody, IbodyInv, vec3(5, 0, 7), rotation, omega, angMoment, linMoment, vec3(-0.13, 0.16, 0.11), torque);
	RigidBody *rigidBody7 = new RigidBody(mass, Ibody, IbodyInv, vec3(0, 12, 3), rotation, omega, angMoment, linMoment, vec3(0.18, 0.16, -0.18), torque);
	RigidBody *rigidBody8 = new RigidBody(mass, Ibody, IbodyInv, vec3(0, 8, 2), rotation, omega, angMoment, linMoment, vec3(0.13, 0.21, 0.01), torque);
	RigidBody *rigidBody9 = new RigidBody(mass, Ibody, IbodyInv, vec3(3, 6, 4), rotation, omega, angMoment, linMoment, vec3(-0.193, 0.06, 0.8), torque);
	RigidBody *rigidBody10 = new RigidBody(mass, Ibody, IbodyInv, vec3(6, 4, 2), rotation, omega, angMoment, linMoment, vec3(-0.18, 0.32, -0.03), torque);


	cout << "num of vertices = " << vertices.size() << endl;


	//Load object to VBO
	GLuint vertexbuffer, vertexArray;
	glGenBuffers(1, &vertexbuffer);
	glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	GLuint normalbuffer;
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glBindVertexArray(0);

	//Bounding Box
	GLuint boundingBoxVB, boundingBoxVA;
	glGenBuffers(1, &boundingBoxVB);
	glGenVertexArrays(1, &boundingBoxVA);

	glBindVertexArray(boundingBoxVA);
	glBindBuffer(GL_ARRAY_BUFFER, boundingBoxVB);
	glBufferData(GL_ARRAY_BUFFER, sizeof(boundingBox), &boundingBox, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glBindVertexArray(0);
	
	// Setup Cube 
	GLuint cubeVAO, cubeVBO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	
	glEnableVertexAttribArray(1); // For the normal
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glBindVertexArray(0);
	

	// Setup skybox VAO and VBO
	GLuint skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glBindVertexArray(0);

	
	#pragma endregion

	// Cubemap (Skybox)
	vector<const GLchar*> faces;
	faces.push_back("skybox/right.jpg");
	faces.push_back("skybox/left.jpg");
	faces.push_back("skybox/top.jpg");
	faces.push_back("skybox/bottom.jpg");
	faces.push_back("skybox/back.jpg");
	faces.push_back("skybox/front.jpg");

	GLuint cubemapTexture = loadCubemap(faces);

	float time = 0.0f;
	float maxX, maxY, maxZ, minX, minY, minZ = 0;
	glm::vec3 center(0, 0, 0);

	//Game Loop
	worldvertex.resize(vertices.size());
	worldvertex2.resize(vertices.size());
	worldvertex3.resize(vertices.size());
	worldvertex4.resize(vertices.size());
	worldvertex5.resize(vertices.size());
	worldvertex6.resize(vertices.size());
	worldvertex7.resize(vertices.size());
	worldvertex8.resize(vertices.size());
	worldvertex9.resize(vertices.size());
	worldvertex10.resize(vertices.size());

	GLfloat timeElasped = 0;

	while (!glfwWindowShouldClose(window)){

		//Calculate deltatime of current frame
		GLfloat currentFrame = 0; 
		
		if (demoPause == false) {
			currentFrame = glfwGetTime();
			deltaTime = (currentFrame - lastFrame);  // 1/60 = 0.016 Approx, for steady 60 fps
			cameradeltaTime = (currentFrame - lastFrame);
			lastFrame = currentFrame;
		}
		else {
			currentFrame = lastFrame;
			timeElasped = glfwGetTime();
			deltaTime = (currentFrame - lastFrame);  // 1/60 = 0.016 Approx, for steady 60 fps
			cameradeltaTime = timeElasped - currentFrame;
			lastFrame = timeElasped;
		}


		//time += deltaTime;

		//std::cout << "current time : " << currentFrame << std::endl;
		//std::cout << "deltatime : " << deltaTime << std::endl;
		//std::cout << "last frame : " << lastFrame << std::endl;
		//std::cout << "time : " << time << std::endl;
		//std::cout << std::endl;
		//std::cout << std::endl;

		//Check if any events have been activated (Key pressed, mouse moved etc.) and call corresponding response functions

		
		glfwPollEvents();
		do_movement();


		//RENDER SKYBOX
		//Clear the colorbuffer
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glDepthMask(GL_FALSE);


		skyboxShader.Use();
		glm::mat4 projection = glm::perspective(camera.Zoom, (float)screenWidth / (float)screenHeight, 0.1f, 15000.0f);
		glm::mat4 view2 = glm::mat4(glm::mat3(camera.GetViewMatrix()));	// Remove any translation component of the view matrix
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view2));
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		
		
		// Skybox Map
		glBindVertexArray(skyboxVAO);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthMask(GL_TRUE);
		

		//	Simpe Cube and Light Source
		extraObject.Use();
		glm::mat4 model2(1.0);
		glm::vec3 lightPos(4.3, 0.5, 0.0);
		glm::mat4 view3 = camera.GetViewMatrix();
		GLfloat xRot = sin(glfwGetTime()) * radius;
		GLfloat yRot = cos(glfwGetTime()) * radius;
		//	model2 = glm::translate(model2, lightPos);
		model2 = glm::translate(model2, glm::vec3(xRot, 0.5, yRot));
		model2 = glm::scale(model2, glm::vec3(2, 2, 2));
		//	glm::mat4 view3 = camera.GetViewMatrix();
		glUniformMatrix4fv(glGetUniformLocation(extraObject.Program, "model"), 1, GL_FALSE, glm::value_ptr(model2));
		glUniformMatrix4fv(glGetUniformLocation(extraObject.Program, "view"), 1, GL_FALSE, glm::value_ptr(view3));
		glUniformMatrix4fv(glGetUniformLocation(extraObject.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniform3f(glGetUniformLocation(extraObject.Program, "cameraPos"), camera.Position.x, camera.Position.y, camera.Position.z);

		glBindVertexArray(cubeVAO);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);



		// Box
		fresnel.Use();
		// ------------------------------------ 1
		glm::mat4 model(1.0);
		glm::vec3 boxPos(4.3, -20.5, 0.0);
		model = glm::translate(model, boxPos);
		model = glm::scale(model, glm::vec3(24, 24, 24));
		view3 = camera.GetViewMatrix();

		GLint lightPosLoc = glGetUniformLocation(fresnel.Program, "lightPos");
		glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
		glUniformMatrix4fv(glGetUniformLocation(fresnel.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(fresnel.Program, "view"), 1, GL_FALSE, glm::value_ptr(view3));
		glUniformMatrix4fv(glGetUniformLocation(fresnel.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniform3f(glGetUniformLocation(fresnel.Program, "cameraPos"), camera.Position.x, camera.Position.y, camera.Position.z);

		glBindVertexArray(cubeVAO);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		
	//	while (deltaTime >= 1.0) { // MAIN UPDATE FUNCTION
		
		if (keys[GLFW_KEY_T])
		{
			if (demoPause == false)
				demoPause = true;
			else
				demoPause = false;
		}
			//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

			// PHYSICS------------------
			box.Use();
			// ------------------------------------ 1

			glBindVertexArray(vertexArray);
			// UPDATING HERE 
		
				//glm::vec3 vertex(vertices[i].x, vertices[i].y, vertices[i].z);
				worldvertex = vertices;
				worldvertex2 = vertices; worldvertex3 = vertices; worldvertex4 = vertices;
				worldvertex5 = vertices; worldvertex6 = vertices; worldvertex7 = vertices;
				worldvertex8 = vertices; worldvertex9 = vertices; worldvertex10 = vertices;

			
				rigidBody->update(deltaTime,&worldvertex,worldvertex.size());
				rigidBody2->update(deltaTime, &worldvertex2, worldvertex2.size());
				rigidBody3->update(deltaTime, &worldvertex3, worldvertex3.size());
				rigidBody4->update(deltaTime, &worldvertex4, worldvertex4.size());
				rigidBody5->update(deltaTime, &worldvertex5, worldvertex5.size());
				rigidBody6->update(deltaTime, &worldvertex6, worldvertex6.size());
				rigidBody7->update(deltaTime, &worldvertex7, worldvertex7.size());
				rigidBody8->update(deltaTime, &worldvertex8, worldvertex8.size());
				rigidBody9->update(deltaTime, &worldvertex9, worldvertex9.size());
				rigidBody10->update(deltaTime, &worldvertex10, worldvertex10.size());
			
				
				
			glm::mat4 modelc(1.0);

			glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
			glUniformMatrix4fv(glGetUniformLocation(box.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelc));
			glUniformMatrix4fv(glGetUniformLocation(box.Program, "view"), 1, GL_FALSE, glm::value_ptr(view3));
			glUniformMatrix4fv(glGetUniformLocation(box.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
			glUniform3f(glGetUniformLocation(box.Program, "cameraPos"), camera.Position.x, camera.Position.y, camera.Position.z);
		
			glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
			glBufferData(GL_ARRAY_BUFFER, worldvertex.size() * sizeof(glm::vec3), &worldvertex[0], GL_DYNAMIC_DRAW);

			glDrawArrays(GL_TRIANGLES, 0, worldvertex.size());
			glBindVertexArray(0);
			//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			
			renderRigid(&worldvertex2, &vertexArray, &vertexbuffer);
			renderRigid(&worldvertex3, &vertexArray, &vertexbuffer);
			renderRigid(&worldvertex4, &vertexArray, &vertexbuffer);
			renderRigid(&worldvertex5, &vertexArray, &vertexbuffer);
			renderRigid(&worldvertex6, &vertexArray, &vertexbuffer);
			renderRigid(&worldvertex7, &vertexArray, &vertexbuffer);
			renderRigid(&worldvertex8, &vertexArray, &vertexbuffer);
			renderRigid(&worldvertex9, &vertexArray, &vertexbuffer);
			renderRigid(&worldvertex10, &vertexArray, &vertexbuffer);
			



			//------------------AABB-BOUNDING-BOX-CALCULATION----------------------------
#pragma region "Bounding Box"
			minX = 1000;minY = 1000;minZ = 1000;maxX = 0;maxY = 0;maxZ = 0;
			for (int i = 0; i < worldvertex.size(); i++) {
				if (worldvertex[i].x < minX)
					minX = worldvertex[i].x;
				if (worldvertex[i].y < minY)
					minY = worldvertex[i].y;
				if (worldvertex[i].z < minZ)
					minZ = worldvertex[i].z;
				if (worldvertex[i].x > maxX)
					maxX = worldvertex[i].x;
				if (worldvertex[i].y > maxY)
					maxY = worldvertex[i].y;
				if (worldvertex[i].z > maxZ)
					maxZ = worldvertex[i].z;
			}
			AABB box1(glm::vec3(minX, minY, minZ), glm::vec3(maxX, maxY, maxZ));
			box1.id = 1;
		

			minX = 1000;minY = 1000;minZ = 1000;maxX = 0;maxY = 0;	maxZ = 0;
			for (int i = 0; i < worldvertex2.size(); i++) {
				if (worldvertex2[i].x < minX)
					minX = worldvertex2[i].x;
				if (worldvertex2[i].y < minY)
					minY = worldvertex2[i].y;
				if (worldvertex2[i].z < minZ)
					minZ = worldvertex2[i].z;
				if (worldvertex2[i].x > maxX)
					maxX = worldvertex2[i].x;
				if (worldvertex2[i].y > maxY)
					maxY = worldvertex2[i].y;
				if (worldvertex2[i].z > maxZ)
					maxZ = worldvertex2[i].z;

			}
			AABB box2(glm::vec3(minX, minY, minZ), glm::vec3(maxX, maxY, maxZ));
			box2.id = 2;
			

			minX = 1000;minY = 1000;minZ = 1000;maxX = 0;maxY = 0;	maxZ = 0;
			for (int i = 0; i < worldvertex3.size(); i++) {
				if (worldvertex3[i].x < minX)
					minX = worldvertex3[i].x;
				if (worldvertex3[i].y < minY)
					minY = worldvertex3[i].y;
				if (worldvertex3[i].z < minZ)
					minZ = worldvertex3[i].z;
				if (worldvertex3[i].x > maxX)
					maxX = worldvertex3[i].x;
				if (worldvertex3[i].y > maxY)
					maxY = worldvertex3[i].y;
				if (worldvertex3[i].z > maxZ)
					maxZ = worldvertex3[i].z;

			}
			AABB box3(glm::vec3(minX, minY, minZ), glm::vec3(maxX, maxY, maxZ));
			box3.id = 3;
		
			minX = 1000; minY = 1000; minZ = 1000; maxX = 0; maxY = 0;	maxZ = 0;
			for (int i = 0; i < worldvertex4.size(); i++) {
				if (worldvertex4[i].x < minX)
					minX = worldvertex4[i].x;
				if (worldvertex4[i].y < minY)
					minY = worldvertex4[i].y;
				if (worldvertex4[i].z < minZ)
					minZ = worldvertex4[i].z;
				if (worldvertex4[i].x > maxX)
					maxX = worldvertex4[i].x;
				if (worldvertex4[i].y > maxY)
					maxY = worldvertex4[i].y;
				if (worldvertex4[i].z > maxZ)
					maxZ = worldvertex4[i].z;
			}
			AABB box4(glm::vec3(minX, minY, minZ), glm::vec3(maxX, maxY, maxZ));
			box4.id = 4;
		
			minX = 1000; minY = 1000; minZ = 1000; maxX = 0; maxY = 0;	maxZ = 0;
			for (int i = 0; i < worldvertex5.size(); i++) {
				if (worldvertex5[i].x < minX)
					minX = worldvertex5[i].x;
				if (worldvertex5[i].y < minY)
					minY = worldvertex5[i].y;
				if (worldvertex5[i].z < minZ)
					minZ = worldvertex5[i].z;
				if (worldvertex5[i].x > maxX)
					maxX = worldvertex5[i].x;
				if (worldvertex5[i].y > maxY)
					maxY = worldvertex5[i].y;
				if (worldvertex5[i].z > maxZ)
					maxZ = worldvertex5[i].z;
			}
			AABB box5(glm::vec3(minX, minY, minZ), glm::vec3(maxX, maxY, maxZ));
			box5.id = 5;
	
			minX = 1000; minY = 1000; minZ = 1000; maxX = 0; maxY = 0;	maxZ = 0;
			for (int i = 0; i < worldvertex6.size(); i++) {
				if (worldvertex6[i].x < minX)
					minX = worldvertex6[i].x;
				if (worldvertex6[i].y < minY)
					minY = worldvertex6[i].y;
				if (worldvertex6[i].z < minZ)
					minZ = worldvertex6[i].z;
				if (worldvertex6[i].x > maxX)
					maxX = worldvertex6[i].x;
				if (worldvertex6[i].y > maxY)
					maxY = worldvertex6[i].y;
				if (worldvertex6[i].z > maxZ)
					maxZ = worldvertex6[i].z;
			}
			AABB box6(glm::vec3(minX, minY, minZ), glm::vec3(maxX, maxY, maxZ));
			box6.id = 6;

			minX = 1000; minY = 1000; minZ = 1000; maxX = 0; maxY = 0;	maxZ = 0;
			for (int i = 0; i < worldvertex7.size(); i++) {
				if (worldvertex7[i].x < minX)
					minX = worldvertex7[i].x;
				if (worldvertex7[i].y < minY)
					minY = worldvertex7[i].y;
				if (worldvertex7[i].z < minZ)
					minZ = worldvertex7[i].z;
				if (worldvertex7[i].x > maxX)
					maxX = worldvertex7[i].x;
				if (worldvertex7[i].y > maxY)
					maxY = worldvertex7[i].y;
				if (worldvertex7[i].z > maxZ)
					maxZ = worldvertex7[i].z;
			}
			AABB box7(glm::vec3(minX, minY, minZ), glm::vec3(maxX, maxY, maxZ));
			box7.id = 7;

			minX = 1000; minY = 1000; minZ = 1000; maxX = 0; maxY = 0;	maxZ = 0;
			for (int i = 0; i < worldvertex8.size(); i++) {
				if (worldvertex8[i].x < minX)
					minX = worldvertex8[i].x;
				if (worldvertex8[i].y < minY)
					minY = worldvertex8[i].y;
				if (worldvertex8[i].z < minZ)
					minZ = worldvertex8[i].z;
				if (worldvertex8[i].x > maxX)
					maxX = worldvertex8[i].x;
				if (worldvertex8[i].y > maxY)
					maxY = worldvertex8[i].y;
				if (worldvertex8[i].z > maxZ)
					maxZ = worldvertex8[i].z;
			}
			AABB box8(glm::vec3(minX, minY, minZ), glm::vec3(maxX, maxY, maxZ));
			box8.id = 8;

			minX = 1000; minY = 1000; minZ = 1000; maxX = 0; maxY = 0;	maxZ = 0;
			for (int i = 0; i < worldvertex9.size(); i++) {
				if (worldvertex9[i].x < minX)
					minX = worldvertex9[i].x;
				if (worldvertex9[i].y < minY)
					minY = worldvertex9[i].y;
				if (worldvertex9[i].z < minZ)
					minZ = worldvertex9[i].z;
				if (worldvertex9[i].x > maxX)
					maxX = worldvertex9[i].x;
				if (worldvertex9[i].y > maxY)
					maxY = worldvertex9[i].y;
				if (worldvertex9[i].z > maxZ)
					maxZ = worldvertex9[i].z;
			}
			AABB box9(glm::vec3(minX, minY, minZ), glm::vec3(maxX, maxY, maxZ));
			box9.id = 9;

			minX = 1000; minY = 1000; minZ = 1000; maxX = 0; maxY = 0;	maxZ = 0;
			for (int i = 0; i < worldvertex10.size(); i++) {
				if (worldvertex10[i].x < minX)
					minX = worldvertex10[i].x;
				if (worldvertex10[i].y < minY)
					minY = worldvertex10[i].y;
				if (worldvertex10[i].z < minZ)
					minZ = worldvertex10[i].z;
				if (worldvertex10[i].x > maxX)
					maxX = worldvertex10[i].x;
				if (worldvertex10[i].y > maxY)
					maxY = worldvertex10[i].y;
				if (worldvertex10[i].z > maxZ)
					maxZ = worldvertex10[i].z;
			}
			AABB box10(glm::vec3(minX, minY, minZ), glm::vec3(maxX, maxY, maxZ));
			box10.id = 10;

			//----------------------------------------------------------------------------------------

			
#pragma endregion


			vector<AABB> boxes; 
			boxes.clear();
			boxes.push_back(box1); boxes.push_back(box2); boxes.push_back(box3); boxes.push_back(box4);
			boxes.push_back(box5); boxes.push_back(box6); boxes.push_back(box7); boxes.push_back(box8);
			boxes.push_back(box9); boxes.push_back(box10);
			

			// Reset the table 
			for (int i = 0; i < 10; i++)
				for (int j = 0; j < 10; j++)
					table[i][j] = 0;

			// check if the bounding boxes are colliding 
			for (int i = 0; i < boxes.size(); i++) {
				for (int j = i+1; j < boxes.size(); j++) {
					if (checkaabbX(boxes[i], boxes[j]))
							table[i][j]++;
					if (checkaabbY(boxes[i], boxes[j]))
							table[i][j]++;
					if (checkaabbZ(boxes[i], boxes[j]))
							table[i][j]++;
				}
			}

			//--------------------------------------------------------------------------

			//-------------------------BOUNDING BOX---------------------------
				
			GLfloat boundingBox[] = {
				// Positions          
				box1.m_min.x, box1.m_max.y, box1.m_max.z,  box1.m_min.x, box1.m_max.y, box1.m_max.z,  box1.m_min.x,box1.m_min.y, box1.m_max.z,      // v0-v1-v2 (front)
				box1.m_min.x,box1.m_min.y, box1.m_max.z,   box1.m_max.x,box1.m_min.y, box1.m_max.z,   box1.m_max.x, box1.m_max.y, box1.m_max.z,      // v2-v3-v0

				box1.m_max.x, box1.m_max.y, box1.m_max.z,   box1.m_max.x,box1.m_min.y, box1.m_max.z,   box1.m_max.x,box1.m_min.y,box1.m_min.z,      // v0-v3-v4 (right)
				box1.m_max.x,box1.m_min.y,box1.m_min.z,   box1.m_max.x, box1.m_max.y,box1.m_min.z,   box1.m_max.x, box1.m_max.y, box1.m_max.z,      // v4-v5-v0

				box1.m_max.x, box1.m_max.y, box1.m_max.z,   box1.m_max.x, box1.m_max.y,box1.m_min.z,  box1.m_min.x, box1.m_max.y,box1.m_min.z,      // v0-v5-v6 (top)
				box1.m_min.x, box1.m_max.y,box1.m_min.z,  box1.m_min.x, box1.m_max.y, box1.m_max.z,   box1.m_max.x, box1.m_max.y, box1.m_max.z,      // v6-v1-v0

				box1.m_min.x, box1.m_max.y, box1.m_max.z,  box1.m_min.x, box1.m_max.y,box1.m_min.z,  box1.m_min.x,box1.m_min.y,box1.m_min.z,      // v1-v6-v7 (left)
				box1.m_min.x,box1.m_min.y,box1.m_min.z,  box1.m_min.x,box1.m_min.y, box1.m_max.z,  box1.m_min.x, box1.m_max.y, box1.m_max.z,      // v7-v2-v1

				box1.m_min.x,box1.m_min.y,box1.m_min.z,   box1.m_max.x,box1.m_min.y,box1.m_min.z,   box1.m_max.x,box1.m_min.y, box1.m_max.z,      // v7-v4-v3 (bottom)
				box1.m_max.x,box1.m_min.y, box1.m_max.z,  box1.m_min.x,box1.m_min.y, box1.m_max.z,  box1.m_min.x,box1.m_min.y,box1.m_min.z,      // v3-v2-v7

				box1.m_max.x,box1.m_min.y,box1.m_min.z,	   box1.m_min.x,box1.m_min.y,box1.m_min.z,	box1.m_min.x,box1.m_max.y,box1.m_min.z,      // v4-v7-v6 (back)
				box1.m_min.x, box1.m_max.y,box1.m_min.z,   box1.m_max.x, box1.m_max.y,box1.m_min.z,   box1.m_max.x,box1.m_min.y,box1.m_min.z
			};
			GLfloat boundingBox2[] = {
				// Positions          
				box2.m_min.x, box2.m_max.y, box2.m_max.z,  box2.m_min.x, box2.m_max.y, box2.m_max.z,  box2.m_min.x,box2.m_min.y, box2.m_max.z,      // v0-v1-v2 (front)
				box2.m_min.x,box2.m_min.y, box2.m_max.z,   box2.m_max.x,box2.m_min.y, box2.m_max.z,   box2.m_max.x, box2.m_max.y, box2.m_max.z,      // v2-v3-v0

				box2.m_max.x, box2.m_max.y, box2.m_max.z,   box2.m_max.x,box2.m_min.y, box2.m_max.z,   box2.m_max.x,box2.m_min.y,box2.m_min.z,      // v0-v3-v4 (right)
				box2.m_max.x,box2.m_min.y,box2.m_min.z,   box2.m_max.x, box2.m_max.y,box2.m_min.z,   box2.m_max.x, box2.m_max.y, box2.m_max.z,      // v4-v5-v0

				box2.m_max.x, box2.m_max.y, box2.m_max.z,   box2.m_max.x, box2.m_max.y,box2.m_min.z,  box2.m_min.x, box2.m_max.y,box2.m_min.z,      // v0-v5-v6 (top)
				box2.m_min.x, box2.m_max.y,box2.m_min.z,  box2.m_min.x, box2.m_max.y, box2.m_max.z,   box2.m_max.x, box2.m_max.y, box2.m_max.z,      // v6-v1-v0

				box2.m_min.x, box2.m_max.y, box2.m_max.z,  box2.m_min.x, box2.m_max.y,box2.m_min.z,  box2.m_min.x,box2.m_min.y,box2.m_min.z,      // v1-v6-v7 (left)
				box2.m_min.x,box2.m_min.y,box2.m_min.z,  box2.m_min.x,box2.m_min.y, box2.m_max.z,  box2.m_min.x, box2.m_max.y, box2.m_max.z,      // v7-v2-v1

				box2.m_min.x,box2.m_min.y,box2.m_min.z,   box2.m_max.x,box2.m_min.y,box2.m_min.z,   box2.m_max.x,box2.m_min.y, box2.m_max.z,      // v7-v4-v3 (bottom)
				box2.m_max.x,box2.m_min.y, box2.m_max.z,  box2.m_min.x,box2.m_min.y, box2.m_max.z,  box2.m_min.x,box2.m_min.y,box2.m_min.z,      // v3-v2-v7

				box2.m_max.x,box2.m_min.y,box2.m_min.z,	   box2.m_min.x,box2.m_min.y,box2.m_min.z,	box2.m_min.x,box2.m_max.y,box2.m_min.z,      // v4-v7-v6 (back)
				box2.m_min.x, box2.m_max.y,box2.m_min.z,   box2.m_max.x, box2.m_max.y,box2.m_min.z,   box2.m_max.x,box2.m_min.y,box2.m_min.z
			};
			GLfloat boundingBox3[] = {
				// Positions    
				box3.m_min.x, box3.m_max.y, box3.m_max.z,  box3.m_min.x, box3.m_max.y, box3.m_max.z,  box3.m_min.x,box3.m_min.y, box3.m_max.z,      // v0-v1-v2 (front)
				box3.m_min.x,box3.m_min.y, box3.m_max.z,   box3.m_max.x,box3.m_min.y, box3.m_max.z,   box3.m_max.x, box3.m_max.y, box3.m_max.z,      // v2-v3-v0

				box3.m_max.x, box3.m_max.y, box3.m_max.z,   box3.m_max.x,box3.m_min.y, box3.m_max.z,   box3.m_max.x,box3.m_min.y,box3.m_min.z,      // v0-v3-v4 (right)
				box3.m_max.x,box3.m_min.y,box3.m_min.z,   box3.m_max.x, box3.m_max.y,box3.m_min.z,   box3.m_max.x, box3.m_max.y, box3.m_max.z,      // v4-v5-v0

				box3.m_max.x, box3.m_max.y, box3.m_max.z,   box3.m_max.x, box3.m_max.y,box3.m_min.z,  box3.m_min.x, box3.m_max.y,box3.m_min.z,      // v0-v5-v6 (top)
				box3.m_min.x, box3.m_max.y,box3.m_min.z,  box3.m_min.x, box3.m_max.y, box3.m_max.z,   box3.m_max.x, box3.m_max.y, box3.m_max.z,      // v6-v1-v0

				box3.m_min.x, box3.m_max.y, box3.m_max.z,  box3.m_min.x, box3.m_max.y,box3.m_min.z,  box3.m_min.x,box3.m_min.y,box3.m_min.z,      // v1-v6-v7 (left)
				box3.m_min.x,box3.m_min.y,box3.m_min.z,  box3.m_min.x,box3.m_min.y, box3.m_max.z,  box3.m_min.x, box3.m_max.y, box3.m_max.z,      // v7-v2-v1

				box3.m_min.x,box3.m_min.y,box3.m_min.z,   box3.m_max.x,box3.m_min.y,box3.m_min.z,   box3.m_max.x,box3.m_min.y, box3.m_max.z,      // v7-v4-v3 (bottom)
				box3.m_max.x,box3.m_min.y, box3.m_max.z,  box3.m_min.x,box3.m_min.y, box3.m_max.z,  box3.m_min.x,box3.m_min.y,box3.m_min.z,      // v3-v2-v7

				box3.m_max.x,box3.m_min.y,box3.m_min.z,	   box3.m_min.x,box3.m_min.y,box3.m_min.z,	box3.m_min.x,box3.m_max.y,box3.m_min.z,      // v4-v7-v6 (back)
				box3.m_min.x, box3.m_max.y,box3.m_min.z,   box3.m_max.x, box3.m_max.y,box3.m_min.z,   box3.m_max.x,box3.m_min.y,box3.m_min.z
			};
			GLfloat boundingBox4[] = {
				// Positions    
				box4.m_min.x, box4.m_max.y, box4.m_max.z,  box4.m_min.x, box4.m_max.y, box4.m_max.z,  box4.m_min.x,box4.m_min.y, box4.m_max.z,      // v0-v1-v2 (front)
				box4.m_min.x,box4.m_min.y, box4.m_max.z,   box4.m_max.x,box4.m_min.y, box4.m_max.z,   box4.m_max.x, box4.m_max.y, box4.m_max.z,      // v2-v3-v0

				box4.m_max.x, box4.m_max.y, box4.m_max.z,   box4.m_max.x,box4.m_min.y, box4.m_max.z,   box4.m_max.x,box4.m_min.y,box4.m_min.z,      // v0-v3-v4 (right)
				box4.m_max.x,box4.m_min.y,box4.m_min.z,   box4.m_max.x, box4.m_max.y,box4.m_min.z,   box4.m_max.x, box4.m_max.y, box4.m_max.z,      // v4-v5-v0

				box4.m_max.x, box4.m_max.y, box4.m_max.z,   box4.m_max.x, box4.m_max.y,box4.m_min.z,  box4.m_min.x, box4.m_max.y,box4.m_min.z,      // v0-v5-v6 (top)
				box4.m_min.x, box4.m_max.y,box4.m_min.z,  box4.m_min.x, box4.m_max.y, box4.m_max.z,   box4.m_max.x, box4.m_max.y, box4.m_max.z,      // v6-v1-v0

				box4.m_min.x, box4.m_max.y, box4.m_max.z,  box4.m_min.x, box4.m_max.y,box4.m_min.z,  box4.m_min.x,box4.m_min.y,box4.m_min.z,      // v1-v6-v7 (left)
				box4.m_min.x,box4.m_min.y,box4.m_min.z,  box4.m_min.x,box4.m_min.y, box4.m_max.z,  box4.m_min.x, box4.m_max.y, box4.m_max.z,      // v7-v2-v1

				box4.m_min.x,box4.m_min.y,box4.m_min.z,   box4.m_max.x,box4.m_min.y,box4.m_min.z,   box4.m_max.x,box4.m_min.y, box4.m_max.z,      // v7-v4-v3 (bottom)
				box4.m_max.x,box4.m_min.y, box4.m_max.z,  box4.m_min.x,box4.m_min.y, box4.m_max.z,  box4.m_min.x,box4.m_min.y,box4.m_min.z,      // v3-v2-v7

				box4.m_max.x,box4.m_min.y,box4.m_min.z,	   box4.m_min.x,box4.m_min.y,box4.m_min.z,	box4.m_min.x,box4.m_max.y,box4.m_min.z,      // v4-v7-v6 (back)
				box4.m_min.x, box4.m_max.y,box4.m_min.z,   box4.m_max.x, box4.m_max.y,box4.m_min.z,   box4.m_max.x,box4.m_min.y,box4.m_min.z
			};
			GLfloat boundingBox5[] = {
				// Positions    
				box5.m_min.x, box5.m_max.y, box5.m_max.z,  box5.m_min.x, box5.m_max.y, box5.m_max.z,  box5.m_min.x,box5.m_min.y, box5.m_max.z,      // v0-v1-v2 (front)
				box5.m_min.x,box5.m_min.y, box5.m_max.z,   box5.m_max.x,box5.m_min.y, box5.m_max.z,   box5.m_max.x, box5.m_max.y, box5.m_max.z,      // v2-v3-v0

				box5.m_max.x, box5.m_max.y, box5.m_max.z,   box5.m_max.x,box5.m_min.y, box5.m_max.z,   box5.m_max.x,box5.m_min.y,box5.m_min.z,      // v0-v3-v4 (right)
				box5.m_max.x,box5.m_min.y,box5.m_min.z,   box5.m_max.x, box5.m_max.y,box5.m_min.z,   box5.m_max.x, box5.m_max.y, box5.m_max.z,      // v4-v5-v0

				box5.m_max.x, box5.m_max.y, box5.m_max.z,   box5.m_max.x, box5.m_max.y,box5.m_min.z,  box5.m_min.x, box5.m_max.y,box5.m_min.z,      // v0-v5-v6 (top)
				box5.m_min.x, box5.m_max.y,box5.m_min.z,  box5.m_min.x, box5.m_max.y, box5.m_max.z,   box5.m_max.x, box5.m_max.y, box5.m_max.z,      // v6-v1-v0

				box5.m_min.x, box5.m_max.y, box5.m_max.z,  box5.m_min.x, box5.m_max.y,box5.m_min.z,  box5.m_min.x,box5.m_min.y,box5.m_min.z,      // v1-v6-v7 (left)
				box5.m_min.x,box5.m_min.y,box5.m_min.z,  box5.m_min.x,box5.m_min.y, box5.m_max.z,  box5.m_min.x, box5.m_max.y, box5.m_max.z,      // v7-v2-v1

				box5.m_min.x,box5.m_min.y,box5.m_min.z,   box5.m_max.x,box5.m_min.y,box5.m_min.z,   box5.m_max.x,box5.m_min.y, box5.m_max.z,      // v7-v4-v3 (bottom)
				box5.m_max.x,box5.m_min.y, box5.m_max.z,  box5.m_min.x,box5.m_min.y, box5.m_max.z,  box5.m_min.x,box5.m_min.y,box5.m_min.z,      // v3-v2-v7

				box5.m_max.x,box5.m_min.y,box5.m_min.z,	   box5.m_min.x,box5.m_min.y,box5.m_min.z,	box5.m_min.x,box5.m_max.y,box5.m_min.z,      // v4-v7-v6 (back)
				box5.m_min.x, box5.m_max.y,box5.m_min.z,   box5.m_max.x, box5.m_max.y,box5.m_min.z,   box5.m_max.x,box5.m_min.y,box5.m_min.z
			};
			GLfloat boundingBox6[] = {
				// Positions    
				box6.m_min.x, box6.m_max.y, box6.m_max.z,  box6.m_min.x, box6.m_max.y, box6.m_max.z,  box6.m_min.x,box6.m_min.y, box6.m_max.z,      // v0-v1-v2 (front)
				box6.m_min.x,box6.m_min.y, box6.m_max.z,   box6.m_max.x,box6.m_min.y, box6.m_max.z,   box6.m_max.x, box6.m_max.y, box6.m_max.z,      // v2-v3-v0

				box6.m_max.x, box6.m_max.y, box6.m_max.z,   box6.m_max.x,box6.m_min.y, box6.m_max.z,   box6.m_max.x,box6.m_min.y,box6.m_min.z,      // v0-v3-v4 (right)
				box6.m_max.x,box6.m_min.y,box6.m_min.z,   box6.m_max.x, box6.m_max.y,box6.m_min.z,   box6.m_max.x, box6.m_max.y, box6.m_max.z,      // v4-v5-v0

				box6.m_max.x, box6.m_max.y, box6.m_max.z,   box6.m_max.x, box6.m_max.y,box6.m_min.z,  box6.m_min.x, box6.m_max.y,box6.m_min.z,      // v0-v5-v6 (top)
				box6.m_min.x, box6.m_max.y,box6.m_min.z,  box6.m_min.x, box6.m_max.y, box6.m_max.z,   box6.m_max.x, box6.m_max.y, box6.m_max.z,      // v6-v1-v0

				box6.m_min.x, box6.m_max.y, box6.m_max.z,  box6.m_min.x, box6.m_max.y,box6.m_min.z,  box6.m_min.x,box6.m_min.y,box6.m_min.z,      // v1-v6-v7 (left)
				box6.m_min.x,box6.m_min.y,box6.m_min.z,  box6.m_min.x,box6.m_min.y, box6.m_max.z,  box6.m_min.x, box6.m_max.y, box6.m_max.z,      // v7-v2-v1

				box6.m_min.x,box6.m_min.y,box6.m_min.z,   box6.m_max.x,box6.m_min.y,box6.m_min.z,   box6.m_max.x,box6.m_min.y, box6.m_max.z,      // v7-v4-v3 (bottom)
				box6.m_max.x,box6.m_min.y, box6.m_max.z,  box6.m_min.x,box6.m_min.y, box6.m_max.z,  box6.m_min.x,box6.m_min.y,box6.m_min.z,      // v3-v2-v7

				box6.m_max.x,box6.m_min.y,box6.m_min.z,	   box6.m_min.x,box6.m_min.y,box6.m_min.z,	box6.m_min.x,box6.m_max.y,box6.m_min.z,      // v4-v7-v6 (back)
				box6.m_min.x, box6.m_max.y,box6.m_min.z,   box6.m_max.x, box6.m_max.y,box6.m_min.z,   box6.m_max.x,box6.m_min.y,box6.m_min.z
			};
			GLfloat boundingBox7[] = {
				// Positions    
				box7.m_min.x, box7.m_max.y, box7.m_max.z,  box7.m_min.x, box7.m_max.y, box7.m_max.z,  box7.m_min.x,box7.m_min.y, box7.m_max.z,      // v0-v1-v2 (front)
				box7.m_min.x,box7.m_min.y, box7.m_max.z,   box7.m_max.x,box7.m_min.y, box7.m_max.z,   box7.m_max.x, box7.m_max.y, box7.m_max.z,      // v2-v3-v0

				box7.m_max.x, box7.m_max.y, box7.m_max.z,   box7.m_max.x,box7.m_min.y, box7.m_max.z,   box7.m_max.x,box7.m_min.y,box7.m_min.z,      // v0-v3-v4 (right)
				box7.m_max.x,box7.m_min.y,box7.m_min.z,   box7.m_max.x, box7.m_max.y,box7.m_min.z,   box7.m_max.x, box7.m_max.y, box7.m_max.z,      // v4-v5-v0

				box7.m_max.x, box7.m_max.y, box7.m_max.z,   box7.m_max.x, box7.m_max.y,box7.m_min.z,  box7.m_min.x, box7.m_max.y,box7.m_min.z,      // v0-v5-v6 (top)
				box7.m_min.x, box7.m_max.y,box7.m_min.z,  box7.m_min.x, box7.m_max.y, box7.m_max.z,   box7.m_max.x, box7.m_max.y, box7.m_max.z,      // v6-v1-v0

				box7.m_min.x, box7.m_max.y, box7.m_max.z,  box7.m_min.x, box7.m_max.y,box7.m_min.z,  box7.m_min.x,box7.m_min.y,box7.m_min.z,      // v1-v6-v7 (left)
				box7.m_min.x,box7.m_min.y,box7.m_min.z,  box7.m_min.x,box7.m_min.y, box7.m_max.z,  box7.m_min.x, box7.m_max.y, box7.m_max.z,      // v7-v2-v1

				box7.m_min.x,box7.m_min.y,box7.m_min.z,   box7.m_max.x,box7.m_min.y,box7.m_min.z,   box7.m_max.x,box7.m_min.y, box7.m_max.z,      // v7-v4-v3 (bottom)
				box7.m_max.x,box7.m_min.y, box7.m_max.z,  box7.m_min.x,box7.m_min.y, box7.m_max.z,  box7.m_min.x,box7.m_min.y,box7.m_min.z,      // v3-v2-v7

				box7.m_max.x,box7.m_min.y,box7.m_min.z,	   box7.m_min.x,box7.m_min.y,box7.m_min.z,	box7.m_min.x,box7.m_max.y,box7.m_min.z,      // v4-v7-v6 (back)
				box7.m_min.x, box7.m_max.y,box7.m_min.z,   box7.m_max.x, box7.m_max.y,box7.m_min.z,   box7.m_max.x,box7.m_min.y,box7.m_min.z
			};
			GLfloat boundingBox8[] = {
				// Positions    
				box8.m_min.x, box8.m_max.y, box8.m_max.z,  box8.m_min.x, box8.m_max.y, box8.m_max.z,  box8.m_min.x,box8.m_min.y, box8.m_max.z,      // v0-v1-v2 (front)
				box8.m_min.x,box8.m_min.y, box8.m_max.z,   box8.m_max.x,box8.m_min.y, box8.m_max.z,   box8.m_max.x, box8.m_max.y, box8.m_max.z,      // v2-v3-v0

				box8.m_max.x, box8.m_max.y, box8.m_max.z,   box8.m_max.x,box8.m_min.y, box8.m_max.z,   box8.m_max.x,box8.m_min.y,box8.m_min.z,      // v0-v3-v4 (right)
				box8.m_max.x,box8.m_min.y,box8.m_min.z,   box8.m_max.x, box8.m_max.y,box8.m_min.z,   box8.m_max.x, box8.m_max.y, box8.m_max.z,      // v4-v5-v0

				box8.m_max.x, box8.m_max.y, box8.m_max.z,   box8.m_max.x, box8.m_max.y,box8.m_min.z,  box8.m_min.x, box8.m_max.y,box8.m_min.z,      // v0-v5-v6 (top)
				box8.m_min.x, box8.m_max.y,box8.m_min.z,  box8.m_min.x, box8.m_max.y, box8.m_max.z,   box8.m_max.x, box8.m_max.y, box8.m_max.z,      // v6-v1-v0

				box8.m_min.x, box8.m_max.y, box8.m_max.z,  box8.m_min.x, box8.m_max.y,box8.m_min.z,  box8.m_min.x,box8.m_min.y,box8.m_min.z,      // v1-v6-v7 (left)
				box8.m_min.x,box8.m_min.y,box8.m_min.z,  box8.m_min.x,box8.m_min.y, box8.m_max.z,  box8.m_min.x, box8.m_max.y, box8.m_max.z,      // v7-v2-v1

				box8.m_min.x,box8.m_min.y,box8.m_min.z,   box8.m_max.x,box8.m_min.y,box8.m_min.z,   box8.m_max.x,box8.m_min.y, box8.m_max.z,      // v7-v4-v3 (bottom)
				box8.m_max.x,box8.m_min.y, box8.m_max.z,  box8.m_min.x,box8.m_min.y, box8.m_max.z,  box8.m_min.x,box8.m_min.y,box8.m_min.z,      // v3-v2-v7

				box8.m_max.x,box8.m_min.y,box8.m_min.z,	   box8.m_min.x,box8.m_min.y,box8.m_min.z,	box8.m_min.x,box8.m_max.y,box8.m_min.z,      // v4-v7-v6 (back)
				box8.m_min.x, box8.m_max.y,box8.m_min.z,   box8.m_max.x, box8.m_max.y,box8.m_min.z,   box8.m_max.x,box8.m_min.y,box8.m_min.z
			};
			GLfloat boundingBox9[] = {
				// Positions    
				box9.m_min.x, box9.m_max.y, box9.m_max.z,  box9.m_min.x, box9.m_max.y, box9.m_max.z,  box9.m_min.x,box9.m_min.y, box9.m_max.z,      // v0-v1-v2 (front)
				box9.m_min.x,box9.m_min.y, box9.m_max.z,   box9.m_max.x,box9.m_min.y, box9.m_max.z,   box9.m_max.x, box9.m_max.y, box9.m_max.z,      // v2-v3-v0

				box9.m_max.x, box9.m_max.y, box9.m_max.z,   box9.m_max.x,box9.m_min.y, box9.m_max.z,   box9.m_max.x,box9.m_min.y,box9.m_min.z,      // v0-v3-v4 (right)
				box9.m_max.x,box9.m_min.y,box9.m_min.z,   box9.m_max.x, box9.m_max.y,box9.m_min.z,   box9.m_max.x, box9.m_max.y, box9.m_max.z,      // v4-v5-v0

				box9.m_max.x, box9.m_max.y, box9.m_max.z,   box9.m_max.x, box9.m_max.y,box9.m_min.z,  box9.m_min.x, box9.m_max.y,box9.m_min.z,      // v0-v5-v6 (top)
				box9.m_min.x, box9.m_max.y,box9.m_min.z,  box9.m_min.x, box9.m_max.y, box9.m_max.z,   box9.m_max.x, box9.m_max.y, box9.m_max.z,      // v6-v1-v0

				box9.m_min.x, box9.m_max.y, box9.m_max.z,  box9.m_min.x, box9.m_max.y,box9.m_min.z,  box9.m_min.x,box9.m_min.y,box9.m_min.z,      // v1-v6-v7 (left)
				box9.m_min.x,box9.m_min.y,box9.m_min.z,  box9.m_min.x,box9.m_min.y, box9.m_max.z,  box9.m_min.x, box9.m_max.y, box9.m_max.z,      // v7-v2-v1

				box9.m_min.x,box9.m_min.y,box9.m_min.z,   box9.m_max.x,box9.m_min.y,box9.m_min.z,   box9.m_max.x,box9.m_min.y, box9.m_max.z,      // v7-v4-v3 (bottom)
				box9.m_max.x,box9.m_min.y, box9.m_max.z,  box9.m_min.x,box9.m_min.y, box9.m_max.z,  box9.m_min.x,box9.m_min.y,box9.m_min.z,      // v3-v2-v7

				box9.m_max.x,box9.m_min.y,box9.m_min.z,	   box9.m_min.x,box9.m_min.y,box9.m_min.z,	box9.m_min.x,box9.m_max.y,box9.m_min.z,      // v4-v7-v6 (back)
				box9.m_min.x, box9.m_max.y,box9.m_min.z,   box9.m_max.x, box9.m_max.y,box9.m_min.z,   box9.m_max.x,box9.m_min.y,box9.m_min.z
			};
			GLfloat boundingBox10[] = {
				// Positions    
				box10.m_min.x, box10.m_max.y, box10.m_max.z,  box10.m_min.x, box10.m_max.y, box10.m_max.z,  box10.m_min.x,box10.m_min.y, box10.m_max.z,      // v0-v1-v2 (front)
				box10.m_min.x,box10.m_min.y, box10.m_max.z,   box10.m_max.x,box10.m_min.y, box10.m_max.z,   box10.m_max.x, box10.m_max.y, box10.m_max.z,      // v2-v3-v0

				box10.m_max.x, box10.m_max.y, box10.m_max.z,   box10.m_max.x,box10.m_min.y, box10.m_max.z,   box10.m_max.x,box10.m_min.y,box10.m_min.z,      // v0-v3-v4 (right)
				box10.m_max.x,box10.m_min.y,box10.m_min.z,   box10.m_max.x, box10.m_max.y,box10.m_min.z,   box10.m_max.x, box10.m_max.y, box10.m_max.z,      // v4-v5-v0

				box10.m_max.x, box10.m_max.y, box10.m_max.z,   box10.m_max.x, box10.m_max.y,box10.m_min.z,  box10.m_min.x, box10.m_max.y,box10.m_min.z,      // v0-v5-v6 (top)
				box10.m_min.x, box10.m_max.y,box10.m_min.z,  box10.m_min.x, box10.m_max.y, box10.m_max.z,   box10.m_max.x, box10.m_max.y, box10.m_max.z,      // v6-v1-v0

				box10.m_min.x, box10.m_max.y, box10.m_max.z,  box10.m_min.x, box10.m_max.y,box10.m_min.z,  box10.m_min.x,box10.m_min.y,box10.m_min.z,      // v1-v6-v7 (left)
				box10.m_min.x,box10.m_min.y,box10.m_min.z,  box10.m_min.x,box10.m_min.y, box10.m_max.z,  box10.m_min.x, box10.m_max.y, box10.m_max.z,      // v7-v2-v1

				box10.m_min.x,box10.m_min.y,box10.m_min.z,   box10.m_max.x,box10.m_min.y,box10.m_min.z,   box10.m_max.x,box10.m_min.y, box10.m_max.z,      // v7-v4-v3 (bottom)
				box10.m_max.x,box10.m_min.y, box10.m_max.z,  box10.m_min.x,box10.m_min.y, box10.m_max.z,  box10.m_min.x,box10.m_min.y,box10.m_min.z,      // v3-v2-v7

				box10.m_max.x,box10.m_min.y,box10.m_min.z,	   box10.m_min.x,box10.m_min.y,box10.m_min.z,	box10.m_min.x,box10.m_max.y,box10.m_min.z,      // v4-v7-v6 (back)
				box10.m_min.x, box10.m_max.y,box10.m_min.z,   box10.m_max.x, box10.m_max.y,box10.m_min.z,   box10.m_max.x,box10.m_min.y,box10.m_min.z
			};

			// set the flag if the boxes collide in every axis
			for (int i = 0; i < 10; i++)
				for (int j = 0; j < 10; j++)
				{
					if (table[i][j] >= 3) {
						boxes[i].collide(true);
						boxes[j].collide(true);
						cout << "Box : (" << i << ", " << j << ") , Colliding : " << endl;
					}
					else{
					
					}
				}

			// -----------------------------RENDER-BOXES---------------------------------------
		#pragma region "object_render"

			
			boundBox.Use();
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

			glBindVertexArray(boundingBoxVA);

			glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
			glUniformMatrix4fv(glGetUniformLocation(boundBox.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelc));
			glUniformMatrix4fv(glGetUniformLocation(boundBox.Program, "view"), 1, GL_FALSE, glm::value_ptr(view3));
			glUniformMatrix4fv(glGetUniformLocation(boundBox.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
			glUniform3f(glGetUniformLocation(boundBox.Program, "cameraPos"), camera.Position.x, camera.Position.y, camera.Position.z);

			if(!boxes[0].colliding)
				glUniform3f(glGetUniformLocation(boundBox.Program, "boxColor"), 0.2, 0.72, 0.9);
			else
				glUniform3f(glGetUniformLocation(boundBox.Program, "boxColor"), 1.0f, 0.0f, 0.0f);

			glBindBuffer(GL_ARRAY_BUFFER, boundingBoxVB);
			glBufferData(GL_ARRAY_BUFFER, sizeof(boundingBox), &boundingBox, GL_DYNAMIC_DRAW);

			glDrawArrays(GL_TRIANGLES, 0,sizeof(boundingBox));
			glBindVertexArray(0);

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			//---Render-AABB-with-and-color/collision-information----
	
				renderBoundBox(&boundingBoxVA, &boundingBoxVB, &boundingBox2,&boundBox, boxes[1].colliding);
				renderBoundBox(&boundingBoxVA, &boundingBoxVB, &boundingBox3,&boundBox, boxes[2].colliding);
				renderBoundBox(&boundingBoxVA, &boundingBoxVB, &boundingBox4,&boundBox, boxes[3].colliding);
				renderBoundBox(&boundingBoxVA, &boundingBoxVB, &boundingBox5,&boundBox, boxes[4].colliding);
				renderBoundBox(&boundingBoxVA, &boundingBoxVB, &boundingBox6,&boundBox, boxes[5].colliding);
				renderBoundBox(&boundingBoxVA, &boundingBoxVB, &boundingBox7,&boundBox, boxes[6].colliding);
				renderBoundBox(&boundingBoxVA, &boundingBoxVB, &boundingBox8,&boundBox, boxes[7].colliding);
				renderBoundBox(&boundingBoxVA, &boundingBoxVB, &boundingBox9,&boundBox, boxes[8].colliding);
				renderBoundBox(&boundingBoxVA, &boundingBoxVB, &boundingBox10,&boundBox, boxes[9].colliding);
			

			//-----------------------------------------------------------------------------------

#pragma endregion


	//		deltaTime--;
		//}
			spherePosition.clear();
			//worldvertex.clear();

		// Swap the buffers
		glfwSwapBuffers(window);
	}
	
	glDeleteBuffers(1,&skyboxVBO);
	glDeleteBuffers(1,&vertexbuffer);
	glDeleteBuffers(1,&normalbuffer);
	glDeleteBuffers(1, &cubeVBO);
	glDeleteBuffers(1, &boundingBoxVB);


	glfwTerminate();
	return 0;
}

bool checkaabbX(AABB &a, AABB &b) {
	if ( (b.m_min.x >= a.m_min.x && b.m_min.x <= a.m_max.x) || (a.m_min.x >= b.m_min.x && a.m_min.x <= b.m_max.x))
		return true;
	else if ((b.m_max.x >= a.m_min.x && b.m_max.x <= a.m_max.x) || (a.m_max.x >= b.m_min.x && a.m_max.x <= b.m_max.x))
		return true;
	else
		return false;
}

bool checkaabbY(AABB &a, AABB &b) {
	if ((b.m_min.y >= a.m_min.y && b.m_min.y <= a.m_max.y) || (a.m_min.y >= b.m_min.y && a.m_min.y <= b.m_max.y))
		return true;
	else if ((b.m_max.y >= a.m_min.y && b.m_max.y <= a.m_max.y) || (a.m_max.y >= b.m_min.y && a.m_max.y <= b.m_max.y))
		return true;
	else
		return false;
}

bool checkaabbZ(AABB &a, AABB &b) {
	if ((b.m_min.z >= a.m_min.z && b.m_min.z <= a.m_max.z) || (a.m_min.z >= b.m_min.z && a.m_min.z <= b.m_max.z))
		return true;
	else if ((b.m_max.z >= a.m_min.z && b.m_max.z <= a.m_max.z) || (a.m_max.z >= b.m_min.z && a.m_max.z <= b.m_max.z))
		return true;
	else
		return false;
}


void renderBoundBox(GLuint *va, GLuint *vb, GLfloat(*boundbox)[108],Shader *shader,bool collide){


	if (!collide)
		glUniform3f(glGetUniformLocation(shader->Program, "boxColor"), 0.2, 0.72, 0.9);
	else
		glUniform3f(glGetUniformLocation(shader->Program, "boxColor"), 1.0f, 0.0f, 0.0f);


	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glBindVertexArray(*va);
	glBindBuffer(GL_ARRAY_BUFFER, *vb);
	glBufferData(GL_ARRAY_BUFFER, sizeof(*boundbox), boundbox, GL_DYNAMIC_DRAW);

	glDrawArrays(GL_TRIANGLES, 0, sizeof(*boundbox));
	glBindVertexArray(0);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

}

void renderRigid(std::vector<glm::vec3> *vec, GLuint *va, GLuint *vb) {

	glBindVertexArray(*va);
	glBindBuffer(GL_ARRAY_BUFFER, *vb);
	glBufferData(GL_ARRAY_BUFFER, (*vec).size() * sizeof(glm::vec3), &vec->at(0), GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLES, 0, (*vec).size());

	glBindVertexArray(0);
}


void do_movement(){
	// Camera Controls
	if (keys[GLFW_KEY_W])
		camera.ProcessKeyboard(FORWARD, cameradeltaTime);
	if (keys[GLFW_KEY_S])
		camera.ProcessKeyboard(BACKWARD, cameradeltaTime);
	if (keys[GLFW_KEY_A])
		camera.ProcessKeyboard(LEFT, cameradeltaTime);
	if (keys[GLFW_KEY_D])
		camera.ProcessKeyboard(RIGHT, cameradeltaTime);

	if (keys[GLFW_KEY_E]) {
		camera.ProcessKeyboard(UP, cameradeltaTime);
	}
	if (keys[GLFW_KEY_Q]) {
		camera.ProcessKeyboard(DOWN, cameradeltaTime);
	}
	if (keys[GLFW_KEY_I]) {
		camera.ProcessKeyboard(LOOKUP, cameradeltaTime);
	}
	if (keys[GLFW_KEY_J]) {
		camera.ProcessKeyboard(LOOKDOWN, cameradeltaTime);
	}
}


// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (action == GLFW_PRESS)
		keys[key] = true;
	else if (action == GLFW_RELEASE)
		keys[key] = false;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}
