#pragma once
#include "SuperChunk.hpp"
#include <unordered_set>
#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "ThreadPool/ThreadPool.h"
#include "FastNoiseSIMD/FastNoiseSIMD.h"
#include "Util.hpp"

class World
{
private:
	bool minDistanceCompare(const glm::ivec3 &a, const glm::ivec3 &b);
	void chunkUpdateFunc();
	void chunkLoadingFunc();
	void setTerrain(glm::ivec3 chunkPos, block (&blk)[(CHUNK_SIZE+2) * (CHUNK_SIZE+2) * (CHUNK_SIZE+2)]);
	std::unordered_set<glm::ivec3> chunkLoadedSet;

	std::unordered_set<glm::ivec3> chunkLoadingSet;

	std::unordered_set<glm::ivec3> chunkUpdateSet;

	std::mutex bgMtx;
	FastNoiseSIMD *fastNoise;
public:
	SuperChunk Voxels;

	std::list<glm::ivec3> ChunkRenderList;

	void UpdateChunkLists();

	void InitNoise();
};
