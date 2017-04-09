#include <GL/glew.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <memory>
#include <iterator>
#include <queue>
#include "chunk.hpp"
#include "superChunk.hpp"
#include "resource.hpp"
using namespace std;
chunk::chunk(superChunk *_parent,glm::ivec3 _chunkPos,std::string _chunkLabel)
		:parent(_parent),chunkPos(_chunkPos),chunkLabel(_chunkLabel)
{
	std::uninitialized_fill(&beCoverd[0][0], &beCoverd[CHUNK_SIZE-1][CHUNK_SIZE-1]+1, false);
	std::uninitialized_fill(begin(blk), end(blk), blocks::air);
	std::uninitialized_fill(begin(lightMap), end(lightMap), 0);
	for(int i=0; i<6; ++i)
	{
		updatedNeighbourLighting[i] = true;
		updatedNeighbourMeshing[i] = true;
	}
}
chunk::~chunk()
{
	//blk.close();
}
bool chunk::isValidPos(int x, int y, int z)
{
	return x>=0 && x<CHUNK_SIZE && y>=0 && y<CHUNK_SIZE && z>=0 && z<CHUNK_SIZE;
}
bool chunk::isValidPos(const glm::ivec3 &pos)
{
	return isValidPos(pos.x, pos.y, pos.z);
}
int chunk::getNumFromPos(int x, int y, int z)
{
	return x+CHUNK_SIZE*(y+CHUNK_SIZE*z);
}
int chunk::getNumFromPos(const glm::ivec3 &pos)
{
	return getNumFromPos(pos.x, pos.y, pos.z);
}
glm::ivec3 chunk::getPosFromNum(int num)
{
	glm::ivec3 pos;
	pos.z = num / (CHUNK_SIZE*CHUNK_SIZE);
	num %= CHUNK_SIZE*CHUNK_SIZE;
	pos.y = num / CHUNK_SIZE;
	num %= CHUNK_SIZE;
	pos.x = num;
	return pos;
}
block chunk::get(int x,int y,int z)
{
	if(x < 0 || x >= CHUNK_SIZE ||
	   y < 0 || y >= CHUNK_SIZE ||
	   z < 0 || z >= CHUNK_SIZE)
		return parent->getBlock(chunkPos.x*CHUNK_SIZE+x, chunkPos.y*CHUNK_SIZE+y, chunkPos.z*CHUNK_SIZE+z);
	return (block)blk[getNumFromPos(x, y, z)];
}
block chunk::get(const glm::ivec3 &pos)
{
	return get(pos.x,pos.y,pos.z);
}
void chunk::set(int x, int y, int z, const block &b)
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

	blk[getNumFromPos(x, y, z)]=b;
	if(b!=blocks::air)
		beCoverd[x][z]=true;
	updatedMesh=false;
	updatedLight=false;
}
void chunk::set(const glm::ivec3 &pos,const block &b)
{
	set(pos.x,pos.y,pos.z,b);
}

void chunk::setl(int x,int y,int z,light_t v)
{
	if(!isValidPos(x, y, z))
	{
		parent->setLight(chunkPos.x*CHUNK_SIZE+x,chunkPos.y*CHUNK_SIZE+y,chunkPos.z*CHUNK_SIZE+z,v);
		return;
	}
	lightMap[getNumFromPos(x, y, z)]=v;
	updatedMesh=false;
}
void chunk::setl(const glm::ivec3 &pos,light_t v)
{
	setl(pos.x,pos.y,pos.z,v);
}
light_t chunk::getl(int x, int y, int z)
{
	if(!isValidPos(x, y, z))
		return parent->getLight(chunkPos.x*CHUNK_SIZE+x,chunkPos.y*CHUNK_SIZE+y,chunkPos.z*CHUNK_SIZE+z);
	return lightMap[getNumFromPos(x, y, z)];
}
light_t chunk::getl(const glm::ivec3 &pos)
{
	return getl(pos.x, pos.y, pos.z);
}

struct lightNode
{
	glm::ivec3 position;
	light_t light_value;
};
void chunk::updateLighting()
{
	if(updatedLight)
		return;

	std::queue <lightNode> sunLightBfsQueue;

	bool be_coverd[CHUNK_SIZE][CHUNK_SIZE];

	std::uninitialized_fill(&be_coverd[0][0], &be_coverd[CHUNK_SIZE-1][CHUNK_SIZE-1]+1, false);

	for(int i=1;;++i)
	{
		chunkPtr up_chk=parent->getChunk(chunkPos.x, chunkPos.y+i, chunkPos.z);
		if(!up_chk)
			break;
		for(int x=0;x<CHUNK_SIZE;++x)
			for(int y=0;y<CHUNK_SIZE;++y)
				if(!be_coverd[x][y] && up_chk->beCoverd[x][y])
					be_coverd[x][y]=true;
	}

	for(int y=CHUNK_SIZE-1;y>=0;--y)
		for(int x=0;x<CHUNK_SIZE;++x)
			for(int z=0;z<CHUNK_SIZE;++z)
			{
				if(!be_coverd[x][z] && get(x, y, z) != blocks::air)
					be_coverd[x][z]=true;
				bool n0 = x - 1 < 0 ? false : be_coverd[x-1][z];
				bool n1 = x + 1 >= CHUNK_SIZE ? false : be_coverd[x+1][z];
				bool n2 = z - 1 < 0 ? false : be_coverd[x][z-1];
				bool n3 = z + 1 >= CHUNK_SIZE ? false : be_coverd[x][z+1];
				if(!be_coverd[x][z] && (y-1 < 0 || (n0 || n1 || n2 || n3) ||
						!block_m::isTransparent(get(x, y-1, z))))
				{
					sunLightBfsQueue.push({{x,y,z}, 15});//add to light queue
					setl(x, y, z, 15);//sun light
				}
			}

	while(!sunLightBfsQueue.empty())
	{
		lightNode &node = sunLightBfsQueue.front();
		int x=node.position.x, y=node.position.y, z=node.position.z;
		light_t lightValue=node.light_value;
		sunLightBfsQueue.pop();

		if(lightValue<=0)
			continue;

		for(short face=0;face<6;++face)
		{
			glm::ivec3 np=glm::ivec3(x,y,z)+funcs::getFaceDirect(face);//new position

			if (block_m::isTransparent(get(np)) && getl(np) + 2 <= lightValue)
			{
				sunLightBfsQueue.push({np, (light_t)(lightValue-1)});
				setl(np, (light_t) (lightValue - 1));
			}
		}
	}
	updatedLight=true;
}

void chunk::updateMeshing()
{
	if(updatedMesh)
		return;

	meshData.clear();

	for(short i=0; i<6; ++i)
	{
		chunkPtr nei = parent->getChunk(chunkPos + funcs::getFaceDirect(i));
		if(nei && !updatedNeighbourMeshing[i])
			nei->updatedMesh = false;
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
	static face_lighting block_lightings[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE][6];
	static block neighboors[27];
	static light_t neighboors_li[27];

	//calculate the lighting data of necessary face vertices
	glm::ivec3 posi;
	for(int iterator = 0; iterator < CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE; ++iterator)
	{
		posi = getPosFromNum(iterator);
		if(get(posi)==blocks::air)
			continue;
		int index=0;
		glm::ivec3 n;
		bool first_face=false;
		for(short face=0;face<6;++face)
		{
			block bn=get(posi),bf=get(posi+funcs::getFaceDirect(face));
			bool trans_n=block_m::isTransparent(bn),trans_f=block_m::isTransparent(bf);
			if(!trans_n && !trans_f)
				continue;
			if(trans_n && bf!=blocks::air)
				continue;

			if(!first_face)//calculate the neighboors only for once
			{
				for(n.x=-1;n.x<=1;++n.x)
					for(n.y=-1;n.y<=1;++n.y)
						for(n.z=-1;n.z<=1;++n.z,++index)
						{
							neighboors[index]=get(posi+n);
							neighboors_li[index]=getl(posi+n);
						}
				first_face=true;
			}

			for(int v=0;v<4;++v)
			{
				block_lightings[chunk::getNumFromPos(posi)][face].ao[v]=
						vertexAO(neighboors[lookup3[face][v][0]],
								 neighboors[lookup3[face][v][1]],
								 neighboors[lookup3[face][v][2]]);

				light_t center_l=getl(posi+funcs::getFaceDirect(face));

				//smooth the light using the average value
				int sum=1, sum_li=center_l;
				for(int nn=0;nn<3;++nn)
				{
					if(!block_m::isTransparent(neighboors[lookup3[face][v][nn]]))
						continue;
					sum++;
					sum_li+=neighboors_li[lookup3[face][v][nn]];
				}
				block_lightings[chunk::getNumFromPos(posi)][face].light[v]= (light_t) (sum_li / sum);
			}
		}
	}

	//greedy meshing
	for (std::size_t axis = 0; axis < 3; ++axis)
	{
		const std::size_t u = (axis + 1) % 3;
		const std::size_t v = (axis + 2) % 3;

		int x[3] = {0}, q[3] = {0}, mask[CHUNK_SIZE*CHUNK_SIZE];
		face_lighting lightmask[CHUNK_SIZE*CHUNK_SIZE];
		// Compute mask
		q[axis] = 1;
		for (x[axis] = -1; x[axis] < CHUNK_SIZE;)
		{
			std::size_t counter = 0;
			for (x[v] = 0; x[v] < CHUNK_SIZE; ++x[v])
				for (x[u] = 0; x[u] < CHUNK_SIZE; ++x[u], ++counter)
				{
					const block a = /*(0 <= x[axis]) ?*/
							get(x[0], x[1], x[2])
					/*: 0*/;
					const block b = /*(x[axis] < CHUNK_SIZE - 1) ?*/
							get(x[0] + q[0],x[1] + q[1],x[2] + q[2])
					/*: 0*/;

					const bool canput_a=(0<=x[axis]);
					const bool canput_b=(x[axis] < CHUNK_SIZE-1);

					const bool trans_a = block_m::isTransparent(a);
					const bool trans_b = block_m::isTransparent(b);

					const int index_a=x[0]+CHUNK_SIZE*(x[1]+CHUNK_SIZE*x[2]);
					const int index_b=(x[0]+q[0])+CHUNK_SIZE*((x[1]+q[1])+CHUNK_SIZE*(x[2]+q[2]));

					const face_lighting li_a=(0<=x[axis])?
											 block_lightings[index_a][axis*2]
														 :face_lighting();
					const face_lighting li_b=(x[axis] < CHUNK_SIZE - 1)?
											 block_lightings[index_b][axis*2+1]
																	   :face_lighting();
					if(!trans_a && trans_b && canput_a)//soild block surface
					{
						mask[counter] = a;
						lightmask[counter] = li_a;
					}
					else if(!trans_b && trans_a && canput_b)
					{
						mask[counter] = -b;
						lightmask[counter] = li_b;
					}
					else if(trans_a && !b && canput_a)//transparent block surface
					{
						mask[counter] = a;
						lightmask[counter] = li_a;
					}
					else if(trans_b && !a && canput_b)
					{
						mask[counter] = -b;
						lightmask[counter] = li_b;
					}
					else
					{
						mask[counter] = 0;
						lightmask[counter] = face_lighting();
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
					face_lighting fli = lightmask[counter];
					if (c)
					{
						// Compute width
						for (width = 1; c == mask[counter + width] && fli == lightmask[counter + width] &&
										i + width < CHUNK_SIZE; ++width);

						// Compute height
						bool done = false;
						for (height = 1; j + height < CHUNK_SIZE; ++height)
						{
							for (std::size_t k = 0; k < width; ++k)
								if (c != mask[counter + k + height * CHUNK_SIZE] ||
									fli != lightmask[counter + k + height * CHUNK_SIZE])
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

						int tex=block_m::getTexture((block) c, f);
						float vx=chunkPos.x*CHUNK_SIZE+x[0]-(!q[0])*TJUNC_DELTA,
								vy=chunkPos.y*CHUNK_SIZE+x[1]-(!q[1])*TJUNC_DELTA,
								vz=chunkPos.z*CHUNK_SIZE+x[2]-(!q[2])*TJUNC_DELTA;
						vert_block v00={vx, vy, vz,
										(float)tex,(float)f,(float)fli.ao[0],(float)fli.light[0]};
						vert_block v01={vx + du[0], vy + du[1], vz + du[2],
										(float)tex,(float)f,(float)fli.ao[1],(float)fli.light[1]};
						vert_block v10={vx + du[0] + dv[0], vy + du[1] + dv[1], vz + du[2] + dv[2],
										(float)tex,(float)f,(float)fli.ao[2],(float)fli.light[2]};
						vert_block v11={vx + dv[0], vy + dv[1], vz + dv[2],
										(float)tex,(float)f,(float)fli.ao[3],(float)fli.light[3]};

						bool flip=(fli.ao[0]==fli.ao[1] && fli.ao[1]==fli.ao[2] && fli.ao[2]==fli.ao[3])?
								  (fli.light[0]+fli.light[2] > fli.light[1]+fli.light[3]):
								  (fli.ao[0]+fli.ao[2] > fli.ao[1]+fli.ao[3]);
						//bool flip=(fli.ao[0]+fli.ao[2] > fli.ao[1]+fli.ao[3]);

						//bool flip=(fli.ao[0]+fli.light[0]+fli.ao[2]+fli.light[2] >
						//		   fli.ao[1]+fli.light[1]+fli.ao[3]+fli.light[3]);
						if(flip)
						{
							//11--------10
							//|       / |
							//|    /    |
							//| /       |
							//00--------01
							meshData.push_back(v00);
							meshData.push_back(v01);
							meshData.push_back(v10);

							meshData.push_back(v00);
							meshData.push_back(v10);
							meshData.push_back(v11);
						}
						else
						{
							//11--------10
							//| \       |
							//|    \    |
							//|       \ |
							//00--------01
							meshData.push_back(v01);
							meshData.push_back(v10);
							meshData.push_back(v11);

							meshData.push_back(v00);
							meshData.push_back(v01);
							meshData.push_back(v11);
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
	updatedMesh=true;
}
void chunk::updateAll()
{
	updateLighting();
	updateMeshing();
}
