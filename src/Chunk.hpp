#pragma once
#include "MyGL/VertexObject.hpp"
#include "Util.hpp"
#include "Blocks.hpp"
#include "Settings.hpp"
#include <vector>
#include <memory>

class SuperChunk;
class Chunk
{
private:
	block blk[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE];
	light_t lightMap[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE];
	bool beCoverd[CHUNK_SIZE][CHUNK_SIZE];
	bool updatedNeighbourMeshing[6];
	bool updatedNeighbourLighting[6];
	SuperChunk *parent;
public:
	std::vector<vert_block> MeshData;
	glm::ivec3 ChunkPos;
	std::string ChunkLabel;
	bool LoadedTerrain = false;

	Chunk(SuperChunk *_parent,glm::ivec3 _chunkPos,std::string _chunkLabel);
	~Chunk();
	static bool IsValidPos(int x, int y, int z);
	static bool IsValidPos(const glm::ivec3 &pos);
	static int GetNumFromPos(int x, int y, int z);
	static int GetNumFromPos(const glm::ivec3 &pos);
	static glm::ivec3 GetPosFromNum(int num);

	void SetBlock(const glm::ivec3 &pos, const block &b);
	void SetBlock(int x, int y, int z, const block &b);
	block GetBlock(const glm::ivec3 &pos);
	block GetBlock(int x, int y, int z);

	void SetLight(const glm::ivec3 &pos, light_t v);
	void SetLight(int x, int y, int z, light_t v);
	light_t GetLight(const glm::ivec3 &pos);
	light_t GetLight(int x, int y, int z);

	void UpdateLighting();
	void UpdateMeshing();
	void UpdateAll();

	bool UpdatedMesh=false;
	bool UpdatedLight=false;

	std::unique_ptr<MyGL::VertexObject> MeshObject = std::unique_ptr<MyGL::VertexObject>(new MyGL::VertexObject());
};
