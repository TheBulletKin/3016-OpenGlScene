#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    
    vec3 aNormal;
} fs_in;


uniform sampler2D texture_diffuse1;

void main()
{    
    FragColor = texture(texture_diffuse1, fs_in.TexCoords);
}