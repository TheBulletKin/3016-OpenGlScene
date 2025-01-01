#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in mat4 instanceMatrix;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform bool useInstancing; 


void main()
{
    
    TexCoords = aTexCoords;    

    if(useInstancing){
        gl_Position = projection * view * instanceMatrix * vec4(aPos, 1.0);
    }
    else {
        gl_Position = projection * view * model * vec4(aPos, 1.0);
    }
    
    
}