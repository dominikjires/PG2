#include <iostream>

#include "ShaderProgram.hpp"
#include "Mesh.hpp"

#define print(x) std::cout << x << "\n"

Mesh::Mesh(GLenum primitive_type, std::vector<Vertex>& vertices, std::vector<GLuint>& indices, GLuint texture_id) :
    primitive_type(primitive_type),
    vertices(vertices),
    indices(indices),
    texture_id(texture_id)
{
    // Create and initialize VAO, VBO, EBO and parameters
    // Generate the VAO and VBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    // Bind VAO (set as the current)
    glBindVertexArray(VAO);
    // Bind the VBO, set type as GL_ARRAY_BUFFER
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // Fill-in data into the VBO
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    // Bind EBO, set type GL_ELEMENT_ARRAY_BUFFER
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    // Fill-in data into the EBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    // Set and enable the Vertex Attribute for position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(0 + offsetof(Vertex, position)));
    glEnableVertexAttribArray(0);
    // Set end enable Vertex Attribute for Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(0 + offsetof(Vertex, normal)));
    glEnableVertexAttribArray(1);
    // Set end enable Vertex Attribute for Texture Coordinates
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(0 + offsetof(Vertex, tex_coords)));
    glEnableVertexAttribArray(2);

    // Bind VBO and VAO to 0 to prevent unintended modification of VAO/VBO
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
};

void Mesh::Draw(ShaderProgram& shader, glm::mat4 mx_model)
{
    if (texture_id > 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        shader.SetUniform("u_material.textura", 0); // We're only using texturing unit no. 0
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

    // delete all allocations 
    //glDeleteBuffers... //VBO a EBO
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    //glDeleteVertexArrays... // VAO
    if (VAO) { glDeleteVertexArrays(1, &VAO); VAO = 0; }
    if (texture_id) { glDeleteTextures(1, &texture_id); texture_id = 0; }

    // Destruktor ne-e
};
