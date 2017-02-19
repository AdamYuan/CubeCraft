#pragma once
#include <glm/glm.hpp>

#ifndef GLM_HAS_CXX11_STL
#define GLM_HAS_CXX11_STL 1
#endif
#include <glm/gtx/hash.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define MAX_INT __INT_MAX__
struct box
{
	glm::vec3 min,max;
	box() = default;
	box(glm::vec3 mi,glm::vec3 ma) : min(mi), max(ma){}
	glm::vec3 getCenter();
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

struct face_lighting
{
	int ao[4]={-1,-1,-1,-1};
	light_t light[4]={-1,-1,-1,-1};
	face_lighting()=default;
	bool operator== (face_lighting f)
	{
		for(int i=0;i<4;++i)
		{
			if(ao[i]!=f.ao[i])
				return false;
			if(light[i]!=f.light[i])
				return false;
		}
		return true;
	}
	bool operator!= (face_lighting f)
	{
		for(int i=0;i<4;++i)
		{
			if(ao[i]!=f.ao[i])
				return true;
			if(light[i]!=f.light[i])
				return true;
		}
		return false;
	}
	void setData(int d[4],int _light[4])
	{
		for(int i=0;i<4;++i)
		{
			ao[i]=d[i];
			light[i]=_light[i];
		}
	}
	face_lighting(int d[4],int _light[4])
	{
		setData(d,_light);
	}
};


enum{RIGHT=0,LEFT,TOP,BOTTOM,FRONT,BACK};
namespace funcs
{
extern bool intersect(box a,box b,bool touchForTrue=false);
extern glm::ivec3 getFaceDirect(short face);
template <typename T>
inline int sgn(T val)
{
    return (T(0) < val) - (val < T(0));
}

};

extern glm::ivec3 fdirects[];
