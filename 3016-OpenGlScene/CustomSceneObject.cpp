#include "CustomSceneObject.h"
#include "stb_image.h"
#include <iostream>

using namespace std;

CustomSceneObject::CustomSceneObject() {
	VAO = 0;
	VBO = 0;
	EBO = 0;
}

CustomSceneObject::~CustomSceneObject() {
	CleanUp();
}

//--- Vertex buffers and attributes
//VBO
// A buffer holding all the vertex attribute data shown above, held on the GPU, but has no meaning at the moment
//
//VAO
// Holds references to the VBOs, as well as the configuration of the vertex attributes, which VBO holds which attribute, how it's laid out etc


/// <summary>
/// Generate vertex arrays with an ID held on this object. Sets that VAO as the currently bound VAO.
/// </summary>
void CustomSceneObject::PrepareAndBindVAO() {
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
}

/// <summary>
/// Generates vertex buffer objects, binds them to the array buffer then fills it with vertex data
/// </summary>
/// <param name="vertices">Vertex data as an array</param>
/// <param name="verticesDataSize">sizeof(vertices), exact size in bytes</param>
/// <param name="verticesCount">sizeof(cubeVertices) / (<values in one vertex> * sizeof(<type>)</param>
void CustomSceneObject::PrepareAndBindVBO(float vertices[], size_t verticesDataSize, int verticesCount) {
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	this->verticesCount = verticesCount;
	glBufferData(GL_ARRAY_BUFFER, verticesDataSize, vertices, GL_STATIC_DRAW);
}

/// <summary>
/// Generates the Element buffer object, binds them to thr element array buffer then fille it with index data
/// </summary>
/// <param name="indices"></param>
/// <param name="indicesDataSize"></param>
/// <param name="indicesCount"></param>
void CustomSceneObject::PrepareAndBindEBO(unsigned int indices[], size_t indicesDataSize, int indicesCount) {
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	this->indicesCount = indicesCount;
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesDataSize, indices, GL_STATIC_DRAW);
}

/// <summary>
/// Enables the vertex attribute pointers in the VAO, required per VAO.
/// Position = 0
/// TextureCoord = 1
/// </summary>
void CustomSceneObject::PrepareVertexAttributeArrays() {
	//--- Vertex attribute pointers
	// Will tell OpenGL how to interpret the vertex buffer data.
	// Relates to variables in the shader, like the first 3 relating to position, last two are texture coords.

	// First parameter: Attribute location for the shader, should match 'location = 0' in the vertex shader
	// Second parameter: Each vertex has 3 values, xyz coordinates in this case. Colour for instance can bring it to 4
	// Third parameter: Type of each component is float (each coordinate)
	// Fourth parameter: NO normalisation is applied
	// Fifth parameter: The stride - or numer of bytes between the start of one vertex and the start of the next. Float * 3 given there are 3 components
	// Sixth parameter: Offset in the buffer where this attribute data begins, set to 0 as this is the only attribute

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

}

/// <summary>
/// Render the object using its indices if it has any
/// </summary>
void CustomSceneObject::DrawMesh() {
	if (indicesCount > 0)
	{
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, 0);
	}
	else {
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, verticesCount);
	}
}

unsigned int CustomSceneObject::GetVAOID() {
	return VAO;
}

unsigned int CustomSceneObject::GetVBOID() {
	return VBO;
}

unsigned int CustomSceneObject::GetEBOID() {
	return EBO;
}

/// <summary>
/// Deletes the VAO, VBO and EBO buffers, wipes ID values
/// </summary>
void CustomSceneObject::CleanUp() {
	if (VAO != 0)
	{
		glDeleteVertexArrays(1, &VAO);
		VAO = 0;
	}

	if (VBO != 0)
	{
		glDeleteBuffers(1, &VBO);
		VBO = 0;
	}

	if (EBO != 0)
	{
		glDeleteBuffers(1, &EBO);
		EBO = 0;
	}
}

