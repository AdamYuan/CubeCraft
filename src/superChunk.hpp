#pragma once
#include <unordered_map>
#include "chunk.hpp"
#include "types.hpp"

class superChunk
{
  public:
	std::unordered_map<glm::ivec3,chunk*> chunks;

	static glm::ivec3 getChunkPos(int x,int y,int z);
	static glm::ivec3 getChunkPos(const glm::ivec3 &pos);
	void setChunk(int x,int y,int z);
	void setChunk(const glm::ivec3 &chunkPos);
	chunk *getChunk(int x,int y,int z);
	chunk *getChunk(const glm::ivec3 &chunkPos);
	void eraseChunk(int x,int y,int z);
	void eraseChunk(const glm::ivec3 &chunkPos);

	void setBlock(int x,int y,int z,const block &blk);
	void setBlock(const glm::ivec3 &pos,const block &blk);
	block getBlock(int x,int y,int z);
	block getBlock(const glm::ivec3 &pos);

	void setLight(const glm::ivec3 &pos,light_t v);
	void setLight(int x,int y,int z,light_t v);
	light_t getLight(int x,int y,int z);
	light_t getLight(const glm::ivec3 &pos);
};
