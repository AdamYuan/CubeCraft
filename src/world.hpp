#pragma once
#include "superChunk.hpp"
#include <set>
#include <list>
#include <thread>
#include <mutex>
#include "FastNoise/FastNoise.h"
#include "util.hpp"

class world
{
  private:
	bool minDistanceCompare(const glm::ivec3 &a, const glm::ivec3 &b);
	void chunkMeshingThread();
	void chunkLoadingThread();
	void setTerrain(chunkPtr chk);
	std::set<glm::ivec3, ivec3Compare> chunkLoadedSet;

	std::set<glm::ivec3, ivec3Compare> chunkLoadingSet;

	std::set<glm::ivec3, ivec3Compare> chunkMeshingSet;

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
