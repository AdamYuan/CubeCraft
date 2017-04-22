#pragma once
#include <iostream>
#include <glm/glm.hpp>
class camera
{
  public:
	static glm::vec3 position,front;
	static float yaw,pitch;
	static void lock();
	static glm::mat4 getViewMatrix();
	static void moveForward(const float &dist,const float &dir);
	static void moveUp(const float &dist);
};
