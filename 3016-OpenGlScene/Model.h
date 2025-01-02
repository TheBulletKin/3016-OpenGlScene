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
    Model(string path, unsigned int baseTextureUnit)
    {
        loadModel(path, baseTextureUnit);
    }
    void Draw(Shader& shader, unsigned int baseTextureUnit);

    //TEMPORARY
    vector<Mesh> meshes;
    vector<Texture> textures_loaded;
    string directory;
private:
    // model data
   // vector<Mesh> meshes;
 

    void loadModel(string path, unsigned int baseTextureUnit);
    void processNode(aiNode* node, const aiScene* scene, unsigned int baseTextureUnit);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene, unsigned int baseTextureUnit);
    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type,
        string typeName, unsigned int baseTextureUnit);
    
};

#endif


