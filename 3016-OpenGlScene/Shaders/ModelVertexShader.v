#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in mat4 instanceMatrix;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    
    vec3 aNormal;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform bool useInstancing; 

uniform mat3 inverseModelMat;



void main()
{
   
    vs_out.TexCoords = aTexCoords;     
    

    
    vs_out.aNormal = inverseModelMat * aNormal; 

    if(useInstancing){
        //gl_Position = projection * view * instanceMatrix * vec4(aPos, 1.0);
        vs_out.FragPos = vec3(instanceMatrix * vec4(aPos, 1.0)); 
        gl_Position = projection * view * instanceMatrix * vec4(vs_out.FragPos, 1.0);
        
    }
    else {
        //gl_Position = projection * view * model * vec4(aPos, 1.0);
        vs_out.FragPos = vec3(model * vec4(aPos, 1.0)); 
        gl_Position = projection * view * model * vec4(vs_out.FragPos, 1.0);
        
    }
    
    
}