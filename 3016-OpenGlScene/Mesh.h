#ifndef MESH_H
#define MESH_H

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include <vector>
#include <string>

#include "Shader.h"


using namespace glm;
using namespace std;

struct Vertex {
    vec3 Position;
    vec3 Normal;
    vec2 TexCoords;
};

struct Texture {
    unsigned int id;
    string type;
    string path;
};

class Mesh {
public:
    // mesh data
    vector<Vertex>       vertices;
    vector<unsigned int> indices;
    vector<Texture>      textures;

    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures, unsigned int baseTextureUnit);
    void Draw(Shader& shader, unsigned int baseTextureUnit = 0);
private:
    //  render data
    unsigned int VAO, VBO, EBO;

    void setupMesh(unsigned int baseTextureUnit);
};



#endif