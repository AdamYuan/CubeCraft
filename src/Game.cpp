#include "Game.hpp"
#include "Renderer.hpp"

namespace Game
{
	GLFWwindow *Window;
	int Width=1080, Height=680;//window size
	World world;
	Player player;

	bool control=true;
	bool showWireframe=false;
	std::string FpsInfo;

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
	extern void mouseButtonCallback(GLFWwindow*, int button, int action, int mods);

	void Init()
	{
		glfwInit();

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		//glfwWindowHint(GLFW_SAMPLES, 4);

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
		//glEnable(GL_MULTISAMPLE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_CCW);

		srand((unsigned int)time(0));

		world.InitNoise();

		glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

		//set glfw event callbacks
		glfwSetFramebufferSizeCallback(Window, framebufferSizeCallback);
		glfwSetKeyCallback(Window, keyCallback);
		glfwSetWindowFocusCallback(Window, focusCallback);
		glfwSetMouseButtonCallback(Window, mouseButtonCallback);

		framebufferSizeCallback(Window, Width, Height);
	}
	void Loop()
	{
		int last=0;

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
			glfwSwapInterval(1);
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
	void mouseButtonCallback(GLFWwindow*, int button, int action, int mods)
	{
		//if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		//	world.Voxels.SetBlock(player.SelectedPosition, Blocks::Air);
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
		if(glfwGetMouseButton(Window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
			if(glfwGetTime() - i >= 0.2) {
				world.Voxels.SetBlock(player.SelectedPosition, Blocks::Air);
				i = glfwGetTime();
			}
		}
		else if(glfwGetMouseButton(Window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
			if(glfwGetTime() - i >= 0.2) {
				world.Voxels.SetBlock(player.SelectedPosition + Funcs::GetFaceDirect(player.SelectedFace), Blocks::Stone);
				i = glfwGetTime();
			}
		}
		else
			i = glfwGetTime() - 1.0;
	}
	void Render()
	{
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		glClearColor(0.6, 0.8, 1.0, 1);

		Renderer::RenderWorld(&world);

		//std::cout << Funcs::Vec3ToString(player.SelectedPosition) << std::endl;

		Renderer::RenderSelectionBox();
		Renderer::RenderCross();
		std::string PositionInfo=" Position:" + Funcs::Vec3ToString(player.Position) +
				" ChunkPos:" + Funcs::Vec3ToString(player.ChunkPos);
		Renderer::RenderText(glm::vec2(0), FpsInfo + PositionInfo, 10, 20,
							 glm::vec4(1), glm::vec4(0, 0, 0, 0.3f), matrices.Matrix2d,
							 Renderer::TextStyle::regular);
	}

	void Terminate()
	{
		glfwDestroyWindow(Window);
		glfwTerminate();
	}
};
