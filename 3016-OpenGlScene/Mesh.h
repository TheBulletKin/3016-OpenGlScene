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

    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures, vector<Texture>& loadedTextures, string name);
    void Draw(Shader& shader, vector<Texture>& loadedTextures);

    //  render data
    unsigned int VAO, VBO, EBO;
    string name;
private:
    

    void setupMesh(vector<Texture>& loadedTextures);
};



#endif