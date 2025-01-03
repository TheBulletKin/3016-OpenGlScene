#version 330 core
//Following's location value will be used by the vertex attribute pointer
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;

//Pass the texture coordinate to the fragment shader
out vec2 TexCoord;
out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{	
	TexCoord = aTexCoord;

	//Frag pos for this vertex. Fragments inbetween interpolate it
	FragPos = vec3(model * vec4(aPos, 1.0));
	
	//Normal = aNormal;

	//This is computationally expensive, better to pass to the shader as a uniform
	Normal = mat3(transpose(inverse(model))) * aNormal;  

	gl_Position = projection * view * vec4(FragPos, 1.0);
}