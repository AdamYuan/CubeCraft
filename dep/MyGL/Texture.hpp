#pragma once
#include <string>
#include <GL/glew.h>
namespace MyGL
{
	class Texture
	{
	private:
		GLuint id;
		GLenum type;
	public:
		//void Load(const string &file,int min_mag_filter=GL_NEAREST,int wrap_filter=GL_REPEAT);
		void Load2d(const std::string &file);
		void Load2dArray(const std::string &file, int depth);
		void BuildMipmap();
		void SetParameters(GLenum min_filter, GLenum mag_filter, GLenum wrap_filter);
		void Bind();
		void Unbind();
		~Texture();
	};
}
