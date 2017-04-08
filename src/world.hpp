#pragma once
#include "superChunk.hpp"
#include <unordered_set>
#include <list>
#include <thread>
#include <mutex>
#include "FastNoise/FastNoise.h"
#include "util.hpp"

class world
{
  private:
	void chunkUpdateThread();
	void chunkLoadingThread();
	void setTerrain(chunkPtr chk);
	std::unordered_set<glm::ivec3> chunkLoadedSet;

	std::unordered_set<glm::ivec3> chunkLoadingSet;

	std::unordered_set<glm::ivec3> chunkMeshingSet;

	std::mutex bgMtx;
	FastNoise fn;
  public:
	superChunk voxels;

	std::list<glm::ivec3> chunkRenderList;

	void updateChunkLists();
	void launchThreads();
	void quitThreads();
	void initNoise();
};
