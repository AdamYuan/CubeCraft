#pragma once
#include "Util.hpp"
#include <cstdint>
typedef u_int8_t block;
#define BLOCK_NUM 8
#define BLOCK_TEX_NUM 9
enum Blocks
{
	Air=0,
	Dirt=1,
	Grass=2,
	Stone=3,
	Water=4,
	Wood=5,
	Leaves=6,
	Sand=7
};
//extern int block_textures[BLOCK_NUM][6];
namespace BlockUtil
{
	extern bool IsTransparent(block b);
	extern bool HaveHitbox(block b);
	extern int GetTexture(block b, short face);
	extern Box GetBox(glm::ivec3 pos);
};
