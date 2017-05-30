#include "Texture.hpp"
#include <SOIL/SOIL.h>
namespace MyGL
{
	void Texture::Load2d(const std::string &file)
	{
		type=GL_TEXTURE_2D;

		glGenTextures(1, &id);

		Bind();

		int width, height;
		unsigned char* image = SOIL_load_image(file.c_str(), &width, &height, 0, SOIL_LOAD_RGBA);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
				GL_RGBA, GL_UNSIGNED_BYTE, image);

		SOIL_free_image_data(image);

		Unbind();
	}
	void Texture::Load2dArray(const std::string &file, int depth)
	{
		type = GL_TEXTURE_2D_ARRAY;

		glGenTextures(1, &id);

		Bind();

		int width, height;
		unsigned char* image = SOIL_load_image(file.c_str(), &width, &height, 0, SOIL_LOAD_RGBA);

		height /= depth;

		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, width, height, depth, 0,
				GL_RGBA, GL_UNSIGNED_BYTE, image);

		SOIL_free_image_data(image);

		Unbind();
	}
	void Texture::BuildMipmap()
	{
		Bind();
		glGenerateMipmap(type);
		Unbind();
	}
	void Texture::SetParameters(GLenum min_filter, GLenum mag_filter, GLenum wrap_filter)
	{
		Bind();
		glTexParameteri(type, GL_TEXTURE_MIN_FILTER, min_filter);
		glTexParameteri(type, GL_TEXTURE_MAG_FILTER, mag_filter);
		glTexParameteri(type, GL_TEXTURE_WRAP_S, wrap_filter);
		glTexParameteri(type, GL_TEXTURE_WRAP_T, wrap_filter);
		Unbind();
	}
	void Texture::Bind()
	{
		glBindTexture(type, id);
	}
	void Texture::Unbind()
	{
		glBindTexture(type, 0);
	}
	Texture::~Texture()
	{
		glDeleteTextures(1, &id);
	}
}
