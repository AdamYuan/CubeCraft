#include <chrono>
#include "World.hpp"
#include "Game.hpp"
#include "Renderer.hpp"

ThreadPool threadPool(CHUNK_LOAD_DISTANCE * CHUNK_LOAD_DISTANCE * CHUNK_LOAD_DISTANCE);

glm::ivec3 lastPlayerChunkPos((int) pow(CHUNK_LOAD_DISTANCE * 2 + 1, 3) * 2);
bool World::minDistanceCompare(const glm::ivec3 &a, const glm::ivec3 &b)
{
	if(b.x == INT_MAX) return true;
	float af = Game::frustum.CubeInFrustum(a*CHUNK_SIZE + CHUNK_SIZE/2, CHUNK_SIZE/2);
	float bf = Game::frustum.CubeInFrustum(b*CHUNK_SIZE + CHUNK_SIZE/2, CHUNK_SIZE/2);
	float al = glm::length((glm::vec3)(Game::player.ChunkPos - a));
	float bl = glm::length((glm::vec3)(Game::player.ChunkPos - b));
	return al/(af + 1.0f) < bl/(bf + 1.0f);
}
void World::InitNoise()
{
	fn.SetSeed(0);
	fn.SetFrequency(0.005f);
	fn.SetFractalOctaves(3);
	fn.SetNoiseType(FastNoise::NoiseType::SimplexFractal);
}
void World::setTerrain(ChunkPtr chk)
{
	for (int i = 0; i < CHUNK_SIZE; ++i)
		for (int k = 0; k < CHUNK_SIZE; ++k) {
			block current = fn.GetNoise(
					chk->ChunkPos.x*CHUNK_SIZE + i,
					chk->ChunkPos.y*CHUNK_SIZE + CHUNK_SIZE,
					chk->ChunkPos.z*CHUNK_SIZE + k) >= 0.1 ?
							Blocks::Stone : Blocks::Grass;
			for (int j = CHUNK_SIZE; j--;) {
				if (fn.GetNoise(
						chk->ChunkPos.x*CHUNK_SIZE + i,
						chk->ChunkPos.y*CHUNK_SIZE + j,
						chk->ChunkPos.z*CHUNK_SIZE + k) >= 0.1) {
					chk->SetBlock(i, j, k, current);
					current = Blocks::Stone;
				}
				else
					current = Blocks::Grass;
			}
		}
}
void World::chunkLoadingFunc()
{
	std::lock_guard<std::mutex> lk(bgMtx);

	if(chunkLoadingSet.empty())
		return;

	glm::ivec3 pos(INT_MAX);
	for(const glm::ivec3 &i : chunkLoadingSet)
		if(minDistanceCompare(i, pos))
			pos = i;

	ChunkPtr chk = Voxels.GetChunk(pos);
	setTerrain(chk);
	chunkLoadedSet.insert(pos);
	chunkLoadingSet.erase(pos);
}
void World::chunkUpdateFunc()
{
	std::lock_guard<std::mutex> lk(bgMtx);
	if(chunkUpdateSet.empty())
		return;

	glm::ivec3 pos(INT_MAX);
	for(const glm::ivec3 &i : chunkUpdateSet)
		if(minDistanceCompare(i, pos))
			pos = i;

	Voxels.GetChunk(pos)->UpdateAll();

	//chk->UpdateMeshing();
	//printf("Updated: (%d, %d, %d)\n", pos.x, pos.y, pos.z);

	chunkUpdateSet.erase(pos);
}
void World::UpdateChunkLists()
{
	if(!bgMtx.try_lock())
		return;

	ChunkRenderList.clear();
	glm::ivec3 minLoadRange = Game::player.ChunkPos
							  - CHUNK_LOAD_DISTANCE;
	glm::ivec3 maxLoadRange = Game::player.ChunkPos
							  + CHUNK_LOAD_DISTANCE;
	glm::ivec3 i3;
	if(Game::player.ChunkPos != lastPlayerChunkPos)
		for(i3.x = minLoadRange.x; i3.x <= maxLoadRange.x; ++i3.x)
			for(i3.y = minLoadRange.y; i3.y <= maxLoadRange.y; ++i3.y)
				for(i3.z = minLoadRange.z; i3.z <= maxLoadRange.z; ++i3.z)
				{
					if(!chunkLoadingSet.count(i3) && !chunkLoadedSet.count(i3))
					{
						Voxels.SetChunk(i3);
						chunkLoadingSet.insert(i3);
						//printf("Loaded: (%d, %d, %d)\n", i3.x, i3.y, i3.z);
					}
				}
	auto i = Voxels.Chunks.begin();

	while(i != Voxels.Chunks.end())
	{
		ChunkPtr &chk=i->second;
		glm::ivec3 pos=i->first;

		if(!chk)//delete if the chunk is null
		{
			i = Voxels.Chunks.erase(i);
			continue;
		}

		if(pos.x < minLoadRange.x || pos.y < minLoadRange.y || pos.z < minLoadRange.z
		   || pos.x > maxLoadRange.x || pos.y > maxLoadRange.y || pos.z > maxLoadRange.z)
		{// chunk is out of loading range
			++i;
			//completely erase the chunk
			Voxels.EraseChunk(pos);
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

		if(!chk->UpdatedMesh)
			chunkUpdateSet.insert(pos);
		else if(!chk->MeshData.empty())
			Renderer::ApplyChunkMesh(chk);

		if(chk->MeshObject.Empty())//don't Render if there weren't any thing
		{
			++i;
			continue;
		}

		glm::vec3 center=(glm::vec3) pos*(float)CHUNK_SIZE+glm::vec3(CHUNK_SIZE/2);

		//Cull far away Chunks
		if(glm::distance(Game::camera.Position, center) > VIEW_DISTANCE+CHUNK_SIZE)
		{
			++i;
			continue;
		}

		ChunkRenderList.push_back(pos);

		++i;
	}

	lastPlayerChunkPos = Game::player.ChunkPos;

	if(!chunkUpdateSet.empty())
		threadPool.enqueue(&World::chunkUpdateFunc, this);

	if(!chunkLoadingSet.empty())
		threadPool.enqueue(&World::chunkLoadingFunc, this);

	bgMtx.unlock();
}

