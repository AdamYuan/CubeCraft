#include <GL/glew.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <memory>
#include <iterator>
#include <queue>
#include "Chunk.hpp"
#include "SuperChunk.hpp"
#include "Resource.hpp"
Chunk::Chunk(SuperChunk *_parent,glm::ivec3 _chunkPos,std::string _chunkLabel)
		:parent(_parent),ChunkPos(_chunkPos),ChunkLabel(_chunkLabel)
{
	std::uninitialized_fill(std::begin(beCovered), std::end(beCovered), false);
	std::uninitialized_fill(std::begin(blk), std::end(blk), Blocks::Air);
	std::uninitialized_fill(std::begin(lightMap), std::end(lightMap), 0);
	for(int i=0; i<6; ++i)
		updatedNeighbourMeshing[i] = true;
}
Chunk::~Chunk()
{
	//blk.close();
}
bool Chunk::IsValidPos(int x, int y, int z)
{
	return x>=0 && x<CHUNK_SIZE && y>=0 && y<CHUNK_SIZE && z>=0 && z<CHUNK_SIZE;
}
bool Chunk::IsValidPos(const glm::ivec3 &pos)
{
	return IsValidPos(pos.x, pos.y, pos.z);
}
int Chunk::XYZ(int x, int y, int z = 0)
{
	return x+CHUNK_SIZE*(y+CHUNK_SIZE*z);
}
int Chunk::XYZ3(const glm::ivec3 &pos)
{
	return XYZ(pos.x, pos.y, pos.z);
}
glm::ivec3 Chunk::GetPosFromNum(int num)
{
	glm::ivec3 pos;
	pos.z = num / (CHUNK_SIZE*CHUNK_SIZE);
	num %= CHUNK_SIZE*CHUNK_SIZE;
	pos.y = num / CHUNK_SIZE;
	num %= CHUNK_SIZE;
	pos.x = num;
	return pos;
}
block Chunk::GetBlock(int x, int y, int z)
{
	if(x < 0 || x >= CHUNK_SIZE ||
	   y < 0 || y >= CHUNK_SIZE ||
	   z < 0 || z >= CHUNK_SIZE)
		return parent->GetBlock(ChunkPos.x * CHUNK_SIZE + x, ChunkPos.y * CHUNK_SIZE + y, ChunkPos.z * CHUNK_SIZE + z);
	return (block)blk[XYZ(x, y, z)];
}
block Chunk::GetBlock(const glm::ivec3 &pos)
{
	return GetBlock(pos.x, pos.y, pos.z);
}
void Chunk::SetBlock(int x, int y, int z, const block &b)
{
	if(x < 0 || x >= CHUNK_SIZE ||
	   y < 0 || y >= CHUNK_SIZE ||
	   z < 0 || z >= CHUNK_SIZE)
		return;

	if(x == 0)
		updatedNeighbourMeshing[LEFT] = false;
	if(x == CHUNK_SIZE - 1)
		updatedNeighbourMeshing[RIGHT] = false;
	if(y == 0)
		updatedNeighbourMeshing[BOTTOM] = false;
	if(y == CHUNK_SIZE - 1)
		updatedNeighbourMeshing[TOP] = false;
	if(z == 0)
		updatedNeighbourMeshing[BACK] = false;
	if(z == CHUNK_SIZE - 1)
		updatedNeighbourMeshing[FRONT] = false;

	blk[XYZ(x, y, z)]=b;
	if(b!=Blocks::Air)
		beCovered[XYZ(x, z)]=true;
	UpdatedMesh=false;
	UpdatedLight=false;
}
void Chunk::SetBlock(const glm::ivec3 &pos, const block &b)
{
	SetBlock(pos.x, pos.y, pos.z, b);
}

void Chunk::SetLight(int x, int y, int z, light_t v)
{
	if(!IsValidPos(x, y, z))
	{
		parent->SetLight(ChunkPos.x * CHUNK_SIZE + x, ChunkPos.y * CHUNK_SIZE + y, ChunkPos.z * CHUNK_SIZE + z, v);
		return;
	}
	lightMap[XYZ(x, y, z)]=v;
	UpdatedMesh=false;
}
void Chunk::SetLight(const glm::ivec3 &pos, light_t v)
{
	SetLight(pos.x, pos.y, pos.z, v);
}
light_t Chunk::GetLight(int x, int y, int z)
{
	if(!IsValidPos(x, y, z))
		return parent->GetLight(ChunkPos.x * CHUNK_SIZE + x, ChunkPos.y * CHUNK_SIZE + y, ChunkPos.z * CHUNK_SIZE + z);
	return lightMap[XYZ(x, y, z)];
}
light_t Chunk::GetLight(const glm::ivec3 &pos)
{
	return GetLight(pos.x, pos.y, pos.z);
}

struct lightNode
{
	glm::ivec3 position;
	light_t light_value;
};
void Chunk::UpdateLighting()
{
	if(UpdatedLight)
		return;

	std::queue <lightNode> sunLightBfsQueue;

	bool beCovered[CHUNK_SIZE][CHUNK_SIZE];

	std::uninitialized_fill(&beCovered[0][0], &beCovered[CHUNK_SIZE-1][CHUNK_SIZE-1]+1, false);

	ChunkPtr up_chk= parent->GetChunk(ChunkPos.x, ChunkPos.y + 1, ChunkPos.z);
	if(up_chk)
	{
		for(int x=0;x<CHUNK_SIZE;++x)
			for(int y=0;y<CHUNK_SIZE;++y)
				beCovered[x][y]=up_chk->beCovered[XYZ(x, y)];
	}

	for(int y=CHUNK_SIZE-1; y>=0; --y)
		for(int x=0; x<CHUNK_SIZE; ++x)
			for(int z=0; z<CHUNK_SIZE; ++z)
			{
				if(GetBlock(x, y, z) != Blocks::Air)
					beCovered[x][z]=true;

				if(!beCovered[x][z] && GetBlock(x, y, z) == Blocks::Air)
				{
					sunLightBfsQueue.push({{x, y, z}, 15});//add to Light queue
					SetLight(x, y, z, 15);//sun Light
				}
			}

	while(!sunLightBfsQueue.empty())
	{
		const lightNode &node = sunLightBfsQueue.front();
		int x=node.position.x, y=node.position.y, z=node.position.z;
		light_t lightValue=node.light_value;
		sunLightBfsQueue.pop();

		for(short face=0; face<6; ++face)
		{
			glm::ivec3 np = glm::ivec3(x,y,z) + Funcs::GetFaceDirect(face);//new Position

			if (BlockMethods::IsTransparent(GetBlock(np)) && GetLight(np) + 2 <= lightValue && lightValue > 0)
			{
				sunLightBfsQueue.push({np, (light_t)(lightValue - 1)});
				SetLight(np, (light_t) (lightValue - 1));
			}
		}
	}
	UpdatedLight=true;
}

void Chunk::UpdateMeshing()
{
	if(UpdatedMesh)
		return;

	MeshData.clear();

	for(short i=0; i<6; ++i)
	{
		ChunkPtr nei = parent->GetChunk(ChunkPos + Funcs::GetFaceDirect(i));
		if(nei && !updatedNeighbourMeshing[i])
			nei->UpdatedMesh = false;
		updatedNeighbourMeshing[i] = true;
	}

	static auto vertexAO=[&](block side1,block corner,block side2)->int
	{
		bool b1=static_cast<bool>(side1),bc=static_cast<bool>(corner),b2=static_cast<bool>(side2);
		if(b1&&b2)
			return 0;
		return 3-b1-bc-b2;
	};

	static const int lookup3[6][4][3]=
			{
					21,18,19,21,24,25,23,26,25,23,20,19,
					3,0,1,5,2,1,5,8,7,3,6,7,
					15,6,7,17,8,7,17,26,25,15,24,25,
					9,0,1,9,18,19,11,20,19,11,2,1,
					11,2,5,11,20,23,17,26,23,17,8,5,
					9,0,3,15,6,3,15,24,21,9,18,21
			};
	static FaceLighting block_lightings[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE][6];
	static block neighbours[27];
	static light_t neighbours_li[27];

	//calculate the lighting data of necessary face vertices
	glm::ivec3 posi;
	for(int iterator = 0; iterator < CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE; ++iterator)
	{
		posi = GetPosFromNum(iterator);
		if(GetBlock(posi)==Blocks::Air)
			continue;
		int index=0;
		glm::ivec3 n;
		bool first_face=false;
		for(short face=0; face<6; ++face)
		{
			block bn = GetBlock(posi), bf = GetBlock(posi + Funcs::GetFaceDirect(face));
			bool trans_n= BlockMethods::IsTransparent(bn), trans_f= BlockMethods::IsTransparent(bf);
			if(!trans_n && !trans_f)
				continue;
			if(trans_n && bf!=Blocks::Air)
				continue;

			if(!first_face)//calculate the neighbours only for once
			{
				for(n.x=-1; n.x<=1; ++n.x)
					for(n.y=-1; n.y<=1; ++n.y)
						for(n.z=-1; n.z<=1; ++n.z, ++index)
						{
							neighbours[index]= GetBlock(posi + n);
							neighbours_li[index]= GetLight(posi + n);
						}
				first_face=true;
			}

			for(int vertex=0; vertex<4; ++vertex)
			{
				block_lightings[Chunk::XYZ3(posi)][face].AO[vertex]=
						vertexAO(neighbours[lookup3[face][vertex][0]],
								 neighbours[lookup3[face][vertex][1]],
								 neighbours[lookup3[face][vertex][2]]);

				light_t center_l = GetLight(posi + Funcs::GetFaceDirect(face));

				//smooth the Light using the average value
				int sum=1, sum_li=center_l;
				bool isSolid[3];
				for(int nn=0; nn<3; ++nn)
					isSolid[nn] = !BlockMethods::IsTransparent(neighbours[lookup3[face][vertex][nn]]);

				if(!isSolid[0] || !isSolid[2])
					for(int nn=0; nn<3; ++nn)
					{
						if(isSolid[nn])
							continue;
						sum++;
						sum_li+=neighbours_li[lookup3[face][vertex][nn]];
					}
				block_lightings[Chunk::XYZ3(posi)][face].Light[vertex]= (light_t) (sum_li / sum);
			}
		}
	}

	//greedy meshing
	for (std::size_t axis = 0; axis < 3; ++axis)
	{
		const std::size_t u = (axis + 1) % 3;
		const std::size_t v = (axis + 2) % 3;

		int x[3] = {0}, q[3] = {0}, mask[CHUNK_SIZE*CHUNK_SIZE];
		FaceLighting lightMask[CHUNK_SIZE*CHUNK_SIZE];
		// Compute mask
		q[axis] = 1;
		ChunkPtr neighbourChunk0 = parent->GetChunk(ChunkPos - glm::ivec3(q[0], q[1], q[2])),
				neighbourChunk1 = parent->GetChunk(ChunkPos + glm::ivec3(q[0], q[1], q[2]));
		bool neighbourChunkExist0 = neighbourChunk0 != nullptr;
		if(neighbourChunkExist0)
			neighbourChunkExist0 = neighbourChunk0->LoadedTerrain;
		bool neighbourChunkExist1 = neighbourChunk1 != nullptr;
		if(neighbourChunkExist1)
			neighbourChunkExist1 = neighbourChunk1->LoadedTerrain;
		for (x[axis] = -1; x[axis] < CHUNK_SIZE;)
		{
			std::size_t counter = 0;
			for (x[v] = 0; x[v] < CHUNK_SIZE; ++x[v])
				for (x[u] = 0; x[u] < CHUNK_SIZE; ++x[u], ++counter)
				{
					const block a = /*(0 <= x[axis]) ?*/
							GetBlock(x[0], x[1], x[2])
					/*: 0*/;
					const block b = /*(x[axis] < CHUNK_SIZE - 1) ?*/
							GetBlock(x[0] + q[0], x[1] + q[1], x[2] + q[2])
					/*: 0*/;

					const bool canput_a=(0 <= x[axis]);
					const bool canput_b=(x[axis] < CHUNK_SIZE-1);

					const bool trans_a = BlockMethods::IsTransparent(a);
					const bool trans_b = BlockMethods::IsTransparent(b);

					const int index_a=x[0]+CHUNK_SIZE*(x[1]+CHUNK_SIZE*x[2]);
					const int index_b=(x[0]+q[0])+CHUNK_SIZE*((x[1]+q[1])+CHUNK_SIZE*(x[2]+q[2]));

					const FaceLighting li_a=(0<=x[axis])?
											block_lightings[index_a][axis*2]
														:FaceLighting();
					const FaceLighting li_b=(x[axis] < CHUNK_SIZE - 1)?
											block_lightings[index_b][axis*2+1]
																	  :FaceLighting();
					if((x[axis] == -1 && !neighbourChunkExist0) || (x[axis] + 1 == CHUNK_SIZE && !neighbourChunkExist1))
					{
						mask[counter] = 0;
						lightMask[counter] = FaceLighting();
					}
					else if(!trans_a && trans_b && canput_a)//soild block surface
					{
						mask[counter] = a;
						lightMask[counter] = li_a;
					}
					else if(!trans_b && trans_a && canput_b)
					{
						mask[counter] = -b;
						lightMask[counter] = li_b;
					}
					else if(trans_a && !b && canput_a)//transparent block surface
					{
						mask[counter] = a;
						lightMask[counter] = li_a;
					}
					else if(trans_b && !a && canput_b)
					{
						mask[counter] = -b;
						lightMask[counter] = li_b;
					}
					else
					{
						mask[counter] = 0;
						lightMask[counter] = FaceLighting();
					}
				}

			++x[axis];

			// Generate mesh for mask using lexicographic ordering
			std::size_t width = 0, height = 0;

			counter = 0;
			for (std::size_t j = 0; j < CHUNK_SIZE; ++j)
				for (std::size_t i = 0; i < CHUNK_SIZE;)
				{
					int c = mask[counter];
					FaceLighting fli = lightMask[counter];
					if (c)
					{
						// Compute Width
						for (width = 1; c == mask[counter + width] && fli == lightMask[counter + width] &&
										i + width < CHUNK_SIZE; ++width);

						// Compute Height
						bool done = false;
						for (height = 1; j + height < CHUNK_SIZE; ++height)
						{
							for (std::size_t k = 0; k < width; ++k)
								if (c != mask[counter + k + height * CHUNK_SIZE] ||
									fli != lightMask[counter + k + height * CHUNK_SIZE])
								{
									done = true;
									break;
								}

							if (done)
								break;
						}

						// Add quad
						x[u] = (int) i;
						x[v] = (int) j;

						float du[3] = {0}, dv[3] = {0};

						short f= (short) (axis * 2 + (c <= 0));


						if (c > 0)
						{
							dv[v] = height+TJUNC_DELTA*2.0f;
							du[u] = width+TJUNC_DELTA*2.0f;
						}
						else
						{
							c = -c;
							du[v] = height+TJUNC_DELTA*2.0f;
							dv[u] = width+TJUNC_DELTA*2.0f;
						}

						int tex= BlockMethods::GetTexture((block) c, f);
						float vx=ChunkPos.x*CHUNK_SIZE+x[0]-(!q[0])*TJUNC_DELTA,
								vy=ChunkPos.y*CHUNK_SIZE+x[1]-(!q[1])*TJUNC_DELTA,
								vz=ChunkPos.z*CHUNK_SIZE+x[2]-(!q[2])*TJUNC_DELTA;
						vert_block v00={vx, vy, vz,
										(float)tex,(float)f,(float)fli.AO[0],(float)fli.Light[0]};
						vert_block v01={vx + du[0], vy + du[1], vz + du[2],
										(float)tex,(float)f,(float)fli.AO[1],(float)fli.Light[1]};
						vert_block v10={vx + du[0] + dv[0], vy + du[1] + dv[1], vz + du[2] + dv[2],
										(float)tex,(float)f,(float)fli.AO[2],(float)fli.Light[2]};
						vert_block v11={vx + dv[0], vy + dv[1], vz + dv[2],
										(float)tex,(float)f,(float)fli.AO[3],(float)fli.Light[3]};

						/*bool flip=(fli.AO[0]==fli.AO[1] && fli.AO[1]==fli.AO[2] && fli.AO[2]==fli.AO[3])?
								  (fli.Light[0]+fli.Light[2] > fli.Light[1]+fli.Light[3]):
								  (fli.AO[0]+fli.AO[2] > fli.AO[1]+fli.AO[3]);*/
						bool flip=(fli.AO[0]+fli.AO[2] > fli.AO[1]+fli.AO[3]);

						//bool flip=(fli.AO[0]+fli.Light[0]+fli.AO[2]+fli.Light[2] >
						//		   fli.AO[1]+fli.Light[1]+fli.AO[3]+fli.Light[3]);
						if(flip)
						{
							//11--------10
							//|       / |
							//|    /    |
							//| /       |
							//00--------01
							MeshData.push_back(v00);
							MeshData.push_back(v01);
							MeshData.push_back(v10);

							MeshData.push_back(v00);
							MeshData.push_back(v10);
							MeshData.push_back(v11);
						}
						else
						{
							//11--------10
							//| \       |
							//|    \    |
							//|       \ |
							//00--------01
							MeshData.push_back(v01);
							MeshData.push_back(v10);
							MeshData.push_back(v11);

							MeshData.push_back(v00);
							MeshData.push_back(v01);
							MeshData.push_back(v11);
						}

						for (std::size_t b = 0; b < width; ++b)
							for (std::size_t a = 0; a < height; ++a)
								mask[counter + b + a * CHUNK_SIZE] = 0;

						// Increment counters
						i += width; counter += width;
					}
					else
					{
						++i;
						++counter;
					}
				}
		}
	}
	UpdatedMesh=true;
}
void Chunk::UpdateAll()
{
	UpdateLighting();
	UpdateMeshing();
}
