#version 330 core
//Following's location value will be used by the vertex attribute pointer
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aColourVertex;
layout (location = 4) in vec3 aTangent;
layout (location = 5) in vec3 aBitangent;
layout (location = 6) in mat4 instanceMatrix;



out VS_OUT{
	vec3 FragPos;
	vec2 TexCoord;
	vec3 Normal;
	vec3 ColourFrag;
	vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} vs_out

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
//= mat3(transpose(inverse(model)))
uniform mat3 inverseModelMat;
uniform vec3 lightPos;
uniform vec3 viewPos;

//bools
uniform bool useVertexColours;
uniform bool useTexCoords;
uniform bool useInstancing;
uniform bool useNormalMap;


void main()
{	
	if(useTexCoords){
		vs_out.TexCoord = aTexCoord;
	}

	if(useVertexColours){
		vs_out.ColourFrag = aColourVertex;
	}
	
	if(useNormalMap){
		vec3 T = normalize(inverseModelMat * aTangent);
    	vec3 N = normalize(inverseModelMat * aNormal);
    	T = normalize(T - dot(T, N) * N);
   	 	vec3 B = cross(N, T);
		mat3 TBN = transpose(mat3(T, B, N));    
    	vs_out.TangentLightPos = TBN * lightPos;
    	vs_out.TangentViewPos  = TBN * viewPos;
    	vs_out.TangentFragPos  = TBN * vs_out.FragPos; 

	}
	
	vs_out.Normal = inverseModelMat * aNormal;

	if(useInstancing){       
        vs_out.FragPos = vec3(instanceMatrix * vec4(aPos, 1.0)); 
        gl_Position = projection * view * instanceMatrix * vec4(vs_out.FragPos, 1.0);
    }
    else {
    	vs_out.FragPos = vec3(model * vec4(aPos, 1.0)); 
        gl_Position = projection * view * model * vec4(vs_out.FragPos, 1.0);
    }
}