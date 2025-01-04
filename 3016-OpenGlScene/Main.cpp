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

#include "ArcingProjectileObject.h"
#include "Camera.h"
#include "CustomSceneObject.h"
#include "Model.h"
#include "Shader.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <irrklang/irrKlang.h>

#include "FastNoiseLite.h"

#include "PointLight.h"


using namespace glm;
using namespace std;
using namespace irrklang;

//--- Callback method definitions
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);


#pragma region Globals and settings
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
map<ArcingProjectileObject*, ISound*> bubbleSounds;

//--- Sphere object constants
const float sphereRadius = 1.2f;
const int longitudeSteps = 36;
const int latitudeSteps = 18;
const int sphereVerticesCount = latitudeSteps * longitudeSteps;
const int sphereIndicesCount = (longitudeSteps - 1) * (latitudeSteps - 1) * 6;

// -- Texture holder
map<string, unsigned int> texNameToId;
map<string, unsigned int> texNameToUnitNo;
int currentUnit = 0;

//-- For random number generation
random_device rd; //Seed generation
mt19937 gen(rd()); //Random value generator using the this mt19937 method. Creates large integers
uniform_real_distribution<> dis(0.0, 1.0);

//-- Audio
bool isPlayingBackgroundAudio = true;
ISoundEngine* audioEngine;
ISound* backgroundSound;
bool spacePressed = false;
#pragma endregion Globals and settings


//--- Method prototypes
void processInput(GLFWwindow* window);
void CreateObject(string name, float vertices[], int verticesElementCount, unsigned int indices[], int indicesCount, vector<int> sectionSizes, int vertexAttributeCount);
void LoadTexture(unsigned int& textureId, const char* filePath);
void CreateProceduralTerrain(float* terrainVertices, int terrainVerticesCount);
void CreateSphereObject(float sphereVertices[latitudeSteps][longitudeSteps][11], unsigned int sphereIndices[(longitudeSteps - 1) * (latitudeSteps - 1) * 6]);


#pragma region Structures
struct Point {
	float x, y, z;
};


int maxPointLights = 8;
vector<PointLight*> staticPointLights;
map<ArcingProjectileObject*, PointLight*> dynamicBubbleLights;
vector<PointLight*> dynamicPointLights;
#pragma endregion Structures




int main()
{
#pragma region OpenGl Setup
	//--- Initialize GLFW
	glfwInit();


#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	//--- Create window
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Stalker 1.5", NULL, NULL);
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
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
#pragma endregion  OpenGl Setup

#pragma region Audio Setup
	//---- Audio

	audioEngine = createIrrKlangDevice();
	backgroundSound = audioEngine->play2D("Media/Audio/mysterious-ambient-suspense-atmosphere-252023.mp3", true, false, true);
	if (backgroundSound)
	{
		backgroundSound->setVolume(0.04f);
	}
#pragma endregion


#pragma region Base Cube Object
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
	//int cubeIndicesCount = sizeof(cubeIndices) / sizeof(cubeIndices[0]);
	CreateObject("Cube Object", cubeVertices, cubeVerticesCount, NULL, 0, cubeSectionSizes, cubeVertexSize);

#pragma endregion

#pragma region Plane Object
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

#pragma endregion



#pragma region Sphere creation and displacement
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

	float sphereVertices[latitudeSteps][longitudeSteps][11];
	unsigned int sphereIndices[(longitudeSteps - 1) * (latitudeSteps - 1) * 6];


	CreateSphereObject(sphereVertices, sphereIndices);
	// ------------------------
	// Spawning attributes
	// ------------------------
	vec3 spawnCentre = vec3(0.0f, 0.0f, 0.0f);
	float spawnRadius = 3.0f;
	float projectileSpawnCooldown = 2.0f;
	float projectileSpawnTimer = 0.0f;
	int maxBubbles = 7;
	int currentBubbles = 0;

	// ---------------------------
	// Shader
	// ---------------------------

	Shader sphereShader("Shaders/SphereVertexShader.v", "Shaders/SphereFragmentShader.f");

	sphereShader.Use();
	sphereShader.setVec3("objectColor", vec3(1.0f, 0.5f, 0.31f));
	sphereShader.setVec3("lightColor", vec3(1.0f, 1.0f, 1.0f));

	sphereShader.setVec3("material.ambient", 1.0f, 0.5f, 0.31f);
	sphereShader.setVec3("material.diffuse", 85.0f / 255.0f, 140.0f / 255.0f, 158.0f / 255.0f);
	sphereShader.setVec3("material.specular", 0.8f, 0.8f, 0.8f);
	sphereShader.setFloat("material.shininess", 32.0f);
	sphereShader.setVec3("light.ambient", 1.0f, 1.0f, 1.0f);
	sphereShader.setVec3("light.diffuse", 1.0f, 1.0f, 1.0f); // darken diffuse light a bit
	sphereShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
	sphereShader.setInt("currentPointLights", 0);

	// -----------------------
	// Sphere proc gen
	// -----------------------
#pragma endregion




#pragma region Cube and plane shader
	// --------------------------------------------
	// Shader creation
	// --------------------------------------------
	Shader TexturedObjectShader("Shaders/VertexShader.v", "Shaders/FragmentShader.f");
	TexturedObjectShader.Use();
	//The texture sampler on the fragment shader is given value '0' now, means later on in the render loop it will use texture unit zero


	TexturedObjectShader.setInt("texture1", texNameToUnitNo["container"]);
	TexturedObjectShader.setBool("useTexture", true);
	TexturedObjectShader.setVec3("objectColor", vec3(1.0f, 1.0f, 1.0f));
	TexturedObjectShader.setVec3("lightColor", vec3(1.2f, 1.0f, 2.0f));
	TexturedObjectShader.setVec3("material.ambient", .1f, 1.0f, 1.0f);
	TexturedObjectShader.setVec3("material.diffuse", 1.0f, 1.0f, 1.0f);
	TexturedObjectShader.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
	TexturedObjectShader.setFloat("material.shininess", 32.0f);
	TexturedObjectShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
	TexturedObjectShader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
	TexturedObjectShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
	//TexturedObjectShader.setVec3("light.direction", -0.2f, -1.0f, -0.3f);
	TexturedObjectShader.setFloat("light.constant", 1.0f);
	TexturedObjectShader.setFloat("light.linear", 0.09f);
	TexturedObjectShader.setFloat("light.quadratic", 0.032f);
	TexturedObjectShader.setVec3("light.position", camera.Position);
	TexturedObjectShader.setVec3("light.direction", camera.Front);
	TexturedObjectShader.setFloat("light.cutOff", cos(radians(12.5f)));
	TexturedObjectShader.setInt("staticPointLights", 4);
	TexturedObjectShader.setInt("dynamicPointLights", 0);

#pragma endregion


#pragma region Standalone texture loading
	// --------------------------------------------
	// Texture loading
	// --------------------------------------------
	unsigned int containerTextureId;

	LoadTexture(containerTextureId, "Media/GroundTexture/Ground048_1K-JPG_Color.jpg");

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, containerTextureId);

	texNameToId["container"] = containerTextureId;
	texNameToUnitNo["container"] = 0;
#pragma endregion



#pragma region Procedural Terrain Gen
	// -----------------------------------------
	// Procedural terrain generation
	// -----------------------------------------
	//NOTE: Need to look through this later to double check understanding

	float terrainVertices[MAP_SIZE][6];
	int terrainVerticesCount = sizeof(terrainVertices) / sizeof(terrainVertices[0]);

	CreateProceduralTerrain(&terrainVertices[0][0], terrainVerticesCount);

	// --------------------
	// Shader 
	// -------------------
	Shader ProceduralObjectShader("Shaders/TerrainVertexShader.v", "Shaders/TerrainFragmentShader.f");
#pragma endregion



#pragma region Light Setup
	// ----------------------------------------
	// Light creation
	// ----------------------------------------

	vec3 lightColour(1.0f, 1.0f, 1.0f);
	vec3 dirLightColour = vec3(71.0f / 255.0f, 113.0f / 255.0f, 214.0f / 255.0f);
	vec3 ambientLightColour = vec3(3.0f / 255.0f, 10.0f / 255.0f, 28.0f / 255.0f);

	vec3 pointLightPositions[] = {
		glm::vec3(0.7f,  0.5f,  3.0f),
		glm::vec3(12.3f, 0.5f, -4.0f),
		glm::vec3(-2.0f,  0.5f, -12.0f),
		glm::vec3(6.0f,  0.5f, -6.0f)
	};

	for (vec3 lightPos : pointLightPositions)
	{
		PointLight* newLight = new PointLight(
			lightPos,
			1.0f,
			0.09f,
			0.032f,
			ambientLightColour,
			lightColour,
			lightColour
		);
		staticPointLights.push_back(newLight);
	}







	vec3 globalLightPos(5.0f, 7.0f, -2.0f);
	mat4 lightModel = mat4(1.0f);
	lightModel = translate(lightModel, globalLightPos);
	lightModel = scale(lightModel, vec3(1.0f));



	// -------------------------
	// Variable light colour through noise
	// -----------------------------

	FastNoiseLite lightColourNoiseGenerator;
	lightColourNoiseGenerator.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	lightColourNoiseGenerator.SetFrequency(0.2f);

	vec3 RedColour(224.0f / 255.0f, 151.0f / 255.0f, 130.0f / 255.0f);
	vec3 OrangeColour(212.0f / 255.0f, 164.0f / 255.0f, 116.0f / 255.0f);

	const int lightNoiseTextureLength = 512;
	int lightNoiseTextureCurrentIndex = 0;
	float lightNoiseScale = 0.4f;

	float lightNoiseValues[lightNoiseTextureLength];

	for (int i = 0; i < lightNoiseTextureLength; i++)
	{
		lightNoiseValues[i] = lightColourNoiseGenerator.GetNoise((float)i * lightNoiseScale, 0.0f);
	}
#pragma endregion






#pragma region Sphere proc gen
	// ----------------------------------
	// Sphere proc gen setup
	// ---------------------------------
	const int noiseWidth = 512;
	const int noiseHeight = 256;
	vector<float> noiseData(noiseWidth * noiseHeight);


	FastNoiseLite sphereNoiseGenerator;
	sphereNoiseGenerator.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	sphereNoiseGenerator.SetFrequency(0.08f);

	float noiseScale = 0.2f;

	for (int y = 0; y < noiseHeight; ++y) {
		for (int x = 0; x < noiseWidth; ++x) {
			int index = y * noiseWidth + x;

			float scaledX = (float)x * noiseScale;
			float scaledY = (float)y * noiseScale;


			noiseData[index] = (sphereNoiseGenerator.GetNoise(scaledX, scaledY) + 1.0f) * 0.5f;
		}
	}

	unsigned int firstNoiseTexture;
	glGenTextures(1, &firstNoiseTexture);

	texNameToId["firstNoiseTexture"] = firstNoiseTexture;
	texNameToUnitNo["firstNoiseTexture"] = 1;

	glActiveTexture(GL_TEXTURE0 + texNameToUnitNo["firstNoiseTexture"]);
	glBindTexture(GL_TEXTURE_2D, firstNoiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, noiseWidth, noiseHeight, 0, GL_RED, GL_FLOAT, noiseData.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);



	sphereShader.setInt("firstNoiseTexture", texNameToUnitNo["firstNoiseTexture"]);

	sphereNoiseGenerator.SetFrequency(0.01f);

	noiseScale = 0.35f;

	for (int y = 0; y < noiseHeight; ++y) {
		for (int x = 0; x < noiseWidth; ++x) {
			int index = y * noiseWidth + x;

			float scaledX = (float)x * noiseScale;
			float scaledY = (float)y * noiseScale;


			noiseData[index] = (sphereNoiseGenerator.GetNoise(scaledX, scaledY) + 1.0f) * 0.5f;
		}
	}

	unsigned int secondNoiseTexture;
	glGenTextures(1, &secondNoiseTexture);

	texNameToId["secondNoiseTexture"] = secondNoiseTexture;
	texNameToUnitNo["secondNoiseTexture"] = 5;

	glActiveTexture(GL_TEXTURE0 + texNameToUnitNo["secondNoiseTexture"]);
	glBindTexture(GL_TEXTURE_2D, secondNoiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, noiseWidth, noiseHeight, 0, GL_RED, GL_FLOAT, noiseData.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	sphereShader.setInt("secondNoiseTexture", texNameToUnitNo["secondNoiseTexture"]);
#pragma endregion


#pragma region Assimp model importing
	// ----------------------------------
	// Model importing
	// ----------------------------------



	texNameToUnitNo["treeTexture"] = 2;
	Model treeModel("Media/Tree/Tree.obj", texNameToUnitNo["treeTexture"]);


	texNameToUnitNo["wallTexture"] = 3;
	Model wallModel("Media/Wall/Wall.fbx", texNameToUnitNo["wallTexture"]);

	texNameToUnitNo["lampTexture"] = 4;
	Model lampModel("Media/Lamp/lamp.obj", texNameToUnitNo["lampTexture"]);

	// ------------------------
	// Model Shader
	// ----------------------
	Shader modelShader("Shaders/ModelVertexShader.v", "Shaders/ModelFragmentShader.f");

	modelShader.Use();
	modelShader.setVec3("objectColor", vec3(1.0f, 0.5f, 0.31f));
	modelShader.setVec3("material.ambient", 0.25f, 0.25f, 0.25f);
	modelShader.setVec3("material.diffuse", 0.8f, 0.8f, 0.8f);
	modelShader.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
	modelShader.setFloat("material.shininess", 2.0f);
	modelShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
	modelShader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f); // darken diffuse light a bit
	modelShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

	modelShader.setFloat("light.constant", 1.0f);
	modelShader.setFloat("light.linear", 0.09f);
	modelShader.setFloat("light.quadratic", 0.032f);
	modelShader.setVec3("light.position", camera.Position);
	modelShader.setVec3("light.direction", camera.Front);
	modelShader.setFloat("light.cutOff", cos(radians(12.5f)));
	modelShader.setInt("staticPointLights", 4);
	modelShader.setInt("dynamicPointLights", 0);
#pragma endregion



#pragma region Random Tree Locations
	vector<vec3> randomTreePositions;
	vector<float> randomTreeRotations;
	//top left must be smaller to work
	Point treeSpawnTopLeft = { -40.0f, -0.1f, -40.0f }; //Left, up, forward
	Point treeSpawnBottomRight = { 40.0f, -0.1f, -3.0f };

	float minX = std::min(treeSpawnTopLeft.x, treeSpawnBottomRight.x);
	float maxX = std::max(treeSpawnTopLeft.x, treeSpawnBottomRight.x);
	float minZ = std::min(treeSpawnTopLeft.z, treeSpawnBottomRight.z);
	float maxZ = std::max(treeSpawnTopLeft.z, treeSpawnBottomRight.z);

	int numberOfTrees = 120;


	//Pre compute the instanced matrices on the cpu
	mat4* treeModelMatrices;
	treeModelMatrices = new mat4[numberOfTrees];
	for (int i = 0; i < numberOfTrees; i++)
	{
		mat4 model = mat4(1.0);
		float randomX = treeSpawnTopLeft.x + (treeSpawnBottomRight.x - treeSpawnTopLeft.x) * dis(gen);
		float randomY = 0.0f;
		float randomZ = treeSpawnTopLeft.z + (treeSpawnBottomRight.z - treeSpawnTopLeft.z) * dis(gen);


		vec3 spawnPosition = vec3(randomX, randomY, randomZ);
		randomTreePositions.push_back(spawnPosition);
		model = translate(model, spawnPosition);

		float rotationMin = 0.0f;
		float rotationAmount = rotationMin + (359.0f - rotationMin) * dis(gen);
		randomTreeRotations.push_back(rotationAmount);
		model = rotate(model, radians(rotationAmount), vec3(0.0f, 1.0f, 0.0f));

		treeModelMatrices[i] = model;
	}

	unsigned int instanceBuffer;
	glGenBuffers(1, &instanceBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);
	glBufferData(GL_ARRAY_BUFFER, numberOfTrees * sizeof(mat4), &treeModelMatrices[0], GL_STATIC_DRAW);

	unsigned int treeVAO = treeModel.meshes[0].VAO;
	glBindVertexArray(treeVAO);
	// set attribute pointers for matrix (4 times vec4)
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(mat4), (void*)0);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(mat4), (void*)(sizeof(vec4)));
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(mat4), (void*)(2 * sizeof(vec4)));
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(mat4), (void*)(3 * sizeof(vec4)));

	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);
	glVertexAttribDivisor(6, 1);

	glBindVertexArray(0);
#pragma endregion

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
		vec3 backgroundColour = vec3(3.0f / 255.0f, 10.0f / 255.0f, 28.0f / 255.0f);
		glClearColor(backgroundColour.x, backgroundColour.y, backgroundColour.z, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Audio updating
		audioEngine->setListenerPosition(vec3df(camera.Position.x, camera.Position.y, camera.Position.z), vec3df(-camera.Front.x, -camera.Front.y, -camera.Front.z));
		audioEngine->setRolloffFactor(2.0f);

		//---------------------------------------
		// Activate shader
		TexturedObjectShader.Use();

#pragma region Shader view and projection update
		mat4 projection = perspective(radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		mat4 view = camera.GetViewMatrix();

		TexturedObjectShader.Use();
		TexturedObjectShader.setMat4("projection", projection);
		TexturedObjectShader.setMat4("view", view);
		TexturedObjectShader.setVec3("viewPos", camera.Position);

		sphereShader.Use();
		sphereShader.setMat4("projection", projection);
		sphereShader.setMat4("view", view);
		sphereShader.setVec3("viewPos", camera.Position);

		ProceduralObjectShader.Use();
		ProceduralObjectShader.setMat4("projection", projection);
		ProceduralObjectShader.setMat4("view", view);
		ProceduralObjectShader.setVec3("viewPos", camera.Position);

		modelShader.Use();
		modelShader.setMat4("projection", projection);
		modelShader.setMat4("view", view);
		modelShader.setVec3("viewPos", camera.Position);


#pragma endregion


#pragma region Shader lighting Updates
		// ------------------------------
		// Light colour
		// ----------------------------
		lightNoiseTextureCurrentIndex = (int)(currentFrame * 0.035f * lightNoiseTextureLength) % lightNoiseTextureLength;
		float noiseValue = lightNoiseValues[lightNoiseTextureCurrentIndex];


		vec3 lightColour = RedColour + (OrangeColour - RedColour) * noiseValue;
		vec3 dirLightColour = vec3(71.0f / 255.0f, 113.0f / 255.0f, 214.0f / 255.0f);
		vec3 ambientLightColour = vec3(3.0f / 255.0f, 10.0f / 255.0f, 28.0f / 255.0f);
		vec3 bubbleLightColour = vec3(78.0f / 255.0f, 146.0f / 255.0f, 156.0f / 255.0f);

		// ------------------------------
		// Cube and plane lighting update
		// -----------------------------
		// directional light

		TexturedObjectShader.Use();
		TexturedObjectShader.setVec3("dirLight.direction", -0.7f, -1.0f, 0.7f);
		TexturedObjectShader.setVec3("dirLight.ambient", ambientLightColour.x * 0.3, ambientLightColour.y * 0.3, ambientLightColour.z * 0.3);
		TexturedObjectShader.setVec3("dirLight.diffuse", dirLightColour.x, dirLightColour.y, dirLightColour.z);
		TexturedObjectShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
		// pointLights
		int pointLightIndex = 0;
		string pointLightUniformTag;
		for (PointLight* light : staticPointLights) {
			pointLightUniformTag = ("pointLights[" + to_string(pointLightIndex) + "]");
			TexturedObjectShader.setVec3(pointLightUniformTag + ".position", light->position);
			TexturedObjectShader.setVec3(pointLightUniformTag + ".ambient", ambientLightColour.x, ambientLightColour.y, ambientLightColour.z);
			TexturedObjectShader.setVec3(pointLightUniformTag + ".diffuse", lightColour);
			TexturedObjectShader.setVec3(pointLightUniformTag + ".specular", lightColour);
			TexturedObjectShader.setFloat(pointLightUniformTag + ".constant", light->constant);
			TexturedObjectShader.setFloat(pointLightUniformTag + ".linear", light->linear);
			TexturedObjectShader.setFloat(pointLightUniformTag + ".quadratic", light->quadratic);
			pointLightIndex++;
		}


		for (PointLight* light : dynamicPointLights) {
			pointLightUniformTag = ("pointLights[" + to_string(pointLightIndex) + "]");
			TexturedObjectShader.setVec3(pointLightUniformTag + ".position", light->position);
			TexturedObjectShader.setVec3(pointLightUniformTag + ".ambient", ambientLightColour.x, ambientLightColour.y, ambientLightColour.z);
			TexturedObjectShader.setVec3(pointLightUniformTag + ".diffuse", bubbleLightColour);
			TexturedObjectShader.setVec3(pointLightUniformTag + ".specular", bubbleLightColour);
			TexturedObjectShader.setFloat(pointLightUniformTag + ".constant", light->constant);
			TexturedObjectShader.setFloat(pointLightUniformTag + ".linear", light->linear);
			TexturedObjectShader.setFloat(pointLightUniformTag + ".quadratic", light->quadratic);
			pointLightIndex++;
		}
		// spotLight
		TexturedObjectShader.setVec3("spotLight.position", camera.Position);
		TexturedObjectShader.setVec3("spotLight.direction", camera.Front);
		TexturedObjectShader.setVec3("spotLight.ambient", ambientLightColour.x, ambientLightColour.y, ambientLightColour.z);
		TexturedObjectShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
		TexturedObjectShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
		TexturedObjectShader.setFloat("spotLight.constant", 1.0f);
		TexturedObjectShader.setFloat("spotLight.linear", 0.09f);
		TexturedObjectShader.setFloat("spotLight.quadratic", 0.032f);
		TexturedObjectShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
		TexturedObjectShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));


		// ----------------------------
		// Model lighting update
		// ---------------------------
		modelShader.Use();
		modelShader.setVec3("dirLight.direction", -0.7f, -1.0f, 0.7f);
		modelShader.setVec3("dirLight.ambient", ambientLightColour.x, ambientLightColour.y, ambientLightColour.z);
		modelShader.setVec3("dirLight.diffuse", dirLightColour.x, dirLightColour.y, dirLightColour.z);
		modelShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
		// pointLights
		pointLightIndex = 0;

		for (PointLight* light : staticPointLights) {
			pointLightUniformTag = ("pointLights[" + to_string(pointLightIndex) + "]");
			modelShader.setVec3(pointLightUniformTag + ".position", light->position);
			modelShader.setVec3(pointLightUniformTag + ".ambient", ambientLightColour.x, ambientLightColour.y, ambientLightColour.z);
			modelShader.setVec3(pointLightUniformTag + ".diffuse", lightColour);
			modelShader.setVec3(pointLightUniformTag + ".specular", lightColour);
			modelShader.setFloat(pointLightUniformTag + ".constant", light->constant);
			modelShader.setFloat(pointLightUniformTag + ".linear", light->linear);
			modelShader.setFloat(pointLightUniformTag + ".quadratic", light->quadratic);
			pointLightIndex++;
		}


		for (PointLight* light : dynamicPointLights) {
			pointLightUniformTag = ("pointLights[" + to_string(pointLightIndex) + "]");
			modelShader.setVec3(pointLightUniformTag + ".position", light->position);
			modelShader.setVec3(pointLightUniformTag + ".ambient", ambientLightColour.x, ambientLightColour.y, ambientLightColour.z);
			modelShader.setVec3(pointLightUniformTag + ".diffuse", bubbleLightColour);
			modelShader.setVec3(pointLightUniformTag + ".specular", bubbleLightColour);
			modelShader.setFloat(pointLightUniformTag + ".constant", light->constant);
			modelShader.setFloat(pointLightUniformTag + ".linear", light->linear);
			modelShader.setFloat(pointLightUniformTag + ".quadratic", light->quadratic);
			pointLightIndex++;
		}
		// spotLight
		modelShader.setVec3("spotLight.position", camera.Position);
		modelShader.setVec3("spotLight.direction", camera.Front);
		modelShader.setVec3("spotLight.ambient", 0.1f, 0.1f, 0.1f);
		modelShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
		modelShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
		modelShader.setFloat("spotLight.constant", 1.0f);
		modelShader.setFloat("spotLight.linear", 0.09f);
		modelShader.setFloat("spotLight.quadratic", 0.032f);
		modelShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
		modelShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

		// ----------------------------
		// Sphere lighting update
		// ---------------------------
		sphereShader.Use();
		sphereShader.setVec3("lightPos", globalLightPos);
		sphereShader.setVec3("lightColour", vec3(1.0f, 1.0f, 1.0f));
		sphereShader.setVec3("light.position", globalLightPos);
		sphereShader.setVec3("light.ambient", ambientLightColour.x, ambientLightColour.y, ambientLightColour.z);
		sphereShader.setVec3("light.diffuse", 0.35f, 0.35f, 0.35f);
		sphereShader.setVec3("light.specular", 0.35f, 0.35f, 0.35f);


#pragma endregion




#pragma region Cube Rendering
		//--- Render cubes		
		for (unsigned int i = 0; i < 10; i++)
		{

			mat4 model = mat4(1.0f);
			model = translate(model, cubePositions[i]);
			float angle = 20.0f * i;
			model = rotate(model, radians(angle), vec3(1.0f, 0.3f, 0.5f));

			TexturedObjectShader.Use();
			TexturedObjectShader.setMat4("model", model);


			//sceneObjectDictionary["Cube Object"]->DrawMesh();
		}
#pragma endregion

#pragma region Plane Rendering


		mat4 model = mat4(1.0f);
		model = translate(model, vec3(0.0f, 0.0f, -20.0f));
		model = scale(model, vec3(90.0f, 1.0f, 50.0f));
		model = rotate(model, radians(90.0f), vec3(1.0f, 0.0f, 0.0f));

		TexturedObjectShader.Use();

		TexturedObjectShader.setMat4("model", model);
		TexturedObjectShader.setBool("useTexture", true);
		TexturedObjectShader.setInt("texture1", 0);

		sceneObjectDictionary["Plane Object"]->DrawMesh();
#pragma endregion

#pragma region Lamp Rendering

		for (vec3 lampPos : pointLightPositions)
		{
			mat4 lampTransform = mat4(1.0f);
			lampTransform = translate(lampTransform, vec3(lampPos.x, lampPos.y - 0.5f, lampPos.z));
			lampTransform = scale(lampTransform, vec3(0.2f, 0.2f, 0.2f));

			modelShader.Use();
			modelShader.setMat4("model", lampTransform);

			lampModel.Draw(modelShader, texNameToUnitNo["lampTexture"]);

		}
#pragma endregion

#pragma region Projectile Spawning
		// ------------------------
		// Projectile spawning
		// -------------------------
		projectileSpawnTimer += deltaTime;
		if (projectileSpawnTimer >= projectileSpawnCooldown && currentBubbles < maxBubbles)
		{
			ArcingProjectileObject* newProjectileObject = new ArcingProjectileObject();
			//newProjectileObject->VAO = sceneObjectDictionary["Projectile Base"]->VAO;
			//newProjectileObject->PrepareAndBindVBO(sceneObjectDictionary["Projectile Base"]->VBO, sizeof(cubeVertices) / (5 * sizeof(float)));

			newProjectileObject->VAO = sceneObjectDictionary["Sphere Object"]->VAO;

			newProjectileObject->PrepareAndBindVBO(sceneObjectDictionary["Sphere Object"]->VBO, sphereVerticesCount);
			newProjectileObject->PrepareAndBindEBO(sceneObjectDictionary["Sphere Object"]->EBO, sphereIndicesCount);

			GLenum error;
			while ((error = glGetError()) != GL_NO_ERROR) {
				cerr << "OpenGL error after VBO binding: " << error << endl;
			}

			//--- Spawn bounds
			Point topLeft = { -40.0f, -0.1f, -40.0f };
			Point bottomRight = { 40.0f, -0.1f, -10.0f };


			float ySpawnValueMin = 1.5f;
			float ySpawnValue = ySpawnValueMin + (2.0f - ySpawnValueMin) * dis(gen);

			float randomX = topLeft.x + (bottomRight.x - topLeft.x) * dis(gen);
			float randomY = ySpawnValue;
			float randomZ = topLeft.z + (bottomRight.z - topLeft.z) * dis(gen);

			vec3 spawnPosition = vec3(randomX, randomY, randomZ);

			//--- Launch angle
			//Angle when looking down the y axis
			float azimuth = dis(gen) * 2 * PI;

			//Phi is the vertical angle from the horizontal plane
			float minPhi = PI / 6;  // 30 degrees in radians
			float maxPhi = 4 * PI / 9;  // 80 degrees in radians

			float phi = minPhi + (maxPhi - minPhi) * dis(gen);

			float x = sin(phi) * cos(azimuth);
			float y = cos(phi);
			float z = sin(phi) * sin(azimuth);

			float initVelMin = 1.8f;
			float velocityMulitplier = initVelMin + (2.2f - initVelMin) * dis(gen);
			vec3 spawnVelocity = normalize(vec3(x, y, z));

			spawnVelocity = spawnVelocity * velocityMulitplier;

			//--- Random speed multiplaier
			float speedMin = 0.3f;
			float movespeedMultiplier = speedMin + (0.7f - speedMin) * dis(gen);

			//--- Random gravity multiplier
			float gravityMultiMin = 0.0f;
			float gravityMultiplier = gravityMultiMin + (0.04f - gravityMultiMin) * dis(gen);

			newProjectileObject->Launch(vec3(spawnVelocity), vec3(spawnPosition), currentFrame, gravityMultiplier, movespeedMultiplier);

			projectileObjects.push_back(newProjectileObject);


			//--- Bubble sound
			ISound* sound = audioEngine->play3D("Media/Audio/bathtub-ambience-27873.mp3", vec3df(spawnPosition.x, spawnPosition.y, spawnPosition.z), true, false, true);
			if (!sound) {
				std::cerr << "Failed to load sound file for bubble" << std::endl;

			}
			else {
				sound->setVolume(2.5f);

				sound->setMinDistance(5.0f);


				bubbleSounds[newProjectileObject] = sound;
			}

			//--- Bubble lighting
			PointLight* newLight = new PointLight(
				newProjectileObject->currentPosition,
				1.0f,
				0.09f,
				0.032f,
				ambientLightColour,
				lightColour,
				lightColour
			);

			dynamicBubbleLights[newProjectileObject] = newLight;
			dynamicPointLights.push_back(newLight);

			currentBubbles++;

			//Random spawn cooldown
			float cooldownMin = 2.0f;
			projectileSpawnCooldown = cooldownMin + (4.0f - cooldownMin) * dis(gen);

			projectileSpawnTimer = 0.0f;
		}
#pragma endregion




#pragma region Projectile Update

		for (size_t i = 0; i < projectileObjects.size();) {
			ArcingProjectileObject* projectileObject = projectileObjects[i];
			if (projectileObject != NULL)
			{
				projectileObject->UpdatePosition(deltaTime);

				if (projectileObject->ShouldDestroy()) {

					bubbleSounds[projectileObject]->drop();
					bubbleSounds.erase(projectileObject);
					PointLight* objectLight = dynamicBubbleLights[projectileObject];
					dynamicPointLights.erase(remove(dynamicPointLights.begin(), dynamicPointLights.end(), objectLight), dynamicPointLights.end());
					delete objectLight;

					dynamicBubbleLights.erase(projectileObject);

					TexturedObjectShader.Use();
					int numberOfPointLights = dynamicPointLights.size();
					TexturedObjectShader.setInt("dynamicPointLights", numberOfPointLights);
					delete projectileObject;
					projectileObjects.erase(projectileObjects.begin() + i);
					currentBubbles--;

					modelShader.Use();					
					modelShader.setInt("dynamicPointLights", numberOfPointLights);


				}
				else {
					mat4 projectileModel = mat4(1.0f);
					projectileModel = translate(projectileModel, projectileObject->initialPosition + (projectileObject->currentPosition - projectileObject->initialPosition));



					sphereShader.Use();


					sphereShader.setMat4("model", projectileModel);
					sphereShader.setFloat("time", currentFrame);
					sphereShader.setFloat("displacementScale", 0.4f);
					sphereShader.setInt("firstNoiseTexture", texNameToUnitNo["firstNoiseTexture"]);
					sphereShader.setInt("secondNoiseTexture", texNameToUnitNo["secondNoiseTexture"]);
					
					
					




					bubbleSounds[projectileObject]->setPosition(vec3df(projectileObject->currentPosition.x, projectileObject->currentPosition.y, projectileObject->currentPosition.z));

					dynamicBubbleLights[projectileObject]->position = projectileObject->currentPosition;

					projectileObject->DrawMesh();


					TexturedObjectShader.Use();
					int numberOfPointLights = dynamicPointLights.size();
					TexturedObjectShader.setInt("dynamicPointLights", numberOfPointLights);

					modelShader.Use();					
					modelShader.setInt("dynamicPointLights", numberOfPointLights);
					GLenum error;
					while ((error = glGetError()) != GL_NO_ERROR) {
						cerr << "OpenGL error post projectile render: " << error << endl;
					}

					++i;
				}
			}


		}
#pragma endregion



		GLenum error;


#pragma region Proc terrain Rendering


		//Terrain
		mat4 terrainModel = mat4(1.0f);
		terrainModel = translate(terrainModel, vec3(15.0f, 0.0f, 45.0f));

		//Looking straight forward
		terrainModel = rotate(terrainModel, radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
		//Elevation to look upon terrain
		terrainModel = scale(terrainModel, vec3(6.0f, 1.3f, 6.0f));
		ProceduralObjectShader.Use();
		ProceduralObjectShader.setMat4("model", terrainModel);


		sceneObjectDictionary["Procedural Terrain"]->DrawMesh();
#pragma endregion



#pragma region Single Sphere Render
		mat4 sphereModel = mat4(1.0f);
		sphereModel = translate(sphereModel, vec3(8.0f, 2.0f, -12.0f));

		sphereShader.Use();

		sphereShader.setMat4("model", sphereModel);
		sphereShader.setFloat("time", currentFrame);
		sphereShader.setFloat("displacementScale", 0.4f);
		sphereShader.setInt("firstNoiseTexture", texNameToUnitNo["firstNoiseTexture"]);
		sphereShader.setInt("secondNoiseTexture", texNameToUnitNo["secondNoiseTexture"]);



		sceneObjectDictionary["Sphere Object"]->DrawMesh();
#pragma endregion



#pragma region Tree Rendering
		// --------------------
		// Instanced rendering
		// ---------------------
		mat4 treeModelBase = mat4(1.0f);

		modelShader.Use();
		modelShader.setMat4("model", treeModelBase);
		modelShader.setBool("useInstancing", true);

		glBindVertexArray(treeModel.meshes[0].VAO);

		GLuint currentTexture;
		glActiveTexture(GL_TEXTURE0 + texNameToUnitNo["treeTexture"]);
		glBindTexture(GL_TEXTURE_2D, treeModel.textures_loaded[0].id);
		modelShader.setInt(("texture_diffuse1"), texNameToUnitNo["treeTexture"]);

		glDrawElementsInstanced(GL_TRIANGLES, treeModel.meshes[0].indices.size(), GL_UNSIGNED_INT, 0, numberOfTrees);
		glBindVertexArray(0);


		// ------------------------
		// Old multi tree rendering
		// -----------------------
		/*
		for (unsigned int i = 0; i < numberOfTrees; i++)
		{
			// calculate the model matrix for each object and pass it to shader before drawing
			mat4 model = mat4(1.0f); // make sure to initialize matrix to identity matrix first



			model = translate(model, randomTreePositions[i]);
			model = rotate(model, radians(randomTreeRotations[i]), vec3(0.0f, 1.0f, 0.0f));



			//TexturedObjectShader.setMat4("model", model);

			//TexturedObjectShader.setVec3("lightPos", lightPos);
			//TexturedObjectShader.setVec3("viewPos", camera.Position);
			//TexturedObjectShader.setVec3("light.position", camera.Position);
			//TexturedObjectShader.setVec3("light.direction", camera.Front);
			//TexturedObjectShader.setFloat("light.cutOff", glm::cos(glm::radians(12.5f)));
			//TexturedObjectShader.setFloat("light.outerCutOff", glm::cos(glm::radians(17.5f)));

			//modelShader.setMat4("model", model);
			//modelShader.setMat4("projection", projection);
			//modelShader.setMat4("view", view);
			//treeModel.Draw(modelShader);


			modelShader.Use();

			modelShader.setMat4("model", model);
			modelShader.setMat4("projection", projection);
			modelShader.setMat4("view", view);
			modelShader.setBool("useInstancing", false);

			//treeModel.Draw(modelShader, texNameToUnitNo["treeTexture"]);
			//sceneObjectDictionary["Cube Object"]->DrawMesh();
			GLenum error;
			while ((error = glGetError()) != GL_NO_ERROR) {
				cerr << "OpenGL error post tree render: " << error << endl;
			}
		}*/


		// ----------------------
		// Single tree render
		// ----------------------
		modelShader.Use();
		mat4 modelLocation = mat4(1.0f);
		modelLocation = scale(modelLocation, vec3(0.8f, 0.8f, 0.8f));
		modelShader.setMat4("model", modelLocation);
		modelShader.setBool("useInstancing", false);

		glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint*)&currentTexture);
		treeModel.Draw(modelShader, texNameToUnitNo["treeTexture"]);
#pragma endregion



#pragma region Wall Rendering
		modelShader.Use();
		modelLocation = mat4(1.0f);
		modelLocation = scale(modelLocation, vec3(0.8f, 0.8f, 0.8f));
		modelLocation = translate(modelLocation, vec3(2.0f, 0.0f, 0.0f));
		modelLocation = rotate(modelLocation, radians(-90.0f), vec3(1.0f, 0.0f, 0.0f));
		modelLocation = rotate(modelLocation, radians(-90.0f), vec3(0.0f, 0.0f, 1.0f));
		modelShader.setMat4("model", modelLocation);
		modelShader.setBool("useInstancing", false);


		wallModel.Draw(modelShader, texNameToUnitNo["wallTexture"]);

#pragma endregion


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
		bubbleSounds[projectile]->drop();
		bubbleSounds.erase(projectile);

		projectile->CleanUp();
	}

	sceneObjectDictionary.clear();
	projectileObjects.clear();

	audioEngine->drop();


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
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		if (!spacePressed)
		{
			spacePressed = true;
			if (backgroundSound != nullptr)
			{
				backgroundSound->stop();
				backgroundSound->drop();
				backgroundSound = nullptr;
				cout << "Sound stopped" << endl;
			}
		}
	}
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
		if (backgroundSound == nullptr)
		{
			backgroundSound = audioEngine->play2D("Media/Audio/mysterious-ambient-suspense-atmosphere-252023.mp3", true, false, true);
			if (backgroundSound)
			{
				backgroundSound->setVolume(0.04f);
			}
			spacePressed = false;
		}			
	}
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
		terrainIndices[i][0] = columnIndicesOffset + rowIndicesOffset; // top left
		terrainIndices[i][1] = RENDER_DISTANCE + columnIndicesOffset + rowIndicesOffset; // bottom left
		terrainIndices[i][2] = 1 + columnIndicesOffset + rowIndicesOffset; // top right

		terrainIndices[i + 1][0] = 1 + columnIndicesOffset + rowIndicesOffset; // top right
		terrainIndices[i + 1][2] = 1 + RENDER_DISTANCE + columnIndicesOffset + rowIndicesOffset; // bottom right
		terrainIndices[i + 1][1] = RENDER_DISTANCE + columnIndicesOffset + rowIndicesOffset; // bottom left

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

				terrainVertices[i * 6 + 3] = 82.0f / 255.0f;
				terrainVertices[i * 6 + 4] = 37.0f / 255.0f;
				terrainVertices[i * 6 + 5] = 24.0f / 255.0f;
			}
			else //Desert
			{
				terrainVertices[i * 6 + 3] = 36.0f / 255.0f;
				terrainVertices[i * 6 + 4] = 66.0f / 255.0f;
				terrainVertices[i * 6 + 5] = 23.0f / 255.0f;
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

void CreateSphereObject(float sphereVertices[latitudeSteps][longitudeSteps][11], unsigned int sphereIndices[(longitudeSteps - 1) * (latitudeSteps - 1) * 6]) {


	//Fill vertices array
	//Access with latitude then longitude, starts top left, moves around then down and around again
	//Each vertex of the sphere is defined by lat and lon, each holding 3 values for position, 3 for colour

	for (int lat = 0; lat < latitudeSteps; lat++)
	{
		for (int lon = 0; lon < longitudeSteps; lon++)
		{
			float theta = 2.0f * PI * lon / longitudeSteps;
			float phi = PI * lat / (latitudeSteps - 1);

			//Performs the formulas described above
			float x = sphereRadius * sin(phi) * cos(theta);
			float y = sphereRadius * cos(phi);
			float z = sphereRadius * sin(phi) * sin(theta);

			//Normal vector for each coord is just the position vector normalised 
			float normalX = x / sphereRadius;
			float normalY = y / sphereRadius;
			float normalZ = z / sphereRadius;

			if (fabs(x) < 1e-6) x = 0.0f;
			if (fabs(y) < 1e-6) y = 0.0f;
			if (fabs(z) < 1e-6) z = 0.0f;

			sphereVertices[lat][lon][0] = x;
			sphereVertices[lat][lon][1] = y;
			sphereVertices[lat][lon][2] = z;

			float u = (float)lon / (longitudeSteps - 1);
			float v = (float)lat / (latitudeSteps - 1);
			v = (cos(phi) + 1.0f) / 2.0f;

			sphereVertices[lat][lon][3] = u;
			sphereVertices[lat][lon][4] = v;

			sphereVertices[lat][lon][5] = 0.0f;
			sphereVertices[lat][lon][6] = 0.75f;
			sphereVertices[lat][lon][7] = 0.25f;

			sphereVertices[lat][lon][8] = normalX;
			sphereVertices[lat][lon][9] = normalY;
			sphereVertices[lat][lon][10] = normalZ;
		}
	}

	//Fill indices array
	int i = 0;
	for (int lat = 0; lat < latitudeSteps - 1; lat++) {
		for (int lon = 0; lon < longitudeSteps - 1; lon++) {

			//Since sphereVertices is 2 dimensional [lat][lon], this will set the index to the 'flattened index'
			//For instance, 2 * longitudeSteps + 3 would be sphereVertices[2][3]
			//In this case, lat + 1 moves it down 1


			int topLeft = lat * longitudeSteps + lon;
			int bottomLeft = (lat + 1) * longitudeSteps + lon;
			int topRight = lat * longitudeSteps + (lon + 1) % longitudeSteps;
			int bottomRight = (lat + 1) * longitudeSteps + (lon + 1) % longitudeSteps;

			//When the vertical slice is the last one
			if (lon == longitudeSteps - 2)
			{
				topLeft = lat * longitudeSteps + lon;
				bottomLeft = (lat + 1) * longitudeSteps + lon;
				topRight = lat * longitudeSteps;
				bottomRight = (lat + 1) * longitudeSteps;
			}


			sphereIndices[i] = topLeft;
			sphereIndices[i + 1] = topRight;
			sphereIndices[i + 2] = bottomLeft;


			sphereIndices[i + 3] = topRight;
			sphereIndices[i + 4] = bottomRight;
			sphereIndices[i + 5] = bottomLeft;

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

	//Removed when array was changed to pass in as a pointer
	//int sphereVerticesCount = sizeof(sphereVertices) / sizeof(sphereVertices[0][0]);
	//int indicesCount = sizeof(sphereIndices) / sizeof(sphereIndices[0]);d
	size_t indicesDataSize = sphereIndicesCount * sizeof(unsigned int);
	CreateObject("Sphere Object", sphereVertices[0][0], sphereVerticesCount, &sphereIndices[0], sphereIndicesCount, sphereSectionSizes, sphereAttributeSize);

}





