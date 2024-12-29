#include "stb_image.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <map>
#include <math.h>
#include <random>
#include <vector>

#include "Camera.h"
#include "CustomSceneObject.h"
#include "ArcingProjectileObject.h"
#include "Shader.h"
#include "Model.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "FastNoiseLite.h"



/* TODO
* Physics projectil objects done
* Figure out random location and trajectory spawning
* Make control variables so I can control the speed, dispersion and launch angle easily


*/

using namespace glm;
using namespace std;

//--- Callback method definitions
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

//--- Other methods
void processInput(GLFWwindow* window);
void CreateObject(string name, float vertices[], int verticesElementCount, unsigned int indices[], int indicesCount, vector<int> sectionSizes, int vertexAttributeCount);
void LoadTexture(unsigned int& textureId, const char* filePath);
void CreateProceduralTerrain(float* terrainVertices, int terrainVerticesCount);
void CreateSphereObject();

//--- Screen settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

//--- Proc gen globals
const unsigned int RENDER_DISTANCE = 128;
const unsigned int MAP_SIZE = RENDER_DISTANCE * RENDER_DISTANCE;
const double PI = acos(-1);

//Chunks across one dimension
const int squaresRow = RENDER_DISTANCE - 1;
const int trianglesPerSquare = 2;
const int trianglesGrid = squaresRow * squaresRow * trianglesPerSquare;

//--- Camera values
Camera camera(vec3(0.0f, 1.8f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

//--- Time tracking
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

//--- Scene object containers
map<string, CustomSceneObject*> sceneObjectDictionary;

vector<ArcingProjectileObject*> projectileObjects;

struct Point {
	float x, y, z;

};

int main()
{
	//--- Initialize GLFW
	glfwInit();


#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	//--- Create window
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	//--- Set the version number of GLFW, prevents running without correct version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//--- Set window as current and set callback methods
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	//--- Lock cursor and track mouse movements
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//--- Load openGL function pointers to Glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	//--- Enable depth buffer
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	glEnable(GL_DEPTH_TEST);


	// ---------------------------------------------------
	// Base cube creation
	// ---------------------------------------------------
	float cubeVertices[] = {
		//Position           //Tex coords  //Normals
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  0.0f, 0.0f, -1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,  0.0f, 0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0.0f, 0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0.0f, 0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0.0f, 0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  0.0f, 0.0f, -1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  0.0f, 0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  0.0f, 0.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  0.0f, 0.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,  0.0f, 0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  0.0f, 0.0f, 1.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  -1.0f, 0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  -1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  -1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  -1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  -1.0f, 0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  -1.0f, 0.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  1.0f, 0.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  1.0f, 0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  1.0f, 0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  1.0f, 0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  1.0f, 0.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  0.0f, -1.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,  0.0f, -1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.0f, -1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.0f, -1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  0.0f, -1.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  0.0f, -1.0f, 0.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0.0f, 1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0.0f, 1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,  0.0f, 1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0.0f, 1.0f, 0.0f
	};

	//--- Duplicate cube positions
	vec3 cubePositions[] = {
		vec3(0.0f,  0.0f,  0.0f),
		vec3(2.0f,  5.0f, -15.0f),
		vec3(-1.5f, -2.2f, -2.5f),
		vec3(-3.8f, -2.0f, -12.3f),
		vec3(2.4f, -0.4f, -3.5f),
		vec3(-1.7f,  3.0f, -7.5f),
		vec3(1.3f, -2.0f, -2.5f),
		vec3(1.5f,  2.0f, -2.5f),
		vec3(1.5f,  0.2f, -1.5f),
		vec3(-1.3f,  1.0f, -1.5f)
	};	

	int cubeVertexSize = 8;
	vector<int> cubeSectionSizes =
	{
		3, //Position
		2, //Tex coords
		3, //Normals
	};

	int cubeVerticesCount = sizeof(cubeVertices) / (sizeof(cubeVertices[0]) * cubeVertexSize);

	CreateObject("Cube Object", cubeVertices, cubeVerticesCount, NULL, 0, cubeSectionSizes, cubeVertexSize);



	// ----------------------------------------------
	// Ground plane creation
	// ----------------------------------------------
	float planeVertices[] = {
		//Positions             //Textures
		0.5f, 0.5f, 0.0f,       1.0f, 1.0f, //top right
		0.5f, -0.5f, 0.0f,      1.0f, 0.0f, //bottom right
		-0.5f, -0.5f, 0.0f,     0.0f, 0.0f, //bottom left
		-0.5f, 0.5f, 0.0f,      0.0f, 1.0f  //top left
	};

	unsigned int planeIndices[] = {
		0, 1, 3, //First triangle
		1, 2, 3  //Second triangle
	};		

	int planeAttributeSize = 5;
	vector<int> planeAttributeSizes =
	{
		3, //Position
		2  //Tex coords
	};

	int planeVerticesCount = sizeof(planeVertices) / sizeof(planeVertices[0]) * planeAttributeSize;
	int planeIndicesCount = sizeof(planeIndices) / sizeof(planeIndices[0]);

	CreateObject("Plane Object", planeVertices, planeIndicesCount, planeIndices, planeIndicesCount, planeAttributeSizes, planeAttributeSize);
	

	// ----------------------------------------------
	// Projectile cube prefab creation
	// ----------------------------------------------
	CreateObject("Projectile Base", cubeVertices, cubeVerticesCount, NULL, 0, cubeSectionSizes, cubeVertexSize);


	//--- Projectile spawning variables
	vec3 spawnCentre = vec3(0.0f, 0.0f, 0.0f);
	float spawnRadius = 3.0f;
	float projectileSpawnCooldown = 0.2f;
	float projectileSpawnTimer = 0.0f;


	// --------------------------------------------
	// Shader creation
	// --------------------------------------------
	Shader TexturedObjectShader("Shaders/VertexShader.v", "Shaders/FragmentShader.f");
	TexturedObjectShader.Use();
	//The texture sampler on the fragment shader is given value '0' now, means later on in the render loop it will use texture unit zero
	TexturedObjectShader.setInt("texture1", 0);
	TexturedObjectShader.setBool("useTexture", false);
	TexturedObjectShader.setVec3("objectColor", vec3(1.0f, 0.5f, 0.31f));
	TexturedObjectShader.setVec3("lightColor", vec3(1.2f, 1.0f, 2.0f));
	TexturedObjectShader.setVec3("material.ambient", 1.0f, 0.5f, 0.31f);
	TexturedObjectShader.setVec3("material.diffuse", 1.0f, 0.5f, 0.31f);
	TexturedObjectShader.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
	TexturedObjectShader.setFloat("material.shininess", 32.0f);
	TexturedObjectShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
	TexturedObjectShader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f); // darken diffuse light a bit
	TexturedObjectShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
	//TexturedObjectShader.setVec3("light.direction", -0.2f, -1.0f, -0.3f);
	TexturedObjectShader.setFloat("light.constant", 1.0f);
	TexturedObjectShader.setFloat("light.linear", 0.09f);
	TexturedObjectShader.setFloat("light.quadratic", 0.032f);
	TexturedObjectShader.setVec3("light.position", camera.Position);
	TexturedObjectShader.setVec3("light.direction", camera.Front);
	TexturedObjectShader.setFloat("light.cutOff", cos(radians(12.5f)));
	
	Shader ProceduralObjectShader("Shaders/TerrainVertexShader.v", "Shaders/TerrainFragmentShader.f");

	// --------------------------------------------
	// Texture loading
	// --------------------------------------------
	unsigned int texture1;

	LoadTexture(texture1, "Media/container.jpg");	


	


	//--- Coordinate space and 3D
	// OpenGL expects all the vertices that we want visible to be in normalised device coordinates -1 - 1.
	// Coordinates outside this range are not visible.
	// We usually specify the coordinates in a range (or space) and in the vertex shader transform these into normalized device coordinates.
	// These are given to the rasterizer to transform them into 2D pixels on the screen.
	/* This is a several step process where coordinates are moved between 5 different coordinate spaces:
	* - Local Space or Object Space
	* - World Space
	* - View space
	* - Clip Space
	* - Screen Space
	*
	* Converting between these coordinate spaces is done through the use of the Model, View and Projection matrices.
	* Vertex coordinates start in local space as local coordinates and are then further processed to world coordinates, view coordinates, clip coordinates and eventually end up as screen coordinates.
	* World to view space coordinates are transformed in such a way that each coordinate is as seen from the camera's or viewers point of view
	* Clip coordinates are processed to the -1 - 1 range and determine which vertices appear on screen,
	* Then the -1 - 1 coordinates transforms it to the range determined by the viewport size, as not everything is a perfect square.
	* Those resulting coordinates then go to the fragment shader.
	* See a better explanation at https://learnopengl.com/Getting-started/Coordinate-Systems
	*
	*/

	

	// -----------------------------------------
	// Procedural terrain generation
	// -----------------------------------------
	//NOTE: Need to look through this later to double check understanding

	float terrainVertices[MAP_SIZE][6];
	int terrainVerticesCount = sizeof(terrainVertices) / sizeof(terrainVertices[0]);

	CreateProceduralTerrain(&terrainVertices[0][0], terrainVerticesCount);

	


	// ----------------------------------------
	// Sphere object creation
	// ----------------------------------------

	/* Sphere vertices principles
	* https://www.songho.ca/opengl/gl_sphere.html
	* A sphere has an infinite number of vertices, so we sample at certain points instead.
	* These samples are 'rings', various points on the sphere that creates sectors and stacks at intervals.
	* Sectors are the flat loops that run from top to bottom, like slicing the sphere horizontally with a knife.
	* Stacks are the verticle slices.
	* Sectors mark the longitude of a sphere, stacks are the latitude.
	* Longitude is therefore measured from 0 - 180, latitude from 0 -360.
	* A point on the surface of a sphere can be deduced by two angles:
	* Theta (vertical stack)(0 with a horizontal cross) for the longitude angle.
	* Ranges from 0 to 2PI, 0 to 360 degrees.
*
	* Phi (horizontal sector)(0 with a vertical line) for latitude, ironically the line is the inverse of the slice that measurement represents
	* Ranges from 0 to PI
	* Any point can be calulcated with the equation:
	* x = (radius *cos(phi)) *cos(theta)
	* y = (radius *cos(phi)) *sin(theta)
	* z = radius *sin(phi)


	*/

	CreateSphereObject();	

	Shader sphereShader("Shaders/SphereVertexShader.v", "Shaders/SphereFragmentShader.f");

	sphereShader.Use();
	sphereShader.setVec3("objectColor", vec3(1.0f, 0.5f, 0.31f));
	sphereShader.setVec3("lightColor", vec3(1.0f, 1.0f, 1.0f));
	sphereShader.setVec3("objectColor", vec3(1.0f, 0.5f, 0.31f));
	sphereShader.setVec3("lightColor", vec3(1.2f, 1.0f, 2.0f));
	sphereShader.setVec3("material.ambient", 1.0f, 0.5f, 0.31f);
	sphereShader.setVec3("material.diffuse", 1.0f, 0.5f, 0.31f);
	sphereShader.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
	sphereShader.setFloat("material.shininess", 32.0f);
	sphereShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
	sphereShader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f); // darken diffuse light a bit
	sphereShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);


	// ----------------------------------------
	// Light object creation
	// ----------------------------------------

	float lightCubeVertices[] = {
		-0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,

		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,

		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,

		-0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f
	};

	int lightCubeAttributesSize = 3;
	vector<int> lightCubeSectionSizes =
	{
		3, //Position
	};

	vec3 pointLightPositions[] = {
		glm::vec3(0.7f,  0.2f,  2.0f),
		glm::vec3(2.3f, -3.3f, -4.0f),
		glm::vec3(-4.0f,  2.0f, -12.0f),
		glm::vec3(0.0f,  0.0f, -3.0f)
	};

	int lightCubeVerticesCount = sizeof(lightCubeVertices) / (sizeof(lightCubeVertices[0] * lightCubeAttributesSize));
	CreateObject("Light Object", lightCubeVertices, lightCubeVerticesCount, NULL, 0, lightCubeSectionSizes, lightCubeAttributesSize);

	vec3 lightColour(1.0f, 1.0f, 1.0f);

	Shader lightShader("Shaders/LightsourceVertexShader.v", "Shaders/LightsourceFragmentShader.f");

	lightShader.Use();
	lightShader.setVec3("objectColor", vec3(1.0f, 1.0f, 1.0f));	

	vec3 lightPos(5.0f, 7.0f, -2.0f);
	mat4 lightModel = mat4(1.0f);
	lightModel = translate(lightModel, lightPos);
	lightModel = scale(lightModel, vec3(1.0f));
	
	TexturedObjectShader.Use();
	TexturedObjectShader.setVec3("lightPos", lightPos);

	sphereShader.Use();
	sphereShader.setVec3("lightPos", lightPos);
	sphereShader.setBool("flatShading", false);

	FastNoiseLite lightColourNoiseGenerator;
	lightColourNoiseGenerator.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	lightColourNoiseGenerator.SetFrequency(0.2f);

	vec3 RedColour(1.0f, 0.0f, 0.0f);
	vec3 OrangeColour(1.0f, 0.65f, 0.0f);

	const int lightNoiseTextureLength = 512;
	int lightNoiseTextureCurrentIndex = 0;
	float lightNoiseScale = 0.4f;

	float lightNoiseValues[lightNoiseTextureLength];

	for (int i = 0; i < lightNoiseTextureLength; i++)
	{
		lightNoiseValues[i] = lightColourNoiseGenerator.GetNoise((float)i * lightNoiseScale, 0.0f);
	}


	// ----------------------------------
	// Sphere proc gen setup
	// ---------------------------------
	const int noiseWidth = 512;
	const int noiseHeight = 256;
	vector<float> noiseData(noiseWidth* noiseHeight);


	FastNoiseLite sphereNoiseGenerator;
	sphereNoiseGenerator.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	sphereNoiseGenerator.SetFrequency(0.08f);

	float noiseScale = 0.25f;

	for (int y = 0; y < noiseHeight; ++y) {
		for (int x = 0; x < noiseWidth; ++x) {
			int index = y * noiseWidth + x;

			float scaledX = (float)x * noiseScale;
			float scaledY = (float)y * noiseScale;


			noiseData[index] = (sphereNoiseGenerator.GetNoise(scaledX, scaledY) + 1.0f) * 0.5f;
		}
	}

	unsigned int noiseTexture;
	glGenTextures(1, &noiseTexture);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, noiseWidth, noiseHeight, 0, GL_RED, GL_FLOAT, noiseData.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	
	sphereShader.setInt("noiseTexture", 1);


	// ----------------------------------
	// Model importing
	// ----------------------------------
	
	Shader modelShader("Shaders/ModelVertexShader.v", "Shaders/ModelFragmentShader.f");
	Model ourModel("Media/BackpackModel/backpack.obj");

	// NOTE:
	// sphere noise texture bound to texture unit 1

	// -----------------------------------
	// Main render loop
	// -----------------------------------
	while (!glfwWindowShouldClose(window))
	{

		// -------------------------------------
		// Deltatime logic
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// -------------------------------------
		// Poll user input
		processInput(window);

		//--------------------------------------
		// Clear screen and set it to the random colour
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//-------------------------------------
		// Texture assigning
		// Set one of the texture units to the texture created, to use all textures in one draw call
		// The uniform values Texture1 and Texture 2 are used here, as the texture units 1 and 0 are created, those shaders use those
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1); //Like last time, future operations will affect this texture


		//---------------------------------------
		// Activate shader
		TexturedObjectShader.Use();

		//--------------------------------------
		// Pass updated projection matrix to vertex shaders
		mat4 projection = perspective(radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		TexturedObjectShader.setMat4("projection", projection);

		//--- Pass updated view matrix to vertex shaders
		//--Creating cameras
		//The camera / view space are all the vector coordinates as seen from the camera's perspective as the origin of the scene
		//THe view matrix transforms all world coordinates into view coordinates relative to the camera's position and direction.

		mat4 view = camera.GetViewMatrix();
		TexturedObjectShader.setMat4("view", view);

		//-----------------------------------
		// Multiple lights
		lightNoiseTextureCurrentIndex = (int)(currentFrame * 0.1f * lightNoiseTextureLength) % lightNoiseTextureLength;
		float noiseValue = lightNoiseValues[lightNoiseTextureCurrentIndex];


		vec3 lightColour = RedColour + (OrangeColour - RedColour) * noiseValue;
			
			//vec3(noiseValue, 1.0f - noiseValue, 0.5f);


		 /*
		   Here we set all the uniforms for the 5/6 types of lights we have. We have to set them manually and index
		   the proper PointLight struct in the array to set each uniform variable. This can be done more code-friendly
		   by defining light types as classes and set their values in there, or by using a more efficient uniform approach
		   by using 'Uniform buffer objects', but that is something we'll discuss in the 'Advanced GLSL' tutorial.
		*/
		// directional light
		TexturedObjectShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
		TexturedObjectShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
		TexturedObjectShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
		TexturedObjectShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
		// point light 1
		TexturedObjectShader.setVec3("pointLights[0].position", pointLightPositions[0]);
		TexturedObjectShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
		TexturedObjectShader.setVec3("pointLights[0].diffuse", lightColour);
		TexturedObjectShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
		TexturedObjectShader.setFloat("pointLights[0].constant", 1.0f);
		TexturedObjectShader.setFloat("pointLights[0].linear", 0.09f);
		TexturedObjectShader.setFloat("pointLights[0].quadratic", 0.032f);
		// point light 2
		TexturedObjectShader.setVec3("pointLights[1].position", pointLightPositions[1]);
		TexturedObjectShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
		TexturedObjectShader.setVec3("pointLights[1].diffuse", lightColour);
		TexturedObjectShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
		TexturedObjectShader.setFloat("pointLights[1].constant", 1.0f);
		TexturedObjectShader.setFloat("pointLights[1].linear", 0.09f);
		TexturedObjectShader.setFloat("pointLights[1].quadratic", 0.032f);
		// point light 3
		TexturedObjectShader.setVec3("pointLights[2].position", pointLightPositions[2]);
		TexturedObjectShader.setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
		TexturedObjectShader.setVec3("pointLights[2].diffuse", lightColour);
		TexturedObjectShader.setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
		TexturedObjectShader.setFloat("pointLights[2].constant", 1.0f);
		TexturedObjectShader.setFloat("pointLights[2].linear", 0.09f);
		TexturedObjectShader.setFloat("pointLights[2].quadratic", 0.032f);
		// point light 4
		TexturedObjectShader.setVec3("pointLights[3].position", pointLightPositions[3]);
		TexturedObjectShader.setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
		TexturedObjectShader.setVec3("pointLights[3].diffuse", lightColour);
		TexturedObjectShader.setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
		TexturedObjectShader.setFloat("pointLights[3].constant", 1.0f);
		TexturedObjectShader.setFloat("pointLights[3].linear", 0.09f);
		TexturedObjectShader.setFloat("pointLights[3].quadratic", 0.032f);
		// spotLight
		TexturedObjectShader.setVec3("spotLight.position", camera.Position);
		TexturedObjectShader.setVec3("spotLight.direction", camera.Front);
		TexturedObjectShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
		TexturedObjectShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
		TexturedObjectShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
		TexturedObjectShader.setFloat("spotLight.constant", 1.0f);
		TexturedObjectShader.setFloat("spotLight.linear", 0.09f);
		TexturedObjectShader.setFloat("spotLight.quadratic", 0.032f);
		TexturedObjectShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
		TexturedObjectShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));


		//--- Render cubes		
		for (unsigned int i = 0; i < 10; i++)
		{
			// calculate the model matrix for each object and pass it to shader before drawing
			mat4 model = mat4(1.0f); // make sure to initialize matrix to identity matrix first
			model = translate(model, cubePositions[i]);
			float angle = 20.0f * i;
			model = rotate(model, radians(angle), vec3(1.0f, 0.3f, 0.5f));
			TexturedObjectShader.setMat4("model", model);

			TexturedObjectShader.setVec3("lightPos", lightPos);
			TexturedObjectShader.setVec3("viewPos", camera.Position);
			//TexturedObjectShader.setVec3("light.position", camera.Position);
			//TexturedObjectShader.setVec3("light.direction", camera.Front);
			//TexturedObjectShader.setFloat("light.cutOff", glm::cos(glm::radians(12.5f)));
			//TexturedObjectShader.setFloat("light.outerCutOff", glm::cos(glm::radians(17.5f)));

			sceneObjectDictionary["Cube Object"]->DrawMesh();
		}

		//--- Render Plane
		mat4 model = mat4(1.0f);
		model = rotate(model, radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
		model = scale(model, vec3(8.0f, 8.0f, 8.0f));

		TexturedObjectShader.setMat4("model", model);

		sceneObjectDictionary["Plane Object"]->DrawMesh();

		//-- Random stuff for projectile generation
		random_device rd; //Seed generation
		mt19937 gen(rd()); //Random value generator using the this mt19937 method. Creates large integers
		uniform_real_distribution<> dis(0.0, 1.0);

		//-- Projectile spawning
		projectileSpawnTimer += deltaTime;
		if (projectileSpawnTimer >= projectileSpawnCooldown)
		{
			ArcingProjectileObject* newProjectileObject = new ArcingProjectileObject();
			newProjectileObject->VAO = sceneObjectDictionary["Projectile Base"]->VAO;
			newProjectileObject->PrepareAndBindVBO(sceneObjectDictionary["Projectile Base"]->VBO, sizeof(cubeVertices) / (5 * sizeof(float)));


			GLenum error;
			while ((error = glGetError()) != GL_NO_ERROR) {
				cerr << "OpenGL error after VBO binding: " << error << endl;
			}

			
			Point topLeft = { -10.0, 0.0, -5.0 };   
			Point bottomRight = { 10.0, 0.0, 5.0 };			

			
			double randomX = topLeft.x + (bottomRight.x - topLeft.x) * dis(gen);
			double randomY = bottomRight.y + (topLeft.y - bottomRight.y) * dis(gen);
			double randomZ = topLeft.z; 

			vec3 spawnPosition = vec3(randomX, randomY, randomZ);			

			//Value between 0 and 1 as before, multiply by what is 45 degrees in radians, means the resulting angle will be less than 45. Could set it to 2*pi for a full circle for instance
			//Vertical angle value
			float theta = dis(gen) * (PI / 4);

			//Horizontal angle value
			float azimuth = dis(gen) * (2 * PI);

			float vx = sin(theta) * cos(azimuth);
			float vy = cos(theta);
			float vz = sin(theta) * sin(azimuth);

			vec3 spawnVelocity = normalize(vec3(vx, vy, vz));
			spawnVelocity = spawnVelocity * 10.0f;

			//cout << "Random Point: (" << x << ", " << y << ", " << z << ")\n";
			//cout << "Random Velocity: (" << vx << ", " << abs(vy) << ", " << vz << ")\n";


			newProjectileObject->Launch(vec3(spawnVelocity), vec3(spawnPosition), currentFrame);

			projectileObjects.push_back(newProjectileObject);

			projectileSpawnTimer = 0.0f;
		}

		for (size_t i = 0; i < projectileObjects.size();) {
			ArcingProjectileObject* projectileObject = projectileObjects[i];
			if (projectileObject != NULL)
			{
				projectileObject->UpdatePosition(deltaTime);

				if (projectileObject->ShouldDestroy()) {
					delete projectileObject;
					projectileObjects.erase(projectileObjects.begin() + i);

				}
				else {
					mat4 projectileModel = mat4(1.0f);
					projectileModel = translate(projectileModel, projectileObject->initialPosition + (projectileObject->currentPosition - projectileObject->initialPosition));
					TexturedObjectShader.setMat4("model", projectileModel);

					GLenum error;
					while ((error = glGetError()) != GL_NO_ERROR) {
						cerr << "OpenGL error pre render: " << error << endl;
					}


					//TEMP LIGHTING STUFF
					TexturedObjectShader.setVec3("lightPos", lightPos);
					TexturedObjectShader.setVec3("viewPos", camera.Position);
					TexturedObjectShader.setVec3("light.position", camera.Position);
					TexturedObjectShader.setVec3("light.direction", camera.Front);
					TexturedObjectShader.setFloat("light.cutOff", glm::cos(glm::radians(12.5f)));
					TexturedObjectShader.setFloat("light.outerCutOff", glm::cos(glm::radians(17.5f)));
					projectileObject->DrawMesh();


					error;
					while ((error = glGetError()) != GL_NO_ERROR) {
						cerr << "OpenGL error post render: " << error << endl;
					}

					++i;
				}
			}


		}

		//Configure procedural generation shader attributes
		ProceduralObjectShader.Use();

		//Terrain
		mat4 terrainModel = mat4(1.0f);
		terrainModel = scale(terrainModel, vec3(2.0f, 2.0f, 2.0f));
		//Looking straight forward
		terrainModel = rotate(terrainModel, radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
		//Elevation to look upon terrain
		terrainModel = translate(terrainModel, vec3(0.0f, -2.f, -1.5f));
		ProceduralObjectShader.setMat4("model", terrainModel);
		ProceduralObjectShader.setMat4("projection", projection);
		ProceduralObjectShader.setMat4("view", view);

		sceneObjectDictionary["Procedural Terrain"]->DrawMesh();


		
		GLenum error;
		while ((error = glGetError()) != GL_NO_ERROR) {
			cerr << "OpenGL error post proc gen render: " << error << endl;
		}

		//---------------------------
		//Sphere displacement and rendering
		mat4 sphereModel = mat4(1.0f);
		sphereModel = translate(sphereModel, vec3(8.0f, 0.0f, -12.0f));

		sphereShader.Use();

		sphereShader.setMat4("model", sphereModel);
		sphereShader.setMat4("projection", projection);
		sphereShader.setMat4("view", view);
			
		sphereShader.setFloat("time", currentFrame);
		sphereShader.setFloat("displacementScale", 1.0f);
		glActiveTexture(GL_TEXTURE1);

		sphereShader.setVec3("lightPos", lightPos);
		sphereShader.setVec3("viewPos", camera.Position);

		
		sceneObjectDictionary["Sphere Object"]->DrawMesh();


		error;
		while ((error = glGetError()) != GL_NO_ERROR) {
			cerr << "OpenGL error post proc gen render: " << error << endl;
		}

		//Light source
		lightShader.Use();
		lightShader.setMat4("model", lightModel);
		lightShader.setMat4("projection", projection);
		lightShader.setMat4("view", view); 

		sceneObjectDictionary["Light Object"]->DrawMesh();
		
		error;
		while ((error = glGetError()) != GL_NO_ERROR) {
			cerr << "OpenGL error post light render: " << error << endl;
		}

		// ---------------------
		// Imported models
		// Doesn't work without custom shader
		modelShader.Use();
		mat4 modelLocation = mat4(1.0f);
		modelLocation = translate(modelLocation, vec3(2.0f, 1.0f, 0.0f));
		modelShader.setMat4("model", modelLocation);
		modelShader.setMat4("projection", projection);
		modelShader.setMat4("view", view);
		ourModel.Draw(modelShader);

		//--- Swap buffers to render to screen, poll IO events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	for (auto& pair : sceneObjectDictionary)
	{
		CustomSceneObject* object = pair.second;
		if (object)
		{
			object->CleanUp();
		}
	}

	for (ArcingProjectileObject* projectile : projectileObjects)
	{
		projectile->CleanUp();
	}

	sceneObjectDictionary.clear();
	projectileObjects.clear();


	glfwTerminate();
	return 0;


}

//--- Input polling
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

//--- Callback method when window is resized
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}


//--- Mouse movement callback to change camera position
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;


	camera.ProcessMouseMovement(xoffset, yoffset);
}

//--- Scroll method to change camera zoom
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}




/// <summary>
/// 
/// </summary>
/// <param name="name">Name to be added to scene object dictionary</param>
/// <param name="vertices">Vertices array, will be passed in as pointer</param>
/// <param name="verticesCount">Total number of vertices</param>
/// <param name="indices">Vertices array, will be passed in as pointer</param>
/// <param name="indicesCount">Total number of indices in array</param>
/// <param name="sectionSizes">Vector listing the sizes of each attribute section in sequence</param>
/// <param name="vertexAttributeCount">Number of floats per vertex</param>
void CreateObject(string name, float vertices[], int verticesElementCount, unsigned int indices[], int indicesCount, vector<int> sectionSizes, int vertexAttributeCount) {
	CustomSceneObject* newObject = new CustomSceneObject();
	newObject->PrepareAndBindVAO();

	size_t verticesDataSize = verticesElementCount * vertexAttributeCount * sizeof(float);
	newObject->PrepareAndBindVBO(vertices, verticesDataSize, verticesElementCount);

	if (indicesCount > 0)
	{
		size_t indicesDataSize = indicesCount * sizeof(unsigned int);
		newObject->PrepareAndBindEBO(indices, indicesDataSize, indicesCount);
	}

	newObject->PrepareVertexAttributeArrays(sectionSizes, vertexAttributeCount);

	sceneObjectDictionary[name] = newObject;
}

void LoadTexture(unsigned int& textureId, const char* filePath) {
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);

	float borderColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	/*With the texture bound we can change it's wrapping method
		GL_CLAMP_TO_BORDER prevents the texture from overspilling
		GL_CLAMP_TO_EDGE allows the edge pixels to continuously overspill
		GL_REPEAT allows the texture to repeatedly render itself adjacently
		GL_MIRRORED_REPEAT accomplishes GL_REPEAT while also flipping textures in the given direction of their given adjacent texture
	*/
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


	// Texture filtering allows for approximation of values based on neigbouring pixels.
	// Nearest looks blocky, a point between two texels is simply the texture coordinate it is in, leads to a pixelly effect
	// Linear performs interpolation between texels, so pixels between two texels are smoothened out
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	// load image, create texture and generate mipmaps
	int imageWidth, imageHeight, nrChannels;
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	unsigned char* data = stbi_load(filePath, &imageWidth, &imageHeight, &nrChannels, 0);
	if (data)
	{
		//Generation of texture from retrieved texture data
		// TExture buffer used
		// Mipmap level
		// Image colour format
		// X dimension of the texture
		// y dimension of the texture
		// set to 0, a legacy feature thing
		// Source image colour format
		// The form of data in which we stored the original image
		// The image data itself
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
}


void CreateProceduralTerrain(float* terrainVertices, int terrainVerticesCount) {
	//Positions to start drawing from
	float drawingStartPosition = 1.0f;
	float columnVerticesOffset = drawingStartPosition;
	float rowVerticesOffset = drawingStartPosition;

	int rowIndex = 0;

	for (int i = 0; i < MAP_SIZE; i++)
	{
		//Generation of x & z vertices for horizontal plane
		terrainVertices[i * 6 + 0] = columnVerticesOffset;
		//terrainVertices[i][1] = 0.0f;
		terrainVertices[i * 6 + 2] = rowVerticesOffset;

		//Colour
		//terrainVertices[i][3] = 0.0f;
		//terrainVertices[i][4] = 0.75f;
		//terrainVertices[i][5] = 0.25f;

		//Shifts x position across for next triangle along grid
		columnVerticesOffset = columnVerticesOffset + -0.0625f;

		//Indexing of each chunk within row
		rowIndex++;
		//True when all triangles of the current row have been generated
		if (rowIndex == RENDER_DISTANCE)
		{
			//Resets for next row of triangles
			rowIndex = 0;
			//Resets x position for next row of triangles
			columnVerticesOffset = drawingStartPosition;
			//Shifts y position
			rowVerticesOffset = rowVerticesOffset + -0.0625f;
		}
	}

	//Generation of height map indices
	unsigned int terrainIndices[trianglesGrid][3];

	//Positions to start mapping indices from
	int columnIndicesOffset = 0;
	int rowIndicesOffset = 0;

	//Generation of map indices in the form of chunks (1x1 right angle triangle squares)
	rowIndex = 0;
	for (int i = 0; i < trianglesGrid - 1; i += 2)
	{
		terrainIndices[i][0] = columnIndicesOffset + rowIndicesOffset; //top left
		terrainIndices[i][2] = RENDER_DISTANCE + columnIndicesOffset + rowIndicesOffset; //bottom left
		terrainIndices[i][1] = 1 + columnIndicesOffset + rowIndicesOffset; //top right

		terrainIndices[i + 1][0] = 1 + columnIndicesOffset + rowIndicesOffset; //top right
		terrainIndices[i + 1][2] = RENDER_DISTANCE + columnIndicesOffset + rowIndicesOffset; //bottom left
		terrainIndices[i + 1][1] = 1 + RENDER_DISTANCE + columnIndicesOffset + rowIndicesOffset; //bottom right

		//Shifts x position across for next chunk along grid
		columnIndicesOffset = columnIndicesOffset + 1;

		//Indexing of each chunk within row
		rowIndex++;

		//True when all chunks of the current row have been generated
		if (rowIndex == squaresRow)
		{
			//Resets for next row of chunks
			rowIndex = 0;
			//Resets x position for next row of chunks
			columnIndicesOffset = 0;
			//Shifts y position
			rowIndicesOffset = rowIndicesOffset + RENDER_DISTANCE;
		}
	}

	//--- Fast noise lite
	FastNoiseLite terrainNoise;

	terrainNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	terrainNoise.SetFrequency(0.05f);
	int terrainSeed = rand() % 100;
	terrainNoise.SetSeed(terrainSeed);

	//Biome generation
	FastNoiseLite biomeNoise;
	biomeNoise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
	biomeNoise.SetFrequency(0.05f);
	int biomeSeed = rand() % 100;
	biomeNoise.SetSeed(biomeSeed);

	//--- Height variation
	//Terrain vertice index
	int i = 0;
	//Using x & y nested for loop in order to apply noise 2-dimensionally
	for (int y = 0; y < RENDER_DISTANCE; y++)
	{
		for (int x = 0; x < RENDER_DISTANCE; x++)
		{
			//Setting of height from 2D noise value at respective x & y coordinate
			terrainVertices[i * 6 + 1] = terrainNoise.GetNoise((float)x, (float)y);

			float biomeValue = biomeNoise.GetNoise((float)x, (float)y);

			if (biomeValue <= -0.75f) //Plains
			{
				terrainVertices[i * 6 + 3] = 0.0f;
				terrainVertices[i * 6 + 4] = 0.75f;
				terrainVertices[i * 6 + 5] = 0.25f;
			}
			else //Desert
			{
				terrainVertices[i * 6 + 3] = 1.0f;
				terrainVertices[i * 6 + 4] = 1.0f;
				terrainVertices[i * 6 + 5] = 0.5f;
			}

			i++;
		}
	}

	int terrainAttributeSize = 6;
	vector<int> terrainSectionSizes =
	{
		3, //Position
		3  //Colour
	};

	int indicesCount = sizeof(terrainIndices) / sizeof(terrainIndices[0][0]);
	size_t indicesDataSize = indicesCount * sizeof(unsigned int);
	CreateObject("Procedural Terrain", terrainVertices, terrainVerticesCount, &terrainIndices[0][0], indicesCount, terrainSectionSizes, terrainAttributeSize);

	
}

void CreateSphereObject() {
	float radius = 2.0f;
	const int longitudeSteps = 36;
	const int latitudeSteps = 18;

	//Access with latitude then longitude, starts top left, moves around then down and around again
	//Each vertex of the sphere is defined by lat and lon, each holding 3 values for position, 3 for colour
	float sphereVertices[latitudeSteps][longitudeSteps][11];

	for (int lat = 0; lat < latitudeSteps; lat++)
	{
		for (int lon = 0; lon < longitudeSteps; lon++)
		{
			float theta = 2.0f * PI * lon / longitudeSteps;
			float phi = PI * lat / (latitudeSteps - 1);

			//Performs the formulas described above
			float x = radius * sin(phi) * cos(theta);
			float y = radius * cos(phi);
			float z = radius * sin(phi) * sin(theta);

			//Normal vector for each coord is just the position vector normalised 
			float normalX = x / radius;
			float normalY = y / radius;
			float normalZ = z / radius;

			if (fabs(x) < 1e-6) x = 0.0f;
			if (fabs(y) < 1e-6) y = 0.0f;
			if (fabs(z) < 1e-6) z = 0.0f;

			sphereVertices[lat][lon][0] = x;
			sphereVertices[lat][lon][1] = y;
			sphereVertices[lat][lon][2] = z;

			sphereVertices[lat][lon][3] = (float)lon / (longitudeSteps - 1); 
			sphereVertices[lat][lon][4] = (float)lat / (latitudeSteps - 1); 

			sphereVertices[lat][lon][5] = 0.0f;
			sphereVertices[lat][lon][6] = 0.75f;
			sphereVertices[lat][lon][7] = 0.25f;

			sphereVertices[lat][lon][8] = normalX;
			sphereVertices[lat][lon][9] = normalY;
			sphereVertices[lat][lon][10] = normalZ;


		}
	}

	//Indices array is one dimensional, but assumes size 3 for each element
	unsigned int sphereIndices[(longitudeSteps - 1) * (latitudeSteps - 1) * 6];



	int i = 0;
	for (int lat = 0; lat < latitudeSteps - 1; lat++) {
		for (int lon = 0; lon < longitudeSteps - 1; lon++) {

			//Since sphereVertices is 2 dimensional [lat][lon], this will set the index to the 'flattened index'
			//For instance, 2 * longitudeSteps + 3 would be sphereVertices[2][3]
			//In this case, lat + 1 moves it down 1

			
			int topLeft = lat * longitudeSteps + lon;          // Top-left vertex
			int bottomLeft = (lat + 1) * longitudeSteps + lon; // Bottom-left vertex
			int topRight = lat * longitudeSteps + (lon + 1);   // Top-right vertex
			int bottomRight = (lat + 1) * longitudeSteps + (lon + 1); // Bottom-right vertex

			
			sphereIndices[i] = topLeft; 
			sphereIndices[i + 1] = bottomRight;
			sphereIndices[i + 2] = topRight; 

			
			sphereIndices[i + 3] = bottomRight;
			sphereIndices[i + 4] = bottomLeft;
			sphereIndices[i + 5] = topLeft;





			i += 6;
		}
	}


	int sphereAttributeSize = 11;
	vector<int> sphereSectionSizes =
	{
		3, //Position
		2, //UV
		3, //Colour
		3  //Normal
	};

	int sphereVerticesCount = sizeof(sphereVertices) / sizeof(sphereVertices[0][0]);
	int indicesCount = sizeof(sphereIndices) / sizeof(sphereIndices[0]);
	size_t indicesDataSize = indicesCount * sizeof(unsigned int);
	CreateObject("Sphere Object", sphereVertices[0][0], sphereVerticesCount, &sphereIndices[0], indicesCount, sphereSectionSizes, sphereAttributeSize);

}





