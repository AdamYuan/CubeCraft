#include "framerate.hpp"
float framerate::time,framerate::delta;
float framerate::i=0,framerate::delta_plus=0;
sf::Clock framerate::clock;
void framerate::update()
{
	float ntime = clock.getElapsedTime().asSeconds();
	delta = ntime - time;
	time = ntime;
	i++;
	delta_plus+=delta;
}
float framerate::getFps()
{
	float fps = i / delta_plus;
	delta_plus=0,i=0;
	return fps;
}
float framerate::getDist(float dis)
{
	return dis*delta*(float)LIMIT_FPS;
}
