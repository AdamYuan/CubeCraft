#pragma once
#include <string>
#include <GL/glew.h>
#include <SFML/Graphics.hpp>
using namespace std;
class texture
{
  public:
	GLuint id;
	GLenum type;
	//void load(const string &file,int min_mag_filter=GL_NEAREST,int wrap_filter=GL_REPEAT);
	static sf::Image loadFile(const string &file);
	void load2d(const string &file);
	void load2dArray(const string &file,int depth);
	void buildMipmap();
	void texParameter(GLenum min_filter,GLenum mag_filter,GLenum wrap_filter);
	void bind();
	void unbind();
	~texture();
};
