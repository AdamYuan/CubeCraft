#pragma once
#include "opengl/object.hpp"
#include "types.hpp"
#include "blocks.hpp"
#include "settings.hpp"
#include <vector>
class superChunk;
class chunk
{
  public:
	std::vector<vert_block> meshData;
	block blk[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE];
	light_t lightMap[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE];
	bool beCoverd[CHUNK_SIZE][CHUNK_SIZE];
	superChunk *parent;
	glm::ivec3 chunkPos;
	std::string chunkLabel;

	chunk(superChunk *_parent,glm::ivec3 _chunkPos,std::string _chunkLabel);
	~chunk();
	static bool isValidPos(int x,int y,int z);
	static bool isValidPos(const glm::ivec3 &pos);
	static int getPosNum(int x,int y,int z);
	static int getPosNum(const glm::ivec3 &pos);

	void set(const glm::ivec3 &pos,const block &b);
	void set(int x,int y,int z,const block &b);
	block get(const glm::ivec3 &pos);
	block get(int x,int y,int z);

	void setl(const glm::ivec3 &pos,light_t v);
	void setl(int x,int y,int z,light_t v);
	light_t getl(const glm::ivec3 &pos);
	light_t getl(int x,int y,int z);

	void updateLighting();
	void updateMeshing();
	void updateAll();

	bool updatedMesh=false;
	bool updatedLight=false;

	object obj;
};
