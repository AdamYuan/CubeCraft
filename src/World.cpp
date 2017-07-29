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
	float al = glm::length(glm::vec3(Game::player.ChunkPos - a));
	float bl = glm::length(glm::vec3(Game::player.ChunkPos - b));
	return al/(af + 1.0f) < bl/(bf + 1.0f);
}
void World::chunkLoadingFunc()
{
	bgMtx.lock();
	if(chunkLoadingSet.empty())
	{
		loadingThreadNum--;
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

	ChunkFuncs::SetTerrain(pos, blk, 0);

	bgMtx.lock();
	ChunkPtr chk = Voxels.GetChunk(pos);
	if(chk)
	{
		std::copy(std::begin(blk), std::end(blk), std::begin(chk->blk));
		chunkLoadedSet.insert(pos);
	}
	loadingThreadNum--;
	bgMtx.unlock();
}
void World::chunkUpdateFunc()
{
	bgMtx.lock();
	if(chunkUpdatingSet.empty()) {
		updateThreadNum--;
		bgMtx.unlock();
		return;
	}

	glm::ivec3 pos(INT_MAX);
	for(const glm::ivec3 &i : chunkUpdatingSet)
		if(minDistanceCompare(i, pos))
			pos = i;

	ChunkPtr chk = Voxels.GetChunk(pos);
	if(!chk)
	{
		updateThreadNum--;
		bgMtx.unlock();
		return;
	}
	chk->UpdatedMesh = true;
	chk->SolidMeshData.clear();
	chk->SemitransMeshData.clear();

	block blk[(CHUNK_SIZE+2) * (CHUNK_SIZE+2) * (CHUNK_SIZE+2)];
	std::uninitialized_copy(std::begin(chk->blk), std::end(chk->blk), std::begin(blk));

	chunkUpdatingSet.erase(pos);
	bgMtx.unlock();

	auto MeshData = ChunkFuncs::GetMesh(pos, blk);

	bgMtx.lock();
	chk = Voxels.GetChunk(pos);
	if(chk)
	{
		chk->SolidMeshData = MeshData.first;
		chk->SemitransMeshData = MeshData.second;
		chunkUpdatedSet.insert(chk->ChunkPos);
	}
	updateThreadNum--;
	bgMtx.unlock();
}
void World::UpdateChunkLists()
{
	std::lock_guard<std::mutex> lockGuard(bgMtx);

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
					if(glm::distance(glm::vec3(Game::player.ChunkPos), glm::vec3(i3)) <= (float)CHUNK_LOAD_DISTANCE &&
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
			std::cout << Util::Vec3ToString(pos) << std::endl;
			i = Voxels.Chunks.erase(i);
			continue;
		}

		if(glm::distance(glm::vec3(Game::player.ChunkPos), glm::vec3(pos)) > (float)CHUNK_LOAD_DISTANCE)
		{// chunk is out of loading range
			++i;
			//completely erase the chunk
			Voxels.EraseChunk(pos);
			chunkLoadingSet.erase(pos);
			chunkUpdatingSet.erase(pos);
			chunkLoadedSet.erase(pos);
			chunkUpdatedSet.erase(pos);
			//printf("Unloaded: (%d, %d, %d)\n", pos.x, pos.y, pos.z);
			continue;
		}

		if(!chunkLoadedSet.count(pos))
			// chunk is not loaded
		{
			++i;
			continue;
		}
		else if(!chk->UpdatedMesh) {
			// chunk is loaded but hasn't meshed
			chunkUpdatingSet.insert(pos);
			chunkUpdatedSet.erase(pos);
		}
		else if(chunkUpdatedSet.count(pos) && !(chk->SolidMeshData.empty() && chk->SemitransMeshData.empty()))
		{
			bool update = true;
			glm::ivec3 neighbour;
			for(short f = 0; f < 6 && update; ++f)
				if(Voxels.GetChunk(neighbour = pos + Util::GetFaceDirect(f)) && !chunkUpdatedSet.count(neighbour))
					update = false;
			if(update)
			{
				// chunk is meshed, then apply the mesh and clear the mesh array
				Renderer::ApplyChunkMesh(chk);

				chk->SolidMeshData.clear();
				chk->SolidMeshData.shrink_to_fit();

				chk->SemitransMeshData.clear();
				chk->SemitransMeshData.shrink_to_fit();
			}
		}

		if(chk->SolidMeshObject.Empty() && chk->SemitransMeshObject.Empty())
			// don't Render if there weren't any thing in the chunk mesh
		{
			++i;
			continue;
		}

		glm::vec3 center = glm::vec3(pos) * (float)CHUNK_SIZE + glm::vec3(CHUNK_SIZE/2);

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

	size_t index = 0;
	while(loadingThreadNum < chunkLoadingSet.size() && index < MAX_LOADING_THREAD_IN_FRAME)
	{
		threadPool.enqueue(&World::chunkLoadingFunc, this);
		loadingThreadNum++;
		index ++;
	}

	index = 0;
	while(updateThreadNum < chunkUpdatingSet.size() && index < MAX_UPDATE_THREAD_IN_FRAME)
	{
		threadPool.enqueue(&World::chunkUpdateFunc, this);
		updateThreadNum++;
		index ++;
	}
}


