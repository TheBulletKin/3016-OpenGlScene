#version 330 core
//Following's location value will be used by the vertex attribute pointer
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in mat4 instanceMatrix;

//Pass the texture coordinate to the fragment shader
out vec2 TexCoord;
out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat3 inverseModelMat;
uniform bool useInstancing; 

void main()
{	
	TexCoord = aTexCoord;

	
	
	
	//Normal = aNormal;

	//This is computationally expensive, better to pass to the shader as a uniform
	Normal = inverseModelMat * aNormal;  

	if(useInstancing){
        //gl_Position = projection * view * instanceMatrix * vec4(aPos, 1.0);
        FragPos = vec3(instanceMatrix * vec4(aPos, 1.0)); 
        gl_Position = projection * view * instanceMatrix * vec4(FragPos, 1.0);
        
    }
    else {
        //gl_Position = projection * view * model * vec4(aPos, 1.0);
        FragPos = vec3(model * vec4(aPos, 1.0)); 
        gl_Position = projection * view * model * vec4(FragPos, 1.0);
        
    }
}