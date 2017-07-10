#include <chrono>
#include "World.hpp"
#include "Game.hpp"
#include "Renderer.hpp"

ThreadPool threadPool(THREAD_NUM);

glm::ivec3 lastPlayerChunkPos(INT_MAX-1);

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
	fastNoise = FastNoiseSIMD::NewFastNoiseSIMD(0);
	fastNoise->SetFractalOctaves(4);
}
void World::chunkLoadingFunc()
{
	bgMtx.lock();
	if(chunkLoadingSet.empty())
	{
		bgMtx.unlock();
		return;
	}

	glm::ivec3 pos(INT_MAX);
	for(const glm::ivec3 &i : chunkLoadingSet)
		if(minDistanceCompare(i, pos))
			pos = i;
	chunkLoadingSet.erase(pos);
	bgMtx.unlock();

	block blk[(CHUNK_SIZE+2) * (CHUNK_SIZE+2) * (CHUNK_SIZE+2)];
	std::uninitialized_fill(std::begin(blk), std::end(blk), Blocks::Air);

	ChunkFuncs::SetTerrain(pos, blk, fastNoise);

	bgMtx.lock();
	ChunkPtr chk = Voxels.GetChunk(pos);
	if(chk)
	{
		std::copy(std::begin(blk), std::end(blk), std::begin(chk->blk));
		chunkLoadedSet.insert(pos);
	}
	bgMtx.unlock();
}
void World::chunkUpdateFunc()
{
	bgMtx.lock();
	if(chunkUpdateSet.empty()) {
		bgMtx.unlock();
		return;
	}

	glm::ivec3 pos(INT_MAX);
	for(const glm::ivec3 &i : chunkUpdateSet)
		if(minDistanceCompare(i, pos))
			pos = i;

	ChunkPtr chk = Voxels.GetChunk(pos);
	if(!chk)
	{
		bgMtx.unlock();
		return;
	}
	chk->UpdatedMesh = true;
	chk->SolidMeshData.clear();
	chk->TransMeshData.clear();

	block blk[(CHUNK_SIZE+2) * (CHUNK_SIZE+2) * (CHUNK_SIZE+2)];
	std::uninitialized_copy(std::begin(chk->blk), std::end(chk->blk), std::begin(blk));

	chunkUpdateSet.erase(pos);
	bgMtx.unlock();

	auto MeshData = ChunkFuncs::GetMesh(pos, blk);

	bgMtx.lock();
	chk = Voxels.GetChunk(pos);
	if(chk)
	{
		chk->SolidMeshData = MeshData.first;
		chk->TransMeshData = MeshData.second;
	}
	bgMtx.unlock();
}
void World::UpdateChunkLists()
{
	//if(!bgMtx.try_lock())
	//	return;
	bgMtx.lock();


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
					if(glm::distance((glm::vec3)Game::player.ChunkPos, (glm::vec3)i3) <= (float)CHUNK_LOAD_DISTANCE &&
							!chunkLoadingSet.count(i3) && !chunkLoadedSet.count(i3))
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

		if(!chk)
			// delete if the chunk is null
		{
			i = Voxels.Chunks.erase(i);
			continue;
		}

		if(glm::distance((glm::vec3)Game::player.ChunkPos, (glm::vec3)pos) > (float)CHUNK_LOAD_DISTANCE)
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
			// chunk is not loaded
		{
			++i;
			continue;
		}
		else if(!chk->UpdatedMesh)
			// chunk is loaded but hasn't meshed
			chunkUpdateSet.insert(pos);
		else if(!chk->SolidMeshData.empty() || !chk->TransMeshData.empty())
		{
			bool update = true;
			glm::ivec3 neighbour;
			for(short f = 0; f < 6 && update; ++f)
				if(Voxels.GetChunk(neighbour = pos + Funcs::GetFaceDirect(f)) && chunkUpdateSet.count(neighbour))
					update = false;
			if(update)
			{
				// chunk is meshed, then apply the mesh and clear the mesh array
				Renderer::ApplyChunkMesh(chk);

				chk->SolidMeshData.clear();
				chk->SolidMeshData.shrink_to_fit();

				chk->TransMeshData.clear();
				chk->TransMeshData.shrink_to_fit();
			}
		}

		if(chk->SolidMeshObject->Empty() && chk->TransMeshObject->Empty())
			// don't Render if there weren't any thing in the chunk mesh
		{
			++i;
			continue;
		}

		glm::vec3 center=(glm::vec3) pos*(float)CHUNK_SIZE+glm::vec3(CHUNK_SIZE/2);

		if(glm::distance(Game::camera.Position, center) > VIEW_DISTANCE+CHUNK_SIZE)
			//cull far away Chunks
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


