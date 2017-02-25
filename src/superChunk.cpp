#include "superChunk.hpp"
#include "renderer.hpp"
#include "game.hpp"
#include "opengl/camera.hpp"
#include <iostream>
#include <algorithm>
#include <functional>

void superChunk::bgWork()
{
	bgMtx.lock();
	getChunk(chunkMeshingList.front())->updateAll();
	chunkInMeshingList.erase(chunkMeshingList.front());
	chunkMeshingList.erase(chunkMeshingList.begin());
	bgMtx.unlock();
}
void superChunk::updateChunkLists()
{
	if(!bgMtx.try_lock())
		return;

	chunkRenderList.clear();
	//chunkLoadingList.clear();
	//chunkMeshingList.clear();
	for(auto i=chunks.begin();i!=chunks.end();)
	{
		chunk *&chk=i->second;
		glm::ivec3 pos=i->first;

		if(!chk)//delete if the chunk is null
		{
			i=chunks.erase(i);
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
			  [=](const glm::ivec3 &a, const glm::ivec3 &b)->bool
			  {
				  return glm::distance((glm::vec3)a,
									   (glm::vec3)game::gamePlayer.chunkPos) <
						 glm::distance((glm::vec3)b,
									   (glm::vec3)game::gamePlayer.chunkPos);
			  });
	bgMtx.unlock();

	if(!chunkMeshingList.empty())
	{
		std::thread _thread(&superChunk::bgWork, this);
		_thread.detach();
	}
}

glm::ivec3 superChunk::getChunkPos(int x,int y,int z)
{
	return glm::ivec3(
		(x+(x<0))/CHUNK_SIZE-(x<0),
		(y+(y<0))/CHUNK_SIZE-(y<0),
		(z+(z<0))/CHUNK_SIZE-(z<0));
}
glm::ivec3 superChunk::getChunkPos(const glm::ivec3 &pos)
{
	return getChunkPos(pos.x,pos.y,pos.z);
}

void superChunk::setChunk(const glm::ivec3 &chunkPos)
{
	std::hash<glm::ivec3> h;
	size_t hash_t = h(chunkPos);
	chunks[chunkPos]=new chunk(this,chunkPos,std::to_string(hash_t));
}
void superChunk::setChunk(int x,int y,int z)
{
	setChunk(glm::ivec3(x,y,z));
}

chunk *superChunk::getChunk(const glm::ivec3 &chunkPos)
{
	return chunks[chunkPos];
}
chunk *superChunk::getChunk(int x,int y,int z)
{
	return getChunk(glm::ivec3(x,y,z));
}

void superChunk::eraseChunk(const glm::ivec3 &chunkPos)
{
	delete chunks[chunkPos];
	chunks.erase(chunkPos);
}
void superChunk::eraseChunk(int x,int y,int z)
{
	eraseChunk(glm::ivec3(x,y,z));
}

void superChunk::setBlock(const glm::ivec3 &pos, const block &blk)
{
	glm::ivec3 chunkPos=getChunkPos(pos);
	if(chunks[chunkPos]==nullptr)
		return;
	getChunk(chunkPos)->set(pos-(chunkPos*CHUNK_SIZE), blk);
}
void superChunk::setBlock(int x,int y,int z,const block &blk)
{
	setBlock(glm::ivec3(x,y,z),blk);
}

block superChunk::getBlock(const glm::ivec3 &pos)
{
	glm::ivec3 chunkPos=getChunkPos(pos);
	if(!chunks[chunkPos])
		return blocks::air;
	return getChunk(chunkPos)->get(pos-(chunkPos*CHUNK_SIZE));
}
block superChunk::getBlock(int x,int y,int z)
{
	return getBlock(glm::ivec3(x,y,z));
}

void superChunk::setLight(const glm::ivec3 &pos, light_t v)
{
	glm::ivec3 chunkPos=getChunkPos(pos);
	if(chunks[chunkPos]==nullptr)
		return;
	getChunk(chunkPos)->setl(pos-(chunkPos*CHUNK_SIZE), v);
}
void superChunk::setLight(int x,int y,int z,light_t v)
{
	setLight(glm::ivec3(x,y,z),v);
}

light_t superChunk::getLight(const glm::ivec3 &pos)
{
	glm::ivec3 chunkPos=getChunkPos(pos);
	if(!chunks[chunkPos])
		return 15;
	return getChunk(chunkPos)->getl(pos-(chunkPos*CHUNK_SIZE));
}
light_t superChunk::getLight(int x,int y,int z)
{
	return getLight(glm::ivec3(x,y,z));
}
