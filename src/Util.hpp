#pragma once
#include <climits>

#define GLM_FORCE_AVX
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Box
{
	glm::vec3 Min, Max;
	Box() = default;
	Box(glm::vec3 mi, glm::vec3 ma) : Min(mi), Max(ma){}
	Box operator+ (const glm::vec3 &v) const;
	glm::vec3 GetCenter();
};
typedef int8_t light_t;

struct vert_block
{
	float x, y, z, u, v, tex, face, ao, lighting;
};

struct vert_text
{
	float x, y, coordx, coordy;
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
namespace Util
{
	extern bool Intersect(Box a, Box b);
	extern glm::ivec3 GetFaceDirect(short face);
	template <typename T>
	inline int Sign(T val)
	{
		return (T(0) < val) - (val < T(0));
	}
	template <typename T>
	inline std::string Vec3ToString(glm::tvec3<T> vec)
	{
		return "(" + std::to_string(vec.x) + ", " + std::to_string(vec.y) + ", " + std::to_string(vec.z) + ")";
	}
	extern std::string GetApplicationPath();
};

extern glm::ivec3 fdirects[];

struct Ivec3Compare
{
	bool operator() (const glm::ivec3&, const glm::ivec3&) const;
};
