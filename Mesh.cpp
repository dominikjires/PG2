// NESAHAT !!!
#include <iostream>

#include "ShaderProgram.hpp"
#include "Mesh.hpp"

// Constructor for Mesh class
Mesh::Mesh(GLenum primitive_type, std::vector<Vertex>& vertices, std::vector<GLuint>& indices, GLuint texture_id) :
    primitive_type(primitive_type),
    vertices(vertices),
    indices(indices),
    texture_id(texture_id)
{
    // Generate vertex array object (VAO), vertex buffer object (VBO), and element buffer object (EBO)
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // Bind the vertex array object
    glBindVertexArray(VAO);

    // Bind and fill the vertex buffer with vertex data
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    // Bind and fill the element buffer with index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    // Define vertex attribute pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, tex_coords)));
    glEnableVertexAttribArray(2);

    // Unbind the vertex array object
    glBindVertexArray(0);
}

// Draw method to render the mesh
void Mesh::Draw(ShaderProgram& shader, glm::mat4 mx_model)
{
    // Activate and bind texture if available
    if (texture_id > 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        shader.SetUniform("u_material.textura", 0);
    }
    // Set model matrix uniform
    shader.SetUniform("u_mx_model", mx_model);

    // Bind vertex array object and draw elements
    glBindVertexArray(VAO);
    glDrawElements(primitive_type, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

// Clear method to release resources
void Mesh::Clear()
{
    // Clear vertex and index data
    vertices.clear();
    indices.clear();
    // Reset primitive type to points
    primitive_type = GL_POINTS;

    // Delete vertex buffer and element buffer objects
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    // Delete vertex array object if exists
    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
        VAO = 0;
    }
    // Delete texture if exists
    if (texture_id != 0) {
        glDeleteTextures(1, &texture_id);
        texture_id = 0;
    }
}