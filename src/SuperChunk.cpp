#include "SuperChunk.hpp"
#include <iostream>
glm::ivec3 SuperChunk::GetChunkPos(int x, int y, int z)
{
	return glm::ivec3(
		(x+(x<0))/CHUNK_SIZE-(x<0),
		(y+(y<0))/CHUNK_SIZE-(y<0),
		(z+(z<0))/CHUNK_SIZE-(z<0));
}
glm::ivec3 SuperChunk::GetChunkPos(const glm::ivec3 &pos)
{
	return GetChunkPos(pos.x, pos.y, pos.z);
}

void SuperChunk::SetChunk(const glm::ivec3 &chunkPos)
{
	std::hash<glm::ivec3> h;
	size_t hash_t = h(chunkPos);
	Chunks[chunkPos]=std::make_shared<Chunk>(this, chunkPos, std::to_string(hash_t));
}
void SuperChunk::SetChunk(int x, int y, int z)
{
	SetChunk(glm::ivec3(x, y, z));
}

ChunkPtr SuperChunk::GetChunk(const glm::ivec3 &chunkPos)
{
	if(Chunks.count(chunkPos))
		return Chunks[chunkPos];
	return nullptr;
}
ChunkPtr SuperChunk::GetChunk(int x, int y, int z)
{
	return GetChunk(glm::ivec3(x, y, z));
}

void SuperChunk::EraseChunk(const glm::ivec3 &chunkPos)
{
	if(Chunks.count(chunkPos))
		Chunks.erase(chunkPos);
}
void SuperChunk::EraseChunk(int x, int y, int z)
{
	EraseChunk(glm::ivec3(x, y, z));
}

void SuperChunk::SetBlock(const glm::ivec3 &pos, const block &blk)
{
	glm::ivec3 chunkPos= GetChunkPos(pos);
	if(Chunks[chunkPos]==nullptr)
		return;
	GetChunk(chunkPos)->SetBlock(pos-(chunkPos*CHUNK_SIZE), blk);
}
void SuperChunk::SetBlock(int x, int y, int z, const block &blk)
{
	SetBlock(glm::ivec3(x, y, z), blk);
}

block SuperChunk::GetBlock(const glm::ivec3 &pos)
{
	glm::ivec3 chunkPos= GetChunkPos(pos);
	if(!Chunks[chunkPos])
		return Blocks::Air;
	return GetChunk(chunkPos)->GetBlock(pos-(chunkPos*CHUNK_SIZE));
}
block SuperChunk::GetBlock(int x, int y, int z)
{
	return GetBlock(glm::ivec3(x, y, z));
}

void SuperChunk::SetLight(const glm::ivec3 &pos, light_t v)
{
	glm::ivec3 chunkPos= GetChunkPos(pos);
	if(Chunks[chunkPos]==nullptr)
		return;
	GetChunk(chunkPos)->SetLight(pos-(chunkPos*CHUNK_SIZE), v);
}
void SuperChunk::SetLight(int x, int y, int z, light_t v)
{
	SetLight(glm::ivec3(x, y, z), v);
}

light_t SuperChunk::GetLight(const glm::ivec3 &pos)
{
	glm::ivec3 chunkPos= GetChunkPos(pos);
	if(!Chunks[chunkPos])
		return 15;
	return GetChunk(chunkPos)->GetLight(pos-(chunkPos*CHUNK_SIZE));
}
light_t SuperChunk::GetLight(int x, int y, int z)
{
	return GetLight(glm::ivec3(x, y, z));
}
