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
	fastNoise = FastNoiseSIMD::NewFastNoiseSIMD();
	fastNoise->SetSeed(0);
	fastNoise->SetFrequency(0.002f);
	fastNoise->SetFractalOctaves(3);
}
void World::setTerrain(ChunkPtr chk)
{
	chk->LoadedTerrain = true;

	float* noiseSet = fastNoise->GetSimplexFractalSet(chk->ChunkPos.x * CHUNK_SIZE, 0, chk->ChunkPos.z * CHUNK_SIZE,
													  CHUNK_SIZE, 1, CHUNK_SIZE);
	int index=0;

	for(int i=0; i<CHUNK_SIZE; ++i)
		for(int j=0; j<CHUNK_SIZE; ++j)
		{
			int k = (int) std::round(noiseSet[index++] * 100) - 32;
			int cy = (k+(k<0))/CHUNK_SIZE-(k<0);
			if(cy == chk->ChunkPos.y) {
				int gy = k - (cy * CHUNK_SIZE);
				chk->SetBlock(i, gy, j, Blocks::Grass);
				for(int y=0; y < gy; ++y)
					chk->SetBlock(i, y, j, Blocks::Stone);
			}
			else if(chk->ChunkPos.y < cy)
				for(int y=0; y < CHUNK_SIZE; ++y)
					chk->SetBlock(i, y, j, Blocks::Stone);
		}

	FastNoiseSIMD::FreeNoiseSet(noiseSet);
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
		{
			Renderer::ApplyChunkMesh(chk);

			chk->MeshData.clear();
			chk->MeshData.shrink_to_fit();
		}

		if(chk->MeshObject->Empty())//don't Render if there weren't any thing
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

