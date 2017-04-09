#pragma once
#include <map>
#include <memory>
#include "chunk.hpp"
#include "util.hpp"

using chunkPtr = std::shared_ptr<chunk>;
class superChunk
{
  public:
	std::map<glm::ivec3, chunkPtr, ivec3Compare> chunks;

	static glm::ivec3 getChunkPos(int x,int y,int z);
	static glm::ivec3 getChunkPos(const glm::ivec3 &pos);
	void setChunk(int x,int y,int z);
	void setChunk(const glm::ivec3 &chunkPos);
	chunkPtr getChunk(int x,int y,int z);
	chunkPtr getChunk(const glm::ivec3 &chunkPos);
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
