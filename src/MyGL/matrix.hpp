#pragma once
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
class matrix
{
  public:
	static glm::mat4 view,projection,matrix2d,matrix2d_center;
	static void updateMatrix(int width,int height);
};
