#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <string>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h";
#include "stb_image.h"
#include "ModelData.h"

using namespace std;
using namespace glm;
class Model
{
public:
    Model(string path, string name, vector<Texture>& loadedTextures)
    {
        this->name = name;
        loadModel(path, loadedTextures);
    }
    void Draw(Shader& shader, vector<Texture>& loadedTextures);

    //TEMPORARY
    vector<Mesh> meshes;
    vector<Texture> textures_loaded;
    string directory;
    string name;
private:
    // model data
   // vector<Mesh> meshes;
 

    void loadModel(string path, vector<Texture>& loadedTextures);
    void processNode(aiNode* node, const aiScene* scene, vector<Texture>& loadedTextures);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene, vector<Texture>& loadedTextures);
    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type,
        string typeName, vector<Texture>& loadedTextures);
    
};

#endif


