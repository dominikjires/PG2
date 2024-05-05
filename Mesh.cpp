#include <iostream>

#include "ShaderProgram.hpp"
#include "Mesh.hpp"

Mesh::Mesh(GLenum primitive_type, std::vector<Vertex>& vertices, std::vector<GLuint>& indices, GLuint texture_id) :
    primitive_type(primitive_type),
    vertices(vertices),
    indices(indices),
    texture_id(texture_id)
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, tex_coords)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void Mesh::Draw(ShaderProgram& shader, glm::mat4 mx_model)
{
    if (texture_id > 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        shader.SetUniform("u_material.textura", 0);
    }
    shader.SetUniform("u_mx_model", mx_model);

    glBindVertexArray(VAO);
    glDrawElements(primitive_type, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Mesh::Clear()
{
    vertices.clear();
    indices.clear();
    primitive_type = GL_POINTS;

    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
        VAO = 0;
    }
    if (texture_id != 0) {
        glDeleteTextures(1, &texture_id);
        texture_id = 0;
    }
}
