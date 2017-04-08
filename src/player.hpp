#pragma once
#include "util.hpp"
#include <SFML/System.hpp>
class player
{
  private:
	glm::vec3 last_pos;
	box playerBox=box(glm::vec3(-0.3f,-1.6f,-0.3f),glm::vec3(0.3f));
	//if player hit the world, then return true
	bool hitTest(short face,bool doAction);//face is the world face to collision by the player
	bool _moveForward(float dist,int degree);
	bool _moveUp(float dist);
	bool isFloating(float delta);
	sf::Clock gravity_timer;
	bool jumping=false;
  public:
	glm::vec3 position=glm::vec3(0);
	glm::ivec3 chunkPos=glm::ivec3(INT_MAX);
	bool moveForward(float dist,int degree);
	bool moveUp(float dist);
	void jump();
	void physics();
	void update();
};
