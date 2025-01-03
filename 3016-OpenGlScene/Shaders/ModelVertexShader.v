#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in mat4 instanceMatrix;

out vec2 TexCoord;
out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform bool useInstancing; 


void main()
{
    
    TexCoord = aTexCoords;   
    FragPos = vec3(model * vec4(aPos, 1.0)); 
    Normal = mat3(transpose(inverse(model))) * aNormal;  

    if(useInstancing){
        FragPos = vec3(instanceMatrix * vec4(aPos, 1.0)); 
        Normal = mat3(transpose(inverse(instanceMatrix))) * aNormal; 
        gl_Position = projection * view * instanceMatrix * vec4(aPos, 1.0);
    }
    else {
        FragPos = vec3(model * vec4(aPos, 1.0)); 
        Normal = mat3(transpose(inverse(model))) * aNormal;  
        gl_Position = projection * view * vec4(FragPos, 1.0);
    }
    
    
}