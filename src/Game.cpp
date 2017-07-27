#include "Game.hpp"
#include "Renderer.hpp"

namespace Game
{
	GLFWwindow *Window;
	int Width=1080, Height=680;//window size
	World world;
	Player player;

	bool control=true;
	std::string FpsInfo;
	block usingBlock = Blocks::Stone;

	MyGL::FrameRateManager frameRateManager;
	MyGL::Camera camera;
	MyGL::Matrices matrices;
	MyGL::Frustum frustum;

	extern void KeyControl();
	extern void MouseControl();
	extern void Render();

	extern void framebufferSizeCallback(GLFWwindow *, int width, int height);
	extern void keyCallback(GLFWwindow*, int key, int scancode, int action, int mods);
	extern void focusCallback(GLFWwindow*, int focused);

	void Init()
	{
		glfwInit();

		glfwWindowHint(GLFW_SAMPLES, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

		Window = glfwCreateWindow(Width, Height, "CubeCraft", nullptr, nullptr);
		if(Window == nullptr)
		{
			std::cout << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
			exit(EXIT_FAILURE);
		}
		glfwMakeContextCurrent(Window);

		glewExperimental = GL_TRUE;
		if(glewInit() != GLEW_OK)
		{
			std::cout << "Failed to initialize GLEW" << std::endl;
			exit(EXIT_FAILURE);
		}

		//SetBlock MyGL default mode
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_CCW);

		srand((unsigned int)time(0));

		glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

		//set glfw event callbacks
		glfwSetFramebufferSizeCallback(Window, framebufferSizeCallback);
		glfwSetKeyCallback(Window, keyCallback);
		glfwSetWindowFocusCallback(Window, focusCallback);

		framebufferSizeCallback(Window, Width, Height);
	}
	void Loop()
	{
		int last = -1;

		player.StartTimer();
		while(!glfwWindowShouldClose(Window))
		{
			frameRateManager.UpdateFrameRateInfo();
			glfwPollEvents();

			if((int)glfwGetTime() != last)
			{
				FpsInfo = "FPS:" + std::to_string(frameRateManager.GetFps());
				last = (int)glfwGetTime();
			}
			if(control)
			{
				KeyControl();
				MouseControl();
				glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
				double x, y;
				glfwGetCursorPos(Window, &x, &y);
				camera.ProcessMouseMovement((float) (Width / 2 - x), (float) (Height / 2 - y),
											MOUSE_SENSITIVITY);
				glfwSetCursorPos(Window, Width / 2, Height / 2);
			}
			else
				glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

			player.UpdatePosition();//UpdatePosition Player data and SetBlock camera Position
			player.UpdateSelectedPosition();

			frustum.CalculatePlanes(matrices.Projection3d * camera.GetViewMatrix());

			world.UpdateChunkLists();

			Render();

			//swap window
			//glfwSwapInterval(1);
			glfwSwapBuffers(Window);
		}
	}
	void framebufferSizeCallback(GLFWwindow *, int width, int height)
	{
		Width = width;
		Height = height;
		glViewport(0, 0, width, height);
		matrices.UpdateMatrices(width, height);
	}
	void keyCallback(GLFWwindow*, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
			control = !control;
	}
	void focusCallback(GLFWwindow*, int focused)
	{
		control = focused != 0;
	}
	void KeyControl()
	{
		if(glfwGetKey(Window, GLFW_KEY_W))
			player.MoveForward(MOVE_DIST, 0);
		if(glfwGetKey(Window, GLFW_KEY_S))
			player.MoveForward(MOVE_DIST, 180);
		if(glfwGetKey(Window, GLFW_KEY_A))
			player.MoveForward(MOVE_DIST, 90);
		if(glfwGetKey(Window, GLFW_KEY_D))
			player.MoveForward(MOVE_DIST, -90);
		if(glfwGetKey(Window, GLFW_KEY_SPACE))
		{
			if(PHYSICS)
				player.Jump();
			else
				player.MoveUp(MOVE_DIST);
		}
		if(glfwGetKey(Window, GLFW_KEY_LEFT_SHIFT) && !PHYSICS)
			player.MoveUp(-MOVE_DIST);
	}
	void MouseControl()
	{
		static double i = 0;
		static bool leftFirst, rightFirst;
		if(glfwGetMouseButton(Window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
		{
			usingBlock = world.Voxels.GetBlock(player.SelectedPosition);
			leftFirst = true, rightFirst = true;
		}
		else if(glfwGetMouseButton(Window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		{
			rightFirst = true;
			if(leftFirst || glfwGetTime() - i >= 0.18)
			{
				world.Voxels.SetBlock(player.SelectedPosition, Blocks::Air);
				i = glfwGetTime();
				leftFirst = false;
			}
		}
		else if(glfwGetMouseButton(Window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
		{
			leftFirst = true;
			if((rightFirst || glfwGetTime() - i >= 0.18) &&
					!Util::Intersect(Box(player.PlayerBox.Min + HITTEST_DELTA, player.PlayerBox.Max - HITTEST_DELTA) + player.Position,
									 BlockUtil::GetBox(player.SelectedPosition + player.SelectedFaceVec)))
			{
				world.Voxels.SetBlock(player.SelectedPosition + player.SelectedFaceVec, usingBlock);
				i = glfwGetTime();
				rightFirst = false;
			}
		}
		else
			leftFirst = true, rightFirst = true;
	}
	void Render()
	{
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		glClearColor(0.6, 0.8, 1.0, 1);

		Renderer::RenderWorld(&world);

		//std::cout << Util::Vec3ToString(player.SelectedPosition) << std::endl;

		Renderer::RenderSelectionBox();
		Renderer::RenderCross();
		std::string PositionInfo1 = "Position:" + Util::Vec3ToString(player.Position);
		std::string PositionInfo2 = "ChunkPos:" + Util::Vec3ToString(player.ChunkPos) +
				" SelectedPos:" + (player.SelectedPosition.x == INT_MAX ? "NULL" : Util::Vec3ToString(player.SelectedPosition));
		Renderer::RenderText(glm::vec2(0), FpsInfo, 10, 20,
							 glm::vec4(1), glm::vec4(0, 0, 0, 0.3f), matrices.Matrix2d,
							 Renderer::TextStyle::regular);
		Renderer::RenderText(glm::vec2(0, 20), PositionInfo1, 10, 20,
							 glm::vec4(1), glm::vec4(0, 0, 0, 0.3f), matrices.Matrix2d,
							 Renderer::TextStyle::regular);
		Renderer::RenderText(glm::vec2(0, 40), PositionInfo2, 10, 20,
							 glm::vec4(1), glm::vec4(0, 0, 0, 0.3f), matrices.Matrix2d,
							 Renderer::TextStyle::regular);
	}

	void Terminate()
	{
		glfwDestroyWindow(Window);
		glfwTerminate();
	}
};
