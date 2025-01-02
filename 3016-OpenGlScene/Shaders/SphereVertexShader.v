#version 330 core
//Following's location value will be used by the vertex attribute pointer
layout (location = 0) in vec3 aPos;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec3 colour;
layout (location = 1) in vec3 aNormal;

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
	//vec3 displacedPosition = aPos + normalize(aPos) * secondaryNoiseValue * displacementScale;
	
	FragPos = vec3(model * vec4(displacedPosition, 1.0));

	gl_Position = projection * view * model * vec4(displacedPosition, 1.0);

	
	colourFrag = colour;

	if (flatShading) {
        // For flat shading, use the normal of the face (cross product of two edge vectors)
        vec3 edge1 = aPos - displacedPosition;
        vec3 edge2 = aNormal; // Adjust as needed based on the face orientation
        Normal = normalize(cross(edge1, edge2));  // Flat normal calculation for the face
    } else {
        // For smooth shading, transform the normal and pass it
        Normal = mat3(transpose(inverse(model))) * aNormal;
    }

	Normal = mat3(transpose(inverse(model))) * aNormal;
}