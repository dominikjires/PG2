#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <glm/ext.hpp>

#include "ShaderProgram.hpp"


ShaderProgram::ShaderProgram(const std::filesystem::path& VS_file, const std::filesystem::path& FS_file) {
    std::vector<GLuint> shader_ids;

    shader_ids.push_back(CompileShader(VS_file, GL_VERTEX_SHADER));
    shader_ids.push_back(CompileShader(FS_file, GL_FRAGMENT_SHADER));

    ID = LinkShader(shader_ids);
    std::cout << "Instantiated shader ID=" << ID << std::endl;
}

void ShaderProgram::Activate() {
    std::cout << "Activating shader ID=" << ID << std::endl;
    glUseProgram(ID);
}

void ShaderProgram::Deactivate() {
    glUseProgram(0);
}

void ShaderProgram::Clear() {
    Deactivate();
    glDeleteProgram(ID);
    ID = 0;
}

void ShaderProgram::SetUniform(const std::string& name, float val) {
    auto loc = glGetUniformLocation(ID, name.c_str());
    if (loc == -1) {
        throw std::runtime_error("no uniform with name: '" + name + "' (ID=" + std::to_string(ID) + ")\n");
    }
    glUniform1f(loc, val);
}

void ShaderProgram::SetUniform(const std::string& name, int val) {
    auto loc = glGetUniformLocation(ID, name.c_str());
    if (loc == -1) {
        throw std::runtime_error("no uniform with name: '" + name + "' (ID=" + std::to_string(ID) + ")\n");
    }
    glUniform1i(loc, val);
}

void ShaderProgram::SetUniform(const std::string& name, const glm::vec3 val) {
    auto loc = glGetUniformLocation(ID, name.c_str());
    if (loc == -1) {
        throw std::runtime_error("no uniform with name: '" + name + "' (ID=" + std::to_string(ID) + ")\n");
    }
    glUniform3fv(loc, 1, glm::value_ptr(val));
}

void ShaderProgram::SetUniform(const std::string& name, const glm::vec4 val) {
    auto loc = glGetUniformLocation(ID, name.c_str());
    if (loc == -1) {
        throw std::runtime_error("no uniform with name: '" + name + "' (ID=" + std::to_string(ID) + ")\n");
    }
    glUniform4fv(loc, 1, glm::value_ptr(val));
}

void ShaderProgram::SetUniform(const std::string& name, const glm::mat3 val) {
    auto loc = glGetUniformLocation(ID, name.c_str());
    if (loc == -1) {
        throw std::runtime_error("no uniform with name: '" + name + "' (ID=" + std::to_string(ID) + ")\n");
    }
    glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(val));
}

void ShaderProgram::SetUniform(const std::string& name, const glm::mat4 val) {
    auto loc = glGetUniformLocation(ID, name.c_str());
    if (loc == -1) {
        throw std::runtime_error("no uniform with name: '" + name + "' (ID=" + std::to_string(ID) + ")\n");
    }
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(val));
}

std::string ShaderProgram::TextFileRead(const std::filesystem::path& fn) {
    std::ifstream file(fn);
    if (!file.is_open()) {
        throw std::runtime_error("Error opening file: " + fn.string() + "\n");
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

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
            throw std::runtime_error(GetShaderInfoLog(shader_h));
        }
    }
    return shader_h;
}

GLuint ShaderProgram::LinkShader(const std::vector<GLuint> shader_ids) {
    GLuint prog_h = glCreateProgram();
    for (const auto id : shader_ids) {
        glAttachShader(prog_h, id);
    }

    glLinkProgram(prog_h);
    {
        GLint status;
        glGetProgramiv(prog_h, GL_LINK_STATUS, &status);
        if (status == GL_FALSE) {
            throw std::runtime_error(GetProgramInfoLog(prog_h));
        }
    }

    return prog_h;
}
