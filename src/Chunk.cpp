#include <GL/glew.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <memory>
#include <iterator>
#include <queue>
#include "Chunk.hpp"
#include "SuperChunk.hpp"
#include "Resource.hpp"
Chunk::Chunk(SuperChunk *_parent,glm::ivec3 _chunkPos,std::string _chunkLabel)
		:parent(_parent),ChunkPos(_chunkPos),ChunkLabel(_chunkLabel)
{
	SolidMeshObject = std::unique_ptr<MyGL::VertexObject>(new MyGL::VertexObject());
	TransMeshObject = std::unique_ptr<MyGL::VertexObject>(new MyGL::VertexObject());
	std::uninitialized_fill(std::begin(beCovered), std::end(beCovered), false);
	std::uninitialized_fill(std::begin(blk), std::end(blk), Blocks::Air);
	std::uninitialized_fill(std::begin(lightMap), std::end(lightMap), 0);
	//for(int i=0; i<6; ++i)
	//	updatedNeighbourMeshing[i] = true;
}
Chunk::~Chunk()
{
	//blk.close();
}
inline bool Chunk::IsValidPos(int x, int y, int z)
{
	return x>=-1 && x<=CHUNK_SIZE && y>=-1 && y<=CHUNK_SIZE && z>=-1 && z<=CHUNK_SIZE;
}
inline bool Chunk::IsValidPos(const glm::ivec3 &pos)
{
	return IsValidPos(pos.x, pos.y, pos.z);
}
int Chunk::XYZ(int x, int y, int z = 0, int digit = CHUNK_SIZE)
{
	return x+digit*(y+digit*z);
}
int Chunk::XYZ3(const glm::ivec3 &pos, int digit = CHUNK_SIZE)
{
	return XYZ(pos.x, pos.y, pos.z, digit);
}
glm::ivec3 Chunk::GetPosFromNum(int num)
{
	glm::ivec3 pos;
	pos.z = num / (CHUNK_SIZE*CHUNK_SIZE);
	num %= CHUNK_SIZE*CHUNK_SIZE;
	pos.y = num / CHUNK_SIZE;
	num %= CHUNK_SIZE;
	pos.x = num;
	return pos;
}
block Chunk::GetBlock(int x, int y, int z)
{
	if(x < -1 || x > CHUNK_SIZE ||
	   y < -1 || y > CHUNK_SIZE ||
	   z < -1 || z > CHUNK_SIZE)
		return Blocks::Air;
	return (block)blk[XYZ(x + 1, y + 1, z + 1, CHUNK_SIZE + 2)];
}
block Chunk::GetBlock(const glm::ivec3 &pos)
{
	return GetBlock(pos.x, pos.y, pos.z);
}
void Chunk::SetBlock(int x, int y, int z, const block &b)
{
	if(x < -1 || x > CHUNK_SIZE ||
	   y < -1 || y > CHUNK_SIZE ||
	   z < -1 || z > CHUNK_SIZE)
		return;
/*
	if(x == 0)
		updatedNeighbourMeshing[LEFT] = false;
	if(x == CHUNK_SIZE - 1)
		updatedNeighbourMeshing[RIGHT] = false;
	if(y == 0)
		updatedNeighbourMeshing[BOTTOM] = false;
	if(y == CHUNK_SIZE - 1)
		updatedNeighbourMeshing[TOP] = false;
	if(z == 0)
		updatedNeighbourMeshing[BACK] = false;
	if(z == CHUNK_SIZE - 1)
		updatedNeighbourMeshing[FRONT] = false;
*/
	blk[XYZ(x + 1, y + 1, z + 1, CHUNK_SIZE + 2)] = b;

	if(b != Blocks::Air && x != -1 && x != CHUNK_SIZE && z != -1 && z != CHUNK_SIZE)
		beCovered[XYZ(x, z)]=true;

	UpdatedMesh=false;
	UpdatedLight=false;
}
void Chunk::SetBlock(const glm::ivec3 &pos, const block &b)
{
	SetBlock(pos.x, pos.y, pos.z, b);
}

void Chunk::SetLight(int x, int y, int z, light_t v)
{
	if(!IsValidPos(x, y, z))
	{
		parent->SetLight(ChunkPos.x * CHUNK_SIZE + x, ChunkPos.y * CHUNK_SIZE + y, ChunkPos.z * CHUNK_SIZE + z, v);
		return;
	}
	lightMap[XYZ(x, y, z)]=v;
	UpdatedMesh=false;
}
void Chunk::SetLight(const glm::ivec3 &pos, light_t v)
{
	SetLight(pos.x, pos.y, pos.z, v);
}
light_t Chunk::GetLight(int x, int y, int z)
{
	if(!IsValidPos(x, y, z))
		return parent->GetLight(ChunkPos.x * CHUNK_SIZE + x, ChunkPos.y * CHUNK_SIZE + y, ChunkPos.z * CHUNK_SIZE + z);
	return lightMap[XYZ(x, y, z)];
}
light_t Chunk::GetLight(const glm::ivec3 &pos)
{
	return GetLight(pos.x, pos.y, pos.z);
}

struct lightNode
{
	glm::ivec3 position;
	light_t light_value;
};
void Chunk::UpdateLighting()
{
	if(UpdatedLight)
		return;

	std::queue <lightNode> sunLightBfsQueue;

	bool beCovered[CHUNK_SIZE][CHUNK_SIZE];

	std::uninitialized_fill(&beCovered[0][0], &beCovered[CHUNK_SIZE-1][CHUNK_SIZE-1]+1, false);

	/*ChunkPtr up_chk= parent->GetChunk(ChunkPos.x, ChunkPos.y + 1, ChunkPos.z);
	if(up_chk)
	{
		for(int x=0;x<CHUNK_SIZE;++x)
			for(int y=0;y<CHUNK_SIZE;++y)
				beCovered[x][y]=up_chk->beCovered[XYZ(x, y)];
	}*/

	for(int y=CHUNK_SIZE-1; y>=0; --y)
		for(int x=0; x<CHUNK_SIZE; ++x)
			for(int z=0; z<CHUNK_SIZE; ++z)
			{
				if(GetBlock(x, y, z) != Blocks::Air)
					beCovered[x][z]=true;

				if(!beCovered[x][z] && GetBlock(x, y, z) == Blocks::Air)
				{
					sunLightBfsQueue.push({{x, y, z}, 15});//add to Light queue
					SetLight(x, y, z, 15);//sun Light
				}
			}

	while(!sunLightBfsQueue.empty())
	{
		const lightNode &node = sunLightBfsQueue.front();
		int x=node.position.x, y=node.position.y, z=node.position.z;
		light_t lightValue=node.light_value;
		sunLightBfsQueue.pop();

		for(short face=0; face<6; ++face)
		{
			glm::ivec3 np = glm::ivec3(x,y,z) + Funcs::GetFaceDirect(face);//new Position

			if (BlockMethods::IsTransparent(GetBlock(np)) && GetLight(np) + 2 <= lightValue && lightValue > 0)
			{
				sunLightBfsQueue.push({np, (light_t)(lightValue - 1)});
				SetLight(np, (light_t) (lightValue - 1));
			}
		}
	}
	UpdatedLight=true;
}


