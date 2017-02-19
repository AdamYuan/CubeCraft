#pragma once
#include <SFML/System.hpp>
#define LIMIT_FPS 10
class framerate
{
  private:
	static float time,delta;
	static float i,delta_plus;
	static sf::Clock clock;
  public:
	static void update();
	static float getFps();
	static float getDist(float dis);
};
