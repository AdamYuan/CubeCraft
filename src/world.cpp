#include <chrono>
#include "world.hpp"
#include "game.hpp"
#include "MyGL/matrix.hpp"
#include "MyGL/camera.hpp"
#include "MyGL/frustum.hpp"
#include "renderer.hpp"

ThreadPool threadPool(4);

glm::ivec3 lastPlayerChunkPos((int) pow(CHUNK_LOAD_DISTANCE * 2 + 1, 3) * 2);
bool world::minDistanceCompare(const glm::ivec3 &a, const glm::ivec3 &b)
{
	if(b.x == INT_MAX) return true;
	float af = frustum::cubeInFrustum(a*CHUNK_SIZE + CHUNK_SIZE/2, CHUNK_SIZE/2);
	float bf = frustum::cubeInFrustum(b*CHUNK_SIZE + CHUNK_SIZE/2, CHUNK_SIZE/2);
	float al = glm::length((glm::vec3)(game::gamePlayer.chunkPos - a));
	float bl = glm::length((glm::vec3)(game::gamePlayer.chunkPos - b));
	return al/(af + 1.0f) < bl/(bf + 1.0f);
}
void world::initNoise()
{
	fn.SetSeed(0);
	fn.SetFrequency(0.005f);
	fn.SetFractalOctaves(3);
	fn.SetNoiseType(FastNoise::NoiseType::SimplexFractal);
}
void world::setTerrain(chunkPtr chk)
{
	for (int i = 0; i < CHUNK_SIZE; ++i)
		for (int k = 0; k < CHUNK_SIZE; ++k) {
			block current = fn.GetNoise(
					chk->chunkPos.x*CHUNK_SIZE + i,
					chk->chunkPos.y*CHUNK_SIZE + CHUNK_SIZE,
					chk->chunkPos.z*CHUNK_SIZE + k) >= 0.1 ?
							blocks::stone : blocks::grass;
			for (int j = CHUNK_SIZE; j--;) {
				if (fn.GetNoise(
						chk->chunkPos.x*CHUNK_SIZE + i,
						chk->chunkPos.y*CHUNK_SIZE + j,
						chk->chunkPos.z*CHUNK_SIZE + k) >= 0.1) {
					chk->set(i, j, k, current);
					current = blocks::stone;
				}
				else
					current = blocks::grass;
			}
		}
}
void world::chunkLoadingFunc()
{
	std::lock_guard<std::mutex> lk(bgMtx);

	if(chunkLoadingSet.empty())
		return;

	glm::ivec3 pos(INT_MAX);
	for(const glm::ivec3 &i : chunkLoadingSet)
		if(minDistanceCompare(i, pos))
			pos = i;

	chunkPtr chk = voxels.getChunk(pos);
	setTerrain(chk);
	chunkLoadedSet.insert(pos);
	chunkLoadingSet.erase(pos);
}
void world::chunkUpdateFunc()
{
	std::lock_guard<std::mutex> lk(bgMtx);
	if(chunkUpdateSet.empty())
		return;

	glm::ivec3 pos(INT_MAX);
	for(const glm::ivec3 &i : chunkUpdateSet)
		if(minDistanceCompare(i, pos))
			pos = i;

	chunkPtr chk = voxels.getChunk(pos);

	chk->updateAll();
	//chk->updateMeshing();
	//printf("Updated: (%d, %d, %d)\n", pos.x, pos.y, pos.z);

	chunkUpdateSet.erase(pos);
}
void world::updateChunkLists()
{
	if(!bgMtx.try_lock())
		return;

	chunkRenderList.clear();
	glm::ivec3 minLoadRange = game::gamePlayer.chunkPos
							  - CHUNK_LOAD_DISTANCE;
	glm::ivec3 maxLoadRange = game::gamePlayer.chunkPos
							  + CHUNK_LOAD_DISTANCE;
	glm::ivec3 i3;
	if(game::gamePlayer.chunkPos != lastPlayerChunkPos)
		for(i3.x = minLoadRange.x; i3.x <= maxLoadRange.x; ++i3.x)
			for(i3.y = minLoadRange.y; i3.y <= maxLoadRange.y; ++i3.y)
				for(i3.z = minLoadRange.z; i3.z <= maxLoadRange.z; ++i3.z)
				{
					if(!chunkLoadingSet.count(i3) && !chunkLoadedSet.count(i3))
					{
						voxels.setChunk(i3);
						chunkLoadingSet.insert(i3);
						//printf("Loaded: (%d, %d, %d)\n", i3.x, i3.y, i3.z);
					}
				}
	auto i = voxels.chunks.begin();

	while(i != voxels.chunks.end())
	{
		chunkPtr &chk=i->second;
		glm::ivec3 pos=i->first;

		if(!chk)//delete if the chunk is null
		{
			i = voxels.chunks.erase(i);
			continue;
		}

		if(pos.x < minLoadRange.x || pos.y < minLoadRange.y || pos.z < minLoadRange.z
		   || pos.x > maxLoadRange.x || pos.y > maxLoadRange.y || pos.z > maxLoadRange.z)
		{// chunk is out of loading range
			++i;
			//completely erase the chunk
			voxels.eraseChunk(pos);
			chunkLoadingSet.erase(pos);
			chunkUpdateSet.erase(pos);
			chunkLoadedSet.erase(pos);
			//printf("Unloaded: (%d, %d, %d)\n", pos.x, pos.y, pos.z);
			continue;
		}

		if(!chunkLoadedSet.count(pos))
		{
			++i;
			continue;
		}

		if(!chk->updatedMesh)
			chunkUpdateSet.insert(pos);
		else if(!chk->meshData.empty())
			renderer::applyChunkMesh(chk);

		if(chk->obj.elements==0)//don't render if there weren't any thing
		{
			++i;
			continue;
		}

		glm::vec3 center=(glm::vec3) pos*(float)CHUNK_SIZE+glm::vec3(CHUNK_SIZE/2);

		//Cull far away chunks
		if(glm::distance(camera::position,center) > VIEW_DISTANCE+CHUNK_SIZE)
		{
			++i;
			continue;
		}

		chunkRenderList.push_back(pos);

		++i;
	}

	lastPlayerChunkPos = game::gamePlayer.chunkPos;

	if(!chunkUpdateSet.empty())
		threadPool.enqueue(&world::chunkUpdateFunc, this);

	if(!chunkLoadingSet.empty())
		threadPool.enqueue(&world::chunkLoadingFunc, this);

	bgMtx.unlock();
}

