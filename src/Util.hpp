#pragma once
#include <glm/glm.hpp>
#include <atomic>

#ifndef GLM_HAS_CXX11_STL
#define GLM_HAS_CXX11_STL 1
#endif
#include <glm/gtx/hash.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define MAX_INT __INT_MAX__
struct Box
{
	glm::vec3 Min, Max;
	Box() = default;
	Box(glm::vec3 mi, glm::vec3 ma) : Min(mi), Max(ma){}
	glm::vec3 GetCenter();
};
typedef int8_t light_t;
struct vert_block
{
	float x,y,z,tex,face,ao,lighting;
};

struct vert_text
{
	float x,y,coordx,coordy;
};

struct FaceLighting
{
	int AO[4]={-1, -1, -1, -1};
	light_t Light[4]={-1, -1, -1, -1};
	FaceLighting()=default;
	bool operator== (FaceLighting f)
	{
		for(int i=0;i<4;++i)
		{
			if(AO[i]!=f.AO[i])
				return false;
			if(Light[i]!=f.Light[i])
				return false;
		}
		return true;
	}
	bool operator!= (FaceLighting f)
	{
		for(int i=0;i<4;++i)
		{
			if(AO[i]!=f.AO[i])
				return true;
			if(Light[i]!=f.Light[i])
				return true;
		}
		return false;
	}
	void SetData(int *d, int *_light)
	{
		for(int i=0;i<4;++i)
		{
			AO[i]=d[i];
			Light[i]=_light[i];
		}
	}
	FaceLighting(int d[4], int _light[4])
	{
		SetData(d, _light);
	}
};


enum{RIGHT=0,LEFT,TOP,BOTTOM,FRONT,BACK};
namespace Funcs
{
extern bool Intersect(Box a, Box b, bool touchForTrue = false);
extern glm::ivec3 GetFaceDirect(short face);
template <typename T>
inline int Sign(T val)
{
    return (T(0) < val) - (val < T(0));
}

};

extern glm::ivec3 fdirects[];

struct Ivec3Compare
{
	bool operator() (const glm::ivec3&, const glm::ivec3&) const;
};
