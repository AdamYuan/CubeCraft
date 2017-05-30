#pragma once
#include "World.hpp"
#include "Player.hpp"
#include <GLFW/glfw3.h>
#include "MyGL/Shader.hpp"
#include "MyGL/Camera.hpp"
#include "MyGL/Matrices.hpp"
#include "MyGL/FrameRate.hpp"
#include "MyGL/Frustum.hpp"
namespace Game
{
	extern GLFWwindow *Window;
	extern int Width, Height;
	extern World world;
	extern Player player;

	extern MyGL::Camera camera;
	extern MyGL::Matrices matrices;
	extern MyGL::Frustum frustum;
	extern MyGL::FrameRateManager frameRateManager;

	extern void Init();
	extern void Loop();
	extern void Terminate();
};
