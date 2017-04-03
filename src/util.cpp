#include "util.hpp"
glm::ivec3 fdirects[]=
{
	glm::ivec3(1,0,0),
	glm::ivec3(-1,0,0),
	glm::ivec3(0,1,0),
	glm::ivec3(0,-1,0),
	glm::ivec3(0,0,1),
	glm::ivec3(0,0,-1)
};

glm::vec3 box::getCenter()
{
	return (min+max)/2.0f;
}

bool funcs::intersect(box a,box b,bool touchForTrue)
{
	if(touchForTrue)
		return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
				(a.min.y <= b.max.y && a.max.y >= b.min.y) &&
				(a.min.z <= b.max.z && a.max.z >= b.min.z);
	return (a.min.x < b.max.x && a.max.x > b.min.x) &&
			(a.min.y < b.max.y && a.max.y > b.min.y) &&
			(a.min.z < b.max.z && a.max.z > b.min.z);
}
glm::ivec3 funcs::getFaceDirect(short face)
{
	return fdirects[face];
}
