#include "matrix.hpp"
glm::mat4 matrix::view=glm::mat4(1.0),
	matrix::projection=glm::mat4(1.0),
	matrix::matrix2d=glm::mat4(1.0),
	matrix::matrix2d_center=glm::mat4(1.0);

void matrix::updateMatrix(int width, int height)
{
	projection = glm::perspective(glm::radians(45.0f),width/(float)height,0.1f,1000.0f);
	matrix2d = glm::ortho(0.0f, (float)width, (float)height, 0.0f);
	matrix2d_center = glm::ortho((float)-width/2.0f, (float)width/2.0f, (float)height/2.0f, (float)-height/2.0f);
}
