#include <GL/glew.h>
#include "shader.hpp"
#include <iostream>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <sstream>
shader::~shader()
{
	clear();
}
void shader::init()
{
	if(inited)
		return;
	inited=true;
	programId=glCreateProgram();
}
void shader::clear()
{
	glDeleteProgram(programId);
}
std::string shader::loadFile(std::string file)
{
	std::ifstream in;
	std::string str;
	in.exceptions(std::ifstream::badbit);//ensures ifstream object can throw exceptions
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
		std::cout << "failed to load shader file: " << file << std::endl;
	}
	return str;
}
void shader::load(std::string &source, unsigned int mode)
{
	init();
	uint shaderId=glCreateShader(mode);
	const char *csource=source.c_str();
	glShaderSource(shaderId, 1, &csource, nullptr);
	glCompileShader(shaderId);
	std::cout << "Loaded shader: \n" << source << std::endl;
	int success;
	char error[1000];
	glGetShaderiv(shaderId,GL_COMPILE_STATUS,&success);
	if(!success)
	{
		glGetShaderInfoLog(shaderId,1000,nullptr,error);
		std::cout << "Shader compile status: \n" << error << std::endl;
	}
	glAttachShader(programId,shaderId);
	glLinkProgram(programId);

	glDeleteShader(shaderId);
}
void shader::loadFromFile(std::string file, unsigned int mode)
{
	std::string source = loadFile(file);
	load(source,mode);
}
int shader::getUniform(std::string str)
{
	return glGetUniformLocation(programId, str.c_str());
}
int shader::getAttribute(std::string str)
{
	return glGetAttribLocation(programId, str.c_str());
}
void shader::use()
{
	glUseProgram(programId);
}
