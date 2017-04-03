#pragma once
#include "util.hpp"
#include <cstdint>
typedef u_int8_t block;
#define BLOCK_NUM 8
#define BTEXTURE_NUM 9
enum blocks
{
	air=0,
	dirt=1,
	grass=2,
	stone=3,
	water=4,
	wood=5,
	leaves=6,
	sand=7
};
//extern int block_textures[BLOCK_NUM][6];
namespace block_m
{

extern bool isTransparent(block b);
extern int getTexture(block b,short face);
extern box getBox(glm::ivec3 pos);

};
