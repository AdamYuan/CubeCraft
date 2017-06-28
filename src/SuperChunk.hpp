#pragma once
#include <unordered_map>
#include <memory>
#include "Chunk.hpp"
#include "Util.hpp"

using ChunkPtr = std::shared_ptr<Chunk>;
class SuperChunk
{
public:
	std::unordered_map<glm::ivec3, ChunkPtr> Chunks;

	static glm::ivec3 GetChunkPos(int x, int y, int z);
	static glm::ivec3 GetChunkPos(const glm::ivec3 &pos);
	void SetChunk(int x, int y, int z);
	void SetChunk(const glm::ivec3 &chunkPos);
	ChunkPtr GetChunk(int x, int y, int z);
	ChunkPtr GetChunk(const glm::ivec3 &chunkPos);
	void EraseChunk(int x, int y, int z);
	void EraseChunk(const glm::ivec3 &chunkPos);

	void SetBlock(int x, int y, int z, const block &blk);
	void SetBlock(const glm::ivec3 &pos, const block &blk);
	block GetBlock(int x, int y, int z);
	block GetBlock(const glm::ivec3 &pos);

	void SetLight(const glm::ivec3 &pos, light_t v);
	void SetLight(int x, int y, int z, light_t v);
	light_t GetLight(int x, int y, int z);
	light_t GetLight(const glm::ivec3 &pos);
};
