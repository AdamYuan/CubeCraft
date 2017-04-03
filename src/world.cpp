#include "world.hpp"
#include "game.hpp"
#include "opengl/matrix.hpp"
#include "opengl/frustum.hpp"
#include "opengl/camera.hpp"
#include "renderer.hpp"
#include <cmath>
#include <chrono>
#include <algorithm>
void world::bgWork()
{
	bgMtx.lock();

	voxels.getChunk(chunkMeshingList.front())->updateAll();
	chunkInMeshingList.erase(chunkMeshingList.front());
	chunkMeshingList.erase(chunkMeshingList.begin());

	bgMtx.unlock();
}
void world::updateChunkLists()
{
	if(!bgMtx.try_lock())
		return;

	chunkRenderList.clear();
	//chunkLoadingList.clear();
	//chunkMeshingList.clear();
	for(auto i=voxels.chunks.begin();i!=voxels.chunks.end();)
	{
		chunkPtr &chk=i->second;
		glm::ivec3 pos=i->first;

		if(!chk)//delete if the chunk is null
		{
			i = voxels.chunks.erase(i);
			continue;
		}

		if(!chk->updatedMesh)
		{
			if(!chunkInMeshingList[pos])
			{
				chunkMeshingList.push_back(pos);
				chunkInMeshingList[pos]=true;
			}
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
	std::sort(chunkMeshingList.begin(), chunkMeshingList.end(),
			  [=](const glm::ivec3 &a, const glm::ivec3 &b)->bool
			  {
                  glm::vec3 v1 = game::gamePlayer.chunkPos - a;
				  glm::vec3 v2 = game::gamePlayer.chunkPos - b;
				  return glm::length(v1) < glm::length(v2);
			  });
	bgMtx.unlock();
	if(!chunkMeshingList.empty()) {
		std::thread thr(&world::bgWork, this);
		thr.detach();
	}
}
