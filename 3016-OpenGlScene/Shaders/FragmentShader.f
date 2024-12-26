#version 330 core
struct Material{
 	vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

uniform Material material;

struct Light {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

uniform Light light;  

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texture1;


uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform bool useTexture;

out vec4 FragColor;

void main()
{
	
    

	//vec4 colour;
	//if (useTexture)
   // {
   //     colour = texture(texture1, TexCoord);   		 
   // }
   // else
   // {        
     //   colour = vec4(objectColor, 1.0); 
    //}

	//vec3 lightDir = normalize(-light.direction); //For directional lights
    //vec3 lightDir = normalize(lightPos - FragPos);
    vec3 lightDir = normalize(light.position - FragPos);

  // ambient		
    //vec3 ambient = lightColor * material.ambient;
	vec3 ambient  = light.ambient * material.ambient;

    
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    
    float diff = max(dot(norm, lightDir), 0.0);
    //vec3 diffuse = lightColor * (diff * material.diffuse);
	vec3 diffuse  = light.diffuse * (diff * material.diffuse);
    
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    //vec3 specular = lightColor * (spec * material.specular); 
	vec3 specular = light.specular * (spec * material.specular); 

    // spotlight (soft edges)
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = (light.cutOff - light.outerCutOff);
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    diffuse  *= intensity;
    specular *= intensity;

    //Distance attentuation
    float distance    = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
    		    light.quadratic * (distance * distance));
    ambient  *= attenuation; 
    diffuse  *= attenuation;
    specular *= attenuation;  
        
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
  //color = vec4(light.ambient * vec3(texture(material.diffuse, TexCoords)), 1.0);
	
	

	//vec3 lighting = ambient + diffuse;      
	//FragColor = vec4(lighting * colour.rgb, colour.a);
    
}