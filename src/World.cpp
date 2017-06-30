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
	fastNoise = FastNoiseSIMD::NewFastNoiseSIMD();
	fastNoise->SetSeed(0);
	fastNoise->SetFractalOctaves(4);
}
void World::setTerrain(glm::ivec3 chunkPos, block (&blk)[(CHUNK_SIZE+2) * (CHUNK_SIZE+2) * (CHUNK_SIZE+2)])
{
#define getBlock(x, y, z) (blk[Chunk::XYZ(x+1, y+1, z+1, CHUNK_SIZE + 2)])
#define SCALE 32

	const int chunkBase = chunkPos.y * CHUNK_SIZE;
	const int seaLevel = -SCALE;

	if(chunkBase > 1) //must be full of air
	{
		return;
	}
	if(chunkBase < -SCALE * 2 - 1) //must be full of stone
	{
		std::fill(std::begin(blk), std::end(blk), Blocks::Stone);
		return;
	}

	fastNoise->SetFrequency(0.001f);
	float* heightNoiseSet = fastNoise->GetSimplexFractalSet(chunkPos.x * CHUNK_SIZE - 2, chunkPos.z * CHUNK_SIZE - 2, 0,
													  CHUNK_SIZE + 4, CHUNK_SIZE + 4, 1);
	fastNoise->SetFrequency(0.01f);
	float* dotsNoiseSet = fastNoise->GetSimplexFractalSet(chunkPos.x * CHUNK_SIZE - 2, chunkPos.z * CHUNK_SIZE - 2, 0,
													  CHUNK_SIZE + 4, CHUNK_SIZE + 4, 1);
	fastNoise->SetFrequency(1.0f);
	float* treeNoiseSet = fastNoise->GetWhiteNoiseSet(chunkPos.x * CHUNK_SIZE - 2, chunkPos.z * CHUNK_SIZE - 2, 0,
													   CHUNK_SIZE + 4, CHUNK_SIZE + 4, 1);
	int index=0;

	for(int i = -2; i <= CHUNK_SIZE + 1; ++i)
		for(int j = -2; j <= CHUNK_SIZE + 1; ++j, ++index)
		{
			const bool treeExist = (int) ((treeNoiseSet[index] + 1.0f) * 128.0f) == 0;

			const int treeHeight = (int) ((treeNoiseSet[index] + 1.0f) * 10000.0f) % 5 + 7;

			const int height = (int) std::round(heightNoiseSet[index] * SCALE) + seaLevel;
			if(i != -2 && i != CHUNK_SIZE + 1 && j != -2 && j != CHUNK_SIZE + 1)
			{
				for(int k = chunkBase - 1; k <= std::max(seaLevel, height) && k <= chunkBase + CHUNK_SIZE; ++k)
				{
					int arrayIndex = Chunk::XYZ(i + 1, k-chunkBase + 1, j + 1, CHUNK_SIZE + 2);
					if(k <= height)
					{
						if(dotsNoiseSet[index] < -0.4f && k == height)
						{
							if(k >= seaLevel)
								blk[arrayIndex] = Blocks::Dirt;
							else
								blk[arrayIndex] = Blocks::Sand;
						}
						else if(k == height && std::abs(k - seaLevel) <= 1)
							blk[arrayIndex] = Blocks::Sand;
						else if(k == height && k >= seaLevel)
						{
							if(treeExist)
								blk[arrayIndex] = Blocks::Dirt;
							else
								blk[arrayIndex] = Blocks::Grass;
						}
						else if(k == height && k < seaLevel)
							blk[arrayIndex] = Blocks::Dirt;
						else
							blk[arrayIndex] = Blocks::Stone;
					}
					else
						blk[arrayIndex] = Blocks::Water;
				}
				if(treeExist && height - 1 > seaLevel && height + treeHeight > chunkBase)
					for(int k=height + 1; k <= height + treeHeight && k <= chunkBase + CHUNK_SIZE; ++k)
					{
						if(k - chunkBase >= -1)
							getBlock(i, k - chunkBase, j) = Blocks::Wood;
					}
			}
			if(treeExist && height - 1 > seaLevel && height + treeHeight > chunkBase) {

				const int leavesHeight = (int) ((treeNoiseSet[index] + 1.0f) * 10000.0f) % 4 + 2;

				for (int k = height + treeHeight - leavesHeight + 1;
					 k <= height + treeHeight && k <= chunkBase + CHUNK_SIZE; ++k)
					for (int x = i - 2; x <= i + 2 && x <= CHUNK_SIZE; ++x)
						for (int y = j - 2; y <= j + 2 && y <= CHUNK_SIZE; ++y) {
							if (x == i && y == j)
								continue;

							if (x >= -1 && y >= -1 && k - chunkBase >= -1
								&& getBlock(x, k - chunkBase, y) == Blocks::Air) {
								getBlock(x, k - chunkBase, y) = Blocks::Leaves;
							}
						}
			}
		}

	FastNoiseSIMD::FreeNoiseSet(heightNoiseSet);
	FastNoiseSIMD::FreeNoiseSet(dotsNoiseSet);
	FastNoiseSIMD::FreeNoiseSet(treeNoiseSet);
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

	setTerrain(pos, std::ref(blk));

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

	auto MeshData = Chunk::GetMesh(pos, std::cref(blk));

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
			// chunk is meshed, then apply the mesh and clear the mesh array
			Renderer::ApplyChunkMesh(chk);

			chk->SolidMeshData.clear();
			chk->SolidMeshData.shrink_to_fit();

			chk->TransMeshData.clear();
			chk->TransMeshData.shrink_to_fit();
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

