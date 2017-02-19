#pragma once
#include "types.hpp"
#include <cstdint>
typedef u_int8_t block;
#define BLOCK_NUM 9
#define BTEXTURE_NUM 10
enum blocks
{
	air=0,
	dirt=1,
	grass=2,
	stone=3,
	bedrock=4,
	water=5,
	wood=6,
	leaves=7,
	sand=8
};
extern int block_textures[BLOCK_NUM][6];
namespace block_m
{

extern bool isTransparent(block b);
extern int getTexture(block b,short face);
extern box getBox(glm::ivec3 pos);

};
