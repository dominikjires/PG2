#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <glm/ext.hpp>

#include "ShaderProgram.hpp"

#define print(x) //std::cout << x << "\n"

ShaderProgram::ShaderProgram(const std::filesystem::path& VS_file, const std::filesystem::path& FS_file)
{
	std::vector<GLuint> shader_ids;
	
	shader_ids.push_back(CompileShader(VS_file, GL_VERTEX_SHADER));
	shader_ids.push_back(CompileShader(FS_file, GL_FRAGMENT_SHADER));

	ID = LinkShader(shader_ids);
	print("Instantiated shader ID=" << ID);
}

void ShaderProgram::Activate(void)
{
	print("Activating shader ID=" << ID);
	glUseProgram(ID);
}

void ShaderProgram::Deactivate(void)
{
	glUseProgram(0);
}

void ShaderProgram::Clear(void)
{
	Deactivate();
	glDeleteProgram(ID);
	ID = 0;
}

void ShaderProgram::SetUniform(const std::string& name, const float val)
{
	auto loc = glGetUniformLocation(ID, name.c_str());
	if (loc == -1) {
		std::cerr << "no uniform with name: '" << name << "' (ID=" << ID << ")\n";
		return;
	}
	glUniform1f(loc, val);
}

void ShaderProgram::SetUniform(const std::string& name, const int val)
{
	auto loc = glGetUniformLocation(ID, name.c_str());
	if (loc == -1) {
		std::cerr << "no uniform with name: '" << name << "' (ID=" << ID << ")\n";
		return;
	}
	glUniform1i(loc, val);
}

void ShaderProgram::SetUniform(const std::string& name, const glm::vec3 val)
{
	auto loc = glGetUniformLocation(ID, name.c_str());
	if (loc == -1) {
		std::cerr << "no uniform with name: '" << name << "' (ID=" << ID << ")\n";
		return;
	}
	glUniform3fv(loc, 1, glm::value_ptr(val));
}

void ShaderProgram::SetUniform(const std::string& name, const glm::vec4 val)
{
	auto loc = glGetUniformLocation(ID, name.c_str());
	if (loc == -1) {
		std::cerr << "no uniform with name: '" << name << "' (ID=" << ID << ")\n";
		return;
	}
	glUniform4fv(loc, 1, glm::value_ptr(val));
}

void ShaderProgram::SetUniform(const std::string& name, const glm::mat3 val)
{
	auto loc = glGetUniformLocation(ID, name.c_str());
	if (loc == -1) {
		std::cerr << "no uniform with name: '" << name << "' (ID=" << ID << ")\n";
		return;
	}
	glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(val));
}

void ShaderProgram::SetUniform(const std::string& name, const glm::mat4 val)
{
	auto loc = glGetUniformLocation(ID, name.c_str());
	if (loc == -1) {
		std::cerr << "no uniform with name: '" << name << "' (ID=" << ID << ")\n";
		return;
	}
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(val));
}

std::string ShaderProgram::GetShaderInfoLog(const GLuint obj)
{
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

std::string ShaderProgram::GetProgramInfoLog(const GLuint obj)
{
	int infologLength = 0;
	std::string s;
	glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infologLength);
	if (infologLength > 0) {
		std::vector<char> v(infologLength); // Opièárna
		glGetProgramInfoLog(obj, infologLength, NULL, v.data());
		s.assign(begin(v), end(v));
	}
	return s;
}

GLuint ShaderProgram::CompileShader(const std::filesystem::path& source_file, const GLenum type)
{
	// create and use shaders
	GLuint shader_h = glCreateShader(type);
	
	//const char* shader_string = TextFileRead(source_file).c_str(); // Dangling pointer, does not work
	std::string str = TextFileRead(source_file);
	const char* shader_string = str.c_str();

	glShaderSource(shader_h, 1, &shader_string, NULL);
	glCompileShader(shader_h);
	{ // check compile result, display error (if any)
		GLint cmpl_status;
		glGetShaderiv(shader_h, GL_COMPILE_STATUS, &cmpl_status);
		if (cmpl_status == GL_FALSE) {
			std::cerr << GetShaderInfoLog(shader_h);
			throw std::exception("Shader compilation err.\n");
		}
	}
	return shader_h;
}

GLuint ShaderProgram::LinkShader(const std::vector<GLuint> shader_ids)
{
	GLuint prog_h = glCreateProgram();
	for (const auto id : shader_ids) {
		glAttachShader(prog_h, id);
	}

	glLinkProgram(prog_h);
	{ // check link result, display error (if any)
		GLint status;
		glGetProgramiv(prog_h, GL_LINK_STATUS, &status);
		if (status == GL_FALSE) {
			std::cerr << GetProgramInfoLog(prog_h);
			throw std::exception("Link err.\n");
		}
	}

	return prog_h;
}

std::string ShaderProgram::TextFileRead(const std::filesystem::path& fn)
{
	std::ifstream file(fn);
	if (!file.is_open())
		throw std::exception("Error opening file.\n");
	std::stringstream ss;
	ss << file.rdbuf();
	return ss.str();
}
