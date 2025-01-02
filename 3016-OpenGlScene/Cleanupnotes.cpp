//Todo Today
// Step1 : add a limit to the number of bubbles so point lights don't break
// Step 2 : Just added normal map to the model. Figure out how to add it to my models class following the tutorial

//Normal maps are becoming a little confusing, I think it'll be easier after I do my major refactor. Can then use one shader for the whole thing and not break things as much. Leave it broken for now, I can always look at the changes to the shader and revert it.



/* Mass Cleanup Start
* Begin by listing all the problems and workarounds
* 
* Structure is all over the place. I'll need to add more headers and region tags.
* Program layout:
*   Global variables and method definitions
*	Cube vertices
*	Main start
*	Create GL context and window
*	Create audio engine
*	Base plane creation
*	Projectile prefab using cube object
*	Projectile spawning variables
*	PROBLEM:
*		I create shaders here and set the colour, material and light values. Isn't based off any existing light and uses a different shader
*		to other objects
*	Texture loading
*	PROBLEM:
*		Loads the container texture randomly, binds to texture unit 0 and adds to the texture collection
*	Procedural terrain generation
*	Sphere object generation
*	PROBLEM: 
*		Creates the sphere shader and sets the light and object colour, material values and light values here
*	Light creation
*	PROBLEM:
*		Creates a list of positions which is alright. But each shader then sets the light position from a single value.
*		THis is because some shaders use only a single light position instead of an array.
*	Light colour noise texture
*	Sphere proc gen setup
*	PROBLEM: 
*		Creates the noise properly but this could go in a separate method.
*		Generates a new texture and assigns to this global texture array with a manual number.
*		Sets the sphere shader's texture sampler to this texture unit
*		Repeats for second noise texture
*	Model importing
*	PROBLEM:
*		Creates the objects and holds the texture object for each. Things get messy as it needs to pass in that texture unit number which is annoying.
*		Means the one that uses a normal texture requires two spaces after it which isn't good.
*	Random tree position generation
*	PROBLEM:
*		Could be moved to a new method.
*		Creates a new buffer to hold the matrices on the GPU
*		Then needs to add the attribute pointers by getting the VAO of the tree which was brute forced. The VAO shouldn't be accessible this way.
*	Model shader lighting setup
*	PROBLEM:
*		Having this here makes no sense
*	Main render loop start
*	Deltatime
*	User input
*	Background colour and clear
*	Audio engine position update
*	PROBLEM: Updates projection of textured shader
*	PROBLEM: Updates view of texture shader and model shader. Does this anyway later on
*	Changes light colour from the noise texture generated earlier
*	Texture object shader lighting shader uniforms
*	PROBLEM: 
*		A lot of repeated code for each point light. Not easy to change.
*		Is defined at an odd part in the loop
*	Cube Rendering
*	PROBLEM:
*		Will probably just remove these once lighting works on the other objects
*	Plane rendering
*	PROBLEM: 
*		Uses Model which should be more specific
*		Alters the diffuse and model part of the shader before rendering which isn't great
*	Lamp rendering
*	PROBLEM:
*		Asks for the texture unit number which has been a bit iffy with other objects and texture numbers
*	Projectile spawning
*	NOTES:	
*		Mostly good. Uses the projectile object preset to draw the rest. Doesn't use any instancing though.
*		Updates audio and launches it, no gripes with this.
*	Projectile updates
*	PROBLEM: 
*		Updates the mvp here as well as time and displacement. As well as the light pos and view pos
*		Doesn't account for multiple lights
*	Procedural terrain shader
*	PROBLEM:
*		In a random place. It's a simple translation but looks out of place
*	Single sphere displacement and rendering
*	NOTES:
*		Will probably remove this one as it won't be used in the main scene.
*		Only works with a single light source
*	Light object rendering
*	NOTES:
*		Can delete this and the light shader as I don't create the light object anymore. Got lamps for this
*	Instanced tree rendering
*	PROBLEMS:
*		Manually binds the vertex array, assigned the texture unit and passes the exact texture ID. 
*		Both not good
*		Then has to manually call the instanced render method
*	Single tree rendering
*	PROBLEM:
*		Applies all the shader uniforms again and draws using the texture list
*	Wall model rendering
*	NOTES: Alright, but still passes in the texture unit number
* 
*	CreateObject method
*	Takes in the attribute sizes and passes it to the prepare VBO method. Doesn't let you skip certain attributes
*	So if the shader has a certain layout but the first two are in a different order you can't change it to ignore them
* 
*	Create terrain
*	Works well, will change when I want to change how the terrain behaves
*	Has to use a different shader for the aforementioned reason. 
* 
*	Create sphere
*	Happy with it aside from the attributes at the end requiring a different shader again
* 
* 
*	MODEL CLASS
*	Explanation and run through
*	Creating the object calls loadModel in constructor. (move to CPP file)
*	Reads the file, scene is the whole scene with all objects and relations. Sets directory.
*	
	processNode runs next, a node is like a group of meshes. For every mesh it creates a new mesh which...
	
	Mesh constructor takes the vertices, indices and textures.
	SetupMesh will generate the VAO, VBO and EBO, set up the buffers to work with the default format of assimp imports - pos, normal, texcoord, tang, bitang
	Then activates the texture unit passed in and binds the first texture to it
	
	adds it to the custom meshes container and..
*	
	processMesh looks through each vertice and creates a new Vertex object. Adds the positions, normals, texture coords, tangents, bitangents.
*	looks at every face and gets its indices directly
*	then starts looking at materials
*	has it set up so the shader samplers are called texture_diffuseN from 1 to a max number.
*	Creates a vector of textures and loads from the file using...
*	
*	LoadMaterialTextures takes a material pointer, texture type and a type name.
*	Gets the texture from the type passed in (it knows which texture is which by the enum). 
*	Skip is false so it loads the texture. Uses a new texture object that holds an id, type as a string, path.
*	TextureFromFile takes a path and generates the texture ID. Reads the texture and binds it to the texture.
*		PROBLEM:
*			Currently uses the baseTextureUnit passed in which isn't useful for multiple textures
*	
*	Back to the material loading, it passes the type name which will be the texture sampler on the shader, like texture_diffuse
* 
*	Repeats this for all meshes
*	Model holds all loaded textures. All meshes for that model.
*	Mesh holds the VAO, VBO and EBO for each
*	
*	Model.Draw iterates through each mesh and runs its own draw method.
*	Iterates through each of the textures. Defines number and name, name is the type string which is the shader uniform
*	Number is the number of textures of each type which for me is just 1.
*	Sets the shader uniform for that texture to the current texture number which will set it to texture unit 1 incorrectly
*	Would have set the current texture unit to the base + the current texture value then bound the texture.
*	Then renders using draw elements.
*	


*/

/* Cleaning plan
* Fixing the texture and model loading requires fixing the shaders too.
* Remove redundant shaders to start with
* Shaders fixing:
*	Create a new shader to hold all these new changes
*	Look at all the differences between the different shaders.
*	Terrain shader should use the regular fragment shader but have the colour layout position.
*	Sort out the layout then look at how I set up these in the object creation method. Look at a way to define the attributes better so I can avoid assigning unecessary ones and make it know which to use
	Define a point light object on CPU side with a struct in the shader that matches
	Improve point light value assigning for textured objects using this new collection
	Now improve how the shader applies the point lights. Do it where it is now then consider potential cleaner alternatives
	Set the cubes to use the new shader for the time being to test things
	Set all values before rendering the cubes to start with
	work on the fragment shader now, combining the ones for cubes, models and base.
	Get this new shader working with the cubes. Figure out what i'm missing from other shaders and smush it all into one 
	Cubes aren't rendering now because the vertex data isn't right
	

	Plane only has position and tex coords, no normal inbetween. This is where I need to add a way to control that.
	Add a new attribute struct
		Length
		Type
	*/