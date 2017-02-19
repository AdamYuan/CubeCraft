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
	if(!chunkMeshingList.empty())
	{
		voxels.getChunk(chunkMeshingList.front())->updateAll();
		chunkInMeshingList.erase(chunkMeshingList.front());
		chunkMeshingList.erase(chunkMeshingList.begin());
	}
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
		chunk *&chk=i->second;
		glm::ivec3 pos=i->first;

		if(!chk)//delete if the chunk is null
		{
			i=voxels.chunks.erase(i);
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

		glm::vec3 center=(glm::vec3)pos*(float)CHUNK_SIZE+glm::vec3(CHUNK_SIZE/2);

		//Cull far away chunks
		if(glm::distance(camera::position,center) > VIEW_DISTANCE+CHUNK_SIZE)
		{
			++i;
			continue;
		}

		chunkRenderList.push_back(pos);

		++i;
	}
	std::sort(chunkMeshingList.begin(),chunkMeshingList.end(),
			  [=](glm::ivec3 a, glm::ivec3 b)->bool
			  {
				  return glm::distance((glm::vec3)a,
									   (glm::vec3)game::gamePlayer.chunkPos) <
						 glm::distance((glm::vec3)b,
									   (glm::vec3)game::gamePlayer.chunkPos);
			  });
	bgMtx.unlock();

	std::thread _thread(&world::bgWork, this);
	_thread.detach();
}
