#include "Blocks.hpp"

/*
	Air=0,
	Dirt=1,
	Grass=2,
	Stone=3,
	Water=4,
	Wood=5,
	Leaves=6,
	Sand=7
*/
unsigned block_textures[BLOCK_NUM][6]=
		{
				{0},
				{0, 0, 0, 0, 0, 0},
				{1, 1, 2, 0, 1, 1},//Grass
				{3, 3, 3, 3, 3, 3},
				{4, 4, 4, 4, 4, 4},
				{5, 5, 6, 6, 5, 5},//Wood
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
				true,
				false
		};

bool block_haveHitbox[BLOCK_NUM]=
		{
				false,
				true,
				true,
				true,
				false,
				true,
				true,
				true
		};

bool BlockUtil::IsTransparent(block b)
{
	return block_transparents[b];
}

bool BlockUtil::HaveHitbox(block b)
{
	return block_haveHitbox[b];
}

int BlockUtil::GetTexture(block b, short face)
{
	return block_textures[b][face];
}

Box BlockUtil::GetBox(glm::ivec3 pos)
{
	return Box(pos, pos+1);
}
