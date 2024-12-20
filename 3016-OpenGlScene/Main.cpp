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
#include "PhysicsObject.h"
#include "Shader.h"

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

	//--- Create shader for base objects
	Shader TexturedObjectShader("Shaders/VertexShader.v", "Shaders/FragmentShader.f");

	//--- Vertex Data for cube
	// remember that the coordinate is screen space, -1 - 1
	float cubeVertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
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

	//--- Plane vertices and indices
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


	//--- Scene object containers
	map<string, CustomSceneObject*> sceneObjectDictionary;

	vector<PhysicsObject*> projectileObjects;


	//--- Cube object
	CustomSceneObject* newSceneObject = new CustomSceneObject();
	newSceneObject->PrepareAndBindVAO();
	newSceneObject->PrepareAndBindVBO(cubeVertices, sizeof(cubeVertices), sizeof(cubeVertices) / (5 * sizeof(float))); //Because of 5 elements per vertex
	newSceneObject->PrepareVertexAttributeArrays();

	sceneObjectDictionary["Cube Object"] = newSceneObject;

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	//--- Ground plane object
	newSceneObject = new CustomSceneObject();
	newSceneObject->PrepareAndBindVAO();
	newSceneObject->PrepareAndBindVBO(planeVertices, sizeof(planeVertices), sizeof(planeVertices) / (5 * sizeof(float)));
	newSceneObject->PrepareAndBindEBO(planeIndices, sizeof(planeIndices), sizeof(planeIndices) / sizeof(planeIndices[0]));
	newSceneObject->PrepareVertexAttributeArrays();

	sceneObjectDictionary["Plane Object"] = newSceneObject;

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	//--- Projectile Object Setup
	unsigned int ProjectileCubeVAO, ProjectileCubeVBO;
	glGenVertexArrays(1, &ProjectileCubeVAO);
	glBindVertexArray(ProjectileCubeVAO);

	glGenBuffers(1, &ProjectileCubeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, ProjectileCubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	//--- Projectile spawning variables
	vec3 spawnCentre = vec3(0.0f, 0.0f, 0.0f);
	float spawnRadius = 3.0f;

	//--- Texture Loading
	//First texture
	unsigned int texture1;
	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);

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
	unsigned char* data = stbi_load("Media/container.jpg", &imageWidth, &imageHeight, &nrChannels, 0);
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


	//--- Set the sampler on each shader to the correct texture
	TexturedObjectShader.Use();
	//The texture sampler on the fragment shader is given value '0', meaning when we put that texture on texture unit 0, it will automatically use it
	TexturedObjectShader.setInt("texture1", 0);


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

	float projectileSpawnCooldown = 0.2f;
	float projectileSpawnTimer = 0.0f;

	//--- Procedural terrain generation

	//--- Create shader for terrain
	//Needed because terrain does not use textures like base objects

	Shader ProceduralObjectShader("Shaders/TerrainVertexShader.v", "Shaders/TerrainFragmentShader.f");

	//Generation of height map vertices
	GLfloat terrainVertices[MAP_SIZE][6];

	//Positions to start drawing from
	float drawingStartPosition = 1.0f;
	float columnVerticesOffset = drawingStartPosition;
	float rowVerticesOffset = drawingStartPosition;

	int rowIndex = 0;

	for (int i = 0; i < MAP_SIZE; i++)
	{
		//Generation of x & z vertices for horizontal plane
		terrainVertices[i][0] = columnVerticesOffset;
		//terrainVertices[i][1] = 0.0f;
		terrainVertices[i][2] = rowVerticesOffset;

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
	GLuint terrainIndices[trianglesGrid][3];

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
			terrainVertices[i][1] = terrainNoise.GetNoise((float)x, (float)y);

			float biomeValue = biomeNoise.GetNoise((float)x, (float)y);

			if (biomeValue <= -0.75f) //Plains
			{
				terrainVertices[i][3] = 0.0f;
				terrainVertices[i][4] = 0.75f;
				terrainVertices[i][5] = 0.25f;
			}
			else //Desert
			{
				terrainVertices[i][3] = 1.0f;
				terrainVertices[i][4] = 1.0f;
				terrainVertices[i][5] = 0.5f;
			}

			i++;
		}
	}



	//--- Buffer generation for proc gen
	unsigned int ProcGenVAO, ProcGenVBO, ProcGenEBO;
	glGenVertexArrays(1, &ProcGenVAO);
	glBindVertexArray(ProcGenVAO);

	glGenBuffers(1, &ProcGenVBO);
	glBindBuffer(GL_ARRAY_BUFFER, ProcGenVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(terrainVertices), terrainVertices, GL_STATIC_DRAW);

	glGenBuffers(1, &ProcGenEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ProcGenEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(terrainIndices), terrainIndices, GL_STATIC_DRAW);

	//Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//Colours
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


	//--- Spherical object generation

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



	float radius = 2.0f;
	const int longitudeSteps = 36;
	const int latitudeSteps = 18;

	//Access with latitude then longitude, starts top left, moves around then down and around again
	//Each vertex of the sphere is defined by lat and lon, each holding 3 values for position, 3 for colour
	GLfloat sphereVertices[latitudeSteps][longitudeSteps][6];

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

			if (fabs(x) < 1e-6) x = 0.0f;
			if (fabs(y) < 1e-6) y = 0.0f;
			if (fabs(z) < 1e-6) z = 0.0f;

			sphereVertices[lat][lon][0] = x;
			sphereVertices[lat][lon][1] = y;
			sphereVertices[lat][lon][2] = z;

			sphereVertices[lat][lon][3] = 0.0f;
			sphereVertices[lat][lon][4] = 0.75f;
			sphereVertices[lat][lon][5] = 0.25f;


		}
	}

	//Indices array is one dimensional, but assumes size 3 for each element
	GLuint sphereIndices[(longitudeSteps - 1) * (latitudeSteps - 1) * 6];



	i = 0;
	for (int lat = 0; lat < latitudeSteps - 1; lat++) {
		for (int lon = 0; lon < longitudeSteps - 1; lon++) {

			//Since sphereVertices is 2 dimensional [lat][lon], this will set the index to the 'flattened index'
			//For instance, 2 * longitudeSteps + 3 would be sphereVertices[2][3]
			//In this case, lat + 1 moves it down 1
			
				sphereIndices[i] = lat * longitudeSteps + lon; // Top left
				sphereIndices[i + 1] = (lat + 1) * longitudeSteps + lon; // Bottom left
				sphereIndices[i + 2] = lat * longitudeSteps + (lon + 1); // Top right


				sphereIndices[i + 3] = lat * longitudeSteps + (lon + 1); // Top right
				sphereIndices[i + 4] = (lat + 1) * longitudeSteps + lon; // Bottom left
				sphereIndices[i + 5] = (lat + 1) * longitudeSteps + (lon + 1); // Bottom right

			


			i += 6;
		}
	}

	//--- Buffer generation for proc gen
	unsigned int SphereVAO, SphereVBO, SphereEBO;
	glGenVertexArrays(1, &SphereVAO);
	glBindVertexArray(SphereVAO);

	glGenBuffers(1, &SphereVBO);
	glBindBuffer(GL_ARRAY_BUFFER, SphereVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphereVertices), sphereVertices, GL_STATIC_DRAW);

	glGenBuffers(1, &SphereEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SphereEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphereIndices), sphereIndices, GL_STATIC_DRAW);

	//Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//Colours
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	Shader sphereShader("Shaders/LitVertexShader.v", "Shaders/LitFragmentShader.f");

	sphereShader.Use();
	sphereShader.setVec3("objectColor", vec3(1.0f, 0.5f, 0.31f));
	sphereShader.setVec3("lightColor", vec3(1.0f, 1.0f, 1.0f));

	//--- Lighting
	vec3 lightColour(1.0f, 1.0f, 1.0f);

	Shader lightShader("Shaders/LightsourceVertexShader.v", "Shaders/LightsourceFragmentShader.f");

	lightShader.Use();
	lightShader.setVec3("objectColor", vec3(1.0f, 1.0f, 1.0f));	

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

	GLuint lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);

	GLuint lightVBO;
	glGenBuffers(1, &lightVBO);
	glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(lightCubeVertices), lightCubeVertices, GL_STATIC_DRAW);


	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	vec3 lightPos(3.0f, 3.0f, 2.0f);
	mat4 lightModel = mat4(1.0f);
	lightModel = translate(lightModel, lightPos);
	lightModel = scale(lightModel, vec3(1.0f));


	//--- Constant render loop
	while (!glfwWindowShouldClose(window))
	{

		//--- Deltatime logic
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//--- Poll user input
		processInput(window);

		//--- Clear screen and set it to the random colour
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//--- Texture assigning
		// Set one of the texture units to the texture created, to use all textures in one draw call
		// The uniform values Texture1 and Texture 2 are used here, as the texture units 1 and 0 are created, those shaders use those
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1); //Like last time, future operations will affect this texture


		//--- Activate shader
		TexturedObjectShader.Use();

		//--- Pass updated projection matrix to vertex shaders
		mat4 projection = perspective(radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		TexturedObjectShader.setMat4("projection", projection);

		//--- Pass updated view matrix to vertex shaders
		//--Creating cameras
		//The camera / view space are all the vector coordinates as seen from the camera's perspective as the origin of the scene
		//THe view matrix transforms all world coordinates into view coordinates relative to the camera's position and direction.

		mat4 view = camera.GetViewMatrix();
		TexturedObjectShader.setMat4("view", view);

		//--- Render cubes		
		for (unsigned int i = 0; i < 10; i++)
		{
			// calculate the model matrix for each object and pass it to shader before drawing
			mat4 model = mat4(1.0f); // make sure to initialize matrix to identity matrix first
			model = translate(model, cubePositions[i]);
			float angle = 20.0f * i;
			model = rotate(model, radians(angle), vec3(1.0f, 0.3f, 0.5f));
			TexturedObjectShader.setMat4("model", model);

			sceneObjectDictionary["Cube Object"]->DrawMesh();
		}

		//--- Render Plane
		mat4 model = mat4(1.0f);
		model = rotate(model, radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
		model = scale(model, vec3(8.0f, 8.0f, 8.0f));

		TexturedObjectShader.setMat4("model", model);

		sceneObjectDictionary["Plane Object"]->DrawMesh();

		//-- Random stuff for projectile generation
		random_device rd;
		mt19937 gen(rd());
		uniform_real_distribution<> dis(0.0, 1.0);

		//-- Projectile spawning
		projectileSpawnTimer += deltaTime;
		if (projectileSpawnTimer >= projectileSpawnCooldown)
		{
			PhysicsObject* newProjectileObject = new PhysicsObject();
			newProjectileObject->VAO = ProjectileCubeVAO;
			newProjectileObject->PrepareAndBindVBO(ProjectileCubeVBO, sizeof(cubeVertices) / (5 * sizeof(float)));


			GLenum error;
			while ((error = glGetError()) != GL_NO_ERROR) {
				cerr << "OpenGL error after VBO binding: " << error << endl;
			}



			//Rand does not return a normalised value, it could return anything from 0 to potentially 32767. Divide by the maximum value to get a normalised value
			//2 * pi represents a full circle in radians, so dividing that by the random value gives a sector
			double angle = dis(gen) * 2.0 * PI;

			// Generate a random radius using sqrt method for uniform distribution
			double r = spawnRadius * sqrt(dis(gen));

			// Convert to Cartesian coordinates
			double x = spawnCentre.x + r * cos(angle);
			double z = spawnCentre.z + r * sin(angle);
			double y = spawnCentre.y;

			vec3 spawnPosition = vec3(x, y, z);

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
			PhysicsObject* projectileObject = projectileObjects[i];
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


		glBindVertexArray(ProcGenVAO);
		glDrawElements(GL_TRIANGLES, MAP_SIZE * 32, GL_UNSIGNED_INT, 0);
		GLenum error;
		while ((error = glGetError()) != GL_NO_ERROR) {
			cerr << "OpenGL error post proc gen render: " << error << endl;
		}

		//Sphere
		mat4 sphereModel = mat4(1.0f);
		sphereModel = translate(sphereModel, vec3(8.0f, 0.0f, -12.0f));

		sphereShader.Use();


		sphereShader.setMat4("model", sphereModel);
		sphereShader.setMat4("projection", projection);
		sphereShader.setMat4("view", view);



		glBindVertexArray(SphereVAO);
		glDrawElements(GL_TRIANGLES, sizeof(sphereIndices) / sizeof(GLuint), GL_UNSIGNED_INT, 0);
		error;
		while ((error = glGetError()) != GL_NO_ERROR) {
			cerr << "OpenGL error post proc gen render: " << error << endl;
		}

		//Light source
		lightShader.Use();
		lightShader.setMat4("model", lightModel);
		lightShader.setMat4("projection", projection);
		lightShader.setMat4("view", view);

		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		error;
		while ((error = glGetError()) != GL_NO_ERROR) {
			cerr << "OpenGL error post light render: " << error << endl;
		}

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

	for (PhysicsObject* projectile : projectileObjects)
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





