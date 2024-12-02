#pragma once

#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class CustomSceneObject
{
public:
	CustomSceneObject();
	~CustomSceneObject();
	void PrepareAndBindVAO();	
	void PrepareAndBindVBO(float vertices[], size_t verticesDataSize, int verticesCount);
	void PrepareAndBindVBO(unsigned int VBO, int verticesCount);
	void PrepareAndBindEBO(unsigned int indices[], size_t indicesDataSize, int indicesCount);
	void PrepareVertexAttributeArrays();
	void DrawMesh();
	unsigned int GetVAOID();
	unsigned int GetVBOID();
	unsigned int GetEBOID();
	void CleanUp();
	unsigned int VAO;
	unsigned int VBO;
	unsigned int EBO;
private:	
	int verticesCount = 0;
	int indicesCount = 0;	
	
};

