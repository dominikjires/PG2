#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <glm/ext.hpp>

#include "ShaderProgram.hpp"

// Constructor for ShaderProgram, takes file paths for vertex and fragment shaders
ShaderProgram::ShaderProgram(const std::filesystem::path& VS_file, const std::filesystem::path& FS_file) {
    std::vector<GLuint> shader_ids;

    // Compile vertex and fragment shaders, link them into a program
    shader_ids.push_back(CompileShader(VS_file, GL_VERTEX_SHADER));
    shader_ids.push_back(CompileShader(FS_file, GL_FRAGMENT_SHADER));

    ID = LinkShader(shader_ids); // Assign the linked shader program ID
    std::cout << "Instantiated shader ID=" << ID << std::endl; // Output shader ID
}

// Activate the shader program for rendering
void ShaderProgram::Activate() {
    std::cout << "Activating shader ID=" << ID << std::endl;
    glUseProgram(ID); // Use the shader program
}

// Deactivate the shader program
void ShaderProgram::Deactivate() {
    glUseProgram(0); // Stop using any shader program
}

// Clear the shader program
void ShaderProgram::Clear() {
    Deactivate(); // Deactivate the shader program
    glDeleteProgram(ID); // Delete the shader program
    ID = 0; // Reset the ID
}

// Set a uniform value in the shader program (float version)
void ShaderProgram::SetUniform(const std::string& name, float val) {
    auto loc = glGetUniformLocation(ID, name.c_str());
    if (loc == -1) {
        throw std::runtime_error("no uniform with name: '" + name + "' (ID=" + std::to_string(ID) + ")\n");
    }
    glUniform1f(loc, val);
}

// Set a uniform value in the shader program (int version)
void ShaderProgram::SetUniform(const std::string& name, int val) {
    auto loc = glGetUniformLocation(ID, name.c_str());
    if (loc == -1) {
        throw std::runtime_error("no uniform with name: '" + name + "' (ID=" + std::to_string(ID) + ")\n");
    }
    glUniform1i(loc, val);
}

// Set a uniform value in the shader program (glm::vec3 version)
void ShaderProgram::SetUniform(const std::string& name, const glm::vec3 val) {
    auto loc = glGetUniformLocation(ID, name.c_str());
    if (loc == -1) {
        throw std::runtime_error("no uniform with name: '" + name + "' (ID=" + std::to_string(ID) + ")\n");
    }
    glUniform3fv(loc, 1, glm::value_ptr(val));
}

// Set a uniform value in the shader program (glm::vec4 version)
void ShaderProgram::SetUniform(const std::string& name, const glm::vec4 val) {
    auto loc = glGetUniformLocation(ID, name.c_str());
    if (loc == -1) {
        throw std::runtime_error("no uniform with name: '" + name + "' (ID=" + std::to_string(ID) + ")\n");
    }
    glUniform4fv(loc, 1, glm::value_ptr(val));
}

// Set a uniform value in the shader program (glm::mat3 version)
void ShaderProgram::SetUniform(const std::string& name, const glm::mat3 val) {
    auto loc = glGetUniformLocation(ID, name.c_str());
    if (loc == -1) {
        throw std::runtime_error("no uniform with name: '" + name + "' (ID=" + std::to_string(ID) + ")\n");
    }
    glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(val));
}

// Set a uniform value in the shader program (glm::mat4 version)
void ShaderProgram::SetUniform(const std::string& name, const glm::mat4 val) {
    auto loc = glGetUniformLocation(ID, name.c_str());
    if (loc == -1) {
        throw std::runtime_error("no uniform with name: '" + name + "' (ID=" + std::to_string(ID) + ")\n");
    }
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(val));
}

// Read the contents of a text file
std::string ShaderProgram::TextFileRead(const std::filesystem::path& fn) {
    std::ifstream file(fn);
    if (!file.is_open()) {
        throw std::runtime_error("Error opening file: " + fn.string() + "\n");
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

// Get the information log for a shader
std::string ShaderProgram::GetShaderInfoLog(GLuint obj) {
    int infologLength = 0;
    std::string s;
    glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength);
    if (infologLength > 0) {
        std::vector<char> v(infologLength);
        glGetShaderInfoLog(obj, infologLength, NULL, v.data());
        s.assign(begin(v), end(v));
    }
    return s;
}

// Get the information log for a shader program
std::string ShaderProgram::GetProgramInfoLog(GLuint obj) {
    int infologLength = 0;
    std::string s;
    glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infologLength);
    if (infologLength > 0) {
        std::vector<char> v(infologLength);
        glGetProgramInfoLog(obj, infologLength, NULL, v.data());
        s.assign(begin(v), end(v));
    }
    return s;
}

// Compile a shader
GLuint ShaderProgram::CompileShader(const std::filesystem::path& source_file, GLenum type) {
    if (!std::filesystem::exists(source_file)) {
        throw std::runtime_error("File not found: " + source_file.string() + "\n");
    }

    GLuint shader_h = glCreateShader(type);

    std::string shader_string = TextFileRead(source_file);
    const char* shader_c_str = shader_string.c_str();

    glShaderSource(shader_h, 1, &shader_c_str, NULL);
    glCompileShader(shader_h);
    {
        GLint cmpl_status;
        glGetShaderiv(shader_h, GL_COMPILE_STATUS, &cmpl_status);
        if (cmpl_status == GL_FALSE) {
            throw std::runtime_error(GetShaderInfoLog(shader_h)); // Retrieve and throw compilation error
        }
    }
    return shader_h;
}

// Link shaders into a shader program
GLuint ShaderProgram::LinkShader(const std::vector<GLuint> shader_ids) {
    GLuint prog_h = glCreateProgram();
    for (const auto id : shader_ids) {
        glAttachShader(prog_h, id); // Attach shaders to the program
    }

    glLinkProgram(prog_h); // Link the program
    {
        GLint status;
        glGetProgramiv(prog_h, GL_LINK_STATUS, &status);
        if (status == GL_FALSE) {
            throw std::runtime_error(GetProgramInfoLog(prog_h)); // Retrieve and throw linking error
        }
    }

    return prog_h; // Return the linked shader program ID
}
