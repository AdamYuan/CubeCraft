#include "blocks.hpp"
int block_textures[BLOCK_NUM][6]=
{
	{-1},
	{0,0,0,0,0,0},
	{1,1,2,0,1,1},
	{3,3,3,3,3,3},
	{4,4,4,4,4,4},
	{5,5,5,5,5,5},
	{6,6,7,7,6,6},
	{8,8,8,8,8,8},
	{9,9,9,9,9,9}
};

bool block_m::isTransparent(block b)
{
	if(b==blocks::air ||
	   b==blocks::leaves ||
	   b==blocks::water)
		return true;
	return false;
}

int block_m::getTexture(block b,short face)
{
	return block_textures[b][face];
}

box block_m::getBox(glm::ivec3 pos)
{
	return box(pos,pos+1);
}
