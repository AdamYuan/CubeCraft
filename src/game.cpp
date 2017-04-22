#include "game.hpp"
#include "world.hpp"
#include "renderer.hpp"
#include "resource.hpp"
#include <iostream>
#define GLEW_STATIC
#include <GL/glew.h>

#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>

#include "MyGL/shader.hpp"
#include "MyGL/camera.hpp"
#include "MyGL/matrix.hpp"
#include "MyGL/framerate.hpp"
#include "MyGL/frustum.hpp"

namespace game
{
	sf::Window win;
	sf::Event evt;
	unsigned width=1080, height=680;//window size
	world wld;
	bool control=true;
	player gamePlayer;

	bool showFrame=false;
	string fps_info;

	void init_gl()
	{
		sf::ContextSettings glc;
		//Init context setting
		glc.depthBits = 24;
		glc.stencilBits = 8;
		glc.majorVersion = 3;
		glc.minorVersion = 3;
		glc.attributeFlags = sf::ContextSettings::Core;
		//Create window
		win.create(sf::VideoMode(width, height, 32),"CubeCraft",
				   sf::Style::Titlebar|sf::Style::Close|sf::Style::Resize,glc);

		//win.setVerticalSyncEnabled(true);//vertical sync

		//init glew
		glewExperimental=GL_TRUE;
		if(glewInit()!=GLEW_OK)
		{
			std::cout << "INIT GLEW FAILED" << std::endl;
			exit(EXIT_FAILURE);
		}
	}
	void init()
	{
		//set MyGL default mode
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_CCW);

		srand((unsigned int)time(0));

        wld.initNoise();

		//gamePlayer.position = {49, -206.5, 15};

	}
	void loop()
	{
		sf::Clock clock;
		uint last=0;

		while(win.isOpen())
		{
			framerate::update();

			if((uint)clock.getElapsedTime().asSeconds() != last)
			{
				fps_info = "fps:" + std::to_string(framerate::getFps());
				last = (uint)clock.getElapsedTime().asSeconds();
			}
			if(control)
			{
				keyControl();
				win.setMouseCursorVisible(false);//hide mouse
				sf::Mouse::setPosition(sf::Vector2i(width/2, height/2),win);
			}
			else
				win.setMouseCursorVisible(true); //show mouse

			procEvent();

			gamePlayer.update();//update player data and set camera position

			matrix::view=camera::getViewMatrix();
			frustum::calculatePlanes(matrix::projection * matrix::view);

			wld.updateChunkLists();

			render();

			//swap window
			win.display();
		}
	}
	void keyControl()
	{
		float dist=1.5f;
		if(sf::Keyboard::isKeyPressed(sf::Keyboard::W))
			gamePlayer.moveForward(dist, 0);
		if(sf::Keyboard::isKeyPressed(sf::Keyboard::S))
			gamePlayer.moveForward(dist, 180);
		if(sf::Keyboard::isKeyPressed(sf::Keyboard::A))
			gamePlayer.moveForward(dist, 90);
		if(sf::Keyboard::isKeyPressed(sf::Keyboard::D))
			gamePlayer.moveForward(dist, -90);
		if(sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
		{
			if(PHYSICS)
				gamePlayer.jump();
			else
				gamePlayer.moveUp(dist);
		}
		if(sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) && !PHYSICS)
			gamePlayer.moveUp(-dist);
	}
	void procEvent()
	{
		while(win.pollEvent(evt))
		{
			if(evt.type==sf::Event::Closed)
				win.close();
			if(evt.type==sf::Event::Resized)
			{
				width=evt.size.width, height=evt.size.height;
				glViewport(0, 0, width, height);
				matrix::updateMatrix(width, height);
				//std::cout << "RESIZED " << width << "x" << height << std::endl;
			}
			if(evt.type==sf::Event::MouseMoved && control)
			{
				camera::yaw+=(width/2-(float)evt.mouseMove.x)*0.2f;
				camera::pitch+=(height/2-(float)evt.mouseMove.y)*0.2f;
				camera::lock();
			}
			if(evt.type==sf::Event::MouseButtonPressed)
				control=true;
			if(evt.type==sf::Event::GainedFocus)
				control=true;
			if(evt.type==sf::Event::LostFocus)
				control=false;
			if(evt.type==sf::Event::KeyReleased)
			{
				if(evt.key.code==sf::Keyboard::Escape)
					control=!control;
				if(evt.key.code==sf::Keyboard::F)
					showFrame=!showFrame;
			}
		}
	}
	void render()
	{
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		glClearColor(0.6, 0.8, 1.0, 1);

		glLineWidth(1.0f);
		if(showFrame)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		renderer::renderWorld(&wld);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		renderer::renderCross();
		string pos_info=" pos:(" + std::to_string(camera::position.x) + "," +
						std::to_string(camera::position.y) + "," +
						std::to_string(camera::position.z) + ")";
		renderer::renderText(glm::vec2(0), fps_info + pos_info, 10, 20,
							 glm::vec4(1), glm::vec4(0,0,0,0.3f), matrix::matrix2d,
							 renderer::textStyle::regular);
	}
};
