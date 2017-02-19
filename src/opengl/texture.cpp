#include "texture.hpp"
#include <iostream>
#include <string>
#include <GL/glew.h>
#include <SFML/Graphics.hpp>
using namespace std;
sf::Image texture::loadFile(const string &file)
{
	sf::Image img_data;
	if (!img_data.loadFromFile(file))
		std::cout << "FAILED TO LOAD IMAGE: " << file << std::endl;
	return img_data;
}
void texture::load2d(const string &file)
{
	sf::Image img_data=loadFile(file);
	type=GL_TEXTURE_2D;

	glGenTextures(1, &id);

	bind();

	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_RGBA,
		img_data.getSize().x, img_data.getSize().y,
		0,
		GL_RGBA, GL_UNSIGNED_BYTE, img_data.getPixelsPtr());

	unbind();
	//cout << "Loaded texture:\n" << file << endl;
}
void texture::load2dArray(const string &file,int depth)
{
	sf::Image img_data=loadFile(file);
	type=GL_TEXTURE_2D_ARRAY;

	glGenTextures(1, &id);

	bind();

	int width=img_data.getSize().x,height=img_data.getSize().y/depth;

	glTexImage3D(
		GL_TEXTURE_2D_ARRAY, 0, GL_RGBA,
		width, height, depth, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, img_data.getPixelsPtr());

	unbind();
	//cout << "Loaded texture:\n" << file << endl;
}
void texture::buildMipmap()
{
	bind();
	glGenerateMipmap(type);
	glTexParameteri(type, GL_TEXTURE_MAX_LEVEL, 7);
	unbind();
}
void texture::texParameter(GLenum min_filter,GLenum mag_filter,GLenum wrap_filter)
{
	bind();
	glTexParameteri(type, GL_TEXTURE_MIN_FILTER, min_filter);
	glTexParameteri(type, GL_TEXTURE_MAG_FILTER, mag_filter);
	glTexParameteri(type, GL_TEXTURE_WRAP_S, wrap_filter);
	glTexParameteri(type, GL_TEXTURE_WRAP_T, wrap_filter);
	unbind();
}
void texture::bind()
{
	glBindTexture(type,id);
}
void texture::unbind()
{
	glBindTexture(type, 0);
}
texture::~texture()
{
	glDeleteTextures(1, &id);
}
