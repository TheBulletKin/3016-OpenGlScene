#version 330 core
struct Material{
 	vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

uniform Material material;

struct DirLight {
    vec3 direction;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  
uniform DirLight dirLight;

struct PointLight {    
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;  

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  
#define NR_POINT_LIGHTS 4
uniform PointLight pointLights[NR_POINT_LIGHTS];

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;       
};

uniform SpotLight spotLight;
 
in VS_OUT {
    vec3 FragPos;
	vec2 TexCoord;
	vec3 Normal;
	vec3 ColourFrag;
	vec3 Tangent;
} fs_in;

vec3 TangentLightPos;
vec3 TangentViewPos;
vec3 TangentFragPos;

//bools
uniform bool useVertexColours;
uniform bool useTexCoords;
uniform bool useInstancing;
uniform bool useNormalMap;
uniform bool useTexture;
uniform bool hasNormals;
uniform int numberOfPointLights;

//Texture samplers (assign correct texture unit on cpu)
uniform sampler2D texture_diffuse;
uniform sampler2D texture_normal;


uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

uniform mat3 inverseModelMat;

mat3 TBN;

out vec4 FragColor;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 colour); 
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 colour);  
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{


	 

    vec3 colour;
    if(useTexture == true && useVertexColours == false)
    {
        colour = texture(texture_diffuse, fs_in.TexCoord).rgb;
    }
    else if(useTexture == false && useVertexColours == true)
    {
        colour = fs_in.ColourFrag;
    }
    else if (useTexture == false && useVertexColours == false)
    {
        colour = material.diffuse;
    }

    
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightDir;
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    if(useNormalMap){ //Has a normal map to sample from
        // obtain normal from normal map in range [0,1]
        normal = texture(texture_normal, fs_in.TexCoord).rgb;
        // transform normal vector to range [-1,1]
        normal = normalize(normal * 2.0 - 1.0);  // this normal is in tangent space

        vec3 T = normalize(inverseModelMat * fs_in.Tangent);
        vec3 N = normalize(inverseModelMat * fs_in.Normal);
        T = normalize(T - dot(T, N) * N);
   	    vec3 B = cross(N, T);
	    TBN = transpose(mat3(T, B, N));    

        //normal = normalize(TBN * normal);
    
        TangentViewPos  = TBN * viewPos;
        TangentFragPos  = TBN * fs_in.FragPos;

        viewDir = normalize(TangentViewPos - TangentFragPos);
    } 
    

    
    vec3 result = vec3(0.0);
    // phase 1: Directional lighting
   // vec3 result = CalcDirLight(dirLight, normal, viewDir,colour);

    // phase 2: Point lights
    for(int i = 0; i < NR_POINT_LIGHTS; i++){
        result += CalcPointLight(pointLights[i], normal, fs_in.FragPos, viewDir, colour);
    }      
    // phase 3: Spot light
    // result += CalcSpotLight(spotLight, norm, FragPos, viewDir);    
    
    FragColor = vec4(result, 1.0);
    
    
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 colour)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    //Calculate the diffuse value based off these normals
        //1 means light source is perpendicular, less means it's more parallel
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    //vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, TexCoords));
    //vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, TexCoords));
    //vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    vec3 ambient  = light.ambient  * material.ambient;
    vec3 diffuse  = light.diffuse  * diff * colour;
    vec3 specular = light.specular * spec * material.specular;
    return (ambient + diffuse + specular);
}  

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 colour)
{
    /*
    vec3 lightDir;
    //if (useNormalMap){
       // TangentLightPos = TBN * light.position;
        //lightDir = normalize(TangentLightPos - TangentFragPos);
   // } else {
       // lightDir = normalize(light.position - fragPos);
    //}
    
     lightDir = normalize(light.position - fragPos);
    // diffuse shading
    //Calculate the diffuse value based off these normals
        //1 means light source is perpendicular, less means it's more parallel
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
  			     light.quadratic * (distance * distance));    
    // combine results
    vec3 ambient  = light.ambient  * material.ambient;
    vec3 diffuse  = light.diffuse  * diff * colour;
    vec3 specular = light.specular * spec * material.specular;
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
*/
    
    
    // ambient
    vec3 ambient = 0.1 * colour;
    // diffuse
    vec3 lightDir = normalize(TangentLightPos - TangentFragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * colour;
    // specular
    viewDir = normalize(TangentViewPos - TangentFragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

    vec3 specular = vec3(0.2) * spec;
    


    return (ambient + diffuse + specular);


} 

// calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    //Calculate the diffuse value based off these normals
        //1 means light source is perpendicular, less means it's more parallel
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient  = light.ambient  * material.ambient;
    vec3 diffuse  = light.diffuse  * diff * material.diffuse;
    vec3 specular = light.specular * spec * material.specular;
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}