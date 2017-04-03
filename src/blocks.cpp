#include "blocks.hpp"

/*
	air=0,
	dirt=1,
	grass=2,
	stone=3,
	water=4,
	wood=5,
	leaves=6,
	sand=7
*/
unsigned block_textures[BLOCK_NUM][6]=
		{
				{0},
				{0, 0, 0, 0, 0, 0},
				{1, 1, 2, 0, 1, 1},//grass
				{3, 3, 3, 3, 3, 3},
				{4, 4, 4, 4, 4, 4},
				{5, 5, 6, 6, 5, 5},
				{7, 7, 7, 7, 7, 7},
				{8, 8, 8, 8, 8, 8}
		};

bool block_transparents[BLOCK_NUM]=
		{
				true,
				false,
                false,
				false,
				true,
				false,
				false,
				false
		};

bool block_m::isTransparent(block b)
{
	return block_transparents[b];
}

int block_m::getTexture(block b,short face)
{
	return block_textures[b][face];
}

box block_m::getBox(glm::ivec3 pos)
{
	return box(pos,pos+1);
}
