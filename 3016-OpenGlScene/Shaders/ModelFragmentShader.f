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

out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
    vec3 attNormal;
    vec3 lightPos;
    vec3 viewPos;
} fs_in;

uniform bool hasNormals;

uniform sampler2D texture_diffuse0;
uniform sampler2D texture_normal0;

void main()
{    
    //FragColor = texture(texture_diffuse1, TexCoords);
    vec3 normal;
    // get diffuse color
    vec3 color = texture(texture_diffuse0, fs_in.TexCoords).rgb;
    // ambient
    
    vec3 lightDir;
    float diff;
    vec3 viewDir;
    if(hasNormals){
        // obtain normal from normal map in range [0,1]
        normal = texture(texture_normal0, fs_in.TexCoords).rgb;
        // transform normal vector to range [-1,1]
        normal = normalize(normal * 2.0 - 1.0);  // this normal is in tangent space
    
        lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
        diff = max(dot(lightDir, normal), 0.0);
        viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    } else{
        normal = normalize(fs_in.attNormal);
        lightDir = normalize(fs_in.lightPos - fs_in.FragPos);
        diff = max(dot(lightDir, normal), 0.0);
        viewDir = normalize(fs_in.viewPos - fs_in.FragPos);
    }
    
   
    

    
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

    
    



    

   
   
    // attenuation
    float distance    = length(fs_in.lightPos - fs_in.FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
  			     light.quadratic * (distance * distance));    
    // combine results
    vec3 ambient  = light.ambient  * material.ambient;
    vec3 diffuse  = light.diffuse  * diff * material.diffuse;
    vec3 specular = light.specular * spec * material.specular;
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    FragColor = vec4(ambient + diffuse + specular, 1.0);
}