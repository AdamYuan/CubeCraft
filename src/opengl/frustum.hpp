#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
namespace frustum
{

extern float planes[6][4];
extern void calculatePlanes(glm::mat4 mvp);
extern bool pointInFrustum(float x,float y,float z);
extern bool pointInFrustum(glm::vec3 pos);

extern bool sphereInFrustum(float x,float y,float z,float rad);
extern bool sphereInFrustum(glm::vec3 pos,float rad);

extern bool cubeInFrustum(float x,float y,float z,float size);
extern bool cubeInFrustum(glm::vec3 pos,float size);

};
