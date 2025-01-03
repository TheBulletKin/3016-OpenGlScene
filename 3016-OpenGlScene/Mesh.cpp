#include "Mesh.h"

Mesh::Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures, unsigned int baseTextureUnit)
{
	this->vertices = vertices;
	this->indices = indices;
	this->textures = textures;

	setupMesh(baseTextureUnit);
}

void Mesh::setupMesh(unsigned int baseTextureUnit)
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
		&indices[0], GL_STATIC_DRAW);

	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
	// vertex texture coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
	// vertex tangent
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
	// vertex bitangent
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));


	glActiveTexture(GL_TEXTURE0 + baseTextureUnit);
	glBindTexture(GL_TEXTURE_2D, textures[0].id);

	glBindVertexArray(0);
}

void Mesh::Draw(Shader& shader, unsigned int baseTextureUnit)
{
	unsigned int diffuseNr = 1;
	unsigned int specularNr = 1;
	unsigned int normalNr = 1;
	unsigned int heightNr = 1;
	for (unsigned int i = 0; i < textures.size(); i++)
	{
		// glActiveTexture(GL_TEXTURE0 + baseTextureUnit + i); // activate proper texture unit before binding
		 // retrieve texture number (the N in diffuse_textureN)
		string number;
		string name = textures[i].type;
		if (name == "texture_diffuse") {
			number = to_string(diffuseNr++);
		}

		else if (name == "texture_specular") {
			number = to_string(specularNr++);
		}
		else if (name == "texture_normal")
			number = std::to_string(normalNr++); // transfer unsigned int to string
		else if (name == "texture_height")
			number = std::to_string(heightNr++); // transfer unsigned int to string

		//Make sure the base texture unit passed in has a gap of two for the diffuse and normal for the trees
		shader.setInt((name + number).c_str(), i);
		glActiveTexture(GL_TEXTURE0 + baseTextureUnit + i);
	    glBindTexture(GL_TEXTURE_2D, textures[i].id);
	}
	//shader.setInt(("texture_diffuse1"), baseTextureUnit);
	//glActiveTexture(GL_TEXTURE0 + baseTextureUnit);

	GLenum error;
	while ((error = glGetError()) != GL_NO_ERROR) {
		cerr << "OpenGL error in mesh drawing: " << error << endl;
	}
	// draw mesh
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

}
