#include "Shader.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
namespace MyGL
{
	Shader::~Shader()
	{
		clear();
	}
	void Shader::init()
	{
		if(!inited)
			programId = glCreateProgram();
		inited = true;
	}
	void Shader::clear()
	{
		glDeleteProgram(programId);
	}
	std::string Shader::loadFile(const std::string &file)
	{
		std::ifstream in;
		std::string str;
		in.exceptions(std::ifstream::badbit);//ensures ifstream VertexObject can throw exceptions
		try
		{
			in.open(file);
			std::stringstream ss;
			ss << in.rdbuf();
			in.close();
			str=ss.str();
		}
		catch(std::ifstream::failure)
		{
			std::cout << "failed to Load Shader file: " << file << std::endl;
		}
		return str;
	}
	void Shader::Load(const std::string &source, unsigned int mode)
	{
		init();
		GLuint shaderId = glCreateShader(mode);
		const char *csource = source.c_str();
		glShaderSource(shaderId, 1, &csource, nullptr);
		glCompileShader(shaderId);
		std::cout << "Loaded Shader: \n" << source << std::endl;
		int success;
		char error[1000];
		glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
		if(!success)
		{
			glGetShaderInfoLog(shaderId, 1000, nullptr, error);
			std::cout << "Shader compile status: \n" << error << std::endl;
		}
		glAttachShader(programId,shaderId);
		glLinkProgram(programId);

		glDeleteShader(shaderId);
	}
	void Shader::LoadFromFile(const std::string &file, unsigned int mode)
	{
		std::string source = loadFile(file);
		Load(source, mode);
	}
	GLint Shader::GetUniform(const std::string &str) const
	{
		return glGetUniformLocation(programId, str.c_str());
	}
	GLint Shader::operator[](const std::string &str) const
	{
		return GetUniform(str);
	}
	GLint Shader::GetAttribute(const std::string &str) const
	{
		return glGetAttribLocation(programId, str.c_str());
	}
	void Shader::Use()
	{
		glUseProgram(programId);
	}

	void Shader::PassMat4(const std::string &str, const glm::mat4 &matrix4)
	{
		glUniformMatrix4fv(GetUniform(str), 1, GL_FALSE, glm::value_ptr(matrix4));
	}

	void Shader::PassMat3(const std::string &str, const glm::mat3 &matrix3)
	{
		glUniformMatrix3fv(GetUniform(str), 1, GL_FALSE, glm::value_ptr(matrix3));
	}

	void Shader::PassVec4(const std::string &str, const glm::vec4 &vector4)
	{
		glUniform4fv(GetUniform(str), 1, glm::value_ptr(vector4));
	}

	void Shader::PassVec3(const std::string &str, const glm::vec3 &vector3)
	{
		glUniform3fv(GetUniform(str), 1, glm::value_ptr(vector3));
	}
}
