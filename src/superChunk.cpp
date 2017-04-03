#include "superChunk.hpp"
#include <iostream>
#include <functional>
glm::ivec3 superChunk::getChunkPos(int x,int y,int z)
{
	return glm::ivec3(
		(x+(x<0))/CHUNK_SIZE-(x<0),
		(y+(y<0))/CHUNK_SIZE-(y<0),
		(z+(z<0))/CHUNK_SIZE-(z<0));
}
glm::ivec3 superChunk::getChunkPos(const glm::ivec3 &pos)
{
	return getChunkPos(pos.x,pos.y,pos.z);
}

void superChunk::setChunk(const glm::ivec3 &chunkPos)
{
	std::hash<glm::ivec3> h;
	size_t hash_t = h(chunkPos);
	chunks[chunkPos]=std::make_shared<chunk>
			(this, chunkPos, std::to_string(hash_t));
}
void superChunk::setChunk(int x,int y,int z)
{
	setChunk(glm::ivec3(x,y,z));
}

std::shared_ptr<chunk> superChunk::getChunk(const glm::ivec3 &chunkPos)
{
	return chunks[chunkPos];
}
std::shared_ptr<chunk> superChunk::getChunk(int x,int y,int z)
{
	return getChunk(glm::ivec3(x,y,z));
}

void superChunk::eraseChunk(const glm::ivec3 &chunkPos)
{
	chunks.erase(chunkPos);
}
void superChunk::eraseChunk(int x,int y,int z)
{
	eraseChunk(glm::ivec3(x,y,z));
}

void superChunk::setBlock(const glm::ivec3 &pos, const block &blk)
{
	glm::ivec3 chunkPos=getChunkPos(pos);
	if(chunks[chunkPos]==nullptr)
		return;
	getChunk(chunkPos)->set(pos-(chunkPos*CHUNK_SIZE), blk);
}
void superChunk::setBlock(int x,int y,int z,const block &blk)
{
	setBlock(glm::ivec3(x,y,z),blk);
}

block superChunk::getBlock(const glm::ivec3 &pos)
{
	glm::ivec3 chunkPos=getChunkPos(pos);
	if(!chunks[chunkPos])
		return blocks::air;
	return getChunk(chunkPos)->get(pos-(chunkPos*CHUNK_SIZE));
}
block superChunk::getBlock(int x,int y,int z)
{
	return getBlock(glm::ivec3(x,y,z));
}

void superChunk::setLight(const glm::ivec3 &pos, light_t v)
{
	glm::ivec3 chunkPos=getChunkPos(pos);
	if(chunks[chunkPos]==nullptr)
		return;
	getChunk(chunkPos)->setl(pos-(chunkPos*CHUNK_SIZE), v);
}
void superChunk::setLight(int x,int y,int z,light_t v)
{
	setLight(glm::ivec3(x,y,z),v);
}

light_t superChunk::getLight(const glm::ivec3 &pos)
{
	glm::ivec3 chunkPos=getChunkPos(pos);
	if(!chunks[chunkPos])
		return 15;
	return getChunk(chunkPos)->getl(pos-(chunkPos*CHUNK_SIZE));
}
light_t superChunk::getLight(int x,int y,int z)
{
	return getLight(glm::ivec3(x,y,z));
}
