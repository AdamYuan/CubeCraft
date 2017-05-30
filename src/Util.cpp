#include "Util.hpp"

glm::ivec3 fdirects[]=
{
	glm::ivec3(1,0,0),
	glm::ivec3(-1,0,0),
	glm::ivec3(0,1,0),
	glm::ivec3(0,-1,0),
	glm::ivec3(0,0,1),
	glm::ivec3(0,0,-1)
};

glm::vec3 Box::GetCenter()
{
	return (Min+Max)/2.0f;
}

bool Funcs::Intersect(Box a, Box b, bool touchForTrue)
{
	if(touchForTrue)
		return (a.Min.x <= b.Max.x && a.Max.x >= b.Min.x) &&
				(a.Min.y <= b.Max.y && a.Max.y >= b.Min.y) &&
				(a.Min.z <= b.Max.z && a.Max.z >= b.Min.z);
	return (a.Min.x < b.Max.x && a.Max.x > b.Min.x) &&
			(a.Min.y < b.Max.y && a.Max.y > b.Min.y) &&
			(a.Min.z < b.Max.z && a.Max.z > b.Min.z);
}
glm::ivec3 Funcs::GetFaceDirect(short face)
{
	return fdirects[face];
}

bool Ivec3Compare::operator()(const glm::ivec3 &lhs, const glm::ivec3 &rhs) const
{
	return lhs.x < rhs.x ||
		   lhs.x == rhs.x && (lhs.y < rhs.y || lhs.y == rhs.y && lhs.z < rhs.z);
}
