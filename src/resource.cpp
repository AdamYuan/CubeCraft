#include "resource.hpp"
#include "settings.hpp"
#include "opengl/texture.hpp"
#include <GL/glew.h>
#include <iostream>
#define SHADER_DIR std::string("resource/shader/")
#define TEXTURE_DIR std::string("resource/texture/")
namespace resource
{

	texture blockTexture,fontTexture,fontBoldTexture;

	shader blockShader;
	GLint blockShader_attr_position,
			blockShader_attr_face,
			blockShader_attr_tex,
			blockShader_attr_lighting,
			blockShader_unif_matrix,
			blockShader_unif_camera,
			blockShader_unif_sampler,
			blockShader_unif_viewdis;


	shader lineShader;
	GLint lineShader_attr_position,
			lineShader_unif_matrix;

	shader textShader;
	GLint textShader_attr_position,
			textShader_attr_texcoord,
			textShader_unif_matrix,
			textShader_unif_sampler,
			textShader_unif_bg_color,
			textShader_unif_text_color;

	object crossObj;


	void init()
	{
		//init block shader
		blockShader.loadFromFile(SHADER_DIR + "block.vert", GL_VERTEX_SHADER);
		blockShader.loadFromFile(SHADER_DIR + "block.frag", GL_FRAGMENT_SHADER);

		blockShader_attr_position=0;
		blockShader_attr_tex=1;
		blockShader_attr_face=2;
		blockShader_attr_lighting=3;

		blockShader_unif_sampler=blockShader.getUniform("sampler");
		blockShader_unif_matrix=blockShader.getUniform("matrix");
		blockShader_unif_camera=blockShader.getUniform("camera");
		blockShader_unif_viewdis=blockShader.getUniform("viewdis");

		//init line shader
		lineShader.loadFromFile(SHADER_DIR + "line.vert", GL_VERTEX_SHADER);
		lineShader.loadFromFile(SHADER_DIR + "line.frag", GL_FRAGMENT_SHADER);
		lineShader_attr_position=0;
		lineShader_unif_matrix=lineShader.getUniform("matrix");

		//init font shader
		textShader.loadFromFile(SHADER_DIR + "text.vert", GL_VERTEX_SHADER);
		textShader.loadFromFile(SHADER_DIR + "text.frag", GL_FRAGMENT_SHADER);
		textShader_attr_position=0;
		textShader_attr_texcoord=1;
		textShader_unif_matrix=textShader.getUniform("matrix");
		textShader_unif_sampler=textShader.getUniform("sampler");
		textShader_unif_bg_color=textShader.getUniform("bg_color");
		textShader_unif_text_color=textShader.getUniform("text_color");

		//init block texture
		blockTexture.load2dArray(TEXTURE_DIR + "blocks.png", BLOCK_TEX_NUM);
		blockTexture.buildMipmap();
		blockTexture.texParameter(GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST, GL_REPEAT);

		//init font textures
		fontTexture.load2d(TEXTURE_DIR + "font.png");
		fontTexture.buildMipmap();
		fontTexture.texParameter(GL_LINEAR_MIPMAP_NEAREST,GL_LINEAR,GL_REPEAT);

		fontBoldTexture.load2d(TEXTURE_DIR + "font_bold.png");
		fontBoldTexture.buildMipmap();
		fontBoldTexture.texParameter(GL_LINEAR_MIPMAP_NEAREST,GL_LINEAR,GL_REPEAT);
	}
};
