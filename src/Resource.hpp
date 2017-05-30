#pragma once
#include <vector>
#include "MyGL/VertexObject.hpp"
#include "MyGL/Shader.hpp"
#include "MyGL/Texture.hpp"
#include "Util.hpp"
namespace Resource
{
	using namespace MyGL;
//****
//textures
//****
//block texture
	extern Texture BlockTexture;
//font textures
	extern Texture FontTexture, FontBoldTexture;

//****
//shaders
//****
	enum Attributes
	{
		Position = 0,
		Texcoord = 1,
		BTexture = 2,
		BFacing = 3,
		BLighting = 4
	};
	extern std::string UniformMatrix, UniformSampler;
//block shader
	extern Shader BlockShader;
	extern std::string
			BlockUniformCamera,
			BlockUniformViewDistance;

//line shader
	extern Shader LineShader;

//font shader
	extern Shader TextShader;
	extern std::string
			TextUniformBgColor,
			TextUniformTextColor;

//****
//objects
//****

//initialize
	extern void Init();
};
