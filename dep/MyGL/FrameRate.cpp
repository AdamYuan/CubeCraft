#include "FrameRate.hpp"
#include <GLFW/glfw3.h>
namespace MyGL
{
	void FrameRateManager::UpdateFrameRateInfo()
	{
		float now = (float) glfwGetTime();
		delta = now - last;
		last = now;
		i++;
		delta_plus += delta;
	}
	float FrameRateManager::GetFps()
	{
		float fps = i / delta_plus;
		delta_plus = 0, i = 0;
		return fps;
	}
	float FrameRateManager::GetMovementDistance(float originDis)
	{
		return originDis * delta * (float)LIMIT_FPS;
	}
}
