#pragma once
#include "SuperChunk.hpp"
#include <set>
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
	void setTerrain(ChunkPtr chk);
	std::set<glm::ivec3, Ivec3Compare> chunkLoadedSet;

	std::set<glm::ivec3, Ivec3Compare> chunkLoadingSet;

	std::set<glm::ivec3, Ivec3Compare> chunkUpdateSet;

	std::mutex bgMtx;
	FastNoiseSIMD *fastNoise;
public:
	SuperChunk Voxels;

	std::list<glm::ivec3> ChunkRenderList;

	void UpdateChunkLists();

	void InitNoise();
};
