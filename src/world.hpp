#pragma once
#include "superChunk.hpp"
#include <unordered_map>
#include <queue>
#include <deque>
#include <vector>
#include <thread>
#include <mutex>
#include "util.hpp"

class world
{
  private:
	void bgWork();
	std::mutex bgMtx;
  public:
	superChunk voxels;

	std::vector<glm::ivec3> chunkRenderList;

	std::unordered_map<glm::ivec3,bool> chunkInLoadingList;
	std::vector<glm::ivec3> chunkLoadingList;

	std::unordered_map<glm::ivec3,bool> chunkInMeshingList;
	std::vector<glm::ivec3> chunkMeshingList;

	void updateChunkLists();
};
