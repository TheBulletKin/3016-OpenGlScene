#version 330 core
//Following's location value will be used by the vertex attribute pointer
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 Normal;

void main()
{	
	gl_Position = projection * view * model * vec4(aPos, 1.0f);	
	Normal = aNormal;
}