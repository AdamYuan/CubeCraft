#pragma once
#include <vector>
#include "opengl/object.hpp"
#include "opengl/shader.hpp"
#include "opengl/texture.hpp"
#include "util.hpp"
namespace resource
{
//****
//textures
//****
//block texture
extern texture blockTexture;
//font textures
extern texture fontTexture,fontBoldTexture;

//****
//shaders
//****
//block shader
extern shader blockShader;
extern GLint blockShader_attr_position,
	blockShader_attr_face,
	blockShader_attr_tex,
	blockShader_attr_lighting,
	blockShader_unif_matrix,
	blockShader_unif_camera,
	blockShader_unif_sampler,
	blockShader_unif_viewdis;

//line shader
extern shader lineShader;
extern GLint lineShader_attr_position,
	lineShader_unif_matrix;

//font shader
extern shader textShader;
extern GLint textShader_attr_position,
	textShader_attr_texcoord,
	textShader_unif_matrix,
	textShader_unif_sampler,
	textShader_unif_bg_color,
	textShader_unif_text_color;

//****
//objects
//****
extern object crossObj;

//initialize
extern void init();
extern void loadBlockTexture();

};
