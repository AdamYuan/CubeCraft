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

Box Box::operator+(const glm::vec3 &v) const
{
	return Box(Min+v, Max+v);
}

bool Util::Intersect(Box a, Box b)
{
	return (a.Min.x < b.Max.x && a.Max.x > b.Min.x) &&
			(a.Min.y < b.Max.y && a.Max.y > b.Min.y) &&
			(a.Min.z < b.Max.z && a.Max.z > b.Min.z);
}
glm::ivec3 Util::GetFaceDirect(short face)
{
	return fdirects[face];
}

std::string Util::GetApplicationPath()
{
#include <limits.h>
#include <stdlib.h>

	char *full_path = realpath(".", NULL);
	free(full_path);
}

bool Ivec3Compare::operator()(const glm::ivec3 &lhs, const glm::ivec3 &rhs) const
{
	return lhs.x < rhs.x ||
		   lhs.x == rhs.x && (lhs.y < rhs.y || lhs.y == rhs.y && lhs.z < rhs.z);
}
