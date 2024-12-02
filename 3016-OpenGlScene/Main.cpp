#include "stb_image.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <map>
#include <vector>

#include "Camera.h"
#include "CustomSceneObject.h"
#include "PhysicsObject.h"
#include "Shader.h"


/* TODO
* Just added classes to make object creation much easier and allow for a sort of scene 'hierarchy'
* Next step is to work on parabolic physics movement
* Regular dictionary will hold static objects
* Movable objects will be its own list of a different type, a subclass of a simple object
* Each object needs to hold its current position
* (figure the rest out later)


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

	//--- Create Shader class passing in vert and frag files
	Shader ourShader("Shaders/VertexShader.v", "Shaders/FragmentShader.f");

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
	ourShader.Use();
	//The texture sampler on the fragment shader is given value '0', meaning when we put that texture on texture unit 0, it will automatically use it
	ourShader.setInt("texture1", 0);
	

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

	float projectileSpawnCooldown = 1.0f;
	float projectileSpawnTimer = 0.0f;

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
		ourShader.Use();

		//--- Pass updated projection matrix to vertex shaders
		mat4 projection = perspective(radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		ourShader.setMat4("projection", projection);

		//--- Pass updated view matrix to vertex shaders
		//--Creating cameras
		//The camera / view space are all the vector coordinates as seen from the camera's perspective as the origin of the scene
		//THe view matrix transforms all world coordinates into view coordinates relative to the camera's position and direction.

		mat4 view = camera.GetViewMatrix();
		ourShader.setMat4("view", view);

		//--- Render cubes		

		for (unsigned int i = 0; i < 10; i++)
		{
			// calculate the model matrix for each object and pass it to shader before drawing
			mat4 model = mat4(1.0f); // make sure to initialize matrix to identity matrix first
			model = translate(model, cubePositions[i]);
			float angle = 20.0f * i;
			model = rotate(model, radians(angle), vec3(1.0f, 0.3f, 0.5f));
			ourShader.setMat4("model", model);

			sceneObjectDictionary["Cube Object"]->DrawMesh();
		}

		mat4 model = mat4(1.0f);
		model = rotate(model, radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
		model = scale(model, vec3(8.0f, 8.0f, 8.0f));

		ourShader.setMat4("model", model);

		sceneObjectDictionary["Plane Object"]->DrawMesh();

		//-- Projectile spawning
		projectileSpawnTimer += deltaTime;
		if (projectileSpawnTimer >= projectileSpawnCooldown)
		{
			PhysicsObject* newProjectileObject = new PhysicsObject(); 
			newProjectileObject->PrepareAndBindVAO();
			newProjectileObject->PrepareAndBindVBO(cubeVertices, sizeof(cubeVertices), sizeof(cubeVertices) / (5 * sizeof(float))); //Because of 5 elements per vertex
			newProjectileObject->PrepareVertexAttributeArrays();

			newProjectileObject->Launch(vec3(0.2f, 1.2f, 0.2f), vec3(5.0f, 0.0f, 2.0f), currentFrame);

			projectileObjects.push_back(newProjectileObject);

			projectileSpawnTimer -= projectileSpawnCooldown;
		}

		for (PhysicsObject* projectileObject : projectileObjects)
		{
			projectileObject->UpdatePosition(deltaTime);

			mat4 projectileModel = mat4(1.0f);
			projectileModel = translate(projectileModel, projectileObject->currentPosition - projectileObject->initialPosition);
			ourShader.setMat4("model", projectileModel);

			projectileObject->DrawMesh();
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

	sceneObjectDictionary.clear();


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



