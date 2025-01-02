#ifndef MODEL_DATA_H
#define MODEL_DATA_H

#include <glm/glm.hpp>
#include <string>
using namespace std;
using namespace glm;

// Vertex struct that holds the vertex attributes (position, normal, texCoords, tangent, bitangent)
struct Vertex {
    // position
    vec3 Position;
    // normal
    vec3 Normal;
    // texCoords
    vec2 TexCoords;
    // tangent
    vec3 Tangent;
    // bitangent
    vec3 Bitangent;
};

enum TextureType {
    DIFFUSE,
    NORMAL,
    SPECULAR
};

/// <summary>
/// Texture struct | 
/// string name | 
/// Uint id | 
/// uint heldUnit | 
/// TextureType type | 
/// string path |
/// Additional textures |
/// </summary>
struct Texture {
    string name;
    unsigned int id;  // Texture ID
    unsigned int heldUnit; // The texture unit that this texture currently exists in
    TextureType type;      // Type of texture (e.g., "diffuse", "specular")
    string path;      // Path to the texture file
    int additionalTextures; // For example if two textures exist for one object, do 1
};



#endif
