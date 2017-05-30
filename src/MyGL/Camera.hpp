#pragma once
#include <iostream>
#include <glm/glm.hpp>
namespace MyGL
{
	class Camera
	{
	public:
		glm::vec3 Position;
		float Yaw = 0.0f, Pitch = 0.0f;
		void Lock();
		glm::mat4 GetViewMatrix();
		void MoveForward(const float &dist, const float &dir);
		void MoveUp(const float &dist);
	};
}
