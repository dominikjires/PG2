// NEŠAHAT !!!
#pragma once

#include <glm/glm.hpp>
#include <GL/glew.h>

#include <filesystem>

class ShaderProgram {
public:
	// you can add more constructors for pipeline with GS, TS etc.
	ShaderProgram(void) = default; // does nothing
	ShaderProgram(const std::filesystem::path& VS_file, const std::filesystem::path& FS_file); // load, compile, and link shader

	void Activate();
	void Deactivate();
	void Clear();

	// set uniform according to name
	// https://docs.gl/gl4/glUniform
	void SetUniform(const std::string& name, const float val);
	void SetUniform(const std::string& name, const int val);
	void SetUniform(const std::string& name, const glm::vec3 val);
	void SetUniform(const std::string& name, const glm::vec4 val);
	void SetUniform(const std::string& name, const glm::mat3 val);
	void SetUniform(const std::string& name, const glm::mat4 val);

private:
	GLuint ID{ 0 }; // default = 0, empty shader
	std::string GetShaderInfoLog(const GLuint obj);   // check for shader compilation error; if any, print compiler output  
	std::string GetProgramInfoLog(const GLuint obj);  // check for linker error; if any, print linker output

	GLuint CompileShader(const std::filesystem::path& source_file, const GLenum type); // try to load and compile shader
	GLuint LinkShader(const std::vector<GLuint> shader_ids);                           // try to link all shader IDs to final program
	std::string TextFileRead(const std::filesystem::path& filename);                   // load text file
};
