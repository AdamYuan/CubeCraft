#pragma once
#include "MyGL/VertexObject.hpp"
#include "Util.hpp"
#include "Blocks.hpp"
#include "Settings.hpp"
#include <FastNoiseSIMD/FastNoiseSIMD.h>
#include <vector>
#include <memory>

namespace ChunkFuncs
{
	extern bool IsValidPos(int x, int y, int z);
	extern bool IsValidPos(const glm::ivec3 &pos);
	extern int XYZ(int x, int y, int z = 0, int digit = CHUNK_SIZE);
	extern int XYZ3(const glm::ivec3 &pos, int digit = CHUNK_SIZE);
	extern glm::ivec3 GetPosFromNum(int num);

	extern void SetTerrain(
			glm::ivec3 chunkPos,
			block (&blk)[(CHUNK_SIZE + 2) * (CHUNK_SIZE + 2) * (CHUNK_SIZE + 2)],
			unsigned long seed
	);
	extern std::pair<std::vector<vert_block>, std::vector<vert_block>> GetMesh(
			glm::ivec3 chunkPos,
			const block (&blk)[(CHUNK_SIZE+2) * (CHUNK_SIZE+2) * (CHUNK_SIZE+2)]
	);
}

class SuperChunk;
class Chunk
{
private:
	light_t lightMap[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE];
	bool beCovered[CHUNK_SIZE*CHUNK_SIZE];
	//bool updatedNeighbourMeshing[6];
	SuperChunk *parent;
public:
	block blk[(CHUNK_SIZE + 2)*(CHUNK_SIZE + 2)*(CHUNK_SIZE + 2)];

	std::vector<vert_block> SolidMeshData;
	std::vector<vert_block> SemitransMeshData;
	glm::ivec3 ChunkPos;
	std::string ChunkLabel;

	Chunk() = default;
	Chunk(SuperChunk *_parent,glm::ivec3 _chunkPos,std::string _chunkLabel);
	~Chunk();

	void SetBlock(const glm::ivec3 &pos, const block &b);
	void SetBlock(int x, int y, int z, const block &b);
	block GetBlock(const glm::ivec3 &pos);
	block GetBlock(int x, int y, int z);

	void SetLight(const glm::ivec3 &pos, light_t v);
	void SetLight(int x, int y, int z, light_t v);
	light_t GetLight(const glm::ivec3 &pos);
	light_t GetLight(int x, int y, int z);

	void UpdateLighting();

	bool UpdatedMesh=false;
	bool UpdatedLight=false;

	std::unique_ptr<MyGL::VertexObject> SolidMeshObject;
	std::unique_ptr<MyGL::VertexObject> SemitransMeshObject;
};
