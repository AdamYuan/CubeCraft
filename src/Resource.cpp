#include "Resource.hpp"
#include "Blocks.hpp"
#include <iostream>
#define SHADER_DIR std::string("resource/shader/")
#define TEXTURE_DIR std::string("resource/texture/")
namespace Resource
{
	std::string UniformMatrix = "matrix", UniformSampler = "sampler";

	Texture BlockTexture, FontTexture, FontBoldTexture;

	Shader BlockShader;
	std::string
			BlockUniformCamera,
			BlockUniformViewDistance;


	Shader LineShader;

	Shader TextShader;
	std::string
			TextUniformBgColor,
			TextUniformTextColor;


	void Init()
	{

		//Init block shader
		BlockShader.LoadFromFile(SHADER_DIR + "block.vert", GL_VERTEX_SHADER);
		BlockShader.LoadFromFile(SHADER_DIR + "block.frag", GL_FRAGMENT_SHADER);

		BlockUniformCamera = "camera";
		BlockUniformViewDistance = "viewDistance";

		//Init line shader
		LineShader.LoadFromFile(SHADER_DIR + "line.vert", GL_VERTEX_SHADER);
		LineShader.LoadFromFile(SHADER_DIR + "line.frag", GL_FRAGMENT_SHADER);

		//Init font shader
		TextShader.LoadFromFile(SHADER_DIR + "text.vert", GL_VERTEX_SHADER);
		TextShader.LoadFromFile(SHADER_DIR + "text.frag", GL_FRAGMENT_SHADER);
		TextUniformBgColor = "bgColor";
		TextUniformTextColor = "textColor";

		//Init block texture
		BlockTexture.Load2dArray(TEXTURE_DIR + "blocks.png", BLOCK_TEX_NUM);
		BlockTexture.SetParameters(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE);

		//Init font textures
		FontTexture.Load2d(TEXTURE_DIR + "font.png");
		FontTexture.BuildMipmap();
		FontTexture.SetParameters(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);

		FontBoldTexture.Load2d(TEXTURE_DIR + "font_bold.png");
		FontBoldTexture.BuildMipmap();
		FontBoldTexture.SetParameters(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);
	}

};
