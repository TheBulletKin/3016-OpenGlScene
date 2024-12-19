#version 330 core
out vec4 FragColour;

in vec3 colourFrag;


void main()
{
	FragColour = vec4(colourFrag, 1.0f);
}