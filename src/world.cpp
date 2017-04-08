#include "world.hpp"
#include "game.hpp"
#include "opengl/matrix.hpp"
#include "opengl/camera.hpp"
#include "opengl/frustum.hpp"
#include "renderer.hpp"
glm::ivec3 lastPlayerChunkPos(INT_MAX);
bool minDistanceCompare(const glm::ivec3 &a, const glm::ivec3 &b)
{
    if(b.x == INT_MAX) return true;
	bool af = frustum::cubeInFrustum(a*CHUNK_SIZE + CHUNK_SIZE/2, CHUNK_SIZE/2);
	bool bf = frustum::cubeInFrustum(b*CHUNK_SIZE + CHUNK_SIZE/2, CHUNK_SIZE/2);
	if(af != bf)
		return af;
	glm::vec3 v1 = game::gamePlayer.chunkPos - a;
	glm::vec3 v2 = game::gamePlayer.chunkPos - b;
	return glm::length(v1) < glm::length(v2);
}
void world::initNoise()
{
	fn.SetSeed(rand());
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
					chk->chunkPos.y*CHUNK_SIZE + CHUNK_SIZE + 1,
					chk->chunkPos.z*CHUNK_SIZE + k) >= 0 ?
							blocks::stone : blocks::grass;
			for (int j = CHUNK_SIZE; j--;) {
				if (fn.GetNoise(
						chk->chunkPos.x*CHUNK_SIZE + i,
						chk->chunkPos.y*CHUNK_SIZE + j,
						chk->chunkPos.z*CHUNK_SIZE + k) >= 0) {
					chk->set(i, j, k, current);
					current = blocks::stone;
				}
				else
					current = blocks::grass;
			}
		}
}
void world::chunkLoadingThread()
{
	std::lock_guard<std::mutex> lk(bgMtx);

	if(chunkLoadingSet.empty())
		return;

 	glm::ivec3 pos(INT_MAX);
	for(const glm::ivec3 &i : chunkLoadingSet)
		if(minDistanceCompare(i, pos))
			pos = i;


	chunkPtr chk = voxels.getChunk(pos);
	if(chk) {
		setTerrain(chk);
		chunkLoadedSet.insert(pos);
	}
	chunkLoadingSet.erase(pos);

}
void world::chunkUpdateThread()
{
	std::lock_guard<std::mutex> lk(bgMtx);

	if(chunkMeshingSet.empty())
		return;

	glm::ivec3 pos(INT_MAX);
	for(const glm::ivec3 &i : chunkMeshingSet)
		if(minDistanceCompare(i, pos))
			pos = i;

	chunkPtr chk = voxels.getChunk(pos);

	if(chk)
		chk->updateAll();

	chunkMeshingSet.erase(pos);
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
						printf("push: (%d, %d, %d)\n", i3.x, i3.y, i3.z);
						voxels.setChunk(i3);
						chunkLoadingSet.insert(i3);
					}
				}
	for(auto i=voxels.chunks.begin();i!=voxels.chunks.end();)
	{
		chunkPtr &chk=i->second;
		glm::ivec3 pos=i->first;

		if(!chk)//delete if the chunk is null
		{
			++i;
			voxels.eraseChunk(pos);
			continue;
		}

		if(pos.x < minLoadRange.x || pos.y < minLoadRange.y || pos.z < minLoadRange.z
		   || pos.x > maxLoadRange.x || pos.y > maxLoadRange.y || pos.z > maxLoadRange.z)
		{// chunk is out of loading range
			++i;
			voxels.eraseChunk(pos);
			chunkLoadedSet.erase(pos);
			printf("erase: (%d, %d, %d)\n", pos.x, pos.y, pos.z);
			continue;
		}

		if(!chunkLoadedSet.count(pos))
		{
			++i;
			continue;
		}

		if(!chk->updatedMesh)
		{
			chunkMeshingSet.insert(pos);
			//++i;
			//continue;
		}
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

	bgMtx.unlock();

	if(!chunkLoadingSet.empty())
	{
		std::thread loadThr(&world::chunkLoadingThread, this);
		loadThr.detach();
	}

	if(!chunkMeshingSet.empty())
	{
		std::thread meshThr(&world::chunkUpdateThread, this);
		meshThr.detach();
	}
}

void world::launchThreads()
{

}

void world::quitThreads()
{
}
