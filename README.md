## Oliver Warry
# OpenGl Scene Report - Zone Wonderer
This repository contains all the content relating to my OpenGL scene, made as part of a university project.

## Overview
Inspired by Stalker 2 and Clear Sky, I have recreated the entrance to the Red Forest and generated multiple trees that scatter themselves randomly.
Bubbles spawn at random intervals at random points and launch vectors and float around the scene, emitting light and sound.
Lighting is present, bubbles emit dynamic lights and lamps emit an animated light that flickers with different colours.

## Video link
https://youtu.be/KkK-eeQ2UkQ

## Used dependencies
Dependencies currently held in the External Dependencies folder, lnked using relative path. It does however require you to have OpenGL installed locally on your device.

[GLAD](https://github.com/Dav1dde/glad)

[GLFW](https://www.glfw.org/)

[ASSIMP](https://github.com/assimp/assimp)

[GLM](https://github.com/g-truc/glm)

[IRRKLANG](https://www.ambiera.com/irrklang/)

[FastNoiseLite](https://github.com/Auburn/FastNoiseLite)

[OpenGL](https://www.opengl.org/)

## Assets used
https://ambientcg.com/view?id=Bark001 

https://ambientcg.com/view?id=Concrete040

https://pixabay.com/sound-effects/bathtub-ambience-27873/

https://pixabay.com/music/ambient-mysterious-ambient-suspense-atmosphere-252023/

https://ambientcg.com/view?id=Ground048

## Other used resources
[LearnOpenGl](https://learnopengl.com/) was used to get to grips with a lot of the fundamental concepts. The Shader header file, Model and Mesh.

Container.png was also taken from this resource.


# Implementation
Here I will outline the implementation of different features in the project.

## Basic Features
### Model, View, Projection
Each vertex shader has 3 mat4 uniforms for the Model, View and Projection matrix respectively. This allows for 3D projection in world space that alters as the camera moves. Example can be seen in `VertexShader.v`.

These uniforms are then updated at the start of the main render loop.
``` C++
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
```
### Model loading and textures
Models are loaded using the Assimp library and a custom `Model` and `Mesh` class, modifications of the ones provided by [LearnOpenGl](https://learnopengl.com/).

Models can be found under the [Media folder](3016-OpenGlScene/Media) in obj and fbx formats. Images located in the same path, textures taken from online sources cited in the [Assets Used](#Assets-Used) section.

The [Model](3016-OpenGlScene/Model.cpp) class uses the assimp importer to read the model data from the file, either obj or fbx in our case. Assimp imported objects are all contained in a Scene object, further consisting of child objects held as nodes, each with meshes and textures. As such, this class recursively iterates through all nodes, creates a vector of vertices to hold on the model object, and reads its materials which will load textures into the model object as well.

Textures are of different formats, with 1 `.png` type and 2 `.jpg` types.

### Keyboard and mouse movement
In the beginning of the render loop in [Main.cpp](3016-OpenGlScene/Main.cpp), `processInput` is run. This polls the glfw input buffer and changes the camera position based on the key pressed, running a method in the `Camera` class that alters its position and forward vector to be used in the calculation of the new view and projection matrices.

Mouse input uses the `mouse_callback` method which is assigned as the OpenGL callback at the top of [Main.cpp](3016-OpenGlScene/Main.cpp), and will take the current mouse position and last mouse position and alter the camera angle based on the difference with deltaTime considered.

### Lighting
Lighting is mostly handled in the fragment shader, for instance the [ModelFragmentShader](3016-OpenGlScene/Shaders/ModelFragmentShader.f).
This is a modification of the lighting methods discussed in the [LearnOpenGl](https://learnopengl.com/) lighting tutorial. Each shader that uses lighting contains a struct for a directional light (sunlight), point light (lamps etc) and a spotlight (player torch). It takes the normal from the vertex shader and calculates the view direction by taking the position of the fragment currently being rendered in world space to the viewPos, which is the camera's position in world space. It then calculates the diffuse, ambient and specular lighting for each type of light while adding it to a result that contains the resulting fragment colour.

In the linked video I explained that I had a slight error with the lighting, that it took a few seconds to update and display the dynamic lights. I have since fixed this and realised that I was using that max point light definition + the number of dynamic lights, and by changing it to this it now starts the scene with lighting and properly updates the dynamic lights. I just haven't been able to update the video to reflect this. This does mean that if I want more light sources I will need to up this number.

```
#define NR_POINT_LIGHTS 8
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform int staticPointLights = 4;
uniform int dynamicPointLights = 0;

Main()
{
    /...
    for(int i = 0; i < staticPointLights + dynamicPointLights; i++)
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);  
    /...
}
```

## Unique Features
### Procedural Terrain Generation
Displayed in Main.cpp, terrain is generated procedurally using FastNoiseLite.

### Sphere generation
The sphere used as the base for the bubbles was put together by the help of this article on [the virtual representation of a sphere](https://www.songho.ca/opengl/gl_sphere.html). It explaines how you can calculate the coordinates of a sphere mesh using phi and theta, iterate through to create a vertex array as well as the indices used for rendering. My implementation takes on a lot of what this resource teaches, but I have changed it considerably to match my needs.

Implementation can be seen in the CreateSphereObject method in Main.cpp.


### Projectile objects
Each of the bubbles seen in the scene are instances of an `ArcingProjectileObject`, a subclass of `CustomSceneObject`, a class I created to make assigning VAOs, EBOs and VBOs easier, while also allowing for me to maintain a collection of created objects. `ArcingProjectileObject`s take in a current position, launch vector and speed multipliers which are all generated here in the main render loop once a certain timer limit has been reached.

```C++
// ------------------------
// Projectile spawning
// -------------------------

// NOTE: Code block simplifed for markdown convenience. Full code in main.cpp
projectileSpawnTimer += deltaTime;
if (projectileSpawnTimer >= projectileSpawnCooldown && currentBubbles < maxBubbles)
{
	ArcingProjectileObject* newProjectileObject = new ArcingProjectileObject();

    //Prepare and bind VAO, VBO and EBO using existing Sphere Object
	newProjectileObject->VAO = sceneObjectDictionary["Sphere Object"]->VAO;
    //[...]

	//--- Generate random spawn position and random launch vector
    //[...]    

    //--- Generate random speed and gavity multiplier
    //[...]   
	newProjectileObject->Launch(vec3(spawnVelocity), vec3(spawnPosition), currentFrame, gravityMultiplier, movespeedMultiplier);
	projectileObjects.push_back(newProjectileObject);

	//--- Bubble sound
	ISound* sound = audioEngine->play3D("Media/Audio/bathtub-ambience-27873.mp3", vec3df(spawnPosition.x, spawnPosition.y, spawnPosition.z), true, false, true);
	//--- Set sound volume and add to sound collection
    //[...]

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
```

Then I iterate through the collection of objects, perform the UpdatePosition method while passing in the time since the last frame, destroy the projectile if the conditions are met or update the shader with the new position to render the sphere again.

```C++
for (size_t i = 0; i < projectileObjects.size();) {
	ArcingProjectileObject* projectileObject = projectileObjects[i];
	if (projectileObject != NULL)
	{
		projectileObject->UpdatePosition(deltaTime);

		if (projectileObject->ShouldDestroy()) {
            //Delete projectile, attached light and attached sound	
		}
		else {
			mat4 projectileModel = mat4(1.0f);
			projectileModel = translate(projectileModel, projectileObject->initialPosition + (projectileObject->currentPosition - projectileObject->initialPosition));

			sphereShader.Use();
			sphereShader.setMat4("model", projectileModel);
			sphereShader.setFloat("time", currentFrame);
			
			bubbleSounds[projectileObject]->setPosition(vec3df(projectileObject->currentPosition.x, projectileObject->currentPosition.y, projectileObject->currentPosition.z));

			dynamicBubbleLights[projectileObject]->position = projectileObject->currentPosition;

			projectileObject->DrawMesh();
			++i;
		}
	}
}
```

### Noise texture mixing
After creating the sphere object programmatically I genenerated two new noise textures using the FastNoiseLite library. It will sample noise from the noise generator for each pixel on a 512 x 256 grid, generated a texture and assigns its held texture unit number to the sphere shader.

```C++
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

//Repeated for second texture
```

The sphere vertex shader takes these two texture samplers and mixes they values together based on an `animatedUV` value which takes the texCoord and shifts it based on the passed in time, which is a uniform passed in the main update loop. As the time increases. This will cause both textures to 'rotate' around the sphere, and the offset of 1.5 for the secondary noise value creates extra variation. The fragPos to be passed to the fragment shader then uses this displaced position instead of the vertex's initial position to create that bubble effect.

```GLSL
#version 330 core
layout (location = 0) in vec3 aPos;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 colour;
layout (location = 3) in vec3 aNormal;

out vec3 colourFrag;
out vec3 Normal;
out vec3 FragPos;

uniform sampler2D firstNoiseTexture;
uniform sampler2D secondNoiseTexture; 
uniform float time; 
uniform float displacementScale; 

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform bool flatShading;

void main(){
	vec2 animatedUV = texCoord + vec2(time * 0.1, 0.0);
	
	float primaryNoiseValue = texture(firstNoiseTexture, animatedUV).r;
    float secondaryNoiseValue = texture(secondNoiseTexture, animatedUV * 1.5).r;

	float combinedNoise = mix(primaryNoiseValue, secondaryNoiseValue, 0.5); // Weighted blend

	vec3 displacedPosition = aPos + normalize(aPos) * combinedNoise * displacementScale;

	FragPos = vec3(model * vec4(displacedPosition, 1.0));

	gl_Position = projection * view * model * vec4(displacedPosition, 1.0);

	colourFrag = colour;

	Normal = mat3(transpose(inverse(model))) * aNormal;
}
```

While this effectively creates the bubble effect I'm after, the normals at each point stay the same, so the surface of the bubble still looks flat. You can see the silohette of the sphere change, but the shading isn't altered yet. I would like to figure out a way to add this, but for the time being I couldn't get it to work. I would need to somehow get the data from neighbouring vertices and their new displaced position, which I haven't figured out yet.

### Audio
To handle audio I used the `irrklang` library. I just need to define a sound engine at the top of Main.cpp as well as a sound to hold the background audio. This is then started at the beginning of main, set to loop constantly.
```C++
//-- Audio
bool isPlayingBackgroundAudio = true;
ISoundEngine* audioEngine;
ISound* backgroundSound;
bool spacePressed = false;

//...Main()
audioEngine = createIrrKlangDevice();
backgroundSound = audioEngine->play2D("Media/Audio/mysterious-ambient-suspense-atmosphere-252023.mp3", true, false, true);
if (backgroundSound)
{
	backgroundSound->setVolume(0.04f);
}
```

Then in my processInput method I can listen for `space` key presses to pause the audio, and `R` key presses to play it again.

## Optimisation
### Instancing
All of the trees in the scene have random positions that are generated within a predefined square. All of this is handled in the `Random tree locations` region of Main.cpp. However once the trees are generated, instead of iterating through each tree position, editing the shader model uniform and drawing the mesh, I create an instance buffer holding the precomputed location matrices on the GPU as an instanceMatrix in the shader.

```C++
int numberOfTrees = 120;


//Pre compute the instanced matrices on the cpu
mat4* treeModelMatrices;
treeModelMatrices = new mat4[numberOfTrees];
for (int i = 0; i < numberOfTrees; i++)
{
	mat4 model = mat4(1.0);
	//...Generate random position
	randomTreePositions.push_back(randomSpawnPosition);
	model = translate(model, randomSpawnPosition);

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
```

Then in the main render loop I can simply call this and it will render all the trees by using the instanceMatrix held on the [ModelVertexShader](3016-OpenGlScene/Shaders/ModelVertexShader.v). Commenting the drawElementsInstanced method alongside the the methods called just above it, and uncommenting the old multi tree rendering section just below it will demonstrate the difference if you up the number of trees to something crazy like 12000.

### Backface and occlusion culling
Using OpenGLs built in methods, I define the front face to be any indices or faces with a anti-clockwise winding order, which will discard the rendering of any backfaces. The depth function will act as a form of occlusion culling, so any fragments behind one already rendered will not be calculated.

```C++
glEnable(GL_DEPTH_TEST);
glDepthFunc(GL_LESS);
glEnable(GL_CULL_FACE);
glCullFace(GL_BACK);
glFrontFace(GL_CCW);
```

## Missed features and future plans
### Further Optimisation
While the current version runs well due to its small size, I realise i can make further optimisations on the memory side of things. I could use `new` and move certain objects to the heap instead of keeping it on the stack which could help avoid potential memory problems in the future. This would require me going through and seeing where I can pass objects in by reference rather than value and considering the use of `new` instead of hard object definitions. It works well enough now but it is something I can tackle later.

### Normal Mapping
Over the last week I tried to get normal maps working, using a map I had baked for the tree in blender, but it just wasn't working right. I had edited the Model and Mesh class to use a global texture container shared across all objects to avoid texture overwriting, but not matter what I did it wouldn't work as I'd intended. I reverted back to a previous state using previous github commits.

### Ground texture fixes
I have added a ground texture to the base plane, but it doesn't quite look right. I will spend some time messing with the light values to try and fix it, but for the time being it would take up too much time and had to leave it as is for now.

### Additional texture planes and billboarding
Currently the trees have no leaves, it's just the trunk and branches. I would have liked to have added several leaf texture planes with alpha transparency to immitate the methods used in games to simulate foliage. In future I may also add grass using a similar method, but using billboarding to orient the planes towards the camera, used by games like The Long Dark to great effect. This, alongside potential ground textures, could enhance the overall aesthetic a lot.

### Trees placed on procedural terrain
The procedural terrain is placed behind the camera for the time being as a temporary measure because if it is what appears in front of you the trees would clip through the surface. In future i'd like to figure out how to generate a tree and have it be positions on the surface of this generated terrain, then the height could be held as a texture that the fragment shader mixes with an image texture to create a proper ground texture.