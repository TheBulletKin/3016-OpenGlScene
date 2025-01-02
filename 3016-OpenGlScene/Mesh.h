#ifndef MESH_H
#define MESH_H

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include <vector>
#include <string>

#include "Shader.h"

#include "ModelData.h"


using namespace glm;
using namespace std;

class Mesh {
public:
    // mesh data
    vector<Vertex>       vertices;
    vector<unsigned int> indices;
    vector<Texture>      textures;

    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures, unsigned int baseTextureUnit);
    void Draw(Shader& shader, unsigned int baseTextureUnit = 0);

    //  render data
    unsigned int VAO, VBO, EBO;
private:
    

    void setupMesh(unsigned int baseTextureUnit);
};



#endif